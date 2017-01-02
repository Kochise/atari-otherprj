// ==========================================================================
//								Class Specification : 
//			COXTabClientWnd & COXTabWorkspace
// ==========================================================================

// Header file : OXTabClientWnd.h

// Copyright © Dundas Software Ltd. 1997 - 1998, All Rights Reserved
                         
// //////////////////////////////////////////////////////////////////////////

#ifndef _OXTABCLIENTWND_H__
#define _OXTABCLIENTWND_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "OXDllExt.h"


#ifndef __AFXTEMPL_H__
#include <afxtempl.h>
#define __AFXTEMPL_H__
#endif



/*

  Description.

We introduce a new interface extension for MDI(multiple document interface) 
based applications - the tabbed MDI interface.

We haven't changed anything in the relationships between the frame window 
(MDIFrame window) and its children (MDIChild windows). We added 
functionality that is usually overlooked while developing
MDI applications. 

The MDIClient window which resides in the client 
area of the MDIFrame window manages MDIChild windows. Instead of
just displaying the MDIClient window we also display standard tab control
(hence tabbed MDI) in which we create an item for every MDIChild
window. The window icon and text will be associated with the corresponding
tab item. 

Using tab control you can switch quickly between MDIChild windows by
clicking on the tab item. If you double click over a tab item then
the corresponding MDIChild window will be maximized/restored.

We use the standard tab control you can customize its
appearance using the standard set of applicable functions.  


We developed two classes in order to provide the above described 
functionality:

	COXTabWorkspace				-	CTabCtrl derived class. Covers 
									MDIClient area. For every MDIChild
									window there will be the tab item
									that will use window text and icon
									as item text and icon. Whenever you
									click on the item the coresponding
									child window will be activated.
									Whenever you double click on the item
									the corresponding MDIChild window
									will be maximized/restored

	COXTabClientWnd				-	CWnd derived class. Subclasses MDIClient
									window. Manages the placement of the 
									MDIClient and tab control regarding each 
									other. 


Almost all the logic of the classes is implented internally and there is no 
that much public members. Refer to COXTabWorkspace reference for the list of 
functions available to customize the tab control appearance.


COXTabClientWnd class has a few of public functions (refer to COXTabClientWnd
reference for details) but primarily you will be interest in the following ones:

	BOOL Attach(const CMDIFrameWnd* pParentFrame, 
		DWORD dwTabCtrlStyle=DEFAULT_TABCTRLSTYLE);
	// --- In  :	pParentFrame	-	pointer to MDIFrame window of 
	//									the application
	//				dwTabCtrlStyle	-	tab control styles that will be 
	//									used while creating the tab control.
	//									Refer to the Windows SDK documentation
	//									for list of all available styles.
	//									The following styles are used by 
	//									default:
	//
	//									TCS_BUTTONS
	//									TCS_MULTILINE
	//									TCS_HOTTRACK
	//
	// --- Out : 
	// --- Returns: TRUE if success or FALSE otherwise.
	// --- Effect : Substitutes the standard MDI interface with enhanced
	//				tabbed MDI
	
	BOOL Detach();
	// --- In  :
	// --- Out : 
	// --- Returns: TRUE if success or FALSE otherwise.
	// --- Effect : Restore the standard MDI interface


So in order to implement tabbed MDI application you could take the 
following steps:

	1)	create the standard MDI application or use the existing one. 
	2)	define the object of COXTabClientWnd class in your 
		CMDIFrameWnd derived class (usually CMainFrame)

		// MTI client window
		COXTabClientWnd m_MTIClientWnd;

	3)	in OnCreate() function of your CMDIFrameWnd derived class call
		COXTabClientWnd::Attach function

		m_MTIClientWnd.Attach(this);

That's it. 



  Example.
We updated CoolToolBar sample that is found in the .\Samples\gui\CoolToolBar
subdirectory of your Ultimate Toolbox directory. There you will see how you 
can customize the tabbed MDI interface appearance.

*/


#define IDT_MDI_STATUS_TIMER	333
#define IDC_TABWORKSPACE		1000

#ifndef OXWM_MTI_GETWINDOWTEXT
#define OXWM_MTI_GETWINDOWTEXT	WM_APP+234
#endif

#define DEFAULT_TABCTRLSTYLE	TCS_BUTTONS|TCS_MULTILINE|TCS_HOTTRACK


// structure used internally to represent information about any created
// MDIChild window 
//
typedef struct _tagTAB_ITEM_ENTRY
{
	CString sText;
	CString sWndClass;
	CWnd* pWnd;
	BOOL bFound;

} TAB_ITEM_ENTRY;
//
//	sText		-	MDIChild window text
//	sWndClass	-	name of the window class name 
//	pWnd		-	pointer to MDIChild window object
//	bFound		-	parameter used for integrity testing. Set to TRUE if 
//					corresponding MDIChild window is still active
//
//////////////////////////////////////////////////////////////////////////


class COXTabClientWnd;

/////////////////////////////////////////////////////////////////////////////
// COXTabWorkspace window
class OX_CLASS_DECL COXTabWorkspace : public CTabCtrl
{
// Construction
public:
	COXTabWorkspace();
	// --- In  :
	// --- Out : 
	// --- Returns:
	// --- Effect : Constructs the object

// Attributes
public:

protected:
	// array of tab items (every item corresponds to a MDIChild window)
    CArray<TAB_ITEM_ENTRY, TAB_ITEM_ENTRY> m_arrTab;
	// array of MDIChild windows icons used in the tab control
    CArray<HICON, HICON> m_arrImage;

	// image list associated with the tab control
    CImageList m_imageList;
	
	// pointer to our substitute for MDIClient window. If it is NULL then
	// the tab control is not defined and there shouldn't be any action 
	// taken on it
	COXTabClientWnd* m_pTabClientWnd;

	// offset from the borders of the client area of the MDIFrame window
	// where the tab control will be displayed
	CRect m_rOffset;

// Operations
public:

	// --- In  :	rOffset	-	offset in points from the MDIFrame window
	//							client area where the tab control will be 
	//							displayed
	// --- Out : 
	// --- Returns:
	// --- Effect : Sets the tab control offset from MDIFrame borders
    inline void SetOffset(LPCRECT rOffset, BOOL bRecalcLayout=TRUE) 
	{ 
		m_rOffset.left = rOffset->left; 
		m_rOffset.top = rOffset->top; 
		m_rOffset.right = rOffset->right; 
		m_rOffset.bottom = rOffset->bottom; 
		if(::IsWindow(GetSafeHwnd()))
		{
			SetWindowPos(NULL,0,0,0,0,
				SWP_NOMOVE|SWP_DRAWFRAME|SWP_NOSIZE|SWP_NOZORDER);
			if(bRecalcLayout)
			{
				CMDIFrameWnd* pFrame=GetParentFrame();
				if(pFrame!=NULL)
					pFrame->RecalcLayout();
			}
		}
	}
	inline void SetOffset(int cxLeft, int cyTop, int cxRight, int cyBot,
                          BOOL bRecalcLayout=TRUE) 
	{ 
		m_rOffset.left = cxLeft; 
		m_rOffset.top = cyTop; 
		m_rOffset.right = cxRight; 
		m_rOffset.bottom = cyBot; 
		if(::IsWindow(GetSafeHwnd()))
		{
			SetWindowPos(NULL,0,0,0,0,
				SWP_NOMOVE|SWP_DRAWFRAME|SWP_NOSIZE|SWP_NOZORDER);
			if(bRecalcLayout)
			{
				CMDIFrameWnd* pFrame=GetParentFrame();
				if(pFrame!=NULL)
					pFrame->RecalcLayout();
			}
		}
	}

	inline CRect GetOffset() const { return m_rOffset; }
	// --- In  :	
	// --- Out : 
	// --- Returns:	offset in points from the MDIFrame window client area 
	//				where the tab control will be displayed
	// --- Effect : Retrievs the tab control offset from MDIFrame borders


protected:
	// scan through all MDIChild windows and update corresponding 
	// tab items if any changes happened (e.g. window text or active MDIChild).
	// If bAddNewWindows is set to TRUE then for any new found MDIChild
	// window the new tab item will be created (this option is useful when
	// it is called for first time and there are already some MDIChild windows
	// created) 
	void UpdateContents(BOOL bAddNewWindows=FALSE);

	// returns the pointer for MDIFrame window
	CMDIFrameWnd* GetParentFrame() const;

	// returns text for child window to be displayed in coresponding
	// tab item
	CString GetTextForTabItem(const CWnd* pChildWnd) const;


	// finds the tab item that corresponds to the specified window
	inline int FindTabItem(const CWnd* pWnd) const {
		ASSERT(pWnd!=NULL);
		ASSERT(::IsWindow(pWnd->GetSafeHwnd()));
		return FindTabItem(pWnd->GetSafeHwnd());
	}

	// finds the tab item that corresponds to the specified window
	int FindTabItem(const HWND hWnd) const;
	// adds new tab item for the specified window
	BOOL AddTabItem(const CWnd* pChildWnd, BOOL bRedraw=TRUE, 
		BOOL bOnlyVisible=TRUE);
	// removes the tab item for the specified window
	BOOL RemoveTabItem(const CWnd* pChildWnd, BOOL bRedraw=TRUE);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COXTabWorkspace)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~COXTabWorkspace();

	// Generated message map functions
protected:
	//{{AFX_MSG(COXTabWorkspace)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg BOOL OnSelchange(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg void OnNcPaint();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

    friend class COXTabClientWnd;
};


/////////////////////////////////////////////////////////////////////////////
// COXTabClientWnd window

class OX_CLASS_DECL COXTabClientWnd : public CWnd
{
// Construction
public:
	COXTabClientWnd();
	// --- In  :
	// --- Out : 
	// --- Returns:
	// --- Effect : Constructs the object

// Attributes
public:

protected:
	// tab control
	COXTabWorkspace m_tab;
	// pointer to the corresponding parent MDIFrame window
	CMDIFrameWnd* m_pParentFrame;

	// flag that specifies that layout of tab control and MDIClient 
	// must be recalculated
	BOOL m_bForceToRecalc;

// Operations
public:
	BOOL Attach(const CMDIFrameWnd* pParentFrame, 
		DWORD dwTabCtrlStyle=DEFAULT_TABCTRLSTYLE);
	// --- In  :	pParentFrame	-	pointer to MDIFrame window of 
	//									the application
	//				dwTabCtrlStyle	-	tab control styles that will be 
	//									used while creating the tab control.
	//									Refer to the Windows SDK documentation
	//									for list of all available styles.
	//									The following styles are used by 
	//									default:
	//
	//									TCS_BUTTONS
	//									TCS_MULTILINE
	//									TCS_HOTTRACK
	//
	// --- Out : 
	// --- Returns: TRUE if success or FALSE otherwise.
	// --- Effect : Substitutes the standard MDI interface with enhanced
	//				tabbed MDI
	
	BOOL Detach();
	// --- In  :
	// --- Out : 
	// --- Returns: TRUE if success or FALSE otherwise.
	// --- Effect : Restore the standard MDI interface

	inline BOOL IsAttached() const { 
	// --- In  :
	// --- Out : 
	// --- Returns: TRUE if the tabbed MDI interface is active.
	// --- Effect : Retrieves the flag that specifies whether the 
	//				standard MDI interface was substituted with enhanced
	//				tabbed MDI or not.

		return (m_pParentFrame!=NULL ? TRUE : FALSE); 
	}

	inline COXTabWorkspace* GetTabCtrl() {
	// --- In  :
	// --- Out : 
	// --- Returns: pointer to the tab control
	// --- Effect : Retrieves pointer to the tab control

		return &m_tab;
	}

	inline CMDIFrameWnd* GetParentFrame() { 
	// --- In  :
	// --- Out : 
	// --- Returns: pointer to the parent MDIFrame window or NULL if none
	//				was attached
	// --- Effect : Retrieves pointer to the parent MDIFrame window

#ifdef _DEBUG
		if(!IsAttached())
			ASSERT(m_pParentFrame==NULL);
		else
		{
			ASSERT(m_pParentFrame!=NULL);
			ASSERT(m_pParentFrame->IsKindOf(RUNTIME_CLASS(CMDIFrameWnd)));
		}
#endif
		return m_pParentFrame; 
	}

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COXTabClientWnd)
	protected:
	virtual void CalcWindowRect(LPRECT lpClientRect,UINT nAdjustType=adjustBorder);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~COXTabClientWnd();
	// --- In  :
	// --- Out : 
	// --- Returns:
	// --- Effect : Destructs the object

	// Generated message map functions
protected:
	//{{AFX_MSG(COXTabClientWnd)
	afx_msg LONG OnMDIActivate(UINT wParam, LONG lParam);
	afx_msg LONG OnMDICreate(UINT wParam, LONG lParam);
	afx_msg LONG OnMDIDestroy(UINT wParam, LONG lParam);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	friend class COXTabWorkspace;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // _OXTABCLIENTWND_H__
