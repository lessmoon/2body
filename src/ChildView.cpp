
// ChildView.cpp : CChildView 类的实现
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

    // TODO: 在此处添加消息处理程序代码
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
    //定义窗口的像素格式
    PIXELFORMATDESCRIPTOR pixelDesc=
    {
        sizeof(PIXELFORMATDESCRIPTOR),           //nSize结构长度
        1,                                       //nVersion结构版本
        PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL| 
        PFD_DOUBLEBUFFER|PFD_SUPPORT_GDI,        //dwFlags告诉OpenGL如何处理像素
         /*
         wFlags能接收以下标志：
            PFD_DRAW_TO_WINDOW 使之能在窗口或者其他设备窗口画图；
            PFD_DRAW_TO_BITMAP 使之能在内存中的位图画图；
            PFD_SUPPORT_GDI 使之能调用GDI函数（注：如果指定了PFD_DOUBLEBUFFER，这个选项将无效）；
            PFD_SUPPORT_OpenGL 使之能调用OpenGL函数；
            PFD_GENERIC_FORMAT 假如这种象素格式由Windows GDI函数库或由第三方硬件设备驱动程序支持，则需指定这一项；
            PFD_NEED_PALETTE 告诉缓冲区是否需要调色板，本程序假设颜色是使用24或 32位色，并且不会覆盖调色板；
            PFD_NEED_SYSTEM_PALETTE 这个标志指明缓冲区是否把系统调色板当作它自身调色板的一部分；
            PFD_DOUBLEBUFFER 指明使用了双缓冲区（注：GDI不能在使用了双缓冲区的窗口中画图）；
            PFD_STEREO 指明左、右缓冲区是否按立体图像来组织。
                        PFD_SWAP_LAYER_BUFFERS
         */
        PFD_TYPE_RGBA,  //iPixelType,颜色模式，包括两种PFD_TYPE_RGBA意味着每一位(bit)组代表着rgb各分量的值。PFD_TYPE_COLORINDEX意味着每一位组代表着在彩色查找表中的索引值
        24,   //cColorBits定义了指定一个颜色的位数。对RGBA来说，位数是在颜色中红、绿、蓝各分量所占的位数。对颜色的索引值来说，指的是表中的颜色数。
        0,0,0,0,0,0,  //cRedBits、cRedShifts、cGreenBits、cGreenShifts、cBlueBits、cBlueShifts用,基本不被采用，一般置0
                  //cRedBits、cGreenBits、cBlueBits用来表明各相应分量所使用的位数。
                      //cRedShift、cGreenShift、cBlue-Shift用来表明各分量从颜色开始的偏移量所占的位数。
        0,                                       //cAlphaBits,RGB颜色缓存中Alpha的位数                            
        0,                                 //cAlphaShift，已经不被采用，置0                   
        0,                                       //cAcuumBits累计缓存的位数
        0,0,0,0,                                 //cAcuumRedBits/cAcuumGreenBits/cAcuumBlueBits/cAcuumAlphaBits,基本不被采用，置0
        32,                                      //cDepthBits深度缓存的位数
        0,                                       //cStencilBits,模板缓存的位数
        0,                                       //cAuxBuffers，辅助缓存的位数，一般置0
        PFD_MAIN_PLANE,                          //iLayerType,说明层面的类型，可忽略置0，是早期的版本，包括
                                                 //PFD_MAIN_PLANE,PFD_OVER_LAY_PLANE,PFD_UNDERLAY_PLANE
        0,0,0,0                                  //bReserved,dwLayerMask,dwVisibleMask,dwDamageMask,必须置0
    };

    this->m_GLPixelIndex = ChoosePixelFormat(hDC,&pixelDesc);//选择最相近的像素格式
    /*
    ChoosePixelFormat接受两个参数：一个是hDc，另一个是一个指向PIXELFORMATDESCRIPTOR结构的指针&pixelDesc
    该函数返回此像素格式的索引值,如果返回0则表示失败。
    假如函数失败，我们只是把索引值设为1并用 DescribePixelFormat得到像素格式描述。
    假如你申请一个没得到支持的像素格式，则ChoosePixelFormat将会返回与你要求的像素格式最接近的一个值
    一旦我们得到一个像素格式的索引值和相应的描述，我们就可以调用SetPixelFormat设置像素格式，并且只需设置一次。
    */
    if(this->m_GLPixelIndex==0)
    {//选择失败
        this->m_GLPixelIndex = 1;//默认的像素格式
        //用默认的像素格式进行设置
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

// CChildView 消息处理程序

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	
    cs.style |= (WS_CLIPCHILDREN | WS_CLIPSIBLINGS);//openGL在MDI中必需
    return CWnd::PreCreateWindow(cs);
}

void CChildView::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);

    // TODO: 在此处添加消息处理程序代码
    /*
    OnSize通过glViewport(0, 0, width, height)定义了视口和视口坐标。
    glViewport的第一、二个参数是视口左下角的像素坐标，第三、四个参数是视口的宽度和高度。

    OnSize中的glMatrixMode是用来设置矩阵模式的，它有三个选项：GL_MODELVIEW、GL_PROJECTION、 GL_TEXTURE。
    GL_MODELVIEW表示从实体坐标系转到人眼坐标系。
    GL_PROJECTION表示从人眼坐标系转到剪裁坐标系。
    GL_TEXTURE表示从定义纹理的坐标系到粘贴纹理的坐标系的变换。

    glLoadIdentity初始化工程矩阵(project matrix)
    gluOrtho2D把工程矩阵设置成显示一个二维直角显示区域。
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
    gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,5000.0f);//透视投影
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
        //WglCreateContext函数创建一个新的OpenGL渲染描述表(RC)
    //此描述表必须适用于绘制到由hdc返回的设备
    //这个渲染描述表将有和设备上下文(dc)一样的像素格式.
    this->m_hGLContext = wglCreateContext(hDC);//创建RC

    if(this->m_hGLContext==NULL)
    {//创建失败
        return FALSE;
    }

    /*
    wglMakeCurrent 函数设定OpenGL当前线程(线程相关性)的渲染环境。
    以后这个线程所有的OpenGL调用都是在这个hdc标识的设备上绘制。
    你也可以使用wglMakeCurrent 函数来改变调用线程的当前渲染环境
    使之不再是当前的渲染环境。
    */
    if(wglMakeCurrent(hDC,this->m_hGLContext)==FALSE)
    {//选为当前RC失败
        return FALSE;
    }
    return TRUE;
}

void CChildView::OnPaint() 
{
	CPaintDC dc(this); // 用于绘制的设备上下文
    
    // TODO: 在此处添加消息处理程序代码
    // 不为绘图消息调用 CView::OnPaint()
    this->DrawGLScene();
    SwapBuffers(dc.m_hDC);
}

int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    // TODO:  在此添加您专用的创建代码
    //得到一个窗口对象（CWnd的派生对象）指针的句柄（HWND）  
    HWND hWnd = this->GetSafeHwnd();  
    //GetDC该函数检索一指定窗口的客户区域或整个屏幕的显示设备上下文环境的句柄
    //以后可以在GDI函数中使用该句柄来在设备上下文环境中绘图。
    HDC hDC = ::GetDC(hWnd);

    if(this->SetWindowPixelFormat(hDC)==FALSE)
    {//设置像素格式
        return 0;
    }
    if(this->CreateViewGLContext(hDC)==FALSE)
    {//创建RC并选为所用
        return 0;
    }
    if(!this->InitGL())
    {//初始化openGL
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
    // TODO: 在此添加消息处理程序代码和/或调用默认值
    CalculatePosition();
    this -> Invalidate();
    this -> Invalidate(false);
    CWnd::OnTimer(nIDEvent);
}

BOOL CChildView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    // TODO: 在此添加消息处理程序代码和/或调用默认值

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
    // TODO: 在此添加消息处理程序代码和/或调用默认值
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
    // TODO: 在此添加消息处理程序代码和/或调用默认值

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
    temp.Set(0,_3DVector(0,1000,0),_3DVector(363104000,0,0),0x0000ff,1);//水星
    this -> AddPlanet(temp);
    temp.Set(0,_3DVector(0,800,0),_3DVector(200104000,0,0),0x00FF00,1);//水星
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
    temp.Set(3.30e23,_3DVector(0,0,5.791E10),_3DVector(0,0,47.87E3),0x996671);//金星
    this -> AddPlanet(temp);
    temp.Set(3.30e23,_3DVector(0,0,5.791E10),_3DVector(0,0,47.87E3),0x996671);//地球
    this -> AddPlanet(temp);
    temp.Set(3.30e23,_3DVector(0,0,5.791E10),_3DVector(0,0,47.87E3),0x996671);//火星
    this -> AddPlanet(temp);
    temp.Set(3.30e23,_3DVector(0,0,5.791E10),_3DVector(0,0,47.87E3),0x996671);//木星
    this -> AddPlanet(temp);
    temp.Set(3.30e23,_3DVector(0,0,5.791E10),_3DVector(0,0,47.87E3),0x996671);//土星
    this -> AddPlanet(temp);
    temp.Set(3.30e23,_3DVector(0,0,5.791E10),_3DVector(0,0,47.87E3),0x996671);//天王星
    this -> AddPlanet(temp);
    temp.Set(3.30e23,_3DVector(0,0,5.791E10),_3DVector(0,0,47.87E3),0x996671);//海王星
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


