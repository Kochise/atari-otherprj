// diblook.h : main header file for the DIBLOOK application
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

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"           // main symbols

#define WM_DOREALIZE   (WM_USER + 0)

/////////////////////////////////////////////////////////////////////////////
// CDibLookApp:
// See diblook.cpp for the implementation of this class
//

class CDibLookApp : public CWinApp
{
public:
	CDibLookApp();
	int m_show_blockgrid;
	int m_show_cropmask;
	int m_endpoint_snap;
	int m_trim;
	int m_optimize_coding;
	int m_copyoption;
	int m_dither;
	int m_processing_mode;
	int m_copyfiletime;
	int m_zoom_reload;
	int m_progressive;
	int m_width;
	int m_height;
	int m_xoffset;
	int m_yoffset;
	int m_cropspec;
	int m_actual;
	char m_infname[256];

// Overrides
	virtual BOOL InitInstance();

// Implementation

	//{{AFX_MSG(CDibLookApp)
	afx_msg void OnAppAbout();
	afx_msg void OnFilePreferences();
	afx_msg void OnViewBlockGrid();
	afx_msg void OnUpdateViewBlockGrid(CCmdUI *pCmdUI);
	afx_msg void OnViewCropMask();
	afx_msg void OnUpdateViewCropMask(CCmdUI *pCmdUI);
	afx_msg void OnOptionsEndpointSnap();
	afx_msg void OnUpdateOptionsEndpointSnap(CCmdUI *pCmdUI);
	afx_msg void OnOptionsTrim();
	afx_msg void OnUpdateOptionsTrim(CCmdUI *pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
