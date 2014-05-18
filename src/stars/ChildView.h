
// ChildView.h : CChildView ��Ľӿ�
//
#include<list>
#include<windows.h>

#include"config.h"

#pragma once


class CChildView : public CWnd
{
// ����
public:
	CChildView();

// ����
public:
    typedef  std::list<Planet> PContainer;
// ����
public:

// ��д
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// ʵ��
public:
	virtual ~CChildView();
private:
    //int day;
    double zoom;
    int internal_per_timer;
    _3DVector x_scale;
    _3DVector y_scale;
    _3DVector z_scale;
    GLsizei width,height;
    PContainer Planets;
    Star CenterStar;
	// ���ɵ���Ϣӳ�亯��
private:
    void CalculatePosition();

protected:

	DECLARE_MESSAGE_MAP()
    BOOL CreateViewGLContext(HDC hDC);
    bool SetWindowPixelFormat(HDC hDC);
    void AddPlanet(const Planet&p);
    void SetCenterStar(const Star&s);
    void DrawCenterStar();
    void InitPlanets();
    void DrawPlanets();
    // �������ƻ���(RC)��ʹ֮��Ϊ��ǰ���ƻ���
    // ��ʼ��openGL
    bool InitGL(void);


    // ��ͼ������
    int DrawGLScene(void);
    //>���ظ�ʽ������ֵ
    int m_GLPixelIndex;
    // ���ƻ�����HGLRC��һ��ָ��rendering context�ľ��
    HGLRC m_hGLContext;
public:
    afx_msg void OnPaint();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnDestroy();
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
private:
    int LoadDataFromFile(const char *);
};

