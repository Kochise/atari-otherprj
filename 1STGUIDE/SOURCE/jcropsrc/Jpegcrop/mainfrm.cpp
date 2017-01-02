// mainfrm.cpp : implementation of the CMainFrame class
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

#include "stdafx.h"
#include "diblook.h"
#include "dibdoc.h"
#include "dibview.h"

#include "mainfrm.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_PALETTECHANGED()
	ON_WM_QUERYNEWPALETTE()
	ON_COMMAND(ID_VIEW_TITLEBAR, OnHideTitleBar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TITLEBAR, OnUpdateHideTitleBar)
	ON_COMMAND(ID_VIEW_TAB_BAR, OnHideTabBar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TAB_BAR, OnUpdateHideTabBar)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// arrays of IDs used to initialize control bars

// toolbar buttons - IDs are command buttons
static UINT BASED_CODE buttons[] =
{
	// same order as in the bitmap 'toolbar.bmp'
	ID_FILE_OPEN,
	ID_FILE_SAVE_COPY_AS,
		ID_SEPARATOR,
	ID_OPTIONS_SAVEZOOM,
	ID_VIEW_BLOCKGRID,
	ID_VIEW_CROPMASK,
	ID_OPTIONS_ENDPOINTSNAP,
	ID_OPTIONS_TRIM,
	ID_OPTIONS_GRAYSCALE,
		ID_SEPARATOR,
	ID_TRANSFORM_ROT_90,
	ID_TRANSFORM_ROT_270,
	ID_TRANSFORM_FLIP_H,
	ID_TRANSFORM_FLIP_V,
	ID_TRANSFORM_ROT_180,
	ID_TRANSFORM_TRANSPOSE,
	ID_TRANSFORM_TRANSVERSE,
	ID_TRANSFORM_ORIGINAL,
	ID_TRANSFORM_WIPE,
		ID_SEPARATOR,
		ID_SEPARATOR,
		ID_SEPARATOR,
		ID_SEPARATOR,
		ID_SEPARATOR,
		ID_SEPARATOR,
		ID_SEPARATOR,
	ID_FILE_PRINT,
	ID_FILE_PREFERENCES,
	ID_APP_ABOUT
};

static UINT BASED_CODE indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_POS,
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL
};

BEGIN_MESSAGE_MAP(CMyToolBar, CToolBar)
	//{{AFX_MSG_MAP(CMyToolBar)
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CMyToolBar::SetTrueColorToolBar(UINT uToolBarType,
                                     UINT uToolBar,
                                     int  nBtnWidth)
{
    CImageList  cImageList;
    CBitmap     cBitmap;
    BITMAP      bmBitmap;

    if (!cBitmap.Attach(LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(uToolBar),
                                  IMAGE_BITMAP, 0, 0,
                                  LR_DEFAULTSIZE|LR_CREATEDIBSECTION)) ||
        !cBitmap.GetBitmap(&bmBitmap))
        return FALSE;

    CSize       cSize(bmBitmap.bmWidth, bmBitmap.bmHeight); 
    int         nNbBtn  = cSize.cx/nBtnWidth;
    RGBTRIPLE*  rgb     = (RGBTRIPLE*)(bmBitmap.bmBits);
    COLORREF    rgbMask = RGB(rgb[0].rgbtRed, rgb[0].rgbtGreen, rgb[0].rgbtBlue);

    if (!cImageList.Create(nBtnWidth, cSize.cy, ILC_COLOR24|ILC_MASK, nNbBtn, 0))
        return FALSE;

    if (cImageList.Add(&cBitmap, rgbMask) == -1)
        return FALSE;

    SendMessage(uToolBarType, 0, (LPARAM)cImageList.m_hImageList);
    cImageList.Detach(); 
    cBitmap.Detach();

    return TRUE;
}

void CMyToolBar::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar)
{
    CMDIFrameWnd *pFrame = (CMDIFrameWnd *) AfxGetApp()->m_pMainWnd;
    if (pFrame == NULL) return;

// Get the active MDI child window.
    CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
    if (pChild == NULL) return;

// or CMDIChildWnd *pChild = pFrame->MDIGetActive();

// Get the active view attached to the active MDI child window.
    CDibView *pView = (CDibView *) pChild->GetActiveView();
	if (pView == NULL)
		switch (((CSliderCtrl *) pScrollBar)->GetPos())
		{
		case  1:
			((CMainFrame *)pFrame)->m_scale.SetWindowText("1/8");
			((CMainFrame *)pFrame)->m_zoom.SetWindowText("12.5 %");
			break;
		case  2:
			((CMainFrame *)pFrame)->m_scale.SetWindowText("2/8");
			((CMainFrame *)pFrame)->m_zoom.SetWindowText("25 %");
			break;
		case  3:
			((CMainFrame *)pFrame)->m_scale.SetWindowText("3/8");
			((CMainFrame *)pFrame)->m_zoom.SetWindowText("37.5 %");
			break;
		case  4:
			((CMainFrame *)pFrame)->m_scale.SetWindowText("4/8");
			((CMainFrame *)pFrame)->m_zoom.SetWindowText("50 %");
			break;
		case  5:
			((CMainFrame *)pFrame)->m_scale.SetWindowText("5/8");
			((CMainFrame *)pFrame)->m_zoom.SetWindowText("62.5 %");
			break;
		case  6:
			((CMainFrame *)pFrame)->m_scale.SetWindowText("6/8");
			((CMainFrame *)pFrame)->m_zoom.SetWindowText("75 %");
			break;
		case  7:
			((CMainFrame *)pFrame)->m_scale.SetWindowText("7/8");
			((CMainFrame *)pFrame)->m_zoom.SetWindowText("87.5 %");
			break;
		case  8:
			((CMainFrame *)pFrame)->m_scale.SetWindowText("8/8");
			((CMainFrame *)pFrame)->m_zoom.SetWindowText("100 %");
			break;
		case  9:
			((CMainFrame *)pFrame)->m_scale.SetWindowText("9/8");
			((CMainFrame *)pFrame)->m_zoom.SetWindowText("112.5 %");
			break;
		case 10:
			((CMainFrame *)pFrame)->m_scale.SetWindowText("10/8");
			((CMainFrame *)pFrame)->m_zoom.SetWindowText("125 %");
			break;
		case 11:
			((CMainFrame *)pFrame)->m_scale.SetWindowText("11/8");
			((CMainFrame *)pFrame)->m_zoom.SetWindowText("137.5 %");
			break;
		case 12:
			((CMainFrame *)pFrame)->m_scale.SetWindowText("12/8");
			((CMainFrame *)pFrame)->m_zoom.SetWindowText("150 %");
			break;
		case 13:
			((CMainFrame *)pFrame)->m_scale.SetWindowText("13/8");
			((CMainFrame *)pFrame)->m_zoom.SetWindowText("162.5 %");
			break;
		case 14:
			((CMainFrame *)pFrame)->m_scale.SetWindowText("14/8");
			((CMainFrame *)pFrame)->m_zoom.SetWindowText("175 %");
			break;
		case 15:
			((CMainFrame *)pFrame)->m_scale.SetWindowText("15/8");
			((CMainFrame *)pFrame)->m_zoom.SetWindowText("187.5 %");
			break;
		case 16:
			((CMainFrame *)pFrame)->m_scale.SetWindowText("16/8");
			((CMainFrame *)pFrame)->m_zoom.SetWindowText("200 %");
			break;
		}
	else
		switch (((CSliderCtrl *) pScrollBar)->GetPos())
		{
		case  1: pView->OnViewZoom1();  break;
		case  2: pView->OnViewZoom2();  break;
		case  3: pView->OnViewZoom3();  break;
		case  4: pView->OnViewZoom4();  break;
		case  5: pView->OnViewZoom5();  break;
		case  6: pView->OnViewZoom6();  break;
		case  7: pView->OnViewZoom7();  break;
		case  8: pView->OnViewZoom8();  break;
		case  9: pView->OnViewZoom9();  break;
		case 10: pView->OnViewZoom10(); break;
		case 11: pView->OnViewZoom11(); break;
		case 12: pView->OnViewZoom12(); break;
		case 13: pView->OnViewZoom13(); break;
		case 14: pView->OnViewZoom14(); break;
		case 15: pView->OnViewZoom15(); break;
		case 16: pView->OnViewZoom16(); break;
		}
}

BEGIN_MESSAGE_MAP(CMySlider, CSliderCtrl)
	//{{AFX_MSG_MAP(CMySlider)
	ON_WM_RBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CMySlider::OnRButtonDown(UINT nFlags, CPoint point)
{
    int i = 1;
    while (point.x > tic[i-1] && i < 16) i++;

    CMDIFrameWnd *pFrame = (CMDIFrameWnd *) AfxGetApp()->m_pMainWnd;
    if (pFrame == NULL) return;

// Get the active MDI child window.
    CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
    if (pChild == NULL) return;

// or CMDIChildWnd *pChild = pFrame->MDIGetActive();

// Get the active view attached to the active MDI child window.
    CDibView *pView = (CDibView *) pChild->GetActiveView();
	if (pView == NULL)
	{
		SetPos(i);
		switch (i)
		{
		case  1:
			((CMainFrame *)pFrame)->m_scale.SetWindowText("1/8");
			((CMainFrame *)pFrame)->m_zoom.SetWindowText("12.5 %");
			break;
		case  2:
			((CMainFrame *)pFrame)->m_scale.SetWindowText("2/8");
			((CMainFrame *)pFrame)->m_zoom.SetWindowText("25 %");
			break;
		case  3:
			((CMainFrame *)pFrame)->m_scale.SetWindowText("3/8");
			((CMainFrame *)pFrame)->m_zoom.SetWindowText("37.5 %");
			break;
		case  4:
			((CMainFrame *)pFrame)->m_scale.SetWindowText("4/8");
			((CMainFrame *)pFrame)->m_zoom.SetWindowText("50 %");
			break;
		case  5:
			((CMainFrame *)pFrame)->m_scale.SetWindowText("5/8");
			((CMainFrame *)pFrame)->m_zoom.SetWindowText("62.5 %");
			break;
		case  6:
			((CMainFrame *)pFrame)->m_scale.SetWindowText("6/8");
			((CMainFrame *)pFrame)->m_zoom.SetWindowText("75 %");
			break;
		case  7:
			((CMainFrame *)pFrame)->m_scale.SetWindowText("7/8");
			((CMainFrame *)pFrame)->m_zoom.SetWindowText("87.5 %");
			break;
		case  8:
			((CMainFrame *)pFrame)->m_scale.SetWindowText("8/8");
			((CMainFrame *)pFrame)->m_zoom.SetWindowText("100 %");
			break;
		case  9:
			((CMainFrame *)pFrame)->m_scale.SetWindowText("9/8");
			((CMainFrame *)pFrame)->m_zoom.SetWindowText("112.5 %");
			break;
		case 10:
			((CMainFrame *)pFrame)->m_scale.SetWindowText("10/8");
			((CMainFrame *)pFrame)->m_zoom.SetWindowText("125 %");
			break;
		case 11:
			((CMainFrame *)pFrame)->m_scale.SetWindowText("11/8");
			((CMainFrame *)pFrame)->m_zoom.SetWindowText("137.5 %");
			break;
		case 12:
			((CMainFrame *)pFrame)->m_scale.SetWindowText("12/8");
			((CMainFrame *)pFrame)->m_zoom.SetWindowText("150 %");
			break;
		case 13:
			((CMainFrame *)pFrame)->m_scale.SetWindowText("13/8");
			((CMainFrame *)pFrame)->m_zoom.SetWindowText("162.5 %");
			break;
		case 14:
			((CMainFrame *)pFrame)->m_scale.SetWindowText("14/8");
			((CMainFrame *)pFrame)->m_zoom.SetWindowText("175 %");
			break;
		case 15:
			((CMainFrame *)pFrame)->m_scale.SetWindowText("15/8");
			((CMainFrame *)pFrame)->m_zoom.SetWindowText("187.5 %");
			break;
		case 16:
			((CMainFrame *)pFrame)->m_scale.SetWindowText("16/8");
			((CMainFrame *)pFrame)->m_zoom.SetWindowText("200 %");
			break;
		}
	} else
		switch (i)
		{
		case  1: pView->OnViewZoom1();  break;
		case  2: pView->OnViewZoom2();  break;
		case  3: pView->OnViewZoom3();  break;
		case  4: pView->OnViewZoom4();  break;
		case  5: pView->OnViewZoom5();  break;
		case  6: pView->OnViewZoom6();  break;
		case  7: pView->OnViewZoom7();  break;
		case  8: pView->OnViewZoom8();  break;
		case  9: pView->OnViewZoom9();  break;
		case 10: pView->OnViewZoom10(); break;
		case 11: pView->OnViewZoom11(); break;
		case 12: pView->OnViewZoom12(); break;
		case 13: pView->OnViewZoom13(); break;
		case 14: pView->OnViewZoom14(); break;
		case 15: pView->OnViewZoom15(); break;
		case 16: pView->OnViewZoom16(); break;
		}
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

    CRect rect;
    rect = m_wndToolBar.GetBorders();
    m_wndToolBar.SetBorders(3, rect.top, 3, rect.bottom);

    if (!m_wndToolBar.Create(this, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_TOOLTIPS | CBRS_FLYBY) ||
	!m_wndToolBar.LoadToolBar(IDB_MAINFRAME) ||
	!m_wndToolBar.SetButtons(buttons, sizeof(buttons)/sizeof(UINT)))
    {
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
    }

    m_wndToolBar.SetSizes(CSize(31,30), CSize(24,24));

    m_wndToolBar.SetTrueColorToolBar(TB_SETIMAGELIST, IDB_TOOLBAR_DRAW, 24);

    m_wndToolBar.SetButtonInfo(20, ID_TOOLBAR_SCALE, TBBS_SEPARATOR, 36);
    m_wndToolBar.GetItemRect(20, &rect);
    if (!m_scale.Create("8/8", SS_CENTERIMAGE | SS_RIGHT | WS_VISIBLE | WS_TABSTOP,
        rect, &m_wndToolBar, ID_TOOLBAR_SCALE))
    {
        TRACE0("Failed to create toolbar scale\n");
		return -1;      // fail to create
    }
    m_scale.SetFont(m_wndToolBar.GetFont());

    m_wndToolBar.SetButtonInfo(22, ID_TOOLBAR_SLIDER, TBBS_SEPARATOR, 102);
    m_wndToolBar.GetItemRect(22, &rect);
    if (!m_slider.Create(TBS_HORZ | TBS_AUTOTICKS | TBS_TOP | TBS_ENABLESELRANGE |
		WS_VISIBLE | WS_TABSTOP, rect, &m_wndToolBar, ID_TOOLBAR_SLIDER))
    {
        TRACE0("Failed to create toolbar slider\n");
		return -1;      // fail to create
    }
    m_slider.SetRange(1, 16);
    m_slider.SetSelection(1, 8);
    m_slider.SetPos(8);
    m_slider.SetPageSize(1);

    /* Adjust slider width for uniform tic grid with distance 5 */
    m_slider.tic[ 1] = m_slider.GetTicPos( 0);
    m_slider.tic[14] = m_slider.GetTicPos(13);
    if (m_slider.tic[14] - m_slider.tic[1] != 13 * 5)
    {
		m_wndToolBar.SetButtonInfo(21, ID_TOOLBAR_SLIDER, TBBS_SEPARATOR,
			102 + 15 * 5 - ((m_slider.tic[14] - m_slider.tic[1]) * 15) / 13);
		m_slider.SetWindowPos(NULL, 0, 0,
			102 + 15 * 5 - ((m_slider.tic[14] - m_slider.tic[1]) * 15) / 13,
		rect.Height(), SWP_NOZORDER | SWP_NOMOVE);
    }

    for (int i = 0; i < 14; i++) m_slider.tic[i+1] = m_slider.GetTicPos(i);
    m_slider.tic[ 0] = m_slider.tic[ 1] * 2 - m_slider.tic[ 2];
    m_slider.tic[15] = m_slider.tic[14] * 2 - m_slider.tic[13];
    for (i = 0; i < 15; i++)
		m_slider.tic[i] = (m_slider.tic[i] + m_slider.tic[i+1]) >> 1;

    m_wndToolBar.SetButtonInfo(24, ID_TOOLBAR_ZOOM, TBBS_SEPARATOR, 52);
    m_wndToolBar.GetItemRect(24, &rect);
    if (!m_zoom.Create("100 %", SS_CENTERIMAGE | WS_VISIBLE | WS_TABSTOP,
        rect, &m_wndToolBar, ID_TOOLBAR_ZOOM))
    {
        TRACE0("Failed to create toolbar zoom\n");
		return -1;      // fail to create
    }
    m_zoom.SetFont(m_wndToolBar.GetFont());

    if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT)))
    {
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
    }

    m_MTIClientWnd.GetTabCtrl()->SetOffset(3, 1, -4, -4, FALSE);

    return 0;
}


/////////////////////////////////////////////////////////////////////////////
// CMainFrame commands

void CMainFrame::OnPaletteChanged(CWnd* pFocusWnd)
{
	CMDIFrameWnd::OnPaletteChanged(pFocusWnd);

	// always realize the palette for the active view
	CMDIChildWnd* pMDIChildWnd = MDIGetActive();
	if (pMDIChildWnd == NULL)
		return; // no active MDI child frame
	CView* pView = pMDIChildWnd->GetActiveView();
	ASSERT(pView != NULL);

	// notify all child windows that the palette has changed
	SendMessageToDescendants(WM_DOREALIZE, (WPARAM)pView->m_hWnd);
}

BOOL CMainFrame::OnQueryNewPalette()
{
	// always realize the palette for the active view
	CMDIChildWnd* pMDIChildWnd = MDIGetActive();
	if (pMDIChildWnd == NULL)
		return FALSE; // no active MDI child frame (no new palette)
	CView* pView = pMDIChildWnd->GetActiveView();
	ASSERT(pView != NULL);

	// just notify the target view
	pView->SendMessage(WM_DOREALIZE, (WPARAM)pView->m_hWnd);
	return TRUE;
}

void CMainFrame::OnHideTitleBar() 
{
    DWORD dwStyle;

    dwStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);
    if (dwStyle & WS_CAPTION)
        dwStyle &= ~WS_CAPTION;
    else
        dwStyle |= WS_CAPTION;

    ::SetWindowLong(m_hWnd, GWL_STYLE, dwStyle);

    SetWindowPos(NULL, 0, 0, 0, 0,
                 SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER |
                 SWP_DRAWFRAME);
}

void CMainFrame::OnUpdateHideTitleBar(CCmdUI* pCmdUI) 
{
    DWORD dwStyle;

    dwStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);

    pCmdUI->SetCheck(dwStyle & WS_CAPTION ? TRUE : FALSE);
}

void CMainFrame::OnHideTabBar() 
{
    if (m_MTIClientWnd.IsAttached())
        m_MTIClientWnd.Detach();
    else
        m_MTIClientWnd.Attach(this);
}

void CMainFrame::OnUpdateHideTabBar(CCmdUI* pCmdUI) 
{
    pCmdUI->SetCheck(m_MTIClientWnd.IsAttached());
}
