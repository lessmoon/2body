
// ChildView.h : CChildView 类的接口
//
#include<list>
#include<windows.h>

#include"config.h"

#pragma once


class CChildView : public CWnd
{
// 构造
public:
	CChildView();

// 特性
public:
    typedef  std::list<Planet> PContainer;
// 操作
public:

// 重写
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// 实现
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
	// 生成的消息映射函数
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
    // 创建绘制环境(RC)并使之成为当前绘制环境
    // 初始化openGL
    bool InitGL(void);


    // 绘图代码区
    int DrawGLScene(void);
    //>像素格式的索引值
    int m_GLPixelIndex;
    // 绘制环境，HGLRC是一个指向rendering context的句柄
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

