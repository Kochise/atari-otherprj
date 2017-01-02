// dibdoc.h : interface of the CDibDoc class
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

#include "dibapi.h"

class CDibDoc : public CDocument
{
protected: // create from serialization only
	CDibDoc();
	DECLARE_DYNCREATE(CDibDoc)

// Attributes
public:
	HDIB GetHDIB() const
		{ return m_hDIB; }
	CPalette* GetDocPalette() const
		{ return m_palDIB; }
	CSize GetDocSize() const
		{ return m_sizeDoc; }

// Operations
public:
	void ReplaceHDIB(HDIB hDIB);
	void InitDIBData();
	void DoTransform();
	int m_image_width;
	int m_image_height;
	int m_MCUwidth;
	int m_MCUheight;
	int m_crop_xoffset;
	int m_crop_yoffset;
	int m_crop_width;
	int m_crop_height;
	int m_transform;
	int m_dithered;
	int m_scale;
	int m_scale_denom;
	int m_force_grayscale;
	int m_save_scale;

// Implementation
protected:
	virtual ~CDibDoc();

	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);

protected:
	HDIB m_hDIB;
	CPalette* m_palDIB;
	CSize m_sizeDoc;

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
protected:
	virtual BOOL    OnNewDocument();

// Generated message map functions
protected:
	//{{AFX_MSG(CDibDoc)
	afx_msg void OnFileSaveCopyAs();
	afx_msg void OnTransformRot90();
	afx_msg void OnUpdateTransformRot90(CCmdUI *pCmdUI);
	afx_msg void OnTransformRot270();
	afx_msg void OnUpdateTransformRot270(CCmdUI *pCmdUI);
	afx_msg void OnTransformFlipH();
	afx_msg void OnUpdateTransformFlipH(CCmdUI *pCmdUI);
	afx_msg void OnTransformFlipV();
	afx_msg void OnUpdateTransformFlipV(CCmdUI *pCmdUI);
	afx_msg void OnTransformRot180();
	afx_msg void OnUpdateTransformRot180(CCmdUI *pCmdUI);
	afx_msg void OnTransformTranspose();
	afx_msg void OnUpdateTransformTranspose(CCmdUI *pCmdUI);
	afx_msg void OnTransformTransverse();
	afx_msg void OnUpdateTransformTransverse(CCmdUI *pCmdUI);
	afx_msg void OnTransformOriginal();
	afx_msg void OnUpdateTransformOriginal(CCmdUI *pCmdUI);
	afx_msg void OnTransformWipe();
	afx_msg void OnUpdateTransformWipe(CCmdUI *pCmdUI);
	afx_msg void OnOptionsGrayscale();
	afx_msg void OnUpdateOptionsGrayscale(CCmdUI *pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
