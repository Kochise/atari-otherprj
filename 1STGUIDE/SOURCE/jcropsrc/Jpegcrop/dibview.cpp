// dibview.cpp : implementation of the CDibView class
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
#include "dibapi.h"
#include "mainfrm.h"

#include "jpegdib.h"

#include "jpeglib.h"
extern "C" {
#include "transupp.h"
}

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDibView

IMPLEMENT_DYNCREATE(CDibView, CScrollView)

BEGIN_MESSAGE_MAP(CDibView, CScrollView)
	//{{AFX_MSG_MAP(CDibView)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
	ON_COMMAND(ID_INDICATOR_POS, OnIndicatorPos)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_POS, OnUpdateIndicatorPos)
	ON_MESSAGE(WM_DOREALIZE, OnDoRealize)
	ON_COMMAND(ID_POPUP1_MOVEFRAME, OnPopup1MoveFrame)
	ON_COMMAND(ID_POPUP1_DEFINEFRAME, OnPopup1DefineFrame)
	ON_COMMAND(ID_POPUP1_SAVEFRAMEAS, OnPopup1SaveFrameAs)
	ON_COMMAND(ID_VIEW_ZOOM1, OnViewZoom1)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOM1, OnUpdateViewZoom1)
	ON_COMMAND(ID_VIEW_ZOOM2, OnViewZoom2)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOM2, OnUpdateViewZoom2)
	ON_COMMAND(ID_VIEW_ZOOM3, OnViewZoom3)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOM3, OnUpdateViewZoom3)
	ON_COMMAND(ID_VIEW_ZOOM4, OnViewZoom4)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOM4, OnUpdateViewZoom4)
	ON_COMMAND(ID_VIEW_ZOOM5, OnViewZoom5)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOM5, OnUpdateViewZoom5)
	ON_COMMAND(ID_VIEW_ZOOM6, OnViewZoom6)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOM6, OnUpdateViewZoom6)
	ON_COMMAND(ID_VIEW_ZOOM7, OnViewZoom7)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOM7, OnUpdateViewZoom7)
	ON_COMMAND(ID_VIEW_ZOOM8, OnViewZoom8)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOM8, OnUpdateViewZoom8)
	ON_COMMAND(ID_VIEW_ZOOM9, OnViewZoom9)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOM9, OnUpdateViewZoom9)
	ON_COMMAND(ID_VIEW_ZOOM10, OnViewZoom10)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOM10, OnUpdateViewZoom10)
	ON_COMMAND(ID_VIEW_ZOOM11, OnViewZoom11)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOM11, OnUpdateViewZoom11)
	ON_COMMAND(ID_VIEW_ZOOM12, OnViewZoom12)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOM12, OnUpdateViewZoom12)
	ON_COMMAND(ID_VIEW_ZOOM13, OnViewZoom13)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOM13, OnUpdateViewZoom13)
	ON_COMMAND(ID_VIEW_ZOOM14, OnViewZoom14)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOM14, OnUpdateViewZoom14)
	ON_COMMAND(ID_VIEW_ZOOM15, OnViewZoom15)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOM15, OnUpdateViewZoom15)
	ON_COMMAND(ID_VIEW_ZOOM16, OnViewZoom16)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOM16, OnUpdateViewZoom16)
	ON_UPDATE_COMMAND_UI(ID_TOOLBAR_SCALE, OnUpdateToolbarScale)
	ON_UPDATE_COMMAND_UI(ID_TOOLBAR_ZOOM, OnUpdateToolbarZoom)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_SETCURSOR()
	ON_WM_MOUSEMOVE()
	ON_WM_KEYDOWN()
	ON_COMMAND(ID_OPTIONS_SAVEZOOM, OnOptionsSaveZoom)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_SAVEZOOM, OnUpdateOptionsSaveZoom)
	//}}AFX_MSG_MAP

	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CScrollView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDibView construction/destruction

CDibView::CDibView()
{
	m_zoomedsize = CSize(8,8);
	m_left = 0;
	m_top = 0;
	m_right = 0;
	m_bottom = 0;
	m_left_zoomed = 0;
	m_top_zoomed = 0;
	m_right_zoomed = 0;
	m_bottom_zoomed = 0;
	m_flag = 0;
	m_x = 0;
	m_y = 0;
	m_zoomval = 8;
	m_setcursor = 0;
	m_save_zoom = 0;
}

CDibView::~CDibView()
{
}

/* The following array holds the exact color representations
 * reported by the system.
 * This is useful for less than 24 bit deep displays as a base
 * for additional dithering to get smoother output.
 */

static unsigned char screen_set[3][256];

/* The following code is based in part on:
 *
 * jquant1.c
 *
 * Copyright (C) 1991-1996, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains 1-pass color quantization (color mapping) routines.
 * These routines provide mapping to a fixed color map using equally spaced
 * color values.  Optional Floyd-Steinberg or ordered dithering is available.
 */

/* Declarations for Floyd-Steinberg dithering.
 *
 * Errors are accumulated into the array fserrors[], at a resolution of
 * 1/16th of a pixel count.  The error at a given pixel is propagated
 * to its not-yet-processed neighbors using the standard F-S fractions,
 *		...	(here)	7/16
 *		3/16	5/16	1/16
 * We work left-to-right on even rows, right-to-left on odd rows.
 *
 * We can get away with a single array (holding one row's worth of errors)
 * by using it to store the current row's errors at pixel columns not yet
 * processed, but the next row's errors at columns already processed.  We
 * need only a few extra variables to hold the errors immediately around the
 * current column.  (If we are lucky, those variables are in registers, but
 * even if not, they're probably cheaper to access than array elements are.)
 *
 * We provide (#columns + 2) entries per component; the extra entry at each
 * end saves us from special-casing the first and last pixels.
 */

typedef short FSERROR;		/* 16 bits should be enough */
typedef int LOCFSERROR;		/* use 'int' for calculation temps */

typedef struct
{
  unsigned char *colorset;
  FSERROR *fserrors;
}
FSBUF;

/* Floyd-Steinberg initialization function.
 *
 * It is called 'fs2_init' since it's specialized for our purpose and
 * could be embedded in a more general FS-package.
 *
 * Returns a malloced FSBUF pointer which has to be passed as first
 * parameter to subsequent 'fs2_dither' calls.
 * The FSBUF structure does not need to be referenced by the calling
 * application, it can be treated from the app like a void pointer.
 *
 * The current implementation does only require to free() this returned
 * pointer after processing.
 *
 * Returns NULL if malloc fails.
 *
 * NOTE: The FSBUF structure is designed to allow the 'fs2_dither'
 * function to work with an *arbitrary* number of color components
 * at runtime! This is an enhancement over the IJG code base :-).
 * Only fs2_init() specifies the (maximum) number of components.
 */

FSBUF *fs2_init(int width, int nc)
{
  FSBUF *fs;
  FSERROR *p;
  int i;

  fs = (FSBUF *)
    malloc(sizeof(FSBUF) * nc + ((size_t)width + 2) * sizeof(FSERROR) * nc);
  if (fs == 0) return fs;

  for (i = 0; i < nc; i++) fs[i].colorset = screen_set[i];

  p = (FSERROR *)(fs + nc);
  memset(p, 0, ((size_t)width + 2) * sizeof(FSERROR) * nc);

  for (i = 0; i < nc; i++) fs[i].fserrors = p++;

  return fs;
}

/* Floyd-Steinberg dithering function.
 *
 * NOTE:
 * (1) The image data referenced by 'ptr' is *overwritten* (input *and*
 *     output) to allow more efficient implementation.
 * (2) Alternate FS dithering is provided by the sign of 'nc'. Pass in
 *     a negative value for right-to-left processing. The return value
 *     provides the right-signed value for subsequent calls!
 */

int fs2_dither(FSBUF *fs, unsigned char *ptr, int nc, int num_rows, int num_cols, int num_scan)
{
  int abs_nc, ci, row, col;
  LOCFSERROR delta, cur, belowerr, bpreverr;
  unsigned char *dataptr, *colsetptr;
  FSERROR *errorptr;

  if ((abs_nc = nc) < 0) abs_nc = -abs_nc;
  num_scan -= abs_nc;
  for (row = 0; row < num_rows; row++) {
    for (ci = 0; ci < abs_nc; ci++, ptr++) {
      dataptr = ptr;
      colsetptr = fs[ci].colorset;
      errorptr = fs[ci].fserrors;
      if (nc < 0) {
		dataptr += (num_cols - 1) * abs_nc;
		errorptr += (num_cols + 1) * abs_nc;
      }
      cur = belowerr = bpreverr = 0;
      for (col = 0; col < num_cols; col++) {
		cur += errorptr[nc];
		cur += 8; cur >>= 4;
		if ((cur += *dataptr) < 0) cur = 0;
		else if (cur > 255) cur = 255;
		*dataptr = colsetptr[cur];
		cur -= colsetptr[cur];
		delta = cur << 1; cur += delta;
		bpreverr += cur; cur += delta;
		belowerr += cur; cur += delta;
		errorptr[0] = (FSERROR)bpreverr;
		bpreverr = belowerr;
		belowerr = delta >> 1;
		dataptr += nc;
		errorptr += nc;
      }
      errorptr[0] = (FSERROR)bpreverr;
    }
    ptr += num_scan;
    nc = -nc;
  }
  return nc;
}

/////////////////////////////////////////////////////////////////////////////
// CDibView drawing

void CDibView::OnDraw(CDC* pDC)
{
	static int init_flag = 0;

	CDibLookApp *myapp = (CDibLookApp *) AfxGetApp();
	CDibDoc *pDoc = GetDocument();

	CRect rcDisp;
	pDC->GetClipBox(rcDisp);

	if (pDoc->GetHDIB() == NULL)
	{
		pDC->FillSolidRect(rcDisp, 0x00808080L);
		return;
	}

	if (pDoc->m_dithered == 0)
		if (myapp->m_dither == 0 || pDC->GetDeviceCaps(BITSPIXEL) != 16)
			pDoc->m_dithered = 1;
		else
		{
			if (init_flag == 0)
			{
				init_flag = 1;
				for (int i = 0; i < 256; i++)
				{
					screen_set[2][i] = (unsigned char)
						(pDC->SetPixel(rcDisp.left, rcDisp.top, (COLORREF)i) & 255);
					screen_set[1][i] = (unsigned char)
						((pDC->SetPixel(rcDisp.left, rcDisp.top, (COLORREF)(i << 8)) >> 8) & 255);
					screen_set[0][i] = (unsigned char)
						((pDC->SetPixel(rcDisp.left, rcDisp.top, (COLORREF)(i << 16)) >> 16) & 255);
				}
			}
			HDIB hDib = pDoc->GetHDIB();
			if (hDib)
			{
				LPBITMAPINFOHEADER lpDIB;
				int DIBLineWidth;
				int DIBScanWidth;
				unsigned char *lpBits;

				lpDIB = (LPBITMAPINFOHEADER)GlobalLock(hDib);

				DIBLineWidth = lpDIB->biWidth * lpDIB->biBitCount / 8;
				DIBScanWidth = (DIBLineWidth + 3) & (-4);

				lpBits = (unsigned char *) FindDIBBits((LPSTR) lpDIB);

				FSBUF *fs = fs2_init(lpDIB->biWidth, lpDIB->biBitCount / 8);
				if (fs) {
					fs2_dither(fs, lpBits, lpDIB->biBitCount / 8,
							   lpDIB->biHeight, lpDIB->biWidth,
							   DIBScanWidth);
					free(fs);
					pDoc->m_dithered = 1;
				}

				GlobalUnlock(hDib);
			}
		}

	CRect rcDIB;
	if (myapp->m_show_cropmask && pDoc->m_transform != 8)
	{
		if (pDoc->m_scale == pDoc->m_scale_denom)
		{
			rcDIB.left = m_left;
			rcDIB.top = m_top;
			rcDIB.right = pDoc->m_image_width - m_right;
			rcDIB.bottom = pDoc->m_image_height - m_bottom;
		}
		else
		{
			rcDIB.left = m_left_zoomed;
			rcDIB.top = m_top_zoomed;
			rcDIB.right = m_zoomedsize.cx - m_right_zoomed;
			rcDIB.bottom = m_zoomedsize.cy - m_bottom_zoomed;
		}
	}
	else
	{
		rcDIB.left = 0;
		rcDIB.top = 0;
		rcDIB.right = pDoc->GetDocSize().cx;
		rcDIB.bottom = pDoc->GetDocSize().cy;
	}

	CRect rcDest;
	if (pDC->IsPrinting())   // printer DC
	{
		// get size of printer page (in pixels)
		int cxPage = pDC->GetDeviceCaps(HORZRES);
		int cyPage = pDC->GetDeviceCaps(VERTRES);
		// get printer pixels per inch
		int cxInch = pDC->GetDeviceCaps(LOGPIXELSX);
		int cyInch = pDC->GetDeviceCaps(LOGPIXELSY);

		int scale = pDoc->m_scale;
		if (scale == pDoc->m_scale_denom) scale = m_zoomval;

		//
		// Best Fit case -- create a rectangle which preserves
		// the DIB's aspect ratio, and fills the page horizontally.
		//
		// The formula in the "->bottom" field below calculates the Y
		// position of the printed bitmap, based on the size of the
		// bitmap, the width of the page, and the relative size of
		// a printed pixel (cyInch / cxInch).
		//
		rcDest.left = 0;
		rcDest.top = 0;
		if (scale < pDoc->m_scale_denom)
		{
			rcDest.right = (cxPage * scale
				+ pDoc->m_scale_denom - 1)
				/ pDoc->m_scale_denom;
			rcDest.bottom = ((int)(((double)(rcDIB.bottom - rcDIB.top) * cxPage *
				cyInch) / ((double)(rcDIB.right - rcDIB.left) * cxInch)) * scale
				+ pDoc->m_scale_denom - 1)
				/ pDoc->m_scale_denom;
		}
		else
		{
			rcDest.right = cxPage;
			rcDest.bottom = (int)(((double)(rcDIB.bottom - rcDIB.top) * cxPage *
				cyInch) / ((double)(rcDIB.right - rcDIB.left) * cxInch));
		}
		::PaintDIB(pDC->m_hDC, &rcDest, pDoc->GetHDIB(),
			&rcDIB, pDoc->GetDocPalette());
		return;
	}

	if (myapp->m_show_cropmask && pDoc->m_transform != 8)
	{
		rcDest.left = m_left_zoomed;
		rcDest.top = m_top_zoomed;
		rcDest.right = m_zoomedsize.cx - m_right_zoomed;
		rcDest.bottom = m_zoomedsize.cy - m_bottom_zoomed;
	}
	else
	{
		rcDest.left = 0;
		rcDest.top = 0;
		rcDest.right = m_zoomedsize.cx;
		rcDest.bottom = m_zoomedsize.cy;
	}

	if (rcDisp.top < rcDest.top)
	{
		int temp = rcDisp.bottom;
		if (temp <= rcDest.top)
		{
			pDC->FillSolidRect(rcDisp, 0x00808080L);
			if (myapp->m_show_cropmask)
				goto after_grid;
			goto after_frame;
		}
		rcDisp.bottom = rcDest.top;
		pDC->FillSolidRect(rcDisp, 0x00808080L);
		rcDisp.bottom = temp;
		rcDisp.top = rcDest.top;
	}
	if (rcDisp.bottom > rcDest.bottom)
	{
		int temp = rcDisp.top;
		if (temp >= rcDest.bottom)
		{
			pDC->FillSolidRect(rcDisp, 0x00808080L);
			if (myapp->m_show_cropmask)
				goto after_grid;
			goto after_frame;
		}
		rcDisp.top = rcDest.bottom;
		pDC->FillSolidRect(rcDisp, 0x00808080L);
		rcDisp.top = temp;
		rcDisp.bottom = rcDest.bottom;
	}
	if (rcDisp.left < rcDest.left)
	{
		int temp = rcDisp.right;
		if (temp <= rcDest.left)
		{
			pDC->FillSolidRect(rcDisp, 0x00808080L);
			if (myapp->m_show_cropmask)
				goto after_grid;
			goto after_frame;
		}
		rcDisp.right = rcDest.left;
		pDC->FillSolidRect(rcDisp, 0x00808080L);
		rcDisp.right = temp;
		rcDisp.left = rcDest.left;
	}
	if (rcDisp.right > rcDest.right)
	{
		int temp = rcDisp.left;
		if (temp >= rcDest.right)
		{
			pDC->FillSolidRect(rcDisp, 0x00808080L);
			if (myapp->m_show_cropmask)
				goto after_grid;
			goto after_frame;
		}
		rcDisp.left = rcDest.right;
		pDC->FillSolidRect(rcDisp, 0x00808080L);
		rcDisp.left = temp;
		rcDisp.right = rcDest.right;
	}

	::PaintDIB(pDC->m_hDC, &rcDest, pDoc->GetHDIB(),
		&rcDIB, pDoc->GetDocPalette());

	if (myapp->m_show_blockgrid)
	{
		int scale = pDoc->m_scale;
		if (scale == pDoc->m_scale_denom) scale = m_zoomval;
#if 0
		LOGBRUSH mybrush;
		mybrush.lbStyle = BS_SOLID;
		mybrush.lbColor = (COLORREF)0x00000000L;
		DWORD mystyle[2];
		mystyle[0] = 1;
		mystyle[1] = 1;
		CPen mypen(PS_USERSTYLE | PS_COSMETIC, 1, &mybrush, 2, mystyle);
#else
		// CPen mypen(PS_ALTERNATE | PS_COSMETIC, 1, (COLORREF)0x00000000L);
		CPen mypen(0, 0, (COLORREF)0x00555555L);
#endif
		CPen *pold = pDC->SelectObject(&mypen);
		// pDC->SetBkMode(TRANSPARENT);

		int incr = (pDoc->m_MCUwidth * scale) / pDoc->m_scale_denom;
		if (incr >= 8)
		{
			int i = rcDest.left;
			do i += incr; while (i < rcDisp.left);
			for (; i < rcDisp.right; i += incr)
			{
				pDC->MoveTo(i, rcDisp.top);
				pDC->LineTo(i, rcDisp.bottom);
			}
		}
		incr = (pDoc->m_MCUheight * scale) / pDoc->m_scale_denom;
		if (incr >= 8)
		{
			int i = rcDest.top;
			do i += incr; while (i < rcDisp.top);
			for (; i < rcDisp.bottom; i += incr)
			{
				pDC->MoveTo(rcDisp.left, i);
				pDC->LineTo(rcDisp.right, i);
			}
		}

		pDC->SelectObject(pold);
	}

	after_grid:

	if (myapp->m_show_cropmask && pDoc->m_transform == 8)
	{
		rcDisp.left = m_left_zoomed;
		rcDisp.top = m_top_zoomed;
		rcDisp.right = m_zoomedsize.cx - m_right_zoomed;
		rcDisp.bottom = m_zoomedsize.cy - m_bottom_zoomed;

		pDC->FillSolidRect(rcDisp, 0x00808080L);

		if (m_right_zoomed == 0 || m_bottom_zoomed == 0)
		{
			CPen mypen(PS_DOT, 0, (COLORREF)0x00000000L);
			CPen *pold = pDC->SelectObject(&mypen);
			pDC->SetBkColor((COLORREF)0x00FFFFFFL);
			int x = m_zoomedsize.cx;
			int y = m_zoomedsize.cy;
			pDC->MoveTo(0, y);
			pDC->LineTo(x, y);
			pDC->LineTo(x, 0);
			pDC->SelectObject(pold);
		}
	}
	else if (m_left_zoomed || m_top_zoomed || m_right_zoomed || m_bottom_zoomed)
	{
		CPen mypen(PS_DOT, 0, (COLORREF)0x00000000L);
		CPen *pold = pDC->SelectObject(&mypen);
		pDC->SetBkColor((COLORREF)0x00FFFFFFL);
		if (myapp->m_show_cropmask)
		{
			int x = m_zoomedsize.cx;
			int y = m_zoomedsize.cy;
			pDC->MoveTo(0, y);
			pDC->LineTo(x, y);
			pDC->LineTo(x, 0);
		}
		else
		{
			int x1 = m_left_zoomed;
			int y1 = m_top_zoomed;
			int x2 = m_zoomedsize.cx - m_right_zoomed - 1;
			int y2 = m_zoomedsize.cy - m_bottom_zoomed - 1;
			pDC->MoveTo(x1, y1);
			pDC->LineTo(x2, y1);
			pDC->LineTo(x2, y2);
			pDC->LineTo(x1, y2);
			pDC->LineTo(x1, y1);
		}
		pDC->SelectObject(pold);
	}

	after_frame:

	if (m_save_zoom)
	{
		pDoc->m_crop_xoffset = m_left_zoomed;
		pDoc->m_crop_yoffset = m_top_zoomed;
		pDoc->m_crop_width = m_zoomedsize.cx - m_right_zoomed - m_left_zoomed;
		pDoc->m_crop_height = m_zoomedsize.cy - m_bottom_zoomed - m_top_zoomed;
		pDoc->m_save_scale =
			pDoc->m_scale == pDoc->m_scale_denom ? m_zoomval : pDoc->m_scale;
	}
	else
	{
		pDoc->m_crop_xoffset = m_left;
		pDoc->m_crop_yoffset = m_top;
		pDoc->m_crop_width = pDoc->m_image_width - m_right - m_left;
		pDoc->m_crop_height = pDoc->m_image_height - m_bottom - m_top;
		pDoc->m_save_scale = 0;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDibView printing

BOOL CDibView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

/////////////////////////////////////////////////////////////////////////////
// CDibView commands


LRESULT CDibView::OnDoRealize(WPARAM wParam, LPARAM)
{
	ASSERT(wParam != NULL);
	CDibDoc* pDoc = GetDocument();
	if (pDoc->GetHDIB() == NULL)
		return 0L;  // must be a new document

	CPalette* pPal = pDoc->GetDocPalette();
	if (pPal != NULL)
	{
		CMainFrame* pAppFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
		ASSERT_KINDOF(CMainFrame, pAppFrame);

		CClientDC appDC(pAppFrame);
		// All views but one should be a background palette.
		// wParam contains a handle to the active view, so the SelectPalette
		// bForceBackground flag is FALSE only if wParam == m_hWnd (this view)
		CPalette* oldPalette = appDC.SelectPalette(pPal, ((HWND)wParam) != m_hWnd);

		if (oldPalette != NULL)
		{
			UINT nColorsChanged = appDC.RealizePalette();
			if (nColorsChanged > 0)
				pDoc->UpdateAllViews(NULL);
			appDC.SelectPalette(oldPalette, TRUE);
		}
		else
		{
			TRACE0("\tSelectPalette failed in CDibView::OnPaletteChanged\n");
		}
	}
	SetClassLong(m_hWnd, GCL_HBRBACKGROUND, NULL);

	return 0L;
}

void CDibView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();
	if (GetDocument() == NULL) return;

	m_zoomval = GetDocument()->m_scale_denom;
	m_zoomedsize = GetDocument()->GetDocSize();
	SetScrollSizes(MM_TEXT,	m_zoomedsize);
	ResizeParentToFit();

	m_left = 0;
	m_top = 0;
	m_left_zoomed = 0;
	m_top_zoomed = 0;
	m_right = 0;
	m_bottom = 0;
	if (((CDibLookApp *)AfxGetApp())->m_trim)
	{
		switch (GetDocument()->m_transform)
		{
		case 1:
		case 5:
			if (GetDocument()->m_image_width > GetDocument()->m_MCUwidth)
				m_right = GetDocument()->m_image_width % GetDocument()->m_MCUwidth;
			break;
		case 2:
		case 7:
			if (GetDocument()->m_image_height > GetDocument()->m_MCUheight)
				m_bottom = GetDocument()->m_image_height % GetDocument()->m_MCUheight;
			break;
		case 4:
		case 6:
			if (GetDocument()->m_image_width > GetDocument()->m_MCUwidth)
				m_right = GetDocument()->m_image_width % GetDocument()->m_MCUwidth;
			if (GetDocument()->m_image_height > GetDocument()->m_MCUheight)
				m_bottom = GetDocument()->m_image_height % GetDocument()->m_MCUheight;
			break;
		}
	}
	if (GetDocument()->m_scale == m_zoomval)
	{
		m_right_zoomed = m_right;
		m_bottom_zoomed = m_bottom;
	}
	else
	{
		m_right_zoomed = m_zoomedsize.cx -
			((GetDocument()->m_image_width - m_right)
			* GetDocument()->m_scale
			+ m_zoomval - 1)
			/ m_zoomval;
		m_bottom_zoomed = m_zoomedsize.cy -
			((GetDocument()->m_image_height - m_bottom)
			* GetDocument()->m_scale
			+ m_zoomval - 1)
			/ m_zoomval;
	}
	SetClassLong(m_hWnd, GCL_HBRBACKGROUND, NULL);
}

void CDibView::OnUpdate(CView* pSender,
						LPARAM lHint,
						CObject* pHint)
{
	if ((int)lHint >= 4 && (int)lHint <= 19)
	{
		m_zoomval = (int)lHint - 3;
	}
	CScrollView::OnUpdate(pSender, lHint, pHint);
	if (lHint == 0 || GetDocument() == NULL) return;
	if (lHint == 2 || ((int)lHint >= 4 && (int)lHint <= 19))
	{
		if (m_zoomval == GetDocument()->m_scale_denom)
		{
			m_zoomedsize = GetDocument()->GetDocSize();
		}
		else
		{
			int xsize =
				(GetDocument()->GetDocSize().cx * m_zoomval
				+ GetDocument()->m_scale_denom - 1)
				/ GetDocument()->m_scale_denom;
			int ysize =
				(GetDocument()->GetDocSize().cy * m_zoomval
				+ GetDocument()->m_scale_denom - 1)
				/ GetDocument()->m_scale_denom;
			m_zoomedsize = CSize(xsize, ysize);
		}
		SetScrollSizes(MM_TEXT,	m_zoomedsize);
		if ((int)lHint >= 4 && (int)lHint <= 19)
		{
			int scale = GetDocument()->m_scale;
			int scale_denom = GetDocument()->m_scale_denom;
			if (scale == scale_denom) scale = m_zoomval;
			if (scale == scale_denom)
			{
				m_left_zoomed = m_left;
				m_top_zoomed = m_top;
				m_right_zoomed = m_right;
				m_bottom_zoomed = m_bottom;
			}
			else
			{
				m_left_zoomed = (m_left * scale
					+ scale_denom - 1)
					/ scale_denom;
				m_top_zoomed = (m_top * scale
					+ scale_denom - 1)
					/ scale_denom;
				m_right_zoomed = m_zoomedsize.cx -
					((GetDocument()->m_image_width - m_right) * scale
					+ scale_denom - 1)
					/ scale_denom;
				m_bottom_zoomed = m_zoomedsize.cy -
					((GetDocument()->m_image_height - m_bottom) * scale
					+ scale_denom - 1)
					/ scale_denom;
			}

			if (m_save_zoom)
			{
				char buf[64];
				unsigned int paru[2];
				int pari[2];
				sprintf(buf, " %dx%d+%d+%d",
					m_zoomedsize.cx - m_right_zoomed - m_left_zoomed,
					m_zoomedsize.cy - m_bottom_zoomed - m_top_zoomed,
					m_left_zoomed, m_top_zoomed);
				((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.GetPaneInfo(1, paru[0], paru[1], pari[0]);
				((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.SetPaneInfo(1, ID_INDICATOR_POS, SBPS_NORMAL, pari[0]);
				((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.SetPaneText(1, buf, TRUE);
			}
			return;
		}
	}
	m_left = 0;
	m_top = 0;
	m_left_zoomed = 0;
	m_top_zoomed = 0;
	m_right = 0;
	m_bottom = 0;
	if (((CDibLookApp *)AfxGetApp())->m_trim)
	{
		switch (GetDocument()->m_transform)
		{
		case 1:
		case 5:
			if (GetDocument()->m_image_width > GetDocument()->m_MCUwidth)
				m_right = GetDocument()->m_image_width % GetDocument()->m_MCUwidth;
			break;
		case 2:
		case 7:
			if (GetDocument()->m_image_height > GetDocument()->m_MCUheight)
				m_bottom = GetDocument()->m_image_height % GetDocument()->m_MCUheight;
			break;
		case 4:
		case 6:
			if (GetDocument()->m_image_width > GetDocument()->m_MCUwidth)
				m_right = GetDocument()->m_image_width % GetDocument()->m_MCUwidth;
			if (GetDocument()->m_image_height > GetDocument()->m_MCUheight)
				m_bottom = GetDocument()->m_image_height % GetDocument()->m_MCUheight;
			break;
		}
	}
	int scale = GetDocument()->m_scale;
	int scale_denom = GetDocument()->m_scale_denom;
	if (scale == scale_denom) scale = m_zoomval;
	if (scale == scale_denom)
	{
		m_right_zoomed = m_right;
		m_bottom_zoomed = m_bottom;
	}
	else
	{
		m_right_zoomed = m_zoomedsize.cx -
			((GetDocument()->m_image_width - m_right) * scale
			+ scale_denom - 1)
			/ scale_denom;
		m_bottom_zoomed = m_zoomedsize.cy -
			((GetDocument()->m_image_height - m_bottom) * scale
			+ scale_denom - 1)
			/ scale_denom;
	}
	if (lHint == 3) return;
	char buf[64];
	unsigned int paru[2];
	int pari[2];
	if (m_save_zoom)
	{
		sprintf(buf, " %dx%d+%d+%d",
			m_zoomedsize.cx - m_right_zoomed,
			m_zoomedsize.cy - m_bottom_zoomed, 0, 0);
	}
	else
	{
		sprintf(buf, " %dx%d+%d+%d",
			GetDocument()->m_image_width - m_right,
			GetDocument()->m_image_height - m_bottom, 0, 0);
	}
	((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.GetPaneInfo(1, paru[0], paru[1], pari[0]);
	((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.SetPaneInfo(1, ID_INDICATOR_POS, SBPS_NORMAL, pari[0]);
	((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.SetPaneText(1, buf, TRUE);
	SetClassLong(m_hWnd, GCL_HBRBACKGROUND, NULL);
}

void CDibView::OnActivateView(BOOL bActivate, CView* pActivateView,
					CView* pDeactiveView)
{
	CScrollView::OnActivateView(bActivate, pActivateView, pDeactiveView);

	if (bActivate)
	{
		ASSERT(pActivateView == this);
		OnDoRealize((WPARAM)m_hWnd, 0);   // same as SendMessage(WM_DOREALIZE);

		char buf[64];
		unsigned int paru[2];
		int pari[2];
		if (m_save_zoom)
		{
			sprintf(buf, " %dx%d+%d+%d",
				m_zoomedsize.cx - m_right_zoomed - m_left_zoomed,
				m_zoomedsize.cy - m_bottom_zoomed - m_top_zoomed,
				m_left_zoomed, m_top_zoomed);
		}
		else
		{
			sprintf(buf, " %dx%d+%d+%d",
				GetDocument()->m_image_width - m_right - m_left,
				GetDocument()->m_image_height - m_bottom - m_top,
				m_left, m_top);
		}
		((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.GetPaneInfo(1, paru[0], paru[1], pari[0]);
		((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.SetPaneInfo(1, ID_INDICATOR_POS, SBPS_NORMAL, pari[0]);
		((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.SetPaneText(1, buf, TRUE);
	}
	SetClassLong(m_hWnd, GCL_HBRBACKGROUND, NULL);
}

void CDibView::OnEditCopy()
{
	CDibDoc* pDoc = GetDocument();
	// Clean clipboard of contents, and copy the DIB.

	if (OpenClipboard())
	{
		BeginWaitCursor();
		EmptyClipboard();
		SetClipboardData (CF_DIB, CopyHandle((HANDLE) pDoc->GetHDIB()) );
		CloseClipboard();
		EndWaitCursor();
	}
}



void CDibView::OnUpdateEditCopy(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetDocument()->GetHDIB() != NULL);
}


void CDibView::OnEditPaste()
{
	HDIB hNewDIB = NULL;

	if (OpenClipboard())
	{
		BeginWaitCursor();

		hNewDIB = (HDIB) CopyHandle(::GetClipboardData(CF_DIB));

		CloseClipboard();

		if (hNewDIB != NULL)
		{
			CDibDoc* pDoc = GetDocument();
			pDoc->ReplaceHDIB(hNewDIB); // and free the old DIB
			pDoc->InitDIBData();    // set up new size & palette
			pDoc->SetModifiedFlag(TRUE);

			SetScrollSizes(MM_TEXT, pDoc->GetDocSize());
			OnDoRealize((WPARAM)m_hWnd,0);  // realize the new palette
			pDoc->UpdateAllViews(NULL);
		}
		EndWaitCursor();
	}
}


void CDibView::OnUpdateEditPaste(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(::IsClipboardFormatAvailable(CF_DIB));
}


void CDibView::OnEditSelectAll()
{
	OnUpdate(NULL, 1);
}


void CDibView::OnIndicatorPos()
{
}


void CDibView::OnUpdateIndicatorPos(CCmdUI* pCmdUI)
{
}


void CDibView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CScrollView::OnLButtonDown(nFlags, point);

	if (m_flag == 0)
	{
		m_x = point.x;
		m_y = point.y;
		m_flag = 1;
	}
}


void CDibView::OnLButtonUp(UINT nFlags, CPoint point)
{
	CScrollView::OnLButtonUp(nFlags, point);

	if (m_flag)
	{
		if ((m_flag == 2 || m_flag == 101) &&
			((CDibLookApp *)AfxGetApp())->m_show_cropmask == 0)
			OnUpdate(NULL);
		SetCursor(LoadCursor(NULL, IDC_ARROW));
		m_flag = 0;
	}
}


void CDibView::OnRButtonDown(UINT nFlags, CPoint point)
{
	CScrollView::OnRButtonDown(nFlags, point);

	CMenu mnuTop;
	mnuTop.LoadMenu(IDR_POPUP1);
	CMenu* pPopup = mnuTop.GetSubMenu(0);
	ClientToScreen(&point);
	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON,
		point.x, point.y, this);
}


BOOL CDibView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
    // Skip setting cursor if we are inside some mode.
    if (m_flag && m_setcursor)
	{
		m_setcursor = 0;
		return TRUE;
	}
	return CScrollView::OnSetCursor(pWnd, nHitTest, message);
}


void CDibView::OnMouseMove(UINT nFlags, CPoint point)
{
  CScrollView::OnMouseMove(nFlags, point);

  CDibLookApp *myapp = (CDibLookApp *) AfxGetApp();
  CDibDoc *pDoc = GetDocument();

  CPoint scroll = GetDeviceScrollPosition();
  int scale, x_zoomed, y_zoomed, x, y;
  scale = pDoc->m_scale;
  if (scale == pDoc->m_scale_denom) scale = m_zoomval;
  x_zoomed = point.x + scroll.x;
  y_zoomed = point.y + scroll.y;
  if (scale == pDoc->m_scale_denom)
  {
	x = x_zoomed;
	y = y_zoomed;
  }
  else
  {
	x = (x_zoomed * pDoc->m_scale_denom + ((scale - 1) >> 1)) / scale;
	y = (y_zoomed * pDoc->m_scale_denom + ((scale - 1) >> 1)) / scale;
  }
  char buf[64];
  unsigned int paru[2];
  int pari[2];

  // Override OnSetCursor, we want to set our own.
  m_setcursor = 1;

  if (m_flag == 100)
  {
	int width = pDoc->m_image_width - m_right - m_left;
	int height = pDoc->m_image_height - m_bottom - m_top;
	m_left = x - width / 2; m_left -= m_left % pDoc->m_MCUwidth;
	if (m_left < 0) m_left = 0;
	m_top = y - height / 2; m_top -= m_top % pDoc->m_MCUheight;
	if (m_top < 0) m_top = 0;
	m_right = pDoc->m_image_width - m_left - width;
	m_bottom = pDoc->m_image_height - m_top - height;
	int lim_x = 0;
	int lim_y = 0;
	if (myapp->m_trim)
	{
		switch (pDoc->m_transform)
		{
		case 1:
		case 5:
			if (width > pDoc->m_image_width % pDoc->m_MCUwidth)
				lim_x = pDoc->m_image_width % pDoc->m_MCUwidth;
			break;
		case 2:
		case 7:
			if (height > pDoc->m_image_height % pDoc->m_MCUheight)
				lim_y = pDoc->m_image_height % pDoc->m_MCUheight;
			break;
		case 4:
		case 6:
			if (width > pDoc->m_image_width % pDoc->m_MCUwidth)
				lim_x = pDoc->m_image_width % pDoc->m_MCUwidth;
			if (height > pDoc->m_image_height % pDoc->m_MCUheight)
				lim_y = pDoc->m_image_height % pDoc->m_MCUheight;
			break;
		}
	}
	if (m_right < lim_x)
	{
	  m_left += m_right - lim_x;
	  m_right = lim_x + (m_left % pDoc->m_MCUwidth);
	  m_left -= m_right - lim_x;
	}
	if (m_bottom < lim_y)
	{
	  m_top += m_bottom - lim_y;
	  m_bottom = lim_y + (m_top % pDoc->m_MCUheight);
	  m_top -= m_bottom - lim_y;
	}
	width = m_zoomedsize.cx - m_right_zoomed - m_left_zoomed;
	height = m_zoomedsize.cy - m_bottom_zoomed - m_top_zoomed;
	m_left_zoomed = x_zoomed - width / 2;
	m_left_zoomed -=
		m_left_zoomed % ((pDoc->m_MCUwidth * scale) / pDoc->m_scale_denom);
	if (m_left_zoomed < 0) m_left_zoomed = 0;
	m_top_zoomed = y_zoomed - height / 2;
	m_top_zoomed -=
		m_top_zoomed % ((pDoc->m_MCUheight * scale) / pDoc->m_scale_denom);
	if (m_top_zoomed < 0) m_top_zoomed = 0;
	m_right_zoomed = m_zoomedsize.cx - m_left_zoomed - width;
	m_bottom_zoomed = m_zoomedsize.cy - m_top_zoomed - height;
	if (scale != pDoc->m_scale_denom)
	{
		lim_x = m_zoomedsize.cx -
			((pDoc->m_image_width - lim_x) * scale
			+ pDoc->m_scale_denom - 1)
			/ pDoc->m_scale_denom;
		lim_y = m_zoomedsize.cy -
			((pDoc->m_image_height - lim_y) * scale
			+ pDoc->m_scale_denom - 1)
			/ pDoc->m_scale_denom;
	}
	if (m_right_zoomed < lim_x)
	{
	  m_left_zoomed += m_right_zoomed - lim_x;
	  m_right_zoomed = lim_x +
		  (m_left_zoomed % ((pDoc->m_MCUwidth * scale) / pDoc->m_scale_denom));
	  m_left_zoomed -= m_right_zoomed - lim_x;
	}
	if (m_bottom_zoomed < lim_y)
	{
	  m_top_zoomed += m_bottom_zoomed - lim_y;
	  m_bottom_zoomed = lim_y +
		  (m_top_zoomed % ((pDoc->m_MCUheight * scale) / pDoc->m_scale_denom));
	  m_top_zoomed -= m_bottom_zoomed - lim_y;
	}
	if (myapp->m_show_cropmask) OnUpdate(NULL);
	else
	{
		SIZE size;
		RECT rect2;
		size.cx = 1;
		size.cy = 1;
		rect2.left = m_left_zoomed - scroll.x;
		rect2.top = m_top_zoomed - scroll.y;
		rect2.right = m_zoomedsize.cx - m_right_zoomed - scroll.x;
		rect2.bottom = m_zoomedsize.cy - m_bottom_zoomed - scroll.y;
		GetDC()->DrawDragRect(&rect2, size, NULL, size);
	}
    SetCursor(LoadCursor(NULL, IDC_SIZEALL));
	m_flag = 101;
	return;
  }
  if (m_flag == 101)
  {
	SIZE size;
	RECT rect1, rect2;
	rect1.left = m_left_zoomed - scroll.x;
	rect1.top = m_top_zoomed - scroll.y;
	rect1.right = m_zoomedsize.cx - m_right_zoomed - scroll.x;
	rect1.bottom = m_zoomedsize.cy - m_bottom_zoomed - scroll.y;
	int width = pDoc->m_image_width - m_right - m_left;
	int height = pDoc->m_image_height - m_bottom - m_top;
	m_left = x - width / 2; m_left -= m_left % pDoc->m_MCUwidth;
	if (m_left < 0) m_left = 0;
	m_top = y - height / 2; m_top -= m_top % pDoc->m_MCUheight;
	if (m_top < 0) m_top = 0;
	m_right = pDoc->m_image_width - m_left - width;
	m_bottom = pDoc->m_image_height - m_top - height;
	int lim_x = 0;
	int lim_y = 0;
	if (myapp->m_trim)
	{
		switch (pDoc->m_transform)
		{
		case 1:
		case 5:
			if (width > pDoc->m_image_width % pDoc->m_MCUwidth)
				lim_x = pDoc->m_image_width % pDoc->m_MCUwidth;
			break;
		case 2:
		case 7:
			if (height > pDoc->m_image_height % pDoc->m_MCUheight)
				lim_y = pDoc->m_image_height % pDoc->m_MCUheight;
			break;
		case 4:
		case 6:
			if (width > pDoc->m_image_width % pDoc->m_MCUwidth)
				lim_x = pDoc->m_image_width % pDoc->m_MCUwidth;
			if (height > pDoc->m_image_height % pDoc->m_MCUheight)
				lim_y = pDoc->m_image_height % pDoc->m_MCUheight;
			break;
		}
	}
	if (m_right < lim_x)
	{
	  m_left += m_right - lim_x;
	  m_right = lim_x + (m_left % pDoc->m_MCUwidth);
	  m_left -= m_right - lim_x;
	}
	if (m_bottom < lim_y)
	{
	  m_top += m_bottom - lim_y;
	  m_bottom = lim_y + (m_top % pDoc->m_MCUheight);
	  m_top -= m_bottom - lim_y;
	}
	width = m_zoomedsize.cx - m_right_zoomed - m_left_zoomed;
	height = m_zoomedsize.cy - m_bottom_zoomed - m_top_zoomed;
	m_left_zoomed = x_zoomed - width / 2;
	m_left_zoomed -=
		m_left_zoomed % ((pDoc->m_MCUwidth * scale) / pDoc->m_scale_denom);
	if (m_left_zoomed < 0) m_left_zoomed = 0;
	m_top_zoomed = y_zoomed - height / 2;
	m_top_zoomed -=
		m_top_zoomed % ((pDoc->m_MCUheight * scale) / pDoc->m_scale_denom);
	if (m_top_zoomed < 0) m_top_zoomed = 0;
	m_right_zoomed = m_zoomedsize.cx - m_left_zoomed - width;
	m_bottom_zoomed = m_zoomedsize.cy - m_top_zoomed - height;
	if (scale != pDoc->m_scale_denom)
	{
		lim_x = m_zoomedsize.cx -
			((pDoc->m_image_width - lim_x) * scale
			+ pDoc->m_scale_denom - 1)
			/ pDoc->m_scale_denom;
		lim_y = m_zoomedsize.cy -
			((pDoc->m_image_height - lim_y) * scale
			+ pDoc->m_scale_denom - 1)
			/ pDoc->m_scale_denom;
	}
	if (m_right_zoomed < lim_x)
	{
	  m_left_zoomed += m_right_zoomed - lim_x;
	  m_right_zoomed = lim_x +
		  (m_left_zoomed % ((pDoc->m_MCUwidth * scale) / pDoc->m_scale_denom));
	  m_left_zoomed -= m_right_zoomed - lim_x;
	}
	if (m_bottom_zoomed < lim_y)
	{
	  m_top_zoomed += m_bottom_zoomed - lim_y;
	  m_bottom_zoomed = lim_y +
		  (m_top_zoomed % ((pDoc->m_MCUheight * scale) / pDoc->m_scale_denom));
	  m_top_zoomed -= m_bottom_zoomed - lim_y;
	}
	if (myapp->m_show_cropmask) OnUpdate(NULL);
	else
	{
		size.cx = 1;
		size.cy = 1;
		rect2.left = m_left_zoomed - scroll.x;
		rect2.top = m_top_zoomed - scroll.y;
		rect2.right = m_zoomedsize.cx - m_right_zoomed - scroll.x;
		rect2.bottom = m_zoomedsize.cy - m_bottom_zoomed - scroll.y;
		GetDC()->DrawDragRect(&rect2, size, &rect1, size);
	}
    SetCursor(LoadCursor(NULL, IDC_SIZEALL));

//	CRect rcDisp;
//	GetClientRect(rcDisp);
//	if (point.x < rcDisp.left)
//		ScrollToPosition(CPoint(point.x, scroll.y));

	if (m_save_zoom)
	{
		sprintf(buf, " %dx%d+%d+%d",
			m_zoomedsize.cx - m_right_zoomed - m_left_zoomed,
			m_zoomedsize.cy - m_bottom_zoomed - m_top_zoomed,
			m_left_zoomed, m_top_zoomed);
	}
	else
	{
		sprintf(buf, " %dx%d+%d+%d",
			pDoc->m_image_width - m_right - m_left,
			pDoc->m_image_height - m_bottom - m_top,
			m_left, m_top);
	}
	((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.GetPaneInfo(1, paru[0], paru[1], pari[0]);
	((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.SetPaneInfo(1, ID_INDICATOR_POS, SBPS_NORMAL, pari[0]);
	((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.SetPaneText(1, buf, TRUE);
	return;
  }

  if (nFlags & MK_LBUTTON)
  {
	switch (m_flag) {
	case 1:
	  if (point.x == m_x && point.y == m_y) break;
	  m_left_zoomed = m_x + scroll.x;
	  m_top_zoomed = m_y + scroll.y;
	  if (scale == pDoc->m_scale_denom)
	  {
		m_left = m_left_zoomed;
		m_top = m_top_zoomed;
	  }
	  else
	  {
		m_left = (m_left_zoomed * pDoc->m_scale_denom + ((scale - 1) >> 1))
			/ scale;
		m_top = (m_top_zoomed * pDoc->m_scale_denom + ((scale - 1) >> 1))
			/ scale;
	  }
	  if (m_left >= pDoc->m_image_width) m_left = pDoc->m_image_width - 1;
	  if (m_top >= pDoc->m_image_height) m_top = pDoc->m_image_height - 1;
	  m_left -= m_left % pDoc->m_MCUwidth;
	  m_top -= m_top % pDoc->m_MCUheight;
	  if (scale == pDoc->m_scale_denom)
	  {
		m_left_zoomed = m_left;
		m_top_zoomed = m_top;
	  }
	  else
	  {
		if (m_left_zoomed >= m_zoomedsize.cx) m_left_zoomed = m_zoomedsize.cx - 1;
		if (m_top_zoomed >= m_zoomedsize.cy) m_top_zoomed = m_zoomedsize.cy - 1;
		m_left_zoomed -=
			m_left_zoomed % ((pDoc->m_MCUwidth * scale) / pDoc->m_scale_denom);
		m_top_zoomed -=
			m_top_zoomed % ((pDoc->m_MCUheight * scale) / pDoc->m_scale_denom);
	  }
	  if (x < m_left) x = m_left;
	  if (y < m_top) y = m_top;
	  if (x_zoomed < m_left_zoomed) x_zoomed = m_left_zoomed;
	  if (y_zoomed < m_top_zoomed) y_zoomed = m_top_zoomed;
	  if (myapp->m_endpoint_snap)
	  {
		x -= x % pDoc->m_MCUwidth;
		x += pDoc->m_MCUwidth - 1;
		y -= y % pDoc->m_MCUheight;
		y += pDoc->m_MCUheight - 1;
		if (scale == pDoc->m_scale_denom)
		{
			x_zoomed = x;
			y_zoomed = y;
		}
		else
		{
			x_zoomed -=
				x_zoomed % ((pDoc->m_MCUwidth * scale) / pDoc->m_scale_denom);
			x_zoomed += (pDoc->m_MCUwidth * scale) / pDoc->m_scale_denom - 1;
			y_zoomed -=
				y_zoomed % ((pDoc->m_MCUheight * scale) / pDoc->m_scale_denom);
			y_zoomed += (pDoc->m_MCUheight * scale) / pDoc->m_scale_denom - 1;
		}
	  }
	  if (x >= pDoc->m_image_width) x = pDoc->m_image_width - 1;
	  if (y >= pDoc->m_image_height) y = pDoc->m_image_height - 1;
	  if (x_zoomed >= m_zoomedsize.cx) x_zoomed = m_zoomedsize.cx - 1;
	  if (y_zoomed >= m_zoomedsize.cy) y_zoomed = m_zoomedsize.cy - 1;
	  if (myapp->m_trim)
	  {
		switch (pDoc->m_transform)
		{
		case 1:
		case 5:
			if (x >= pDoc->m_image_width - (pDoc->m_image_width % pDoc->m_MCUwidth) &&
				pDoc->m_image_width - (pDoc->m_image_width % pDoc->m_MCUwidth) > m_left)
			{
				x = pDoc->m_image_width - (pDoc->m_image_width % pDoc->m_MCUwidth);
				x_zoomed = (x * scale) / pDoc->m_scale_denom - 1;
				x--;
			}
			break;
		case 2:
		case 7:
			if (y >= pDoc->m_image_height - (pDoc->m_image_height % pDoc->m_MCUheight) &&
				pDoc->m_image_height - (pDoc->m_image_height % pDoc->m_MCUheight) > m_top)
			{
				y = pDoc->m_image_height - (pDoc->m_image_height % pDoc->m_MCUheight);
				y_zoomed = (y * scale) / pDoc->m_scale_denom - 1;
				y--;
			}
			break;
		case 4:
		case 6:
			if (x >= pDoc->m_image_width - (pDoc->m_image_width % pDoc->m_MCUwidth) &&
				pDoc->m_image_width - (pDoc->m_image_width % pDoc->m_MCUwidth) > m_left)
			{
				x = pDoc->m_image_width - (pDoc->m_image_width % pDoc->m_MCUwidth);
				x_zoomed = (x * scale) / pDoc->m_scale_denom - 1;
				x--;
			}
			if (y >= pDoc->m_image_height - (pDoc->m_image_height % pDoc->m_MCUheight) &&
				pDoc->m_image_height - (pDoc->m_image_height % pDoc->m_MCUheight) > m_top)
			{
				y = pDoc->m_image_height - (pDoc->m_image_height % pDoc->m_MCUheight);
				y_zoomed = (y * scale) / pDoc->m_scale_denom - 1;
				y--;
			}
			break;
		}
	  }
	  m_right = pDoc->m_image_width - 1 - x;
	  m_bottom = pDoc->m_image_height - 1 - y;
	  m_right_zoomed = m_zoomedsize.cx - 1 - x_zoomed;
	  m_bottom_zoomed = m_zoomedsize.cy - 1 - y_zoomed;
	  if (myapp->m_show_cropmask) OnUpdate(NULL);
	  else
	  {
		SIZE size;
		RECT rect2;
		size.cx = 1;
		size.cy = 1;
		rect2.left = m_left_zoomed - scroll.x;
		rect2.top = m_top_zoomed - scroll.y;
		rect2.right = x_zoomed - scroll.x + 1;
		rect2.bottom = y_zoomed - scroll.y + 1;
		GetDC()->DrawDragRect(&rect2, size, NULL, size);
	  }
	  m_flag = 2;
	  break;
	case 2:
	  if (x < m_left) x = m_left;
	  if (y < m_top) y = m_top;
	  if (x_zoomed < m_left_zoomed) x_zoomed = m_left_zoomed;
	  if (y_zoomed < m_top_zoomed) y_zoomed = m_top_zoomed;
	  if (myapp->m_endpoint_snap)
	  {
		x -= x % pDoc->m_MCUwidth;
		x += pDoc->m_MCUwidth - 1;
		y -= y % pDoc->m_MCUheight;
		y += pDoc->m_MCUheight - 1;
		if (scale == pDoc->m_scale_denom)
		{
			x_zoomed = x;
			y_zoomed = y;
		}
		else
		{
			x_zoomed -=
				x_zoomed % ((pDoc->m_MCUwidth * scale) / pDoc->m_scale_denom);
			x_zoomed += (pDoc->m_MCUwidth * scale) / pDoc->m_scale_denom - 1;
			y_zoomed -=
				y_zoomed % ((pDoc->m_MCUheight * scale) / pDoc->m_scale_denom);
			y_zoomed += (pDoc->m_MCUheight * scale) / pDoc->m_scale_denom - 1;
		}
	  }
	  if (x >= pDoc->m_image_width) x = pDoc->m_image_width - 1;
	  if (y >= pDoc->m_image_height) y = pDoc->m_image_height - 1;
	  if (x_zoomed >= m_zoomedsize.cx) x_zoomed = m_zoomedsize.cx - 1;
	  if (y_zoomed >= m_zoomedsize.cy) y_zoomed = m_zoomedsize.cy - 1;
	  if (myapp->m_trim)
	  {
		switch (pDoc->m_transform)
		{
		case 1:
		case 5:
			if (x >= pDoc->m_image_width - (pDoc->m_image_width % pDoc->m_MCUwidth) &&
				pDoc->m_image_width - (pDoc->m_image_width % pDoc->m_MCUwidth) > m_left)
			{
				x = pDoc->m_image_width - (pDoc->m_image_width % pDoc->m_MCUwidth);
				x_zoomed = (x * scale) / pDoc->m_scale_denom - 1;
				x--;
			}
			break;
		case 2:
		case 7:
			if (y >= pDoc->m_image_height - (pDoc->m_image_height % pDoc->m_MCUheight) &&
				pDoc->m_image_height - (pDoc->m_image_height % pDoc->m_MCUheight) > m_top)
			{
				y = pDoc->m_image_height - (pDoc->m_image_height % pDoc->m_MCUheight);
				y_zoomed = (y * scale) / pDoc->m_scale_denom - 1;
				y--;
			}
			break;
		case 4:
		case 6:
			if (x >= pDoc->m_image_width - (pDoc->m_image_width % pDoc->m_MCUwidth) &&
				pDoc->m_image_width - (pDoc->m_image_width % pDoc->m_MCUwidth) > m_left)
			{
				x = pDoc->m_image_width - (pDoc->m_image_width % pDoc->m_MCUwidth);
				x_zoomed = (x * scale) / pDoc->m_scale_denom - 1;
				x--;
			}
			if (y >= pDoc->m_image_height - (pDoc->m_image_height % pDoc->m_MCUheight) &&
				pDoc->m_image_height - (pDoc->m_image_height % pDoc->m_MCUheight) > m_top)
			{
				y = pDoc->m_image_height - (pDoc->m_image_height % pDoc->m_MCUheight);
				y_zoomed = (y * scale) / pDoc->m_scale_denom - 1;
				y--;
			}
			break;
		}
	  }
	  m_right = pDoc->m_image_width - 1 - x;
	  m_bottom = pDoc->m_image_height - 1 - y;
	  if (myapp->m_show_cropmask)
	  {
		m_right_zoomed = m_zoomedsize.cx - 1 - x_zoomed;
		m_bottom_zoomed = m_zoomedsize.cy - 1 - y_zoomed;
		OnUpdate(NULL);
	  }
	  else
	  {
		SIZE size;
		RECT rect1, rect2;
		size.cx = 1;
		size.cy = 1;
		rect1.left = m_left_zoomed - scroll.x;
		rect1.top = m_top_zoomed - scroll.y;
		rect1.right = m_zoomedsize.cx - m_right_zoomed - scroll.x;
		rect1.bottom = m_zoomedsize.cy - m_bottom_zoomed - scroll.y;
		m_right_zoomed = m_zoomedsize.cx - 1 - x_zoomed;
		m_bottom_zoomed = m_zoomedsize.cy - 1 - y_zoomed;
		rect2.left = rect1.left;
		rect2.top = rect1.top;
		rect2.right = x_zoomed - scroll.x + 1;
		rect2.bottom = y_zoomed - scroll.y + 1;
		GetDC()->DrawDragRect(&rect2, size, &rect1, size);
	  }

	  if (m_save_zoom)
	  {
		sprintf(buf, " %dx%d+%d+%d",
			m_zoomedsize.cx - m_right_zoomed - m_left_zoomed,
			m_zoomedsize.cy - m_bottom_zoomed - m_top_zoomed,
			m_left_zoomed, m_top_zoomed);
	  }
	  else
	  {
		sprintf(buf, " %dx%d+%d+%d",
			pDoc->m_image_width - m_right - m_left,
			pDoc->m_image_height - m_bottom - m_top,
			m_left, m_top);
	  }
	  ((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.GetPaneInfo(1, paru[0], paru[1], pari[0]);
	  ((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.SetPaneInfo(1, ID_INDICATOR_POS, SBPS_NORMAL, pari[0]);
	  ((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.SetPaneText(1, buf, TRUE);
	  break;
	case 4:
	  if (x > pDoc->m_image_width - 1 - m_right)
		  x = pDoc->m_image_width - 1 - m_right;
	  x -= x % pDoc->m_MCUwidth;
	  if (scale == pDoc->m_scale_denom)
	  {
		x_zoomed = x;
	  }
	  else
	  {
		if (x_zoomed > m_zoomedsize.cx - 1 - m_right_zoomed)
			x_zoomed = m_zoomedsize.cx - 1 - m_right_zoomed;
		x_zoomed -=
			x_zoomed % ((pDoc->m_MCUwidth * scale) / pDoc->m_scale_denom);
	  }
	  SetCursor(LoadCursor(NULL, IDC_SIZEWE));
	  if (x_zoomed == m_left_zoomed) break;
	  m_left_zoomed = x_zoomed;
	  m_left = x;
	  if (m_save_zoom)
	  {
		sprintf(buf, " %dx%d+%d+%d",
			m_zoomedsize.cx - m_right_zoomed - m_left_zoomed,
			m_zoomedsize.cy - m_bottom_zoomed - m_top_zoomed,
			m_left_zoomed, m_top_zoomed);
	  }
	  else
	  {
		sprintf(buf, " %dx%d+%d+%d",
			pDoc->m_image_width - m_right - m_left,
			pDoc->m_image_height - m_bottom - m_top,
			m_left, m_top);
	  }
	  ((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.GetPaneInfo(1, paru[0], paru[1], pari[0]);
	  ((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.SetPaneInfo(1, ID_INDICATOR_POS, SBPS_NORMAL, pari[0]);
	  ((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.SetPaneText(1, buf, TRUE);
	  OnUpdate(NULL);
	  break;
	case 5:
	  if (y > pDoc->m_image_height - 1 - m_bottom)
		  y = pDoc->m_image_height - 1 - m_bottom;
	  y -= y % pDoc->m_MCUheight;
	  if (scale == pDoc->m_scale_denom)
	  {
		y_zoomed = y;
	  }
	  else
	  {
		if (y_zoomed > m_zoomedsize.cy - 1 - m_bottom_zoomed)
			y_zoomed = m_zoomedsize.cy - 1 - m_bottom_zoomed;
		y_zoomed -=
			y_zoomed % ((pDoc->m_MCUheight * scale) / pDoc->m_scale_denom);
	  }
	  SetCursor(LoadCursor(NULL, IDC_SIZENS));
	  if (y_zoomed == m_top_zoomed) break;
	  m_top_zoomed = y_zoomed;
	  m_top = y;
	  if (m_save_zoom)
	  {
		sprintf(buf, " %dx%d+%d+%d",
			m_zoomedsize.cx - m_right_zoomed - m_left_zoomed,
			m_zoomedsize.cy - m_bottom_zoomed - m_top_zoomed,
			m_left_zoomed, m_top_zoomed);
	  }
	  else
	  {
		sprintf(buf, " %dx%d+%d+%d",
			pDoc->m_image_width - m_right - m_left,
			pDoc->m_image_height - m_bottom - m_top,
			m_left, m_top);
	  }
	  ((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.GetPaneInfo(1, paru[0], paru[1], pari[0]);
	  ((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.SetPaneInfo(1, ID_INDICATOR_POS, SBPS_NORMAL, pari[0]);
	  ((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.SetPaneText(1, buf, TRUE);
	  OnUpdate(NULL);
	  break;
	case 6:
	  if (x < m_left) x = m_left;
	  if (x_zoomed < m_left_zoomed) x_zoomed = m_left_zoomed;
	  if (myapp->m_endpoint_snap)
	  {
		x -= x % pDoc->m_MCUwidth;
		x += pDoc->m_MCUwidth - 1;
		if (scale == pDoc->m_scale_denom)
		{
			x_zoomed = x;
		}
		else
		{
			x_zoomed -=
				x_zoomed % ((pDoc->m_MCUwidth * scale) / pDoc->m_scale_denom);
			x_zoomed += (pDoc->m_MCUwidth * scale) / pDoc->m_scale_denom - 1;
		}
	  }
	  if (x >= pDoc->m_image_width) x = pDoc->m_image_width - 1;
	  if (x_zoomed >= m_zoomedsize.cx) x_zoomed = m_zoomedsize.cx - 1;
	  if (myapp->m_trim)
	  {
		switch (pDoc->m_transform)
		{
		case 1:
		case 4:
		case 5:
		case 6:
			if (x >= pDoc->m_image_width - (pDoc->m_image_width % pDoc->m_MCUwidth) &&
				pDoc->m_image_width - (pDoc->m_image_width % pDoc->m_MCUwidth) > m_left)
			{
				x = pDoc->m_image_width - (pDoc->m_image_width % pDoc->m_MCUwidth);
				x_zoomed = (x * scale) / pDoc->m_scale_denom - 1;
				x--;
			}
			break;
		}
	  }
	  SetCursor(LoadCursor(NULL, IDC_SIZEWE));
	  if (m_zoomedsize.cx - 1 - x_zoomed == m_right_zoomed) break;
	  m_right_zoomed = m_zoomedsize.cx - 1 - x_zoomed;
	  m_right = pDoc->m_image_width - 1 - x;
	  if (m_save_zoom)
	  {
		sprintf(buf, " %dx%d+%d+%d",
			m_zoomedsize.cx - m_right_zoomed - m_left_zoomed,
			m_zoomedsize.cy - m_bottom_zoomed - m_top_zoomed,
			m_left_zoomed, m_top_zoomed);
	  }
	  else
	  {
		sprintf(buf, " %dx%d+%d+%d",
			pDoc->m_image_width - m_right - m_left,
			pDoc->m_image_height - m_bottom - m_top,
			m_left, m_top);
	  }
	  ((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.GetPaneInfo(1, paru[0], paru[1], pari[0]);
	  ((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.SetPaneInfo(1, ID_INDICATOR_POS, SBPS_NORMAL, pari[0]);
	  ((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.SetPaneText(1, buf, TRUE);
	  OnUpdate(NULL);
	  break;
	case 7:
	  if (y < m_top) y = m_top;
	  if (y_zoomed < m_top_zoomed) y_zoomed = m_top_zoomed;
	  if (myapp->m_endpoint_snap)
	  {
		y -= y % pDoc->m_MCUheight;
		y += pDoc->m_MCUheight - 1;
		if (scale == pDoc->m_scale_denom)
		{
			y_zoomed = y;
		}
		else
		{
			y_zoomed -=
				y_zoomed % ((pDoc->m_MCUheight * scale) / pDoc->m_scale_denom);
			y_zoomed += (pDoc->m_MCUheight * scale) / pDoc->m_scale_denom - 1;
		}
	  }
	  if (y >= pDoc->m_image_height) y = pDoc->m_image_height - 1;
	  if (y_zoomed >= m_zoomedsize.cy) y_zoomed = m_zoomedsize.cy - 1;
	  if (myapp->m_trim)
	  {
		switch (pDoc->m_transform)
		{
		case 2:
		case 4:
		case 6:
		case 7:
			if (y >= pDoc->m_image_height - (pDoc->m_image_height % pDoc->m_MCUheight) &&
				pDoc->m_image_height - (pDoc->m_image_height % pDoc->m_MCUheight) > m_top)
			{
				y = pDoc->m_image_height - (pDoc->m_image_height % pDoc->m_MCUheight);
				y_zoomed = (y * scale) / pDoc->m_scale_denom - 1;
				y--;
			}
			break;
		}
	  }
	  SetCursor(LoadCursor(NULL, IDC_SIZENS));
	  if (m_zoomedsize.cy - 1 - y_zoomed == m_bottom_zoomed) break;
	  m_bottom_zoomed = m_zoomedsize.cy - 1 - y_zoomed;
	  m_bottom = pDoc->m_image_height - 1 - y;
	  if (m_save_zoom)
	  {
		sprintf(buf, " %dx%d+%d+%d",
			m_zoomedsize.cx - m_right_zoomed - m_left_zoomed,
			m_zoomedsize.cy - m_bottom_zoomed - m_top_zoomed,
			m_left_zoomed, m_top_zoomed);
	  }
	  else
	  {
		sprintf(buf, " %dx%d+%d+%d",
			pDoc->m_image_width - m_right - m_left,
			pDoc->m_image_height - m_bottom - m_top,
			m_left, m_top);
	  }
	  ((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.GetPaneInfo(1, paru[0], paru[1], pari[0]);
	  ((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.SetPaneInfo(1, ID_INDICATOR_POS, SBPS_NORMAL, pari[0]);
	  ((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.SetPaneText(1, buf, TRUE);
	  OnUpdate(NULL);
	  break;
	case 8:
	  if (x > pDoc->m_image_width - 1 - m_right)
		  x = pDoc->m_image_width - 1 - m_right;
	  if (y > pDoc->m_image_height - 1 - m_bottom)
		  y = pDoc->m_image_height - 1 - m_bottom;
	  x -= x % pDoc->m_MCUwidth;
	  y -= y % pDoc->m_MCUheight;
	  if (scale == pDoc->m_scale_denom)
	  {
		x_zoomed = x;
		y_zoomed = y;
	  }
	  else
	  {
		if (x_zoomed > m_zoomedsize.cx - 1 - m_right_zoomed)
			x_zoomed = m_zoomedsize.cx - 1 - m_right_zoomed;
		x_zoomed -=
			x_zoomed % ((pDoc->m_MCUwidth * scale) / pDoc->m_scale_denom);
		if (y_zoomed > m_zoomedsize.cy - 1 - m_bottom_zoomed)
			y_zoomed = m_zoomedsize.cy - 1 - m_bottom_zoomed;
		y_zoomed -=
			y_zoomed % ((pDoc->m_MCUheight * scale) / pDoc->m_scale_denom);
	  }
	  SetCursor(LoadCursor(NULL, IDC_SIZENWSE));
	  if (x_zoomed == m_left_zoomed && y_zoomed == m_top_zoomed) break;
	  m_left_zoomed = x_zoomed;
	  m_top_zoomed = y_zoomed;
	  m_left = x;
	  m_top = y;
	  if (m_save_zoom)
	  {
		sprintf(buf, " %dx%d+%d+%d",
			m_zoomedsize.cx - m_right_zoomed - m_left_zoomed,
			m_zoomedsize.cy - m_bottom_zoomed - m_top_zoomed,
			m_left_zoomed, m_top_zoomed);
	  }
	  else
	  {
		sprintf(buf, " %dx%d+%d+%d",
			pDoc->m_image_width - m_right - m_left,
			pDoc->m_image_height - m_bottom - m_top,
			m_left, m_top);
	  }
	  ((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.GetPaneInfo(1, paru[0], paru[1], pari[0]);
	  ((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.SetPaneInfo(1, ID_INDICATOR_POS, SBPS_NORMAL, pari[0]);
	  ((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.SetPaneText(1, buf, TRUE);
	  OnUpdate(NULL);
	  break;
	case 9:
	  if (x < m_left) x = m_left;
	  if (x_zoomed < m_left_zoomed) x_zoomed = m_left_zoomed;
	  if (myapp->m_endpoint_snap)
	  {
		x -= x % pDoc->m_MCUwidth;
		x += pDoc->m_MCUwidth - 1;
		if (scale == pDoc->m_scale_denom)
		{
			x_zoomed = x;
		}
		else
		{
			x_zoomed -=
				x_zoomed % ((pDoc->m_MCUwidth * scale) / pDoc->m_scale_denom);
			x_zoomed += (pDoc->m_MCUwidth * scale) / pDoc->m_scale_denom - 1;
		}
	  }
	  if (x >= pDoc->m_image_width) x = pDoc->m_image_width - 1;
	  if (x_zoomed >= m_zoomedsize.cx) x_zoomed = m_zoomedsize.cx - 1;
	  if (myapp->m_trim)
	  {
		switch (pDoc->m_transform)
		{
		case 1:
		case 4:
		case 5:
		case 6:
			if (x >= pDoc->m_image_width - (pDoc->m_image_width % pDoc->m_MCUwidth) &&
				pDoc->m_image_width - (pDoc->m_image_width % pDoc->m_MCUwidth) > m_left)
			{
				x = pDoc->m_image_width - (pDoc->m_image_width % pDoc->m_MCUwidth);
				x_zoomed = (x * scale) / pDoc->m_scale_denom - 1;
				x--;
			}
			break;
		}
	  }
	  if (y > pDoc->m_image_height - 1 - m_bottom)
		  y = pDoc->m_image_height - 1 - m_bottom;
	  y -= y % pDoc->m_MCUheight;
	  if (scale == pDoc->m_scale_denom)
	  {
		y_zoomed = y;
	  }
	  else
	  {
		if (y_zoomed > m_zoomedsize.cy - 1 - m_bottom_zoomed)
			y_zoomed = m_zoomedsize.cy - 1 - m_bottom_zoomed;
		y_zoomed -=
			y_zoomed % ((pDoc->m_MCUheight * scale) / pDoc->m_scale_denom);
	  }
	  SetCursor(LoadCursor(NULL, IDC_SIZENESW));
	  if (m_zoomedsize.cx - 1 - x_zoomed == m_right_zoomed &&
		  y_zoomed == m_top_zoomed) break;
	  m_right_zoomed = m_zoomedsize.cx - 1 - x_zoomed;
	  m_top_zoomed = y_zoomed;
	  m_right = pDoc->m_image_width - 1 - x;
	  m_top = y;
	  if (m_save_zoom)
	  {
		sprintf(buf, " %dx%d+%d+%d",
			m_zoomedsize.cx - m_right_zoomed - m_left_zoomed,
			m_zoomedsize.cy - m_bottom_zoomed - m_top_zoomed,
			m_left_zoomed, m_top_zoomed);
	  }
	  else
	  {
		sprintf(buf, " %dx%d+%d+%d",
			pDoc->m_image_width - m_right - m_left,
			pDoc->m_image_height - m_bottom - m_top,
			m_left, m_top);
	  }
	  ((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.GetPaneInfo(1, paru[0], paru[1], pari[0]);
	  ((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.SetPaneInfo(1, ID_INDICATOR_POS, SBPS_NORMAL, pari[0]);
	  ((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.SetPaneText(1, buf, TRUE);
	  OnUpdate(NULL);
	  break;
	case 10:
	  if (x < m_left) x = m_left;
	  if (y < m_top) y = m_top;
	  if (x_zoomed < m_left_zoomed) x_zoomed = m_left_zoomed;
	  if (y_zoomed < m_top_zoomed) y_zoomed = m_top_zoomed;
	  if (myapp->m_endpoint_snap)
	  {
		x -= x % pDoc->m_MCUwidth;
		x += pDoc->m_MCUwidth - 1;
		y -= y % pDoc->m_MCUheight;
		y += pDoc->m_MCUheight - 1;
		if (scale == pDoc->m_scale_denom)
		{
			x_zoomed = x;
			y_zoomed = y;
		}
		else
		{
			x_zoomed -=
				x_zoomed % ((pDoc->m_MCUwidth * scale) / pDoc->m_scale_denom);
			x_zoomed += (pDoc->m_MCUwidth * scale) / pDoc->m_scale_denom - 1;
			y_zoomed -=
				y_zoomed % ((pDoc->m_MCUheight * scale) / pDoc->m_scale_denom);
			y_zoomed += (pDoc->m_MCUheight * scale) / pDoc->m_scale_denom - 1;
		}
	  }
	  if (x >= pDoc->m_image_width) x = pDoc->m_image_width - 1;
	  if (y >= pDoc->m_image_height) y = pDoc->m_image_height - 1;
	  if (x_zoomed >= m_zoomedsize.cx) x_zoomed = m_zoomedsize.cx - 1;
	  if (y_zoomed >= m_zoomedsize.cy) y_zoomed = m_zoomedsize.cy - 1;
	  if (myapp->m_trim)
	  {
		switch (pDoc->m_transform)
		{
		case 1:
		case 5:
			if (x >= pDoc->m_image_width - (pDoc->m_image_width % pDoc->m_MCUwidth) &&
				pDoc->m_image_width - (pDoc->m_image_width % pDoc->m_MCUwidth) > m_left)
			{
				x = pDoc->m_image_width - (pDoc->m_image_width % pDoc->m_MCUwidth);
				x_zoomed = (x * scale) / pDoc->m_scale_denom - 1;
				x--;
			}
			break;
		case 2:
		case 7:
			if (y >= pDoc->m_image_height - (pDoc->m_image_height % pDoc->m_MCUheight) &&
				pDoc->m_image_height - (pDoc->m_image_height % pDoc->m_MCUheight) > m_top)
			{
				y = pDoc->m_image_height - (pDoc->m_image_height % pDoc->m_MCUheight);
				y_zoomed = (y * scale) / pDoc->m_scale_denom - 1;
				y--;
			}
			break;
		case 4:
		case 6:
			if (x >= pDoc->m_image_width - (pDoc->m_image_width % pDoc->m_MCUwidth) &&
				pDoc->m_image_width - (pDoc->m_image_width % pDoc->m_MCUwidth) > m_left)
			{
				x = pDoc->m_image_width - (pDoc->m_image_width % pDoc->m_MCUwidth);
				x_zoomed = (x * scale) / pDoc->m_scale_denom - 1;
				x--;
			}
			if (y >= pDoc->m_image_height - (pDoc->m_image_height % pDoc->m_MCUheight) &&
				pDoc->m_image_height - (pDoc->m_image_height % pDoc->m_MCUheight) > m_top)
			{
				y = pDoc->m_image_height - (pDoc->m_image_height % pDoc->m_MCUheight);
				y_zoomed = (y * scale) / pDoc->m_scale_denom - 1;
				y--;
			}
			break;
		}
	  }
	  SetCursor(LoadCursor(NULL, IDC_SIZENWSE));
	  if (m_zoomedsize.cx - 1 - x_zoomed == m_right_zoomed &&
		  m_zoomedsize.cy - 1 - y_zoomed == m_bottom_zoomed) break;
	  m_right_zoomed = m_zoomedsize.cx - 1 - x_zoomed;
	  m_bottom_zoomed = m_zoomedsize.cy - 1 - y_zoomed;
	  m_right = pDoc->m_image_width - 1 - x;
	  m_bottom = pDoc->m_image_height - 1 - y;
	  if (m_save_zoom)
	  {
		sprintf(buf, " %dx%d+%d+%d",
			m_zoomedsize.cx - m_right_zoomed - m_left_zoomed,
			m_zoomedsize.cy - m_bottom_zoomed - m_top_zoomed,
			m_left_zoomed, m_top_zoomed);
	  }
	  else
	  {
		sprintf(buf, " %dx%d+%d+%d",
			pDoc->m_image_width - m_right - m_left,
			pDoc->m_image_height - m_bottom - m_top,
			m_left, m_top);
	  }
	  ((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.GetPaneInfo(1, paru[0], paru[1], pari[0]);
	  ((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.SetPaneInfo(1, ID_INDICATOR_POS, SBPS_NORMAL, pari[0]);
	  ((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.SetPaneText(1, buf, TRUE);
	  OnUpdate(NULL);
	  break;
	case 11:
	  if (x > pDoc->m_image_width - 1 - m_right)
		  x = pDoc->m_image_width - 1 - m_right;
	  x -= x % pDoc->m_MCUwidth;
	  if (scale == pDoc->m_scale_denom)
	  {
		x_zoomed = x;
	  }
	  else
	  {
		if (x_zoomed > m_zoomedsize.cx - 1 - m_right_zoomed)
			x_zoomed = m_zoomedsize.cx - 1 - m_right_zoomed;
		x_zoomed -=
			x_zoomed % ((pDoc->m_MCUwidth * scale) / pDoc->m_scale_denom);
	  }
	  if (y < m_top) y = m_top;
	  if (y_zoomed < m_top_zoomed) y_zoomed = m_top_zoomed;
	  if (myapp->m_endpoint_snap)
	  {
		y -= y % pDoc->m_MCUheight;
		y += pDoc->m_MCUheight - 1;
		if (scale == pDoc->m_scale_denom)
		{
			y_zoomed = y;
		}
		else
		{
			y_zoomed -=
				y_zoomed % ((pDoc->m_MCUheight * scale) / pDoc->m_scale_denom);
			y_zoomed += (pDoc->m_MCUheight * scale) / pDoc->m_scale_denom - 1;
		}
	  }
	  if (y >= pDoc->m_image_height) y = pDoc->m_image_height - 1;
	  if (y_zoomed >= m_zoomedsize.cy) y_zoomed = m_zoomedsize.cy - 1;
	  if (myapp->m_trim)
	  {
		switch (pDoc->m_transform)
		{
		case 2:
		case 4:
		case 6:
		case 7:
			if (y >= pDoc->m_image_height - (pDoc->m_image_height % pDoc->m_MCUheight) &&
				pDoc->m_image_height - (pDoc->m_image_height % pDoc->m_MCUheight) > m_top)
			{
				y = pDoc->m_image_height - (pDoc->m_image_height % pDoc->m_MCUheight);
				y_zoomed = (y * scale) / pDoc->m_scale_denom - 1;
				y--;
			}
			break;
		}
	  }
	  SetCursor(LoadCursor(NULL, IDC_SIZENESW));
	  if (x_zoomed == m_left_zoomed &&
		  m_zoomedsize.cy - 1 - y_zoomed == m_bottom_zoomed) break;
	  m_left_zoomed = x_zoomed;
	  m_bottom_zoomed = m_zoomedsize.cy - 1 - y_zoomed;
	  m_left = x;
	  m_bottom = pDoc->m_image_height - 1 - y;
	  if (m_save_zoom)
	  {
		sprintf(buf, " %dx%d+%d+%d",
			m_zoomedsize.cx - m_right_zoomed - m_left_zoomed,
			m_zoomedsize.cy - m_bottom_zoomed - m_top_zoomed,
			m_left_zoomed, m_top_zoomed);
	  }
	  else
	  {
		sprintf(buf, " %dx%d+%d+%d",
			pDoc->m_image_width - m_right - m_left,
			pDoc->m_image_height - m_bottom - m_top,
			m_left, m_top);
	  }
	  ((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.GetPaneInfo(1, paru[0], paru[1], pari[0]);
	  ((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.SetPaneInfo(1, ID_INDICATOR_POS, SBPS_NORMAL, pari[0]);
	  ((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.SetPaneText(1, buf, TRUE);
	  OnUpdate(NULL);
	  break;
	}
  }
  else
  {
	int x1, y1, x2, y2;
	x1 = x_zoomed;
	y1 = y_zoomed;
	x2 = x1 + 1;
	y2 = y1 + 1;
	x1 -= m_left_zoomed;
	y1 -= m_top_zoomed;
	x2 -= m_zoomedsize.cx - m_right_zoomed;
	y2 -= m_zoomedsize.cy - m_bottom_zoomed;
	if (x1 < 0) x1 = -x1;
	if (x2 < 0) x2 = -x2;
	if (y1 < 0) y1 = -y1;
	if (y2 < 0) y2 = -y2;

	if (x1 <= 1 && y1 <= 1)
	{
	  SetCursor(LoadCursor(NULL, IDC_SIZENWSE));
	  m_flag = 8;
	}
	else if (x2 <= 1 && y1 <= 1)
	{
	  SetCursor(LoadCursor(NULL, IDC_SIZENESW));
	  m_flag = 9;
	}
	else if (x2 <= 1 && y2 <= 1)
	{
	  SetCursor(LoadCursor(NULL, IDC_SIZENWSE));
	  m_flag = 10;
	}
	else if (x1 <= 1 && y2 <= 1)
	{
	  SetCursor(LoadCursor(NULL, IDC_SIZENESW));
	  m_flag = 11;
	}
	else if (x1 <= 1)
	{
	  SetCursor(LoadCursor(NULL, IDC_SIZEWE));
	  m_flag = 4;
	}
	else if (y1 <= 1)
	{
	  SetCursor(LoadCursor(NULL, IDC_SIZENS));
	  m_flag = 5;
	}
	else if (x2 <= 1)
	{
	  SetCursor(LoadCursor(NULL, IDC_SIZEWE));
	  m_flag = 6;
	}
	else if (y2 <= 1)
	{
	  SetCursor(LoadCursor(NULL, IDC_SIZENS));
	  m_flag = 7;
	}
	else if (m_flag)
	{
	  SetCursor(LoadCursor(NULL, IDC_ARROW));
	  m_flag = 0;
	}
  }
}


void CDibView::OnPopup1MoveFrame()
{
  SetCursor(LoadCursor(NULL, IDC_SIZEALL));
  m_flag = 100;
}


static jpeg_transform_info cropinfo;


class CFrameDlg : public CDialog
{
public:
	CFrameDlg() : CDialog(CFrameDlg::IDD)
		{
			//{{AFX_DATA_INIT(CFrameDlg)
			//}}AFX_DATA_INIT
		}
// Dialog Data
	//{{AFX_DATA(CFrameDlg)
		enum { IDD = IDD_FRAMEBOX };
	//}}AFX_DATA
	int m_width;
	int m_height;
	int m_xoffset;
	int m_yoffset;
// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCheck1();
	virtual void OnRadio1();
	virtual void OnRadio2();
	//{{AFX_MSG(CFrameDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

void CFrameDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefDlg)
	//}}AFX_DATA_MAP
}

BOOL CFrameDlg::OnInitDialog()
{
	char buffer[128];
	int width;
	int height;
	int xoffset;
	int yoffset;
	if (((CDibLookApp *)AfxGetApp())->m_actual)
	{
		((CButton *)GetDlgItem(IDC_RADIO2))->SetCheck(TRUE);
		width = m_width;
		height = m_height;
		xoffset = m_xoffset;
		yoffset = m_yoffset;
	}
	else
	{
		((CButton *)GetDlgItem(IDC_RADIO1))->SetCheck(TRUE);
		width = ((CDibLookApp *)AfxGetApp())->m_width;
		height = ((CDibLookApp *)AfxGetApp())->m_height;
		xoffset = ((CDibLookApp *)AfxGetApp())->m_xoffset;
		yoffset = ((CDibLookApp *)AfxGetApp())->m_yoffset;
	}
	sprintf(buffer, "%d", width);
	SetDlgItemText(IDC_EDIT1, buffer);
	sprintf(buffer, "%d", height);
	SetDlgItemText(IDC_EDIT2, buffer);
	if (xoffset < 0)
		buffer[0] = 0;
	else
		sprintf(buffer, "%d", xoffset);
	SetDlgItemText(IDC_EDIT3, buffer);
	if (yoffset < 0)
		buffer[0] = 0;
	else
		sprintf(buffer, "%d", yoffset);
	SetDlgItemText(IDC_EDIT4, buffer);
	if (xoffset < 0)
		sprintf(buffer, "%dx%d", width, height);
	else if (yoffset < 0)
		sprintf(buffer, "%dx%d+%d", width, height, xoffset);
	else
		sprintf(buffer, "%dx%d+%d+%d", width, height, xoffset, yoffset);
	SetDlgItemText(IDC_EDIT5, buffer);
	if (((CDibLookApp *)AfxGetApp())->m_cropspec)
	{
		((CButton *)GetDlgItem(IDC_CHECK1))->SetCheck(TRUE);
		GetDlgItem(IDC_EDIT1)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT2)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT3)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT4)->EnableWindow(FALSE);
	}
	else
		GetDlgItem(IDC_EDIT5)->EnableWindow(FALSE);
	CDialog::OnInitDialog();
	return TRUE;
}

void CFrameDlg::OnOK()
{
	CDialog::OnOK();
	char buffer[128];
	if (((CButton *)GetDlgItem(IDC_RADIO1))->GetCheck())
		((CDibLookApp *)AfxGetApp())->m_actual = 0;
	else
		((CDibLookApp *)AfxGetApp())->m_actual = 1;
	if (((CButton *)GetDlgItem(IDC_CHECK1))->GetCheck())
	{
		GetDlgItemText(IDC_EDIT5, buffer, sizeof(buffer));
		if (jtransform_parse_crop_spec(&cropinfo, buffer))
		{
			if (cropinfo.crop_width_set == JCROP_POS)
			{
				sprintf(buffer, "%d", cropinfo.crop_width);
				SetDlgItemText(IDC_EDIT1, buffer);
			}
			if (cropinfo.crop_height_set == JCROP_POS)
			{
				sprintf(buffer, "%d", cropinfo.crop_height);
				SetDlgItemText(IDC_EDIT2, buffer);
			}
			if (cropinfo.crop_xoffset_set == JCROP_POS)
			{
				sprintf(buffer, "%d", cropinfo.crop_xoffset);
				SetDlgItemText(IDC_EDIT3, buffer);
			}
			if (cropinfo.crop_yoffset_set == JCROP_POS)
			{
				sprintf(buffer, "%d", cropinfo.crop_yoffset);
				SetDlgItemText(IDC_EDIT4, buffer);
			}
		}
		((CDibLookApp *)AfxGetApp())->m_cropspec = 1;
	}
	else
		((CDibLookApp *)AfxGetApp())->m_cropspec = 0;
	GetDlgItemText(IDC_EDIT1, buffer, sizeof(buffer));
	((CDibLookApp *)AfxGetApp())->m_width = atoi(buffer);
	GetDlgItemText(IDC_EDIT2, buffer, sizeof(buffer));
	((CDibLookApp *)AfxGetApp())->m_height = atoi(buffer);
	GetDlgItemText(IDC_EDIT3, buffer, sizeof(buffer));
	if (buffer[0])
		((CDibLookApp *)AfxGetApp())->m_xoffset = atoi(buffer);
	GetDlgItemText(IDC_EDIT4, buffer, sizeof(buffer));
	if (buffer[0])
		((CDibLookApp *)AfxGetApp())->m_yoffset = atoi(buffer);
}

void CFrameDlg::OnCheck1()
{
	char buffer1[32];
	char buffer2[32];
	char buffer3[32];
	char buffer4[32];
	char buf[64];

	if (((CButton *)GetDlgItem(IDC_CHECK1))->GetCheck())
	{
		GetDlgItemText(IDC_EDIT1, buffer1, sizeof(buffer1));
		GetDlgItemText(IDC_EDIT2, buffer2, sizeof(buffer2));
		GetDlgItemText(IDC_EDIT3, buffer3, sizeof(buffer3));
		GetDlgItemText(IDC_EDIT4, buffer4, sizeof(buffer4));
		if (buffer3[0] == 0)
			sprintf(buf, "%sx%s", buffer1, buffer2);
		else if (buffer4[0] == 0)
			sprintf(buf, "%sx%s+%s", buffer1, buffer2, buffer3);
		else
			sprintf(buf, "%sx%s+%s+%s", buffer1, buffer2, buffer3, buffer4);
		SetDlgItemText(IDC_EDIT5, buf);
		GetDlgItem(IDC_EDIT1)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT2)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT3)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT4)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT5)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItemText(IDC_EDIT5, buf, sizeof(buf));
		if (jtransform_parse_crop_spec(&cropinfo, buf))
		{
			if (cropinfo.crop_width_set == JCROP_POS)
			{
				sprintf(buffer1, "%d", cropinfo.crop_width);
				SetDlgItemText(IDC_EDIT1, buffer1);
			}
			if (cropinfo.crop_height_set == JCROP_POS)
			{
				sprintf(buffer2, "%d", cropinfo.crop_height);
				SetDlgItemText(IDC_EDIT2, buffer2);
			}
			if (cropinfo.crop_xoffset_set == JCROP_POS)
			{
				sprintf(buffer3, "%d", cropinfo.crop_xoffset);
				SetDlgItemText(IDC_EDIT3, buffer3);
			}
			if (cropinfo.crop_yoffset_set == JCROP_POS)
			{
				sprintf(buffer4, "%d", cropinfo.crop_yoffset);
				SetDlgItemText(IDC_EDIT4, buffer4);
			}
		}
		GetDlgItem(IDC_EDIT1)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT2)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT3)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT4)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT5)->EnableWindow(FALSE);
	}
}

void CFrameDlg::OnRadio1()
{
	char buffer[128];
	int width;
	int height;
	int xoffset;
	int yoffset;
	width = ((CDibLookApp *)AfxGetApp())->m_width;
	height = ((CDibLookApp *)AfxGetApp())->m_height;
	xoffset = ((CDibLookApp *)AfxGetApp())->m_xoffset;
	yoffset = ((CDibLookApp *)AfxGetApp())->m_yoffset;
	sprintf(buffer, "%d", width);
	SetDlgItemText(IDC_EDIT1, buffer);
	sprintf(buffer, "%d", height);
	SetDlgItemText(IDC_EDIT2, buffer);
	if (xoffset < 0)
		buffer[0] = 0;
	else
		sprintf(buffer, "%d", xoffset);
	SetDlgItemText(IDC_EDIT3, buffer);
	if (yoffset < 0)
		buffer[0] = 0;
	else
		sprintf(buffer, "%d", yoffset);
	SetDlgItemText(IDC_EDIT4, buffer);
	if (xoffset < 0)
		sprintf(buffer, "%dx%d", width, height);
	else if (yoffset < 0)
		sprintf(buffer, "%dx%d+%d", width, height, xoffset);
	else
		sprintf(buffer, "%dx%d+%d+%d", width, height, xoffset, yoffset);
	SetDlgItemText(IDC_EDIT5, buffer);
}

void CFrameDlg::OnRadio2()
{
	char buffer[128];
	int width;
	int height;
	int xoffset;
	int yoffset;
	width = m_width;
	height = m_height;
	xoffset = m_xoffset;
	yoffset = m_yoffset;
	sprintf(buffer, "%d", width);
	SetDlgItemText(IDC_EDIT1, buffer);
	sprintf(buffer, "%d", height);
	SetDlgItemText(IDC_EDIT2, buffer);
	if (xoffset < 0)
		buffer[0] = 0;
	else
		sprintf(buffer, "%d", xoffset);
	SetDlgItemText(IDC_EDIT3, buffer);
	if (yoffset < 0)
		buffer[0] = 0;
	else
		sprintf(buffer, "%d", yoffset);
	SetDlgItemText(IDC_EDIT4, buffer);
	if (xoffset < 0)
		sprintf(buffer, "%dx%d", width, height);
	else if (yoffset < 0)
		sprintf(buffer, "%dx%d+%d", width, height, xoffset);
	else
		sprintf(buffer, "%dx%d+%d+%d", width, height, xoffset, yoffset);
	SetDlgItemText(IDC_EDIT5, buffer);
}

BEGIN_MESSAGE_MAP(CFrameDlg, CDialog)
	//{{AFX_MSG_MAP(CFrameDlg)
	ON_BN_CLICKED(IDC_CHECK1, OnCheck1)
	ON_BN_CLICKED(IDC_RADIO1, OnRadio1)
	ON_BN_CLICKED(IDC_RADIO2, OnRadio2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CDibView::OnPopup1DefineFrame()
{
	CDibLookApp *myapp = (CDibLookApp *) AfxGetApp();
	CDibDoc *pDoc = GetDocument();

	CFrameDlg frameDlg;

	if (m_save_zoom)
	{
		frameDlg.m_width = m_zoomedsize.cx - m_right_zoomed - m_left_zoomed;
		frameDlg.m_height = m_zoomedsize.cy - m_bottom_zoomed - m_top_zoomed;
		frameDlg.m_xoffset = m_left_zoomed;
		frameDlg.m_yoffset = m_top_zoomed;
	}
	else
	{
		frameDlg.m_width = pDoc->m_image_width - m_right - m_left;
		frameDlg.m_height = pDoc->m_image_height - m_bottom - m_top;
		frameDlg.m_xoffset = m_left;
		frameDlg.m_yoffset = m_top;
	}

	if (frameDlg.DoModal() != IDOK) return;

	int scale = GetDocument()->m_scale;
	int scale_denom = GetDocument()->m_scale_denom;
	if (scale == scale_denom) scale = m_zoomval;

	if (m_save_zoom)
	{
		if (myapp->m_xoffset >= 0 && myapp->m_xoffset < m_zoomedsize.cx)
			m_left_zoomed = myapp->m_xoffset;
		if (myapp->m_yoffset >= 0 && myapp->m_yoffset < m_zoomedsize.cy)
			m_top_zoomed = myapp->m_yoffset;
		if (scale == scale_denom)
		{
			m_left = m_left_zoomed;
			m_top = m_top_zoomed;
		}
		else
		{
			m_left = (m_left_zoomed * scale_denom + ((scale - 1) >> 1))
				/ scale;
			m_top = (m_top_zoomed * scale_denom + ((scale - 1) >> 1))
				/ scale;
		}
		if (m_left >= pDoc->m_image_width) m_left = pDoc->m_image_width - 1;
		if (m_top >= pDoc->m_image_height) m_top = pDoc->m_image_height - 1;
		m_left -= m_left % pDoc->m_MCUwidth;
		m_top -= m_top % pDoc->m_MCUheight;
		if (scale == scale_denom)
		{
			m_left_zoomed = m_left;
			m_top_zoomed = m_top;
		}
		else
		{
			m_left_zoomed -=
				m_left_zoomed % ((pDoc->m_MCUwidth * scale) / scale_denom);
			m_top_zoomed -=
				m_top_zoomed % ((pDoc->m_MCUheight * scale) / scale_denom);
		}
		int x_zoomed = m_left_zoomed + myapp->m_width - 1;
		int y_zoomed = m_top_zoomed + myapp->m_height - 1;
		int x, y;
		if (scale == scale_denom)
		{
			x = x_zoomed;
			y = y_zoomed;
		}
		else
		{
			x = (x_zoomed * scale_denom + ((scale - 1) >> 1)) / scale;
			y = (y_zoomed * scale_denom + ((scale - 1) >> 1)) / scale;
		}
		if (x < m_left) x = m_left;
		if (y < m_top) y = m_top;
		if (x_zoomed < m_left_zoomed) x_zoomed = m_left_zoomed;
		if (y_zoomed < m_top_zoomed) y_zoomed = m_top_zoomed;
		if (myapp->m_endpoint_snap)
		{
			x -= x % pDoc->m_MCUwidth;
			x += pDoc->m_MCUwidth - 1;
			y -= y % pDoc->m_MCUheight;
			y += pDoc->m_MCUheight - 1;
			if (scale == scale_denom)
			{
				x_zoomed = x;
				y_zoomed = y;
			}
			else
			{
				x_zoomed -=
					x_zoomed % ((pDoc->m_MCUwidth * scale) / scale_denom);
				x_zoomed += (pDoc->m_MCUwidth * scale) / scale_denom - 1;
				y_zoomed -=
					y_zoomed % ((pDoc->m_MCUheight * scale) / scale_denom);
				y_zoomed += (pDoc->m_MCUheight * scale) / scale_denom - 1;
			}
		}
		if (x >= pDoc->m_image_width) x = pDoc->m_image_width - 1;
		if (y >= pDoc->m_image_height) y = pDoc->m_image_height - 1;
		if (x_zoomed >= m_zoomedsize.cx) x_zoomed = m_zoomedsize.cx - 1;
		if (y_zoomed >= m_zoomedsize.cy) y_zoomed = m_zoomedsize.cy - 1;
		if (myapp->m_trim)
		{
			switch (pDoc->m_transform)
			{
			case 1:
			case 5:
				if (x >= pDoc->m_image_width - (pDoc->m_image_width % pDoc->m_MCUwidth) &&
					pDoc->m_image_width - (pDoc->m_image_width % pDoc->m_MCUwidth) > m_left)
				{
					x = pDoc->m_image_width - (pDoc->m_image_width % pDoc->m_MCUwidth);
					x_zoomed = (x * scale) / scale_denom - 1;
					x--;
				}
				break;
			case 2:
			case 7:
				if (y >= pDoc->m_image_height - (pDoc->m_image_height % pDoc->m_MCUheight) &&
					pDoc->m_image_height - (pDoc->m_image_height % pDoc->m_MCUheight) > m_top)
				{
					y = pDoc->m_image_height - (pDoc->m_image_height % pDoc->m_MCUheight);
					y_zoomed = (y * scale) / scale_denom - 1;
					y--;
				}
				break;
			case 4:
			case 6:
				if (x >= pDoc->m_image_width - (pDoc->m_image_width % pDoc->m_MCUwidth) &&
					pDoc->m_image_width - (pDoc->m_image_width % pDoc->m_MCUwidth) > m_left)
				{
					x = pDoc->m_image_width - (pDoc->m_image_width % pDoc->m_MCUwidth);
					x_zoomed = (x * scale) / scale_denom - 1;
					x--;
				}
				if (y >= pDoc->m_image_height - (pDoc->m_image_height % pDoc->m_MCUheight) &&
					pDoc->m_image_height - (pDoc->m_image_height % pDoc->m_MCUheight) > m_top)
				{
					y = pDoc->m_image_height - (pDoc->m_image_height % pDoc->m_MCUheight);
					y_zoomed = (y * scale) / scale_denom - 1;
					y--;
				}
				break;
			}
		}
		m_right = pDoc->m_image_width - 1 - x;
		m_bottom = pDoc->m_image_height - 1 - y;
		m_right_zoomed = m_zoomedsize.cx - 1 - x_zoomed;
		m_bottom_zoomed = m_zoomedsize.cy - 1 - y_zoomed;
	}
	else
	{
		if (myapp->m_xoffset >= 0 && myapp->m_xoffset < pDoc->m_image_width)
		{
			myapp->m_xoffset -= myapp->m_xoffset % pDoc->m_MCUwidth;
			m_left = myapp->m_xoffset;
		}
		if (myapp->m_yoffset >= 0 && myapp->m_yoffset < pDoc->m_image_height)
		{
			myapp->m_yoffset -= myapp->m_yoffset % pDoc->m_MCUheight;
			m_top = myapp->m_yoffset;
		}
		if (myapp->m_width <= 0 || myapp->m_width > pDoc->m_image_width)
			myapp->m_width = pDoc->m_image_width;
		m_right = pDoc->m_image_width - m_left - myapp->m_width;
		if (myapp->m_height <= 0 || myapp->m_height > pDoc->m_image_height)
			myapp->m_height = pDoc->m_image_height;
		m_bottom = pDoc->m_image_height - m_top - myapp->m_height;
		int lim_x = 0;
		int lim_y = 0;
		if (myapp->m_trim)
		{
			switch (pDoc->m_transform)
			{
			case 1:
			case 5:
				if (myapp->m_width > pDoc->m_image_width % pDoc->m_MCUwidth)
					lim_x = pDoc->m_image_width % pDoc->m_MCUwidth;
				break;
			case 2:
			case 7:
				if (myapp->m_height > pDoc->m_image_height % pDoc->m_MCUheight)
					lim_y = pDoc->m_image_height % pDoc->m_MCUheight;
				break;
			case 4:
			case 6:
				if (myapp->m_width > pDoc->m_image_width % pDoc->m_MCUwidth)
					lim_x = pDoc->m_image_width % pDoc->m_MCUwidth;
				if (myapp->m_height > pDoc->m_image_height % pDoc->m_MCUheight)
					lim_y = pDoc->m_image_height % pDoc->m_MCUheight;
				break;
			}
		}
		if (m_right < lim_x)
		{
			m_left += m_right - lim_x;
			if (m_left < 0) m_left = 0;
			m_right = lim_x + (m_left % pDoc->m_MCUwidth);
			m_left -= m_right - lim_x;
		}
		if (m_bottom < lim_y)
		{
			m_top += m_bottom - lim_y;
			if (m_top < 0) m_top = 0;
			m_bottom = lim_y + (m_top % pDoc->m_MCUheight);
			m_top -= m_bottom - lim_y;
		}
		if (scale == scale_denom)
		{
			m_left_zoomed = m_left;
			m_top_zoomed = m_top;
			m_right_zoomed = m_right;
			m_bottom_zoomed = m_bottom;
		}
		else
		{
			m_left_zoomed = (m_left * scale
				+ scale_denom - 1)
				/ scale_denom;
			m_top_zoomed = (m_top * scale
				+ scale_denom - 1)
				/ scale_denom;
			m_right_zoomed = m_zoomedsize.cx -
				((pDoc->m_image_width - m_right) * scale
				+ scale_denom - 1)
				/ scale_denom;
			m_bottom_zoomed = m_zoomedsize.cy -
				((pDoc->m_image_height - m_bottom) * scale
				+ scale_denom - 1)
				/ scale_denom;
		}
	}

	char buf[64];
	unsigned int paru[2];
	int pari[2];
	if (m_save_zoom)
	{
		sprintf(buf, " %dx%d+%d+%d",
			m_zoomedsize.cx - m_right_zoomed - m_left_zoomed,
			m_zoomedsize.cy - m_bottom_zoomed - m_top_zoomed,
			m_left_zoomed, m_top_zoomed);
	}
	else
	{
		sprintf(buf, " %dx%d+%d+%d",
			pDoc->m_image_width - m_right - m_left,
			pDoc->m_image_height - m_bottom - m_top,
			m_left, m_top);
	}
	((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.GetPaneInfo(1, paru[0], paru[1], pari[0]);
	((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.SetPaneInfo(1, ID_INDICATOR_POS, SBPS_NORMAL, pari[0]);
	((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.SetPaneText(1, buf, TRUE);

	OnUpdate(NULL);
}


void CDibView::OnPopup1SaveFrameAs()
{
  GetDocument()->DoSave(NULL, FALSE);
}


void CDibView::OnViewZoom1()
{
	CDibLookApp *myapp = (CDibLookApp *) AfxGetApp();
	CDibDoc *pDoc = GetDocument();

	if (myapp->m_zoom_reload)
	{
		if (pDoc->m_scale == 1) return;
		BeginWaitCursor();
		pDoc->ReplaceHDIB(NULL);

		HDIB hDIB = (HDIB) Read_JPEG_File(pDoc->GetPathName(),
			1, pDoc->m_force_grayscale,
			&pDoc->m_image_width, &pDoc->m_image_height,
			&pDoc->m_MCUwidth, &pDoc->m_MCUheight,
			&pDoc->m_scale_denom);

		pDoc->ReplaceHDIB(hDIB);
		pDoc->m_dithered = 0;
		pDoc->m_scale = 1;
		pDoc->DoTransform();
		pDoc->InitDIBData();
		EndWaitCursor();
		pDoc->UpdateAllViews(NULL, pDoc->m_scale_denom + 3);
	}
	else if (pDoc->m_scale != pDoc->m_scale_denom)
	{
		BeginWaitCursor();
		pDoc->ReplaceHDIB(NULL);

		HDIB hDIB = (HDIB) Read_JPEG_File(pDoc->GetPathName(),
			0, pDoc->m_force_grayscale,
			&pDoc->m_image_width, &pDoc->m_image_height,
			&pDoc->m_MCUwidth, &pDoc->m_MCUheight,
			&pDoc->m_scale_denom);

		pDoc->ReplaceHDIB(hDIB);
		pDoc->m_dithered = 0;
		pDoc->m_scale = pDoc->m_scale_denom;
		pDoc->DoTransform();
		pDoc->InitDIBData();
		EndWaitCursor();
		pDoc->UpdateAllViews(NULL, 4);
	}
	else
	{
		if (m_zoomval == 1) return;
		OnUpdate(NULL, 4);
	}
}

void CDibView::OnViewZoom2()
{
	CDibLookApp *myapp = (CDibLookApp *) AfxGetApp();
	CDibDoc *pDoc = GetDocument();

	if (myapp->m_zoom_reload)
	{
		if (pDoc->m_scale == 2) return;
		BeginWaitCursor();
		pDoc->ReplaceHDIB(NULL);

		HDIB hDIB = (HDIB) Read_JPEG_File(pDoc->GetPathName(),
			2, pDoc->m_force_grayscale,
			&pDoc->m_image_width, &pDoc->m_image_height,
			&pDoc->m_MCUwidth, &pDoc->m_MCUheight,
			&pDoc->m_scale_denom);

		pDoc->ReplaceHDIB(hDIB);
		pDoc->m_dithered = 0;
		pDoc->m_scale = 2;
		pDoc->DoTransform();
		pDoc->InitDIBData();
		EndWaitCursor();
		pDoc->UpdateAllViews(NULL, pDoc->m_scale_denom + 3);
	}
	else if (pDoc->m_scale != pDoc->m_scale_denom)
	{
		BeginWaitCursor();
		pDoc->ReplaceHDIB(NULL);

		HDIB hDIB = (HDIB) Read_JPEG_File(pDoc->GetPathName(),
			0, pDoc->m_force_grayscale,
			&pDoc->m_image_width, &pDoc->m_image_height,
			&pDoc->m_MCUwidth, &pDoc->m_MCUheight,
			&pDoc->m_scale_denom);

		pDoc->ReplaceHDIB(hDIB);
		pDoc->m_dithered = 0;
		pDoc->m_scale = pDoc->m_scale_denom;
		pDoc->DoTransform();
		pDoc->InitDIBData();
		EndWaitCursor();
		pDoc->UpdateAllViews(NULL, 5);
	}
	else
	{
		if (m_zoomval == 2) return;
		OnUpdate(NULL, 5);
	}
}

void CDibView::OnViewZoom3()
{
	CDibLookApp *myapp = (CDibLookApp *) AfxGetApp();
	CDibDoc *pDoc = GetDocument();

	if (myapp->m_zoom_reload)
	{
		if (pDoc->m_scale == 3) return;
		BeginWaitCursor();
		pDoc->ReplaceHDIB(NULL);

		HDIB hDIB = (HDIB) Read_JPEG_File(pDoc->GetPathName(),
			3, pDoc->m_force_grayscale,
			&pDoc->m_image_width, &pDoc->m_image_height,
			&pDoc->m_MCUwidth, &pDoc->m_MCUheight,
			&pDoc->m_scale_denom);

		pDoc->ReplaceHDIB(hDIB);
		pDoc->m_dithered = 0;
		pDoc->m_scale = 3;
		pDoc->DoTransform();
		pDoc->InitDIBData();
		EndWaitCursor();
		pDoc->UpdateAllViews(NULL, pDoc->m_scale_denom + 3);
	}
	else if (pDoc->m_scale != pDoc->m_scale_denom)
	{
		BeginWaitCursor();
		pDoc->ReplaceHDIB(NULL);

		HDIB hDIB = (HDIB) Read_JPEG_File(pDoc->GetPathName(),
			0, pDoc->m_force_grayscale,
			&pDoc->m_image_width, &pDoc->m_image_height,
			&pDoc->m_MCUwidth, &pDoc->m_MCUheight,
			&pDoc->m_scale_denom);

		pDoc->ReplaceHDIB(hDIB);
		pDoc->m_dithered = 0;
		pDoc->m_scale = pDoc->m_scale_denom;
		pDoc->DoTransform();
		pDoc->InitDIBData();
		EndWaitCursor();
		pDoc->UpdateAllViews(NULL, 6);
	}
	else
	{
		if (m_zoomval == 3) return;
		OnUpdate(NULL, 6);
	}
}

void CDibView::OnViewZoom4()
{
	CDibLookApp *myapp = (CDibLookApp *) AfxGetApp();
	CDibDoc *pDoc = GetDocument();

	if (myapp->m_zoom_reload)
	{
		if (pDoc->m_scale == 4) return;
		BeginWaitCursor();
		pDoc->ReplaceHDIB(NULL);

		HDIB hDIB = (HDIB) Read_JPEG_File(pDoc->GetPathName(),
			4, pDoc->m_force_grayscale,
			&pDoc->m_image_width, &pDoc->m_image_height,
			&pDoc->m_MCUwidth, &pDoc->m_MCUheight,
			&pDoc->m_scale_denom);

		pDoc->ReplaceHDIB(hDIB);
		pDoc->m_dithered = 0;
		pDoc->m_scale = 4;
		pDoc->DoTransform();
		pDoc->InitDIBData();
		EndWaitCursor();
		pDoc->UpdateAllViews(NULL, pDoc->m_scale_denom + 3);
	}
	else if (pDoc->m_scale != pDoc->m_scale_denom)
	{
		BeginWaitCursor();
		pDoc->ReplaceHDIB(NULL);

		HDIB hDIB = (HDIB) Read_JPEG_File(pDoc->GetPathName(),
			0, pDoc->m_force_grayscale,
			&pDoc->m_image_width, &pDoc->m_image_height,
			&pDoc->m_MCUwidth, &pDoc->m_MCUheight,
			&pDoc->m_scale_denom);

		pDoc->ReplaceHDIB(hDIB);
		pDoc->m_dithered = 0;
		pDoc->m_scale = pDoc->m_scale_denom;
		pDoc->DoTransform();
		pDoc->InitDIBData();
		EndWaitCursor();
		pDoc->UpdateAllViews(NULL, 7);
	}
	else
	{
		if (m_zoomval == 4) return;
		OnUpdate(NULL, 7);
	}
}

void CDibView::OnViewZoom5()
{
	CDibLookApp *myapp = (CDibLookApp *) AfxGetApp();
	CDibDoc *pDoc = GetDocument();

	if (myapp->m_zoom_reload)
	{
		if (pDoc->m_scale == 5) return;
		BeginWaitCursor();
		pDoc->ReplaceHDIB(NULL);

		HDIB hDIB = (HDIB) Read_JPEG_File(pDoc->GetPathName(),
			5, pDoc->m_force_grayscale,
			&pDoc->m_image_width, &pDoc->m_image_height,
			&pDoc->m_MCUwidth, &pDoc->m_MCUheight,
			&pDoc->m_scale_denom);

		pDoc->ReplaceHDIB(hDIB);
		pDoc->m_dithered = 0;
		pDoc->m_scale = 5;
		pDoc->DoTransform();
		pDoc->InitDIBData();
		EndWaitCursor();
		pDoc->UpdateAllViews(NULL, pDoc->m_scale_denom + 3);
	}
	else if (pDoc->m_scale != pDoc->m_scale_denom)
	{
		BeginWaitCursor();
		pDoc->ReplaceHDIB(NULL);

		HDIB hDIB = (HDIB) Read_JPEG_File(pDoc->GetPathName(),
			0, pDoc->m_force_grayscale,
			&pDoc->m_image_width, &pDoc->m_image_height,
			&pDoc->m_MCUwidth, &pDoc->m_MCUheight,
			&pDoc->m_scale_denom);

		pDoc->ReplaceHDIB(hDIB);
		pDoc->m_dithered = 0;
		pDoc->m_scale = pDoc->m_scale_denom;
		pDoc->DoTransform();
		pDoc->InitDIBData();
		EndWaitCursor();
		pDoc->UpdateAllViews(NULL, 8);
	}
	else
	{
		if (m_zoomval == 5) return;
		OnUpdate(NULL, 8);
	}
}

void CDibView::OnViewZoom6()
{
	CDibLookApp *myapp = (CDibLookApp *) AfxGetApp();
	CDibDoc *pDoc = GetDocument();

	if (myapp->m_zoom_reload)
	{
		if (pDoc->m_scale == 6) return;
		BeginWaitCursor();
		pDoc->ReplaceHDIB(NULL);

		HDIB hDIB = (HDIB) Read_JPEG_File(pDoc->GetPathName(),
			6, pDoc->m_force_grayscale,
			&pDoc->m_image_width, &pDoc->m_image_height,
			&pDoc->m_MCUwidth, &pDoc->m_MCUheight,
			&pDoc->m_scale_denom);

		pDoc->ReplaceHDIB(hDIB);
		pDoc->m_dithered = 0;
		pDoc->m_scale = 6;
		pDoc->DoTransform();
		pDoc->InitDIBData();
		EndWaitCursor();
		pDoc->UpdateAllViews(NULL, pDoc->m_scale_denom + 3);
	}
	else if (pDoc->m_scale != pDoc->m_scale_denom)
	{
		BeginWaitCursor();
		pDoc->ReplaceHDIB(NULL);

		HDIB hDIB = (HDIB) Read_JPEG_File(pDoc->GetPathName(),
			0, pDoc->m_force_grayscale,
			&pDoc->m_image_width, &pDoc->m_image_height,
			&pDoc->m_MCUwidth, &pDoc->m_MCUheight,
			&pDoc->m_scale_denom);

		pDoc->ReplaceHDIB(hDIB);
		pDoc->m_dithered = 0;
		pDoc->m_scale = pDoc->m_scale_denom;
		pDoc->DoTransform();
		pDoc->InitDIBData();
		EndWaitCursor();
		pDoc->UpdateAllViews(NULL, 9);
	}
	else
	{
		if (m_zoomval == 6) return;
		OnUpdate(NULL, 9);
	}
}

void CDibView::OnViewZoom7()
{
	CDibLookApp *myapp = (CDibLookApp *) AfxGetApp();
	CDibDoc *pDoc = GetDocument();

	if (myapp->m_zoom_reload)
	{
		if (pDoc->m_scale == 7) return;
		BeginWaitCursor();
		pDoc->ReplaceHDIB(NULL);

		HDIB hDIB = (HDIB) Read_JPEG_File(pDoc->GetPathName(),
			7, pDoc->m_force_grayscale,
			&pDoc->m_image_width, &pDoc->m_image_height,
			&pDoc->m_MCUwidth, &pDoc->m_MCUheight,
			&pDoc->m_scale_denom);

		pDoc->ReplaceHDIB(hDIB);
		pDoc->m_dithered = 0;
		pDoc->m_scale = 7;
		pDoc->DoTransform();
		pDoc->InitDIBData();
		EndWaitCursor();
		pDoc->UpdateAllViews(NULL, pDoc->m_scale_denom + 3);
	}
	else if (pDoc->m_scale != pDoc->m_scale_denom)
	{
		BeginWaitCursor();
		pDoc->ReplaceHDIB(NULL);

		HDIB hDIB = (HDIB) Read_JPEG_File(pDoc->GetPathName(),
			0, pDoc->m_force_grayscale,
			&pDoc->m_image_width, &pDoc->m_image_height,
			&pDoc->m_MCUwidth, &pDoc->m_MCUheight,
			&pDoc->m_scale_denom);

		pDoc->ReplaceHDIB(hDIB);
		pDoc->m_dithered = 0;
		pDoc->m_scale = pDoc->m_scale_denom;
		pDoc->DoTransform();
		pDoc->InitDIBData();
		EndWaitCursor();
		pDoc->UpdateAllViews(NULL, 10);
	}
	else
	{
		if (m_zoomval == 7) return;
		OnUpdate(NULL, 10);
	}
}

void CDibView::OnViewZoom8()
{
	CDibLookApp *myapp = (CDibLookApp *) AfxGetApp();
	CDibDoc *pDoc = GetDocument();

	if (myapp->m_zoom_reload)
	{
		if (pDoc->m_scale == 8) return;
		BeginWaitCursor();
		pDoc->ReplaceHDIB(NULL);

		HDIB hDIB = (HDIB) Read_JPEG_File(pDoc->GetPathName(),
			8, pDoc->m_force_grayscale,
			&pDoc->m_image_width, &pDoc->m_image_height,
			&pDoc->m_MCUwidth, &pDoc->m_MCUheight,
			&pDoc->m_scale_denom);

		pDoc->ReplaceHDIB(hDIB);
		pDoc->m_dithered = 0;
		pDoc->m_scale = 8;
		pDoc->DoTransform();
		pDoc->InitDIBData();
		EndWaitCursor();
		pDoc->UpdateAllViews(NULL, pDoc->m_scale_denom + 3);
	}
	else if (pDoc->m_scale != pDoc->m_scale_denom)
	{
		BeginWaitCursor();
		pDoc->ReplaceHDIB(NULL);

		HDIB hDIB = (HDIB) Read_JPEG_File(pDoc->GetPathName(),
			0, pDoc->m_force_grayscale,
			&pDoc->m_image_width, &pDoc->m_image_height,
			&pDoc->m_MCUwidth, &pDoc->m_MCUheight,
			&pDoc->m_scale_denom);

		pDoc->ReplaceHDIB(hDIB);
		pDoc->m_dithered = 0;
		pDoc->m_scale = pDoc->m_scale_denom;
		pDoc->DoTransform();
		pDoc->InitDIBData();
		EndWaitCursor();
		pDoc->UpdateAllViews(NULL, 11);
	}
	else
	{
		if (m_zoomval == 8) return;
		OnUpdate(NULL, 11);
	}
}

void CDibView::OnViewZoom9()
{
	CDibLookApp *myapp = (CDibLookApp *) AfxGetApp();
	CDibDoc *pDoc = GetDocument();

	if (myapp->m_zoom_reload)
	{
		if (pDoc->m_scale == 9) return;
		BeginWaitCursor();
		pDoc->ReplaceHDIB(NULL);

		HDIB hDIB = (HDIB) Read_JPEG_File(pDoc->GetPathName(),
			9, pDoc->m_force_grayscale,
			&pDoc->m_image_width, &pDoc->m_image_height,
			&pDoc->m_MCUwidth, &pDoc->m_MCUheight,
			&pDoc->m_scale_denom);

		pDoc->ReplaceHDIB(hDIB);
		pDoc->m_dithered = 0;
		pDoc->m_scale = 9;
		pDoc->DoTransform();
		pDoc->InitDIBData();
		EndWaitCursor();
		pDoc->UpdateAllViews(NULL, pDoc->m_scale_denom + 3);
	}
	else if (pDoc->m_scale != pDoc->m_scale_denom)
	{
		BeginWaitCursor();
		pDoc->ReplaceHDIB(NULL);

		HDIB hDIB = (HDIB) Read_JPEG_File(pDoc->GetPathName(),
			0, pDoc->m_force_grayscale,
			&pDoc->m_image_width, &pDoc->m_image_height,
			&pDoc->m_MCUwidth, &pDoc->m_MCUheight,
			&pDoc->m_scale_denom);

		pDoc->ReplaceHDIB(hDIB);
		pDoc->m_dithered = 0;
		pDoc->m_scale = pDoc->m_scale_denom;
		pDoc->DoTransform();
		pDoc->InitDIBData();
		EndWaitCursor();
		pDoc->UpdateAllViews(NULL, 12);
	}
	else
	{
		if (m_zoomval == 9) return;
		OnUpdate(NULL, 12);
	}
}

void CDibView::OnViewZoom10()
{
	CDibLookApp *myapp = (CDibLookApp *) AfxGetApp();
	CDibDoc *pDoc = GetDocument();

	if (myapp->m_zoom_reload)
	{
		if (pDoc->m_scale == 10) return;
		BeginWaitCursor();
		pDoc->ReplaceHDIB(NULL);

		HDIB hDIB = (HDIB) Read_JPEG_File(pDoc->GetPathName(),
			10, pDoc->m_force_grayscale,
			&pDoc->m_image_width, &pDoc->m_image_height,
			&pDoc->m_MCUwidth, &pDoc->m_MCUheight,
			&pDoc->m_scale_denom);

		pDoc->ReplaceHDIB(hDIB);
		pDoc->m_dithered = 0;
		pDoc->m_scale = 10;
		pDoc->DoTransform();
		pDoc->InitDIBData();
		EndWaitCursor();
		pDoc->UpdateAllViews(NULL, pDoc->m_scale_denom + 3);
	}
	else if (pDoc->m_scale != pDoc->m_scale_denom)
	{
		BeginWaitCursor();
		pDoc->ReplaceHDIB(NULL);

		HDIB hDIB = (HDIB) Read_JPEG_File(pDoc->GetPathName(),
			0, pDoc->m_force_grayscale,
			&pDoc->m_image_width, &pDoc->m_image_height,
			&pDoc->m_MCUwidth, &pDoc->m_MCUheight,
			&pDoc->m_scale_denom);

		pDoc->ReplaceHDIB(hDIB);
		pDoc->m_dithered = 0;
		pDoc->m_scale = pDoc->m_scale_denom;
		pDoc->DoTransform();
		pDoc->InitDIBData();
		EndWaitCursor();
		pDoc->UpdateAllViews(NULL, 13);
	}
	else
	{
		if (m_zoomval == 10) return;
		OnUpdate(NULL, 13);
	}
}

void CDibView::OnViewZoom11()
{
	CDibLookApp *myapp = (CDibLookApp *) AfxGetApp();
	CDibDoc *pDoc = GetDocument();

	if (myapp->m_zoom_reload)
	{
		if (pDoc->m_scale == 11) return;
		BeginWaitCursor();
		pDoc->ReplaceHDIB(NULL);

		HDIB hDIB = (HDIB) Read_JPEG_File(pDoc->GetPathName(),
			11, pDoc->m_force_grayscale,
			&pDoc->m_image_width, &pDoc->m_image_height,
			&pDoc->m_MCUwidth, &pDoc->m_MCUheight,
			&pDoc->m_scale_denom);

		pDoc->ReplaceHDIB(hDIB);
		pDoc->m_dithered = 0;
		pDoc->m_scale = 11;
		pDoc->DoTransform();
		pDoc->InitDIBData();
		EndWaitCursor();
		pDoc->UpdateAllViews(NULL, pDoc->m_scale_denom + 3);
	}
	else if (pDoc->m_scale != pDoc->m_scale_denom)
	{
		BeginWaitCursor();
		pDoc->ReplaceHDIB(NULL);

		HDIB hDIB = (HDIB) Read_JPEG_File(pDoc->GetPathName(),
			0, pDoc->m_force_grayscale,
			&pDoc->m_image_width, &pDoc->m_image_height,
			&pDoc->m_MCUwidth, &pDoc->m_MCUheight,
			&pDoc->m_scale_denom);

		pDoc->ReplaceHDIB(hDIB);
		pDoc->m_dithered = 0;
		pDoc->m_scale = pDoc->m_scale_denom;
		pDoc->DoTransform();
		pDoc->InitDIBData();
		EndWaitCursor();
		pDoc->UpdateAllViews(NULL, 14);
	}
	else
	{
		if (m_zoomval == 11) return;
		OnUpdate(NULL, 14);
	}
}

void CDibView::OnViewZoom12()
{
	CDibLookApp *myapp = (CDibLookApp *) AfxGetApp();
	CDibDoc *pDoc = GetDocument();

	if (myapp->m_zoom_reload)
	{
		if (pDoc->m_scale == 12) return;
		BeginWaitCursor();
		pDoc->ReplaceHDIB(NULL);

		HDIB hDIB = (HDIB) Read_JPEG_File(pDoc->GetPathName(),
			12, pDoc->m_force_grayscale,
			&pDoc->m_image_width, &pDoc->m_image_height,
			&pDoc->m_MCUwidth, &pDoc->m_MCUheight,
			&pDoc->m_scale_denom);

		pDoc->ReplaceHDIB(hDIB);
		pDoc->m_dithered = 0;
		pDoc->m_scale = 12;
		pDoc->DoTransform();
		pDoc->InitDIBData();
		EndWaitCursor();
		pDoc->UpdateAllViews(NULL, pDoc->m_scale_denom + 3);
	}
	else if (pDoc->m_scale != pDoc->m_scale_denom)
	{
		BeginWaitCursor();
		pDoc->ReplaceHDIB(NULL);

		HDIB hDIB = (HDIB) Read_JPEG_File(pDoc->GetPathName(),
			0, pDoc->m_force_grayscale,
			&pDoc->m_image_width, &pDoc->m_image_height,
			&pDoc->m_MCUwidth, &pDoc->m_MCUheight,
			&pDoc->m_scale_denom);

		pDoc->ReplaceHDIB(hDIB);
		pDoc->m_dithered = 0;
		pDoc->m_scale = pDoc->m_scale_denom;
		pDoc->DoTransform();
		pDoc->InitDIBData();
		EndWaitCursor();
		pDoc->UpdateAllViews(NULL, 15);
	}
	else
	{
		if (m_zoomval == 12) return;
		OnUpdate(NULL, 15);
	}
}

void CDibView::OnViewZoom13()
{
	CDibLookApp *myapp = (CDibLookApp *) AfxGetApp();
	CDibDoc *pDoc = GetDocument();

	if (myapp->m_zoom_reload)
	{
		if (pDoc->m_scale == 13) return;
		BeginWaitCursor();
		pDoc->ReplaceHDIB(NULL);

		HDIB hDIB = (HDIB) Read_JPEG_File(pDoc->GetPathName(),
			13, pDoc->m_force_grayscale,
			&pDoc->m_image_width, &pDoc->m_image_height,
			&pDoc->m_MCUwidth, &pDoc->m_MCUheight,
			&pDoc->m_scale_denom);

		pDoc->ReplaceHDIB(hDIB);
		pDoc->m_dithered = 0;
		pDoc->m_scale = 13;
		pDoc->DoTransform();
		pDoc->InitDIBData();
		EndWaitCursor();
		pDoc->UpdateAllViews(NULL, pDoc->m_scale_denom + 3);
	}
	else if (pDoc->m_scale != pDoc->m_scale_denom)
	{
		BeginWaitCursor();
		pDoc->ReplaceHDIB(NULL);

		HDIB hDIB = (HDIB) Read_JPEG_File(pDoc->GetPathName(),
			0, pDoc->m_force_grayscale,
			&pDoc->m_image_width, &pDoc->m_image_height,
			&pDoc->m_MCUwidth, &pDoc->m_MCUheight,
			&pDoc->m_scale_denom);

		pDoc->ReplaceHDIB(hDIB);
		pDoc->m_dithered = 0;
		pDoc->m_scale = pDoc->m_scale_denom;
		pDoc->DoTransform();
		pDoc->InitDIBData();
		EndWaitCursor();
		pDoc->UpdateAllViews(NULL, 16);
	}
	else
	{
		if (m_zoomval == 13) return;
		OnUpdate(NULL, 16);
	}
}

void CDibView::OnViewZoom14()
{
	CDibLookApp *myapp = (CDibLookApp *) AfxGetApp();
	CDibDoc *pDoc = GetDocument();

	if (myapp->m_zoom_reload)
	{
		if (pDoc->m_scale == 14) return;
		BeginWaitCursor();
		pDoc->ReplaceHDIB(NULL);

		HDIB hDIB = (HDIB) Read_JPEG_File(pDoc->GetPathName(),
			14, pDoc->m_force_grayscale,
			&pDoc->m_image_width, &pDoc->m_image_height,
			&pDoc->m_MCUwidth, &pDoc->m_MCUheight,
			&pDoc->m_scale_denom);

		pDoc->ReplaceHDIB(hDIB);
		pDoc->m_dithered = 0;
		pDoc->m_scale = 14;
		pDoc->DoTransform();
		pDoc->InitDIBData();
		EndWaitCursor();
		pDoc->UpdateAllViews(NULL, pDoc->m_scale_denom + 3);
	}
	else if (pDoc->m_scale != pDoc->m_scale_denom)
	{
		BeginWaitCursor();
		pDoc->ReplaceHDIB(NULL);

		HDIB hDIB = (HDIB) Read_JPEG_File(pDoc->GetPathName(),
			0, pDoc->m_force_grayscale,
			&pDoc->m_image_width, &pDoc->m_image_height,
			&pDoc->m_MCUwidth, &pDoc->m_MCUheight,
			&pDoc->m_scale_denom);

		pDoc->ReplaceHDIB(hDIB);
		pDoc->m_dithered = 0;
		pDoc->m_scale = pDoc->m_scale_denom;
		pDoc->DoTransform();
		pDoc->InitDIBData();
		EndWaitCursor();
		pDoc->UpdateAllViews(NULL, 17);
	}
	else
	{
		if (m_zoomval == 14) return;
		OnUpdate(NULL, 17);
	}
}

void CDibView::OnViewZoom15()
{
	CDibLookApp *myapp = (CDibLookApp *) AfxGetApp();
	CDibDoc *pDoc = GetDocument();

	if (myapp->m_zoom_reload)
	{
		if (pDoc->m_scale == 15) return;
		BeginWaitCursor();
		pDoc->ReplaceHDIB(NULL);

		HDIB hDIB = (HDIB) Read_JPEG_File(pDoc->GetPathName(),
			15, pDoc->m_force_grayscale,
			&pDoc->m_image_width, &pDoc->m_image_height,
			&pDoc->m_MCUwidth, &pDoc->m_MCUheight,
			&pDoc->m_scale_denom);

		pDoc->ReplaceHDIB(hDIB);
		pDoc->m_dithered = 0;
		pDoc->m_scale = 15;
		pDoc->DoTransform();
		pDoc->InitDIBData();
		EndWaitCursor();
		pDoc->UpdateAllViews(NULL, pDoc->m_scale_denom + 3);
	}
	else if (pDoc->m_scale != pDoc->m_scale_denom)
	{
		BeginWaitCursor();
		pDoc->ReplaceHDIB(NULL);

		HDIB hDIB = (HDIB) Read_JPEG_File(pDoc->GetPathName(),
			0, pDoc->m_force_grayscale,
			&pDoc->m_image_width, &pDoc->m_image_height,
			&pDoc->m_MCUwidth, &pDoc->m_MCUheight,
			&pDoc->m_scale_denom);

		pDoc->ReplaceHDIB(hDIB);
		pDoc->m_dithered = 0;
		pDoc->m_scale = pDoc->m_scale_denom;
		pDoc->DoTransform();
		pDoc->InitDIBData();
		EndWaitCursor();
		pDoc->UpdateAllViews(NULL, 18);
	}
	else
	{
		if (m_zoomval == 15) return;
		OnUpdate(NULL, 18);
	}
}

void CDibView::OnViewZoom16()
{
	CDibLookApp *myapp = (CDibLookApp *) AfxGetApp();
	CDibDoc *pDoc = GetDocument();

	if (myapp->m_zoom_reload)
	{
		if (pDoc->m_scale == 16) return;
		BeginWaitCursor();
		pDoc->ReplaceHDIB(NULL);

		HDIB hDIB = (HDIB) Read_JPEG_File(pDoc->GetPathName(),
			16, pDoc->m_force_grayscale,
			&pDoc->m_image_width, &pDoc->m_image_height,
			&pDoc->m_MCUwidth, &pDoc->m_MCUheight,
			&pDoc->m_scale_denom);

		pDoc->ReplaceHDIB(hDIB);
		pDoc->m_dithered = 0;
		pDoc->m_scale = 16;
		pDoc->DoTransform();
		pDoc->InitDIBData();
		EndWaitCursor();
		pDoc->UpdateAllViews(NULL, pDoc->m_scale_denom + 3);
	}
	else if (pDoc->m_scale != pDoc->m_scale_denom)
	{
		BeginWaitCursor();
		pDoc->ReplaceHDIB(NULL);

		HDIB hDIB = (HDIB) Read_JPEG_File(pDoc->GetPathName(),
			0, pDoc->m_force_grayscale,
			&pDoc->m_image_width, &pDoc->m_image_height,
			&pDoc->m_MCUwidth, &pDoc->m_MCUheight,
			&pDoc->m_scale_denom);

		pDoc->ReplaceHDIB(hDIB);
		pDoc->m_dithered = 0;
		pDoc->m_scale = pDoc->m_scale_denom;
		pDoc->DoTransform();
		pDoc->InitDIBData();
		EndWaitCursor();
		pDoc->UpdateAllViews(NULL, 19);
	}
	else
	{
		if (m_zoomval == 16) return;
		OnUpdate(NULL, 19);
	}
}

void CDibView::OnUpdateViewZoom1(CCmdUI *pCmdUI)
{
	CDibDoc *pDoc = GetDocument();
	char buf[64];

	int scale = pDoc->m_scale;
	if (scale == pDoc->m_scale_denom) scale = m_zoomval;

	pCmdUI->SetCheck(scale == 1);

	sprintf(buf, "  1/%d    |    %.4g %%", pDoc->m_scale_denom,
		(double) 100 / (double) pDoc->m_scale_denom);
	pCmdUI->SetText(buf);
}

void CDibView::OnUpdateViewZoom2(CCmdUI *pCmdUI)
{
	CDibDoc *pDoc = GetDocument();
	char buf[64];

	int scale = pDoc->m_scale;
	if (scale == pDoc->m_scale_denom) scale = m_zoomval;

	pCmdUI->SetCheck(scale == 2);

	sprintf(buf, "  2/%d    |    %.4g %%", pDoc->m_scale_denom,
		(double) 200 / (double) pDoc->m_scale_denom);
	pCmdUI->SetText(buf);
}

void CDibView::OnUpdateViewZoom3(CCmdUI *pCmdUI)
{
	CDibDoc *pDoc = GetDocument();
	char buf[64];

	int scale = pDoc->m_scale;
	if (scale == pDoc->m_scale_denom) scale = m_zoomval;

	pCmdUI->SetCheck(scale == 3);

	sprintf(buf, "  3/%d    |    %.4g %%", pDoc->m_scale_denom,
		(double) 300 / (double) pDoc->m_scale_denom);
	pCmdUI->SetText(buf);
}

void CDibView::OnUpdateViewZoom4(CCmdUI *pCmdUI)
{
	CDibDoc *pDoc = GetDocument();
	char buf[64];

	int scale = pDoc->m_scale;
	if (scale == pDoc->m_scale_denom) scale = m_zoomval;

	pCmdUI->SetCheck(scale == 4);

	sprintf(buf, "  4/%d    |    %.4g %%", pDoc->m_scale_denom,
		(double) 400 / (double) pDoc->m_scale_denom);
	pCmdUI->SetText(buf);
}

void CDibView::OnUpdateViewZoom5(CCmdUI *pCmdUI)
{
	CDibDoc *pDoc = GetDocument();
	char buf[64];

	int scale = pDoc->m_scale;
	if (scale == pDoc->m_scale_denom) scale = m_zoomval;

	pCmdUI->SetCheck(scale == 5);

	sprintf(buf, "  5/%d    |    %.4g %%", pDoc->m_scale_denom,
		(double) 500 / (double) pDoc->m_scale_denom);
	pCmdUI->SetText(buf);
}

void CDibView::OnUpdateViewZoom6(CCmdUI *pCmdUI)
{
	CDibDoc *pDoc = GetDocument();
	char buf[64];

	int scale = pDoc->m_scale;
	if (scale == pDoc->m_scale_denom) scale = m_zoomval;

	pCmdUI->SetCheck(scale == 6);

	sprintf(buf, "  6/%d    |    %.4g %%", pDoc->m_scale_denom,
		(double) 600 / (double) pDoc->m_scale_denom);
	pCmdUI->SetText(buf);
}

void CDibView::OnUpdateViewZoom7(CCmdUI *pCmdUI)
{
	CDibDoc *pDoc = GetDocument();
	char buf[64];

	int scale = pDoc->m_scale;
	if (scale == pDoc->m_scale_denom) scale = m_zoomval;

	pCmdUI->SetCheck(scale == 7);

	sprintf(buf, "  7/%d    |    %.4g %%", pDoc->m_scale_denom,
		(double) 700 / (double) pDoc->m_scale_denom);
	pCmdUI->SetText(buf);
}

void CDibView::OnUpdateViewZoom8(CCmdUI *pCmdUI)
{
	CDibDoc *pDoc = GetDocument();
	char buf[64];

	int scale = pDoc->m_scale;
	if (scale == pDoc->m_scale_denom) scale = m_zoomval;

	pCmdUI->SetCheck(scale == 8);

	sprintf(buf, "  8/%d    |    %.4g %%", pDoc->m_scale_denom,
		(double) 800 / (double) pDoc->m_scale_denom);
	pCmdUI->SetText(buf);
}

void CDibView::OnUpdateViewZoom9(CCmdUI *pCmdUI)
{
	CDibDoc *pDoc = GetDocument();
	char buf[64];

	int scale = pDoc->m_scale;
	if (scale == pDoc->m_scale_denom) scale = m_zoomval;

	pCmdUI->SetCheck(scale == 9);

	sprintf(buf, "  9/%d    |    %.4g %%", pDoc->m_scale_denom,
		(double) 900 / (double) pDoc->m_scale_denom);
	pCmdUI->SetText(buf);
}

void CDibView::OnUpdateViewZoom10(CCmdUI *pCmdUI)
{
	CDibDoc *pDoc = GetDocument();
	char buf[64];

	int scale = pDoc->m_scale;
	if (scale == pDoc->m_scale_denom) scale = m_zoomval;

	pCmdUI->SetCheck(scale == 10);

	sprintf(buf, "10/%d    |    %.4g %%", pDoc->m_scale_denom,
		(double) 1000 / (double) pDoc->m_scale_denom);
	pCmdUI->SetText(buf);
}

void CDibView::OnUpdateViewZoom11(CCmdUI *pCmdUI)
{
	CDibDoc *pDoc = GetDocument();
	char buf[64];

	int scale = pDoc->m_scale;
	if (scale == pDoc->m_scale_denom) scale = m_zoomval;

	pCmdUI->SetCheck(scale == 11);

	sprintf(buf, "11/%d    |    %.4g %%", pDoc->m_scale_denom,
		(double) 1100 / (double) pDoc->m_scale_denom);
	pCmdUI->SetText(buf);
}

void CDibView::OnUpdateViewZoom12(CCmdUI *pCmdUI)
{
	CDibDoc *pDoc = GetDocument();
	char buf[64];

	int scale = pDoc->m_scale;
	if (scale == pDoc->m_scale_denom) scale = m_zoomval;

	pCmdUI->SetCheck(scale == 12);

	sprintf(buf, "12/%d    |    %.4g %%", pDoc->m_scale_denom,
		(double) 1200 / (double) pDoc->m_scale_denom);
	pCmdUI->SetText(buf);
}

void CDibView::OnUpdateViewZoom13(CCmdUI *pCmdUI)
{
	CDibDoc *pDoc = GetDocument();
	char buf[64];

	int scale = pDoc->m_scale;
	if (scale == pDoc->m_scale_denom) scale = m_zoomval;

	pCmdUI->SetCheck(scale == 13);

	sprintf(buf, "13/%d    |    %.4g %%", pDoc->m_scale_denom,
		(double) 1300 / (double) pDoc->m_scale_denom);
	pCmdUI->SetText(buf);
}

void CDibView::OnUpdateViewZoom14(CCmdUI *pCmdUI)
{
	CDibDoc *pDoc = GetDocument();
	char buf[64];

	int scale = pDoc->m_scale;
	if (scale == pDoc->m_scale_denom) scale = m_zoomval;

	pCmdUI->SetCheck(scale == 14);

	sprintf(buf, "14/%d    |    %.4g %%", pDoc->m_scale_denom,
		(double) 1400 / (double) pDoc->m_scale_denom);
	pCmdUI->SetText(buf);
}

void CDibView::OnUpdateViewZoom15(CCmdUI *pCmdUI)
{
	CDibDoc *pDoc = GetDocument();
	char buf[64];

	int scale = pDoc->m_scale;
	if (scale == pDoc->m_scale_denom) scale = m_zoomval;

	pCmdUI->SetCheck(scale == 15);

	sprintf(buf, "15/%d    |    %.4g %%", pDoc->m_scale_denom,
		(double) 1500 / (double) pDoc->m_scale_denom);
	pCmdUI->SetText(buf);
}

void CDibView::OnUpdateViewZoom16(CCmdUI *pCmdUI)
{
	CDibDoc *pDoc = GetDocument();
	char buf[64];

	int scale = pDoc->m_scale;
	if (scale == pDoc->m_scale_denom) scale = m_zoomval;

	pCmdUI->SetCheck(scale == 16);

	sprintf(buf, "16/%d    |    %.4g %%", pDoc->m_scale_denom,
		(double) 1600 / (double) pDoc->m_scale_denom);
	pCmdUI->SetText(buf);
}

void CDibView::OnUpdateToolbarScale(CCmdUI *pCmdUI)
{
	CDibDoc *pDoc = GetDocument();
	char buf[64];

	int scale = pDoc->m_scale;
	if (scale == pDoc->m_scale_denom) scale = m_zoomval;

	sprintf(buf, "%d/%d", scale, pDoc->m_scale_denom);
	pCmdUI->SetText(buf);
}

void CDibView::OnUpdateToolbarZoom(CCmdUI *pCmdUI)
{
	CDibDoc *pDoc = GetDocument();
	char buf[64];

	int scale = pDoc->m_scale;
	if (scale == pDoc->m_scale_denom) scale = m_zoomval;

	(((CMainFrame *)AfxGetApp()->m_pMainWnd)->m_slider).SendMessage(
		TBM_SETSELEND, TRUE, pDoc->m_scale_denom);
	(((CMainFrame *)AfxGetApp()->m_pMainWnd)->m_slider).SetPos(scale);

	sprintf(buf, "%.4g %%",
		(double) scale * (double) 100 / (double) pDoc->m_scale_denom);
	pCmdUI->SetText(buf);
}

void CDibView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	switch (nChar)
	{
	case VK_HOME:
	    OnHScroll(SB_TOP, 0, GetScrollBarCtrl(SB_HORZ)); return;
	case VK_END:
	    OnHScroll(SB_BOTTOM, 0, GetScrollBarCtrl(SB_HORZ)); return;
	case VK_LEFT:
	    OnHScroll(SB_LINEUP, 0, GetScrollBarCtrl(SB_HORZ)); return;
	case VK_RIGHT:
	    OnHScroll(SB_LINEDOWN, 0, GetScrollBarCtrl(SB_HORZ)); return;
	case VK_UP:
	    OnVScroll(SB_LINEUP, 0, GetScrollBarCtrl(SB_VERT)); return;
	case VK_DOWN:
	    OnVScroll(SB_LINEDOWN, 0, GetScrollBarCtrl(SB_VERT)); return;
	case VK_PRIOR:
	    OnVScroll(SB_PAGEUP, 0, GetScrollBarCtrl(SB_VERT)); return;
	case VK_NEXT:
	    OnVScroll(SB_PAGEDOWN, 0, GetScrollBarCtrl(SB_VERT)); return;
	}

	CScrollView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CDibView::OnOptionsSaveZoom()
{
	m_save_zoom ^= 1;

	CDibDoc *pDoc = GetDocument();

	char buf[64];
	unsigned int paru[2];
	int pari[2];
	if (m_save_zoom)
	{
		pDoc->m_crop_xoffset = m_left_zoomed;
		pDoc->m_crop_yoffset = m_top_zoomed;
		pDoc->m_crop_width = m_zoomedsize.cx - m_right_zoomed - m_left_zoomed;
		pDoc->m_crop_height = m_zoomedsize.cy - m_bottom_zoomed - m_top_zoomed;
		pDoc->m_save_scale =
			pDoc->m_scale == pDoc->m_scale_denom ? m_zoomval : pDoc->m_scale;
	}
	else
	{
		pDoc->m_crop_xoffset = m_left;
		pDoc->m_crop_yoffset = m_top;
		pDoc->m_crop_width = pDoc->m_image_width - m_right - m_left;
		pDoc->m_crop_height = pDoc->m_image_height - m_bottom - m_top;
		pDoc->m_save_scale = 0;
	}

	sprintf(buf, " %dx%d+%d+%d",
		pDoc->m_crop_width,	pDoc->m_crop_height,
		pDoc->m_crop_xoffset, pDoc->m_crop_yoffset);
	((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.GetPaneInfo(1, paru[0], paru[1], pari[0]);
	((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.SetPaneInfo(1, ID_INDICATOR_POS, SBPS_NORMAL, pari[0]);
	((CMainFrame *)AfxGetMainWnd())->m_wndStatusBar.SetPaneText(1, buf, TRUE);
}

void CDibView::OnUpdateOptionsSaveZoom(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_save_zoom != 0);
}
