// mainfrm.h : interface of the CMainFrame class
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1998 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#ifndef __AFXEXT_H__
#include <afxext.h>         // for access to CToolBar and CStatusBar
#endif
#include <afxcmn.h>

// header file for TabbedMDI support
#include "OXTabClientWnd.h"

class CMyToolBar : public CToolBar
{
// Implementierung
public:
    BOOL SetTrueColorToolBar(UINT uToolBarType,
                             UINT uToolBar,
                             int  nBtnWidth);
protected:
	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CMyToolBar)
	afx_msg void OnHScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar );
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class CMySlider : public CSliderCtrl
{
// Implementierung
public:
	int tic[16];
protected:
	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CMySlider)
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class CMainFrame : public CMDIFrameWnd
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

// Implementation
public:
	virtual ~CMainFrame();

// protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CMyToolBar  m_wndToolBar;
	CStatic     m_scale;
	CMySlider   m_slider;
	CStatic     m_zoom;

	// MTI client window
	COXTabClientWnd m_MTIClientWnd;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaletteChanged(CWnd* pFocusWnd);
	afx_msg BOOL OnQueryNewPalette();
	afx_msg void OnHideTitleBar();
	afx_msg void OnUpdateHideTitleBar(CCmdUI* pCmdUI);
	afx_msg void OnHideTabBar();
	afx_msg void OnUpdateHideTabBar(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
