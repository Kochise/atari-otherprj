// dibview.h : interface of the CDibView class
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

class CDibView : public CScrollView
{
protected: // create from serialization only
	CDibView();
	DECLARE_DYNCREATE(CDibView)

// Attributes
public:
	CDibDoc* GetDocument()
		{
			ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDibDoc)));
			return (CDibDoc*) m_pDocument;
		}
	CSize m_zoomedsize;
	int m_left;
	int m_top;
	int m_right;
	int m_bottom;
	int m_left_zoomed;
	int m_top_zoomed;
	int m_right_zoomed;
	int m_bottom_zoomed;
	int m_flag;
	int m_x;
	int m_y;
	int m_zoomval;
	int m_setcursor;
	int m_save_zoom;

// Operations
public:

// Implementation
public:
	virtual ~CDibView();
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view

	virtual void OnInitialUpdate();
	virtual void OnUpdate(CView* pSender,
						  LPARAM lHint = 0L,
						  CObject* pHint = NULL);
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView,
					CView* pDeactiveView);

	// Printing support
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);

// Generated message map functions
// protected:
public:
	//{{AFX_MSG(CDibView)
	afx_msg void OnEditCopy();
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnEditPaste();
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
	afx_msg void OnEditSelectAll();
	afx_msg void OnIndicatorPos();
	afx_msg void OnUpdateIndicatorPos(CCmdUI* pCmdUI);
	afx_msg LRESULT OnDoRealize(WPARAM wParam, LPARAM lParam);  // user message
	afx_msg void OnPopup1MoveFrame();
	afx_msg void OnPopup1DefineFrame();
	afx_msg void OnPopup1SaveFrameAs();
	afx_msg void OnViewZoom1();
	afx_msg void OnUpdateViewZoom1(CCmdUI* pCmdUI);
	afx_msg void OnViewZoom2();
	afx_msg void OnUpdateViewZoom2(CCmdUI* pCmdUI);
	afx_msg void OnViewZoom3();
	afx_msg void OnUpdateViewZoom3(CCmdUI* pCmdUI);
	afx_msg void OnViewZoom4();
	afx_msg void OnUpdateViewZoom4(CCmdUI* pCmdUI);
	afx_msg void OnViewZoom5();
	afx_msg void OnUpdateViewZoom5(CCmdUI* pCmdUI);
	afx_msg void OnViewZoom6();
	afx_msg void OnUpdateViewZoom6(CCmdUI* pCmdUI);
	afx_msg void OnViewZoom7();
	afx_msg void OnUpdateViewZoom7(CCmdUI* pCmdUI);
	afx_msg void OnViewZoom8();
	afx_msg void OnUpdateViewZoom8(CCmdUI* pCmdUI);
	afx_msg void OnViewZoom9();
	afx_msg void OnUpdateViewZoom9(CCmdUI* pCmdUI);
	afx_msg void OnViewZoom10();
	afx_msg void OnUpdateViewZoom10(CCmdUI* pCmdUI);
	afx_msg void OnViewZoom11();
	afx_msg void OnUpdateViewZoom11(CCmdUI* pCmdUI);
	afx_msg void OnViewZoom12();
	afx_msg void OnUpdateViewZoom12(CCmdUI* pCmdUI);
	afx_msg void OnViewZoom13();
	afx_msg void OnUpdateViewZoom13(CCmdUI* pCmdUI);
	afx_msg void OnViewZoom14();
	afx_msg void OnUpdateViewZoom14(CCmdUI* pCmdUI);
	afx_msg void OnViewZoom15();
	afx_msg void OnUpdateViewZoom15(CCmdUI* pCmdUI);
	afx_msg void OnViewZoom16();
	afx_msg void OnUpdateViewZoom16(CCmdUI* pCmdUI);
	afx_msg void OnUpdateToolbarScale(CCmdUI* pCmdUI);
	afx_msg void OnUpdateToolbarZoom(CCmdUI* pCmdUI);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnOptionsSaveZoom();
	afx_msg void OnUpdateOptionsSaveZoom(CCmdUI *pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
