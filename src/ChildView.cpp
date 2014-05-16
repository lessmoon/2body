
// ChildView.cpp : CChildView ���ʵ��
//

#include "stdafx.h"
#include "stars.h"
#include "ChildView.h"
#include "iol.h"
#include "file_io.h"
#include "kb_io.h"

#ifdef _DEBUG
#include<fstream>

#define new DEBUG_NEW
#endif
#define BEGIN_Z 40
#define PI 3.1415926535
#define G_CONST 6.67384E-11
#define h internal_per_timer

#define alph(x,y) (sqrt((x*x + y*y)*(x*x + y*y)*(x*x + y*y)))
#define eq18(x,y,m) (G_CONST * m * x/alph(x,y))
#define eq19(x,y,m) (G_CONST * m * y/alph(x,y))
#define eq20(x,y,m) (G_CONST * m * -x/alph(x,y))
#define eq21(x,y,m) (G_CONST * m * -y/alph(x,y))
#define eq22(x,v_) (x + h*v_/2)
#define eq23(a,v_) (v_ + h * a)
#define eq24(v_,x) (x + h * v_ / 2)
#define eq25(x,v_,a) (x+h*v_/2+h*h*a/4)


#define MINIMU_DIST_MOUSE (4.0*4.0)
#define GoForword(h,scale) {double temp = sqrt(scale.distance());\
                            double temp2 = temp + h;\
                            scale.z = temp2/temp *scale.z;\
                            scale.x = temp2/temp *scale.x;\
                            scale.y = temp2/temp *scale.y;}

void RotateAs(_3DVector&point,double theta,_3DVector&scale){
    double _x = point.x;
    double _y = point.y;
    double _z = point.z;

    double nx = scale.x;double ny = scale.y;double nz = scale.z;
    double len = sqrt(nx * nx + ny * ny + nz * nz);
    nx /= len;ny /= len;nz /= len; \
    point.x = _x * (nx * nx * (1 - cos(theta)) + cos(theta)) +   
              _y * (nx * ny * (1 - cos(theta)) - nz * sin(theta)) + 
              _z * (nx * nz * (1 - cos(theta)) + ny * sin(theta));
    point.y = _x * (nx * ny * (1 - cos(theta)) + nz * sin(theta)) +  
              _y * (ny * ny * (1 - cos(theta)) + cos(theta)) + 
              _z * (ny * nz * (1 - cos(theta)) - nx * sin(theta));
    point.z = _x * (nx * nz * (1 - cos(theta)) - ny * sin(theta)) + 
              _y * (ny * nz * (1 - cos(theta)) + nx * sin(theta)) + 
              _z * (nz * nz * (1 - cos(theta)) + cos(theta));
}

#define RotateViewUp(theta,x_scale,y_scale,z_scale) { \
                                    RotateAs(z_scale,theta,x_scale);\
                                    RotateAs(y_scale,theta,x_scale);\
                                    }

#define RotateViewRight(theta,x_scale,y_scale,z_scale) { \
                                RotateAs(z_scale,theta,y_scale);\
                                RotateAs(x_scale,theta,y_scale);\
                                }

#define RotateViewCircle(theta,x_scale,y_scale,z_scale) { \
                            RotateAs(y_scale,theta,z_scale);\
                            RotateAs(x_scale,theta,z_scale);\
                                }

// CChildView

CChildView::CChildView()
    :m_GLPixelIndex(0),m_hGLContext(NULL),
     x_scale(BEGIN_Z,0,0),y_scale(0,BEGIN_Z,0),z_scale(0,0,BEGIN_Z),
     CenterStar(5.972E24,_3DVector(),0xff9900),zoom(16E6),internal_per_timer(1000)
    {}

CChildView::~CChildView()
{
}

void CChildView::OnDestroy()
{
    CWnd::OnDestroy();

    // TODO: �ڴ˴������Ϣ����������
    if(wglGetCurrentContext()!=NULL)
    {
        wglMakeCurrent(NULL,NULL);
    }
    if(this->m_hGLContext!=NULL)
    {
        wglDeleteContext(this->m_hGLContext);
        this->m_hGLContext = NULL;
    }
}

BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
    ON_WM_CREATE()
    ON_WM_TIMER()
    ON_WM_SIZE()
    ON_WM_TIMER()
    ON_WM_MOUSEWHEEL()
    ON_WM_DESTROY()
    ON_WM_MOUSEMOVE()
    ON_WM_KEYDOWN()
END_MESSAGE_MAP()

bool CChildView::InitGL(void)
{
    glShadeModel(GL_SMOOTH);                            // Enable Smooth Shading
    glClearColor(0.0,0.0,0.0,0.0);// Black Background
    glClearDepth(1.0f);                                    // Depth Buffer Setup
    glEnable(GL_DEPTH_TEST);                            // Enables Depth Testing
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glDepthFunc(GL_LEQUAL);                                // The Type Of Depth Testing To Do
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);    // Really Nice Perspective Calculations
    return TRUE;                                        // Initialization Went OK
}

bool CChildView::SetWindowPixelFormat(HDC hDC)
{
    //���崰�ڵ����ظ�ʽ
    PIXELFORMATDESCRIPTOR pixelDesc=
    {
        sizeof(PIXELFORMATDESCRIPTOR),           //nSize�ṹ����
        1,                                       //nVersion�ṹ�汾
        PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL| 
        PFD_DOUBLEBUFFER|PFD_SUPPORT_GDI,        //dwFlags����OpenGL��δ�������
         /*
         wFlags�ܽ������±�־��
            PFD_DRAW_TO_WINDOW ʹ֮���ڴ��ڻ��������豸���ڻ�ͼ��
            PFD_DRAW_TO_BITMAP ʹ֮�����ڴ��е�λͼ��ͼ��
            PFD_SUPPORT_GDI ʹ֮�ܵ���GDI������ע�����ָ����PFD_DOUBLEBUFFER�����ѡ���Ч����
            PFD_SUPPORT_OpenGL ʹ֮�ܵ���OpenGL������
            PFD_GENERIC_FORMAT �����������ظ�ʽ��Windows GDI��������ɵ�����Ӳ���豸��������֧�֣�����ָ����һ�
            PFD_NEED_PALETTE ���߻������Ƿ���Ҫ��ɫ�壬�����������ɫ��ʹ��24�� 32λɫ�����Ҳ��Ḳ�ǵ�ɫ�壻
            PFD_NEED_SYSTEM_PALETTE �����־ָ���������Ƿ��ϵͳ��ɫ�嵱���������ɫ���һ���֣�
            PFD_DOUBLEBUFFER ָ��ʹ����˫��������ע��GDI������ʹ����˫�������Ĵ����л�ͼ����
            PFD_STEREO ָ�����һ������Ƿ�����ͼ������֯��
                        PFD_SWAP_LAYER_BUFFERS
         */
        PFD_TYPE_RGBA,  //iPixelType,��ɫģʽ����������PFD_TYPE_RGBA��ζ��ÿһλ(bit)�������rgb��������ֵ��PFD_TYPE_COLORINDEX��ζ��ÿһλ��������ڲ�ɫ���ұ��е�����ֵ
        24,   //cColorBits������ָ��һ����ɫ��λ������RGBA��˵��λ��������ɫ�к졢�̡�����������ռ��λ��������ɫ������ֵ��˵��ָ���Ǳ��е���ɫ����
        0,0,0,0,0,0,  //cRedBits��cRedShifts��cGreenBits��cGreenShifts��cBlueBits��cBlueShifts��,�����������ã�һ����0
                  //cRedBits��cGreenBits��cBlueBits������������Ӧ������ʹ�õ�λ����
                      //cRedShift��cGreenShift��cBlue-Shift������������������ɫ��ʼ��ƫ������ռ��λ����
        0,                                       //cAlphaBits,RGB��ɫ������Alpha��λ��                            
        0,                                 //cAlphaShift���Ѿ��������ã���0                   
        0,                                       //cAcuumBits�ۼƻ����λ��
        0,0,0,0,                                 //cAcuumRedBits/cAcuumGreenBits/cAcuumBlueBits/cAcuumAlphaBits,�����������ã���0
        32,                                      //cDepthBits��Ȼ����λ��
        0,                                       //cStencilBits,ģ�建���λ��
        0,                                       //cAuxBuffers�����������λ����һ����0
        PFD_MAIN_PLANE,                          //iLayerType,˵����������ͣ��ɺ�����0�������ڵİ汾������
                                                 //PFD_MAIN_PLANE,PFD_OVER_LAY_PLANE,PFD_UNDERLAY_PLANE
        0,0,0,0                                  //bReserved,dwLayerMask,dwVisibleMask,dwDamageMask,������0
    };

    this->m_GLPixelIndex = ChoosePixelFormat(hDC,&pixelDesc);//ѡ������������ظ�ʽ
    /*
    ChoosePixelFormat��������������һ����hDc����һ����һ��ָ��PIXELFORMATDESCRIPTOR�ṹ��ָ��&pixelDesc
    �ú������ش����ظ�ʽ������ֵ,�������0���ʾʧ�ܡ�
    ���纯��ʧ�ܣ�����ֻ�ǰ�����ֵ��Ϊ1���� DescribePixelFormat�õ����ظ�ʽ������
    ����������һ��û�õ�֧�ֵ����ظ�ʽ����ChoosePixelFormat���᷵������Ҫ������ظ�ʽ��ӽ���һ��ֵ
    һ�����ǵõ�һ�����ظ�ʽ������ֵ����Ӧ�����������ǾͿ��Ե���SetPixelFormat�������ظ�ʽ������ֻ������һ�Ρ�
    */
    if(this->m_GLPixelIndex==0)
    {//ѡ��ʧ��
        this->m_GLPixelIndex = 1;//Ĭ�ϵ����ظ�ʽ
        //��Ĭ�ϵ����ظ�ʽ��������
        //
        if(DescribePixelFormat(hDC,this->m_GLPixelIndex,sizeof(PIXELFORMATDESCRIPTOR),&pixelDesc)==0)
        {
            return FALSE;
        }
    }

    if(SetPixelFormat(hDC,this->m_GLPixelIndex,&pixelDesc)==FALSE)
    {
        return FALSE;
    }
    return TRUE;
}

// CChildView ��Ϣ�������

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	
    cs.style |= (WS_CLIPCHILDREN | WS_CLIPSIBLINGS);//openGL��MDI�б���
    return CWnd::PreCreateWindow(cs);
}

void CChildView::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);

    // TODO: �ڴ˴������Ϣ����������
    /*
    OnSizeͨ��glViewport(0, 0, width, height)�������ӿں��ӿ����ꡣ
    glViewport�ĵ�һ�������������ӿ����½ǵ��������꣬�������ĸ��������ӿڵĿ�Ⱥ͸߶ȡ�

    OnSize�е�glMatrixMode���������þ���ģʽ�ģ���������ѡ�GL_MODELVIEW��GL_PROJECTION�� GL_TEXTURE��
    GL_MODELVIEW��ʾ��ʵ������ϵת����������ϵ��
    GL_PROJECTION��ʾ����������ϵת����������ϵ��
    GL_TEXTURE��ʾ�Ӷ������������ϵ��ճ�����������ϵ�ı任��

    glLoadIdentity��ʼ�����̾���(project matrix)
    gluOrtho2D�ѹ��̾������ó���ʾһ����άֱ����ʾ����
    */
    
    width = cx;
    height = cy;
    if (height==0)                                        // Prevent A Divide By Zero By
    {
        height=1;                                        // Making Height Equal One
    }

    glViewport(0,0,width,height);                        // Reset The Current Viewport

    glMatrixMode(GL_PROJECTION);                        // Select The Projection Matrix
    glLoadIdentity();                                    // Reset The Projection Matrix
    // Calculate The Aspect Ratio Of The Window
    gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,5000.0f);//͸��ͶӰ
    glMatrixMode(GL_MODELVIEW);                            // Select The Modelview Matrix
    glLoadIdentity();                                    // Reset The Modelview Matrix
}

int CChildView::DrawGLScene(void)
{
    // Here's Where We Do All The Drawing
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);    // Clear Screen And Depth Buffer
    glLoadIdentity();                                    // Reset The Current Modelview Matrix

    gluLookAt(z_scale.x, z_scale.y, z_scale.z, 0, 0, 0, y_scale.x, y_scale.y, y_scale.z);
    
    DrawCenterStar();
    DrawPlanets();

    glFlush();
    return TRUE;                                        // Everything Went OK
}


BOOL CChildView::CreateViewGLContext(HDC hDC)
{
        //WglCreateContext��������һ���µ�OpenGL��Ⱦ������(RC)
    //����������������ڻ��Ƶ���hdc���ص��豸
    //�����Ⱦ�������к��豸������(dc)һ�������ظ�ʽ.
    this->m_hGLContext = wglCreateContext(hDC);//����RC

    if(this->m_hGLContext==NULL)
    {//����ʧ��
        return FALSE;
    }

    /*
    wglMakeCurrent �����趨OpenGL��ǰ�߳�(�߳������)����Ⱦ������
    �Ժ�����߳����е�OpenGL���ö��������hdc��ʶ���豸�ϻ��ơ�
    ��Ҳ����ʹ��wglMakeCurrent �������ı�����̵߳ĵ�ǰ��Ⱦ����
    ʹ֮�����ǵ�ǰ����Ⱦ������
    */
    if(wglMakeCurrent(hDC,this->m_hGLContext)==FALSE)
    {//ѡΪ��ǰRCʧ��
        return FALSE;
    }
    return TRUE;
}

void CChildView::OnPaint() 
{
	CPaintDC dc(this); // ���ڻ��Ƶ��豸������
    
    // TODO: �ڴ˴������Ϣ����������
    // ��Ϊ��ͼ��Ϣ���� CView::OnPaint()
    this->DrawGLScene();
    SwapBuffers(dc.m_hDC);
}

int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    // TODO:  �ڴ������ר�õĴ�������
    //�õ�һ�����ڶ���CWnd����������ָ��ľ����HWND��  
    HWND hWnd = this->GetSafeHwnd();  
    //GetDC�ú�������һָ�����ڵĿͻ������������Ļ����ʾ�豸�����Ļ����ľ��
    //�Ժ������GDI������ʹ�øþ�������豸�����Ļ����л�ͼ��
    HDC hDC = ::GetDC(hWnd);

    if(this->SetWindowPixelFormat(hDC)==FALSE)
    {//�������ظ�ʽ
        return 0;
    }
    if(this->CreateViewGLContext(hDC)==FALSE)
    {//����RC��ѡΪ����
        return 0;
    }
    if(!this->InitGL())
    {//��ʼ��openGL
        return 0;
    }
    this -> SetCenterStar(Star(5.972E24,_3DVector(0,0,0),0xFF9900));
    this -> LoadDataFromFile("planets.txt");
    this -> InitPlanets();
    this -> SetTimer(1,25,NULL);
    return 0;
}

void CChildView::OnTimer(UINT_PTR nIDEvent)
{
    // TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
    CalculatePosition();
    this -> Invalidate();
    this -> Invalidate(false);
    CWnd::OnTimer(nIDEvent);
}

BOOL CChildView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    // TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

    float D = (zDelta/WHEEL_DELTA);
    
    GoForword(D,z_scale);
    GoForword(D,x_scale);
    GoForword(D,y_scale);
    this -> Invalidate();
    this -> Invalidate(false);
//#ifdef _DEBUG
    //std::cout<<zDelta<<std::endl;
//#endif
    
    return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}

void CChildView::OnMouseMove(UINT nFlags, CPoint point)
{
    static CPoint copy;
    // TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
    if(MK_LBUTTON&nFlags){
             RotateViewUp((point.y - copy.y)/double(height) * PI ,x_scale,y_scale,z_scale);
             RotateViewRight((point.x - copy.x)/double(width) * PI ,x_scale,y_scale,z_scale);
             this -> Invalidate();
             this -> Invalidate(false);
         //if()
    } 
    copy = point;
    CWnd::OnMouseMove(nFlags, point);
}

void CChildView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    // TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

    switch(nChar){
        case VK_UP:
        case 'W':
            GoForword(-1,z_scale);
            break;
        case 'S':
        case VK_DOWN:
            GoForword(+1,z_scale);
            break;
        case VK_LEFT:
        case 'A':
            RotateViewRight(PI/12,x_scale,y_scale,z_scale);
            break;
        case VK_RIGHT:
        case 'D':
            RotateViewRight(-PI/12,x_scale,y_scale,z_scale);
            break;
        case 'R':
        case VK_SPACE:
            RotateViewCircle(PI/12,x_scale,y_scale,z_scale);
            break;
        default:
            goto exit_entry;
    }
    //std::cout<<"x:"<<z_scale.x<<"y:"<<z_scale.y<<"z:"<<z_scale.z<<std::endl;
    this -> Invalidate();
    this -> Invalidate(false);
exit_entry:
    CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CChildView::AddPlanet(const Planet&p)
{
    this -> Planets.push_back(p);
}

void CChildView::SetCenterStar(const Star&s)
{
    this -> CenterStar = s;
}

void CChildView::InitPlanets()
{
    
    /*Planet temp;
    temp.Set(0,_3DVector(0,1000,0),_3DVector(363104000,0,0),0x0000ff,1);//ˮ��
    this -> AddPlanet(temp);
    temp.Set(0,_3DVector(0,800,0),_3DVector(200104000,0,0),0x00FF00,1);//ˮ��
    this -> AddPlanet(temp);*/    
    for(PContainer::iterator iter = Planets.begin();
        iter != Planets.end();
        iter ++){
            double v_x = iter-> V.x;//text_VX;
            double v_y = iter-> V.y;//text_VY;
            double x = iter -> Pos.x ;//text_X;
            double y = iter -> Pos.y;//text_Y;

            iter -> Acc.x  = eq20(x,y,CenterStar.Mass);
            iter -> Acc.y  = eq21(x,y,CenterStar.Mass);

            iter -> HalfPos.x = eq25(x,v_x,iter -> Acc.x);
            iter -> HalfPos.y = eq25(y,v_y,iter -> Acc.y);
    }

    /*TODO:Read From File*/
    /*
    temp.Set(3.30e23,_3DVector(0,0,5.791E10),_3DVector(0,0,47.87E3),0x996671);//����
    this -> AddPlanet(temp);
    temp.Set(3.30e23,_3DVector(0,0,5.791E10),_3DVector(0,0,47.87E3),0x996671);//����
    this -> AddPlanet(temp);
    temp.Set(3.30e23,_3DVector(0,0,5.791E10),_3DVector(0,0,47.87E3),0x996671);//����
    this -> AddPlanet(temp);
    temp.Set(3.30e23,_3DVector(0,0,5.791E10),_3DVector(0,0,47.87E3),0x996671);//ľ��
    this -> AddPlanet(temp);
    temp.Set(3.30e23,_3DVector(0,0,5.791E10),_3DVector(0,0,47.87E3),0x996671);//����
    this -> AddPlanet(temp);
    temp.Set(3.30e23,_3DVector(0,0,5.791E10),_3DVector(0,0,47.87E3),0x996671);//������
    this -> AddPlanet(temp);
    temp.Set(3.30e23,_3DVector(0,0,5.791E10),_3DVector(0,0,47.87E3),0x996671);//������
    this -> AddPlanet(temp);*/
}

void CChildView::CalculatePosition()
{
    double m = CenterStar.Mass;

    for(PContainer::iterator iter = Planets.begin();
        iter != Planets.end();
        iter ++){
        double v_x = iter-> V.x;//text_VX;
        double v_y = iter-> V.y;//text_VY;
        double x = iter -> Pos.x ;//text_X;
        double y = iter -> Pos.y;//text_Y;
        double ax_half = iter -> Acc.x;
        double ay_half = iter -> Acc.y;
        double x_half = iter -> HalfPos.x ;//text_X;
        double y_half = iter -> HalfPos.y;//text_Y;

        /*
            No.5
         */
        ax_half = eq20(x_half,y_half,m);
        ay_half = eq21(x_half,y_half,m);

        /*
            No.6
            */
        v_x = eq23(ax_half,v_x);
        v_y = eq23(ay_half,v_y);

        /*
            No.7
            */
        x = eq24(v_x,x_half);
        y = eq24(v_y,y_half);

        /*
            No.8
            */
        x_half = eq22(x,v_x);
        y_half = eq22(y,v_y);

        iter -> V.x = v_x;
        iter -> V.y = v_y;
        iter -> Pos.x = x;
        iter -> Pos.y = y;
        iter -> HalfPos.x = x_half;
        iter -> HalfPos.y = y_half;
        iter -> Acc.x = ax_half;
        iter -> Acc.y = ay_half;
    }
}

void CChildView::DrawCenterStar()
{
    ///sun light defination
    GLfloat sun_light_position[] = {0.0f, 0.0f, 0.0f, 1.0f};
    GLfloat sun_light_ambient[] = {0.0f, 0.0f, 0.0f, 1.0f};
    GLfloat sun_light_diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat sun_light_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};

    glLightfv(GL_LIGHT0, GL_POSITION, sun_light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, sun_light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, sun_light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, sun_light_specular);



    ///sun material defination...
    GLfloat sun_mat_ambient[] = {0.0,0.0,0.0, 1.0f};
    GLfloat sun_mat_diffuse[] = {0.0,0.0,0.0, 1.0f};
    GLfloat sun_mat_specular[] = {0.0,0.0,0.0, 1.0f};
    GLfloat sun_mat_emission[] = {(CenterStar.Color>>16)/256.0, ((CenterStar.Color>>8)&0xff)/256.0,((CenterStar.Color)&0xff)/256.0, 1.0f};
    GLfloat sun_mat_shininess = 0.0f;

    glMaterialfv(GL_FRONT, GL_AMBIENT, sun_mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, sun_mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, sun_mat_specular);
    glMaterialfv(GL_FRONT, GL_EMISSION, sun_mat_emission);
    glMaterialf(GL_FRONT, GL_SHININESS, sun_mat_shininess);

    ///red sun
    //glColor3f(1.0f, 0.0f, 0.0f);

    gluSphere(gluNewQuadric(),CenterStar.Radius, 20, 20);
}

void CChildView::DrawPlanets()
{
    for(PContainer::iterator iter = Planets.begin();
        iter != Planets.end();
        iter ++){
        GLfloat mat_ambient[] = {(iter -> Color>>16)/256.0, ((iter -> Color>>8)&0xff)/256.0, ((iter -> Color)&0xff)/256.0, 1.0f};
        GLfloat mat_diffuse[] = {(iter -> Color>>16)/256.0, ((iter -> Color>>8)&0xff)/256.0, ((iter -> Color)&0xff)/256.0, 1.0f};
        GLfloat mat_specular[] = {(iter -> Color>>16)/256.0, ((iter -> Color>>8)&0xff)/256.0, ((iter -> Color)&0xff)/256.0, 1.0f};
        GLfloat mat_emission[] = {0.0f,0.0f,0.0f,1.0f};
        //GLfloat mat_emission[] = {(iter -> Color>>16)/256.0, ((iter -> Color>>8)&0xff)/256.0, ((iter -> Color)&0xff)/256.0, 1.0f};
        GLfloat mat_shininess = 30.0f;

        glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
        glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
        glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);

        double x = iter -> Pos.x/zoom;
        double y = iter -> Pos.y/zoom;
        double z = iter -> Pos.z/zoom;
        
        glTranslatef(x, y, z);
        gluSphere(gluNewQuadric(),iter -> Radius, 20, 20);
        glTranslatef(-x, -y, -z);
    }
}

int CChildView::LoadDataFromFile(const char *fileName)
{
    system_::iol* io = new system_::file_io("planets.txt");
    lexer::lexer*l = new lexer::lexer(io);
    config cf(l);
    try{
        cf.do_parse(&CenterStar,&Planets,&zoom,&internal_per_timer);

    } catch(int i) {
        return 0;
    }
    
    delete l;
    delete io;
    return 0;
}


