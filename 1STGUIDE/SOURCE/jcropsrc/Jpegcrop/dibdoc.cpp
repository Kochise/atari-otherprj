// dibdoc.cpp : implementation of the CDibDoc class
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
#include <limits.h>

#include "dibdoc.h"

#include "jpegdib.h"
#include "docrop.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDibDoc

IMPLEMENT_DYNCREATE(CDibDoc, CDocument)

BEGIN_MESSAGE_MAP(CDibDoc, CDocument)
	//{{AFX_MSG_MAP(CDibDoc)
	ON_COMMAND(ID_FILE_SAVE_COPY_AS, OnFileSaveCopyAs)
	ON_COMMAND(ID_TRANSFORM_ROT_90, OnTransformRot90)
	ON_UPDATE_COMMAND_UI(ID_TRANSFORM_ROT_90, OnUpdateTransformRot90)
	ON_COMMAND(ID_TRANSFORM_ROT_270, OnTransformRot270)
	ON_UPDATE_COMMAND_UI(ID_TRANSFORM_ROT_270, OnUpdateTransformRot270)
	ON_COMMAND(ID_TRANSFORM_FLIP_H, OnTransformFlipH)
	ON_UPDATE_COMMAND_UI(ID_TRANSFORM_FLIP_H, OnUpdateTransformFlipH)
	ON_COMMAND(ID_TRANSFORM_FLIP_V, OnTransformFlipV)
	ON_UPDATE_COMMAND_UI(ID_TRANSFORM_FLIP_V, OnUpdateTransformFlipV)
	ON_COMMAND(ID_TRANSFORM_ROT_180, OnTransformRot180)
	ON_UPDATE_COMMAND_UI(ID_TRANSFORM_ROT_180, OnUpdateTransformRot180)
	ON_COMMAND(ID_TRANSFORM_TRANSPOSE, OnTransformTranspose)
	ON_UPDATE_COMMAND_UI(ID_TRANSFORM_TRANSPOSE, OnUpdateTransformTranspose)
	ON_COMMAND(ID_TRANSFORM_TRANSVERSE, OnTransformTransverse)
	ON_UPDATE_COMMAND_UI(ID_TRANSFORM_TRANSVERSE, OnUpdateTransformTransverse)
	ON_COMMAND(ID_TRANSFORM_ORIGINAL, OnTransformOriginal)
	ON_UPDATE_COMMAND_UI(ID_TRANSFORM_ORIGINAL, OnUpdateTransformOriginal)
	ON_COMMAND(ID_TRANSFORM_WIPE, OnTransformWipe)
	ON_UPDATE_COMMAND_UI(ID_TRANSFORM_WIPE, OnUpdateTransformWipe)
	ON_COMMAND(ID_OPTIONS_GRAYSCALE, OnOptionsGrayscale)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_GRAYSCALE, OnUpdateOptionsGrayscale)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDibDoc construction/destruction

CDibDoc::CDibDoc()
{
	m_hDIB = NULL;
	m_palDIB = NULL;
	m_sizeDoc = CSize(8,8);     // dummy value to make CScrollView happy
	m_image_width = 8;
	m_image_height = 8;
	m_MCUwidth = 8;
	m_MCUheight = 8;
	m_crop_xoffset = 0;
	m_crop_yoffset = 0;
	m_crop_width = 0;
	m_crop_height = 0;
	m_transform = 0;
	m_dithered = 0;
	m_scale = 0;
	m_scale_denom = 8;
	m_force_grayscale = 0;
	m_save_scale = 0;
}

CDibDoc::~CDibDoc()
{
	if (m_hDIB != NULL)
	{
		::GlobalFree((HGLOBAL) m_hDIB);
	}
	if (m_palDIB != NULL)
	{
		delete m_palDIB;
	}
}

BOOL CDibDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	return TRUE;
}

void CDibDoc::InitDIBData()
{
	if (m_palDIB != NULL)
	{
		delete m_palDIB;
		m_palDIB = NULL;
	}
	if (m_hDIB == NULL)
	{
		return;
	}
	// Set up document size
	LPSTR lpDIB = (LPSTR) ::GlobalLock((HGLOBAL) m_hDIB);
	if (::DIBWidth(lpDIB) > INT_MAX ||::DIBHeight(lpDIB) > INT_MAX)
	{
		::GlobalUnlock((HGLOBAL) m_hDIB);
		::GlobalFree((HGLOBAL) m_hDIB);
		m_hDIB = NULL;
		CString strMsg;
		strMsg.LoadString(IDS_DIB_TOO_BIG);
		MessageBox(NULL, strMsg, NULL, MB_ICONINFORMATION | MB_OK);
		return;
	}
	m_sizeDoc = CSize((int) ::DIBWidth(lpDIB), (int) ::DIBHeight(lpDIB));
	::GlobalUnlock((HGLOBAL) m_hDIB);
	// Create copy of palette
	m_palDIB = new CPalette;
	if (m_palDIB == NULL)
	{
		// we must be really low on memory
		::GlobalFree((HGLOBAL) m_hDIB);
		m_hDIB = NULL;
		return;
	}
	if (::CreateDIBPalette(m_hDIB, m_palDIB) == NULL)
	{
		// DIB may not have a palette
		delete m_palDIB;
		m_palDIB = NULL;
		return;
	}
}


BOOL CDibDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	DeleteContents();
	BeginWaitCursor();

	// replace calls to Serialize with ReadDIBFile function
	m_hDIB = (HDIB) Read_JPEG_File(lpszPathName, 0, 0,
		&m_image_width, &m_image_height,
		&m_MCUwidth, &m_MCUheight,
		&m_scale_denom);

	m_scale = m_scale_denom;

	InitDIBData();
	EndWaitCursor();

	if (m_hDIB == NULL)
	{
		// may not be DIB format
		CString strMsg;
		strMsg.LoadString(IDS_CANNOT_LOAD_DIB);
		MessageBox(NULL, strMsg, NULL, MB_ICONINFORMATION | MB_OK);
		return FALSE;
	}
	SetPathName(lpszPathName);
	SetModifiedFlag(FALSE);     // start off with unmodified
	return TRUE;
}


BOOL CDibDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
//	BeginWaitCursor();

	// replace calls to Serialize with SaveDIB function
	BOOL bSuccess = docrop(GetPathName(), lpszPathName,
						   m_crop_xoffset, m_crop_yoffset,
						   m_crop_width, m_crop_height,
						   m_transform, m_force_grayscale, m_save_scale,
						   ((CDibLookApp *)AfxGetApp())->m_optimize_coding,
						   ((CDibLookApp *)AfxGetApp())->m_copyoption,
						   ((CDibLookApp *)AfxGetApp())->m_processing_mode,
						   ((CDibLookApp *)AfxGetApp())->m_copyfiletime,
						   ((CDibLookApp *)AfxGetApp())->m_progressive);

//	EndWaitCursor();
	SetModifiedFlag(FALSE);     // back to unmodified

	if (!bSuccess)
	{
		// may be other-style DIB (load supported but not save)
		//  or other problem in SaveDIB
		CString strMsg;
		strMsg.LoadString(IDS_CANNOT_SAVE_DIB);
		MessageBox(NULL, strMsg, NULL, MB_ICONINFORMATION | MB_OK);
	}

	return /* bSuccess */ TRUE; // No remove in DoSave!
}

void CDibDoc::ReplaceHDIB(HDIB hDIB)
{
	if (m_hDIB != NULL)
	{
		::GlobalFree((HGLOBAL) m_hDIB);
	}
	m_hDIB = hDIB;
}

/////////////////////////////////////////////////////////////////////////////
// CDibDoc diagnostics

#ifdef _DEBUG
void CDibDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CDibDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDibDoc commands

void CDibDoc::OnFileSaveCopyAs()
{
	DoSave(NULL, FALSE);
}

void CDibDoc::OnUpdateTransformRot90(CCmdUI *pCmdUI)
{
  pCmdUI->SetCheck(m_transform == 5);
}

void CDibDoc::OnUpdateTransformRot270(CCmdUI *pCmdUI)
{
  pCmdUI->SetCheck(m_transform == 7);
}

void CDibDoc::OnUpdateTransformFlipH(CCmdUI *pCmdUI)
{
  pCmdUI->SetCheck(m_transform == 1);
}

void CDibDoc::OnUpdateTransformFlipV(CCmdUI *pCmdUI)
{
  pCmdUI->SetCheck(m_transform == 2);
}

void CDibDoc::OnUpdateTransformRot180(CCmdUI *pCmdUI)
{
  pCmdUI->SetCheck(m_transform == 6);
}

void CDibDoc::OnUpdateTransformTranspose(CCmdUI *pCmdUI)
{
  pCmdUI->SetCheck(m_transform == 3);
}

void CDibDoc::OnUpdateTransformTransverse(CCmdUI *pCmdUI)
{
  pCmdUI->SetCheck(m_transform == 4);
}

void CDibDoc::OnUpdateTransformOriginal(CCmdUI *pCmdUI)
{
  pCmdUI->SetCheck(m_transform == 0);
}

void CDibDoc::OnUpdateTransformWipe(CCmdUI *pCmdUI)
{
  pCmdUI->SetCheck(m_transform == 8);
}

void CDibDoc::OnOptionsGrayscale()
{
	m_force_grayscale ^= 1;

	BeginWaitCursor();
	ReplaceHDIB(NULL);

	HDIB hDIB = (HDIB) Read_JPEG_File(GetPathName(),
		m_scale, m_force_grayscale,
		&m_image_width, &m_image_height,
		&m_MCUwidth, &m_MCUheight,
		&m_scale_denom);

	ReplaceHDIB(hDIB);
	DoTransform();
	InitDIBData();
	EndWaitCursor();
	m_dithered = 0;
	UpdateAllViews(NULL);
}

void CDibDoc::OnUpdateOptionsGrayscale(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_force_grayscale != 0);
}
