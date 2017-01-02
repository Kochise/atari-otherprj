#include "stdafx.h"
#include "dibdoc.h"


/* helper functions and macros */
HANDLE CreateDIB(DWORD dwWidth, DWORD dwHeight, WORD wBitCount);


static void DoFlipH(CDibDoc *pDoc)
{
  HDIB hDib = pDoc->GetHDIB();
  if (hDib == NULL) return;

  LPBITMAPINFOHEADER lpDIB;
  int partx;
  int DIBLineWidth;
  int DIBWidth1;
  int DIBScanWidth;
  LPSTR lpBits;

  lpDIB = (LPBITMAPINFOHEADER)GlobalLock(hDib);

  partx = ((pDoc->m_image_width % pDoc->m_MCUwidth) * pDoc->m_scale
		+ pDoc->m_scale_denom - 1)
		/ pDoc->m_scale_denom;

  DIBLineWidth = lpDIB->biWidth * lpDIB->biBitCount / 8;
  DIBWidth1 = lpDIB->biWidth - partx - 1;
  DIBScanWidth = (DIBLineWidth + 3) & (-4);

  lpBits = FindDIBBits((LPSTR) lpDIB);

  if (lpDIB->biBitCount == 8)
  {
    for (int i = 0; i < lpDIB->biHeight; i++)
	{
      for (int j = 0; j * 2 < DIBWidth1; j++)
	  {
	    unsigned char temp = lpBits[j];
	    lpBits[j] = lpBits[DIBWidth1 - j];
	    lpBits[DIBWidth1 - j] = temp;
	  }
      lpBits += DIBScanWidth;
	}
  }
  else if (lpDIB->biBitCount == 24)
  {
    for (int i = 0; i < lpDIB->biHeight; i++)
	{
      for (int j = 0; j * 2 < DIBWidth1; j++)
	  {
	    unsigned char temp = lpBits[3*j];
	    lpBits[3*j] = lpBits[3*(DIBWidth1 - j)];
	    lpBits[3*(DIBWidth1 - j)] = temp;
	    temp = lpBits[3*j + 1];
	    lpBits[3*j + 1] = lpBits[3*(DIBWidth1 - j) + 1];
	    lpBits[3*(DIBWidth1 - j) + 1] = temp;
	    temp = lpBits[3*j + 2];
	    lpBits[3*j + 2] = lpBits[3*(DIBWidth1 - j) + 2];
	    lpBits[3*(DIBWidth1 - j) + 2] = temp;
	  }
      lpBits += DIBScanWidth;
	}
  }

  GlobalUnlock(hDib);
}

static void DoTransformFlipH(CDibDoc *pDoc)
{
  DoFlipH(pDoc);

  pDoc->UpdateAllViews(NULL, 1);
}

static void DoFlipV(CDibDoc *pDoc)
{
  HDIB hDib = pDoc->GetHDIB();
  if (hDib == NULL) return;

  LPBITMAPINFOHEADER lpDIB;
  int party;
  int DIBLineWidth;
  int DIBScanWidth;
  int DIBHeight1;
  LPSTR lpBits1;
  LPSTR lpBits2;

  lpDIB = (LPBITMAPINFOHEADER)GlobalLock(hDib);

  party = ((pDoc->m_image_height % pDoc->m_MCUheight) * pDoc->m_scale
		+ pDoc->m_scale_denom - 1)
		/ pDoc->m_scale_denom;

  if (lpDIB->biBitCount >= 8)
  {
    DIBLineWidth = lpDIB->biWidth * lpDIB->biBitCount / 8;
    DIBScanWidth = (DIBLineWidth + 3) & (-4);
	DIBHeight1 = lpDIB->biHeight - party - 1;

    lpBits1 = FindDIBBits((LPSTR) lpDIB);
    lpBits2 = lpBits1 + DIBScanWidth * lpDIB->biHeight;
	lpBits1 += DIBScanWidth * party;

    for (int i = 0; i * 2 < DIBHeight1; i++)
	{
	  lpBits2 -= DIBScanWidth;
      for (int j = 0; j < DIBLineWidth; j++)
	  {
		unsigned char temp = lpBits1[j];
	    lpBits1[j] = lpBits2[j];
		lpBits2[j] = temp;
	  }
      lpBits1 += DIBScanWidth;
	}
  }

  GlobalUnlock(hDib);
}

static void DoTransformFlipV(CDibDoc *pDoc)
{
  DoFlipV(pDoc);

  pDoc->UpdateAllViews(NULL, 1);
}

static void DoRot90(CDibDoc *pDoc)
{
  HDIB hDib = pDoc->GetHDIB();
  if (hDib == NULL) return;

  LPBITMAPINFOHEADER lpDIB;
  int party;
  int DIBLineWidth;
  int DIBScanWidth;
  unsigned char *lpBits;

  lpDIB = (LPBITMAPINFOHEADER)GlobalLock(hDib);

  party = ((pDoc->m_image_height % pDoc->m_MCUheight) * pDoc->m_scale
		+ pDoc->m_scale_denom - 1)
		/ pDoc->m_scale_denom;

  DIBLineWidth = lpDIB->biWidth * lpDIB->biBitCount / 8;
  DIBScanWidth = (DIBLineWidth + 3) & (-4);

  lpBits = (unsigned char *) FindDIBBits((LPSTR) lpDIB);

  if (lpDIB->biBitCount == 8)
  {
    HDIB hNew = (HDIB) CreateDIB((DWORD)lpDIB->biHeight,
		                         (DWORD)lpDIB->biWidth,
                                 lpDIB->biBitCount);
    LPBITMAPINFOHEADER lpNEW = (LPBITMAPINFOHEADER)GlobalLock(hNew);
    LPBITMAPINFO lpbmi = (LPBITMAPINFO)lpNEW;
    for ( int k = 0; k < 256; k++ ) {
      lpbmi->bmiColors[k].rgbRed = (BYTE)k;
      lpbmi->bmiColors[k].rgbGreen = (BYTE)k;
      lpbmi->bmiColors[k].rgbBlue = (BYTE)k;
    }
    int NEWLineWidth = lpNEW->biWidth * lpNEW->biBitCount / 8;
    int NEWScanWidth = (NEWLineWidth + 3) & (-4);
    unsigned char *NEWBits = (unsigned char *) FindDIBBits((LPSTR) lpNEW);

    for (int i = 0; i < lpDIB->biHeight; i++)
	{
	  int k;
	  if (i < party) 
		k = lpDIB->biHeight - 1 - i;
	  else
		k = i - party;
      for (int j = DIBLineWidth; j;)
	  {
		NEWBits[k] = lpBits[--j];
	    k += NEWScanWidth;
	  }
      lpBits += DIBScanWidth;
	}
	i = pDoc->m_MCUwidth;
	pDoc->m_MCUwidth = pDoc->m_MCUheight;
	pDoc->m_MCUheight = i;
	i = pDoc->m_image_width;
	pDoc->m_image_width = pDoc->m_image_height;
	pDoc->m_image_height = i;
	GlobalUnlock(hNew);
    GlobalUnlock(hDib);
    pDoc->ReplaceHDIB(hNew);
  }
  else if (lpDIB->biBitCount == 24)
  {
    HDIB hNew = (HDIB) CreateDIB((DWORD)lpDIB->biHeight,
		                         (DWORD)lpDIB->biWidth,
                                 lpDIB->biBitCount);
    LPBITMAPINFOHEADER lpNEW = (LPBITMAPINFOHEADER)GlobalLock(hNew);
    int NEWLineWidth = lpNEW->biWidth * lpNEW->biBitCount / 8;
    int NEWScanWidth = (NEWLineWidth + 3) & (-4);
    unsigned char *NEWBits = (unsigned char *) FindDIBBits((LPSTR) lpNEW);

    for (int i = 0; i < lpDIB->biHeight; i++)
	{
	  int k;
	  if (i < party) 
		k = 3*(lpDIB->biHeight - 1 - i);
	  else
		k = 3*(i - party);
      for (int j = DIBLineWidth; j;)
	  {
		NEWBits[k+2] = lpBits[--j];
		NEWBits[k+1] = lpBits[--j];
		NEWBits[k] = lpBits[--j];
	    k += NEWScanWidth;
	  }
      lpBits += DIBScanWidth;
	}
	i = pDoc->m_MCUwidth;
	pDoc->m_MCUwidth = pDoc->m_MCUheight;
	pDoc->m_MCUheight = i;
	i = pDoc->m_image_width;
	pDoc->m_image_width = pDoc->m_image_height;
	pDoc->m_image_height = i;
	GlobalUnlock(hNew);
    GlobalUnlock(hDib);
    pDoc->ReplaceHDIB(hNew);
  }
}

static void DoTransformRot90(CDibDoc *pDoc)
{
  DoRot90(pDoc);

  pDoc->InitDIBData();
  pDoc->UpdateAllViews(NULL, 2);
}

static void DoRot270(CDibDoc *pDoc)
{
  HDIB hDib = pDoc->GetHDIB();
  if (hDib == NULL) return;

  LPBITMAPINFOHEADER lpDIB;
  int partx;
  int DIBLineWidth;
  int DIBWidth1;
  int DIBScanWidth;
  unsigned char *lpBits;

  lpDIB = (LPBITMAPINFOHEADER)GlobalLock(hDib);

  partx = ((pDoc->m_image_width % pDoc->m_MCUwidth) * pDoc->m_scale
		+ pDoc->m_scale_denom - 1)
		/ pDoc->m_scale_denom;

  DIBLineWidth = lpDIB->biWidth * lpDIB->biBitCount / 8;
  DIBWidth1 = lpDIB->biWidth - partx;
  DIBScanWidth = (DIBLineWidth + 3) & (-4);

  lpBits = (unsigned char *) FindDIBBits((LPSTR) lpDIB);

  if (lpDIB->biBitCount == 8)
  {
    HDIB hNew = (HDIB) CreateDIB((DWORD)lpDIB->biHeight,
		                         (DWORD)lpDIB->biWidth,
                                 lpDIB->biBitCount);
    LPBITMAPINFOHEADER lpNEW = (LPBITMAPINFOHEADER)GlobalLock(hNew);
    LPBITMAPINFO lpbmi = (LPBITMAPINFO)lpNEW;
    for ( int k = 0; k < 256; k++ ) {
      lpbmi->bmiColors[k].rgbRed = (BYTE)k;
      lpbmi->bmiColors[k].rgbGreen = (BYTE)k;
      lpbmi->bmiColors[k].rgbBlue = (BYTE)k;
    }
    int NEWLineWidth = lpNEW->biWidth * lpNEW->biBitCount / 8;
    int NEWScanWidth = (NEWLineWidth + 3) & (-4);
    unsigned char *NEWBits = (unsigned char *) FindDIBBits((LPSTR) lpNEW);

    for (int i = lpDIB->biHeight; i;)
	{
	  int k = --i;
      for (int j = DIBLineWidth; j > DIBWidth1;)
	  {
		NEWBits[k] = lpBits[--j];
	    k += NEWScanWidth;
	  }
      for (j = 0; j < DIBWidth1; j++)
	  {
		NEWBits[k] = lpBits[j];
	    k += NEWScanWidth;
	  }
      lpBits += DIBScanWidth;
	}
	i = pDoc->m_MCUwidth;
	pDoc->m_MCUwidth = pDoc->m_MCUheight;
	pDoc->m_MCUheight = i;
	i = pDoc->m_image_width;
	pDoc->m_image_width = pDoc->m_image_height;
	pDoc->m_image_height = i;
	GlobalUnlock(hNew);
    GlobalUnlock(hDib);
    pDoc->ReplaceHDIB(hNew);
  }
  else if (lpDIB->biBitCount == 24)
  {
    HDIB hNew = (HDIB) CreateDIB((DWORD)lpDIB->biHeight,
		                         (DWORD)lpDIB->biWidth,
                                 lpDIB->biBitCount);
    LPBITMAPINFOHEADER lpNEW = (LPBITMAPINFOHEADER)GlobalLock(hNew);
    int NEWLineWidth = lpNEW->biWidth * lpNEW->biBitCount / 8;
    int NEWScanWidth = (NEWLineWidth + 3) & (-4);
    unsigned char *NEWBits = (unsigned char *) FindDIBBits((LPSTR) lpNEW);

	DIBWidth1 *= 3;
    for (int i = 3 * lpDIB->biHeight; i;)
	{
	  int k = i -= 3;
      for (int j = DIBLineWidth; j > DIBWidth1;)
	  {
		NEWBits[k+2] = lpBits[--j];
		NEWBits[k+1] = lpBits[--j];
		NEWBits[k] = lpBits[--j];
	    k += NEWScanWidth;
	  }
      for (j = 0; j < DIBWidth1;)
	  {
		NEWBits[k] = lpBits[j++];
		NEWBits[k+1] = lpBits[j++];
		NEWBits[k+2] = lpBits[j++];
	    k += NEWScanWidth;
	  }
      lpBits += DIBScanWidth;
	}
	i = pDoc->m_MCUwidth;
	pDoc->m_MCUwidth = pDoc->m_MCUheight;
	pDoc->m_MCUheight = i;
	i = pDoc->m_image_width;
	pDoc->m_image_width = pDoc->m_image_height;
	pDoc->m_image_height = i;
	GlobalUnlock(hNew);
    GlobalUnlock(hDib);
    pDoc->ReplaceHDIB(hNew);
  }
}

static void DoTransformRot270(CDibDoc *pDoc)
{
  DoRot270(pDoc);

  pDoc->InitDIBData();
  pDoc->UpdateAllViews(NULL, 2);
}

static void DoRot180(CDibDoc *pDoc)
{
  HDIB hDib = pDoc->GetHDIB();
  if (hDib == NULL) return;

  LPBITMAPINFOHEADER lpDIB;
  int partx, party;
  int DIBLineWidth;
  int DIBWidth1;
  int DIBHeight1;
  int DIBScanWidth;
  LPSTR lpBits1;
  LPSTR lpBits2;

  lpDIB = (LPBITMAPINFOHEADER)GlobalLock(hDib);

  partx = ((pDoc->m_image_width % pDoc->m_MCUwidth) * pDoc->m_scale
		+ pDoc->m_scale_denom - 1)
		/ pDoc->m_scale_denom;
  party = ((pDoc->m_image_height % pDoc->m_MCUheight) * pDoc->m_scale
		+ pDoc->m_scale_denom - 1)
		/ pDoc->m_scale_denom;

  DIBLineWidth = lpDIB->biWidth * lpDIB->biBitCount / 8;
  DIBWidth1 = lpDIB->biWidth - partx - 1;
  DIBScanWidth = (DIBLineWidth + 3) & (-4);

  lpBits1 = FindDIBBits((LPSTR) lpDIB);
  lpBits2 = lpBits1 + DIBScanWidth * lpDIB->biHeight;
  DIBHeight1 = lpDIB->biHeight - party - 1;

  if (lpDIB->biBitCount == 8)
  {
    for (int i = 0; i < party; i++)
	{
      for (int j = 0; j * 2 < DIBWidth1; j++)
	  {
	    unsigned char temp = lpBits1[j];
	    lpBits1[j] = lpBits1[DIBWidth1 - j];
	    lpBits1[DIBWidth1 - j] = temp;
	  }
      lpBits1 += DIBScanWidth;
	}
    for (i = 0; i * 2 < DIBHeight1; i++)
	{
	  lpBits2 -= DIBScanWidth;
      for (int j = 0; j <= DIBWidth1; j++)
	  {
	    unsigned char temp = lpBits1[j];
	    lpBits1[j] = lpBits2[DIBWidth1 - j];
	    lpBits2[DIBWidth1 - j] = temp;
	  }
      for (; j < DIBLineWidth; j++)
	  {
	    unsigned char temp = lpBits1[j];
	    lpBits1[j] = lpBits2[j];
	    lpBits2[j] = temp;
	  }
      lpBits1 += DIBScanWidth;
	}
	if (i * 2 == DIBHeight1)
      for (int j = 0; j * 2 < DIBWidth1; j++)
	  {
	    unsigned char temp = lpBits1[j];
	    lpBits1[j] = lpBits1[DIBWidth1 - j];
	    lpBits1[DIBWidth1 - j] = temp;
	  }
  }
  else if (lpDIB->biBitCount == 24)
  {
	for (int i = 0; i < party; i++)
	{
      for (int j = 0; j * 2 < DIBWidth1; j++)
	  {
	    unsigned char temp = lpBits1[3*j];
	    lpBits1[3*j] = lpBits1[3*(DIBWidth1 - j)];
	    lpBits1[3*(DIBWidth1 - j)] = temp;
	    temp = lpBits1[3*j + 1];
	    lpBits1[3*j + 1] = lpBits1[3*(DIBWidth1 - j) + 1];
	    lpBits1[3*(DIBWidth1 - j) + 1] = temp;
	    temp = lpBits1[3*j + 2];
	    lpBits1[3*j + 2] = lpBits1[3*(DIBWidth1 - j) + 2];
	    lpBits1[3*(DIBWidth1 - j) + 2] = temp;
	  }
      lpBits1 += DIBScanWidth;
	}
	for (i = 0; i * 2 < DIBHeight1; i++)
	{
	  lpBits2 -= DIBScanWidth;
      for (int j = 0; j <= DIBWidth1; j++)
	  {
	    unsigned char temp = lpBits1[3*j];
	    lpBits1[3*j] = lpBits2[3*(DIBWidth1 - j)];
	    lpBits2[3*(DIBWidth1 - j)] = temp;
	    temp = lpBits1[3*j + 1];
	    lpBits1[3*j + 1] = lpBits2[3*(DIBWidth1 - j) + 1];
	    lpBits2[3*(DIBWidth1 - j) + 1] = temp;
	    temp = lpBits1[3*j + 2];
	    lpBits1[3*j + 2] = lpBits2[3*(DIBWidth1 - j) + 2];
	    lpBits2[3*(DIBWidth1 - j) + 2] = temp;
	  }
      for (; j < lpDIB->biWidth; j++)
	  {
	    unsigned char temp = lpBits1[3*j];
	    lpBits1[3*j] = lpBits2[3*j];
	    lpBits2[3*j] = temp;
	    temp = lpBits1[3*j + 1];
	    lpBits1[3*j + 1] = lpBits2[3*j + 1];
	    lpBits2[3*j + 1] = temp;
	    temp = lpBits1[3*j + 2];
	    lpBits1[3*j + 2] = lpBits2[3*j + 2];
	    lpBits2[3*j + 2] = temp;
	  }
      lpBits1 += DIBScanWidth;
	}
	if (i * 2 == DIBHeight1)
      for (int j = 0; j * 2 < DIBWidth1; j++)
	  {
	    unsigned char temp = lpBits1[3*j];
	    lpBits1[3*j] = lpBits1[3*(DIBWidth1 - j)];
	    lpBits1[3*(DIBWidth1 - j)] = temp;
	    temp = lpBits1[3*j + 1];
	    lpBits1[3*j + 1] = lpBits1[3*(DIBWidth1 - j) + 1];
	    lpBits1[3*(DIBWidth1 - j) + 1] = temp;
	    temp = lpBits1[3*j + 2];
	    lpBits1[3*j + 2] = lpBits1[3*(DIBWidth1 - j) + 2];
	    lpBits1[3*(DIBWidth1 - j) + 2] = temp;
	  }
  }

  GlobalUnlock(hDib);
}

static void DoTransformRot180(CDibDoc *pDoc)
{
  DoRot180(pDoc);

  pDoc->UpdateAllViews(NULL, 1);
}

static void DoTranspose(CDibDoc *pDoc)
{
  HDIB hDib = pDoc->GetHDIB();
  if (hDib == NULL) return;

  LPBITMAPINFOHEADER lpDIB;
  int DIBLineWidth;
  int DIBScanWidth;
  unsigned char *lpBits;

  lpDIB = (LPBITMAPINFOHEADER)GlobalLock(hDib);

  DIBLineWidth = lpDIB->biWidth * lpDIB->biBitCount / 8;
  DIBScanWidth = (DIBLineWidth + 3) & (-4);

  lpBits = (unsigned char *) FindDIBBits((LPSTR) lpDIB);

  if (lpDIB->biBitCount == 8)
  {
    HDIB hNew = (HDIB) CreateDIB((DWORD)lpDIB->biHeight,
		                         (DWORD)lpDIB->biWidth,
                                 lpDIB->biBitCount);
    LPBITMAPINFOHEADER lpNEW = (LPBITMAPINFOHEADER)GlobalLock(hNew);
    LPBITMAPINFO lpbmi = (LPBITMAPINFO)lpNEW;
    for ( int k = 0; k < 256; k++ ) {
      lpbmi->bmiColors[k].rgbRed = (BYTE)k;
      lpbmi->bmiColors[k].rgbGreen = (BYTE)k;
      lpbmi->bmiColors[k].rgbBlue = (BYTE)k;
    }
    int NEWLineWidth = lpNEW->biWidth * lpNEW->biBitCount / 8;
    int NEWScanWidth = (NEWLineWidth + 3) & (-4);
    unsigned char *NEWBits = (unsigned char *) FindDIBBits((LPSTR) lpNEW);

    for (int i = lpDIB->biHeight; i;)
	{
	  int k = --i;
      for (int j = DIBLineWidth; j;)
	  {
		NEWBits[k] = lpBits[--j];
	    k += NEWScanWidth;
	  }
      lpBits += DIBScanWidth;
	}
	i = pDoc->m_MCUwidth;
	pDoc->m_MCUwidth = pDoc->m_MCUheight;
	pDoc->m_MCUheight = i;
	i = pDoc->m_image_width;
	pDoc->m_image_width = pDoc->m_image_height;
	pDoc->m_image_height = i;
	GlobalUnlock(hNew);
    GlobalUnlock(hDib);
    pDoc->ReplaceHDIB(hNew);
  }
  else if (lpDIB->biBitCount == 24)
  {
    HDIB hNew = (HDIB) CreateDIB((DWORD)lpDIB->biHeight,
		                         (DWORD)lpDIB->biWidth,
                                 lpDIB->biBitCount);
    LPBITMAPINFOHEADER lpNEW = (LPBITMAPINFOHEADER)GlobalLock(hNew);
    int NEWLineWidth = lpNEW->biWidth * lpNEW->biBitCount / 8;
    int NEWScanWidth = (NEWLineWidth + 3) & (-4);
    unsigned char *NEWBits = (unsigned char *) FindDIBBits((LPSTR) lpNEW);

    for (int i = 3 * lpDIB->biHeight; i;)
	{
	  int k = i -= 3;
      for (int j = DIBLineWidth; j;)
	  {
		NEWBits[k+2] = lpBits[--j];
		NEWBits[k+1] = lpBits[--j];
		NEWBits[k] = lpBits[--j];
	    k += NEWScanWidth;
	  }
      lpBits += DIBScanWidth;
	}
	i = pDoc->m_MCUwidth;
	pDoc->m_MCUwidth = pDoc->m_MCUheight;
	pDoc->m_MCUheight = i;
	i = pDoc->m_image_width;
	pDoc->m_image_width = pDoc->m_image_height;
	pDoc->m_image_height = i;
	GlobalUnlock(hNew);
    GlobalUnlock(hDib);
    pDoc->ReplaceHDIB(hNew);
  }
}

static void DoTransformTranspose(CDibDoc *pDoc)
{
  DoTranspose(pDoc);

  pDoc->InitDIBData();
  pDoc->UpdateAllViews(NULL, 2);
}

static void DoTransverse(CDibDoc *pDoc)
{
  HDIB hDib = pDoc->GetHDIB();
  if (hDib == NULL) return;

  LPBITMAPINFOHEADER lpDIB;
  int partx, party;
  int DIBLineWidth;
  int DIBWidth1;
  int DIBScanWidth;
  unsigned char *lpBits;

  lpDIB = (LPBITMAPINFOHEADER)GlobalLock(hDib);

  partx = ((pDoc->m_image_width % pDoc->m_MCUwidth) * pDoc->m_scale
		+ pDoc->m_scale_denom - 1)
		/ pDoc->m_scale_denom;
  party = ((pDoc->m_image_height % pDoc->m_MCUheight) * pDoc->m_scale
		+ pDoc->m_scale_denom - 1)
		/ pDoc->m_scale_denom;

  DIBLineWidth = lpDIB->biWidth * lpDIB->biBitCount / 8;
  DIBWidth1 = lpDIB->biWidth - partx;
  DIBScanWidth = (DIBLineWidth + 3) & (-4);

  lpBits = (unsigned char *) FindDIBBits((LPSTR) lpDIB);

  if (lpDIB->biBitCount == 8)
  {
    HDIB hNew = (HDIB) CreateDIB((DWORD)lpDIB->biHeight,
		                         (DWORD)lpDIB->biWidth,
                                 lpDIB->biBitCount);
    LPBITMAPINFOHEADER lpNEW = (LPBITMAPINFOHEADER)GlobalLock(hNew);
    LPBITMAPINFO lpbmi = (LPBITMAPINFO)lpNEW;
    for ( int k = 0; k < 256; k++ ) {
      lpbmi->bmiColors[k].rgbRed = (BYTE)k;
      lpbmi->bmiColors[k].rgbGreen = (BYTE)k;
      lpbmi->bmiColors[k].rgbBlue = (BYTE)k;
    }
    int NEWLineWidth = lpNEW->biWidth * lpNEW->biBitCount / 8;
    int NEWScanWidth = (NEWLineWidth + 3) & (-4);
    unsigned char *NEWBits = (unsigned char *) FindDIBBits((LPSTR) lpNEW);

    for (int i = 0; i < lpDIB->biHeight; i++)
	{
	  int k;
	  if (i < party) 
		k = lpDIB->biHeight - 1 - i;
	  else
		k = i - party;
      for (int j = DIBLineWidth; j > DIBWidth1;)
	  {
		NEWBits[k] = lpBits[--j];
	    k += NEWScanWidth;
	  }
      for (j = 0; j < DIBWidth1; j++)
	  {
		NEWBits[k] = lpBits[j];
	    k += NEWScanWidth;
	  }
      lpBits += DIBScanWidth;
	}
	i = pDoc->m_MCUwidth;
	pDoc->m_MCUwidth = pDoc->m_MCUheight;
	pDoc->m_MCUheight = i;
	i = pDoc->m_image_width;
	pDoc->m_image_width = pDoc->m_image_height;
	pDoc->m_image_height = i;
	GlobalUnlock(hNew);
    GlobalUnlock(hDib);
    pDoc->ReplaceHDIB(hNew);
  }
  else if (lpDIB->biBitCount == 24)
  {
    HDIB hNew = (HDIB) CreateDIB((DWORD)lpDIB->biHeight,
		                         (DWORD)lpDIB->biWidth,
                                 lpDIB->biBitCount);
    LPBITMAPINFOHEADER lpNEW = (LPBITMAPINFOHEADER)GlobalLock(hNew);
    int NEWLineWidth = lpNEW->biWidth * lpNEW->biBitCount / 8;
    int NEWScanWidth = (NEWLineWidth + 3) & (-4);
    unsigned char *NEWBits = (unsigned char *) FindDIBBits((LPSTR) lpNEW);

	DIBWidth1 *= 3;
    for (int i = 0; i < lpDIB->biHeight; i++)
	{
	  int k;
	  if (i < party) 
		k = 3*(lpDIB->biHeight - 1 - i);
	  else
		k = 3*(i - party);
      for (int j = DIBLineWidth; j > DIBWidth1;)
	  {
		NEWBits[k+2] = lpBits[--j];
		NEWBits[k+1] = lpBits[--j];
		NEWBits[k] = lpBits[--j];
	    k += NEWScanWidth;
	  }
      for (j = 0; j < DIBWidth1;)
	  {
		NEWBits[k] = lpBits[j++];
		NEWBits[k+1] = lpBits[j++];
		NEWBits[k+2] = lpBits[j++];
	    k += NEWScanWidth;
	  }
      lpBits += DIBScanWidth;
	}
	i = pDoc->m_MCUwidth;
	pDoc->m_MCUwidth = pDoc->m_MCUheight;
	pDoc->m_MCUheight = i;
	i = pDoc->m_image_width;
	pDoc->m_image_width = pDoc->m_image_height;
	pDoc->m_image_height = i;
	GlobalUnlock(hNew);
    GlobalUnlock(hDib);
    pDoc->ReplaceHDIB(hNew);
  }
}

static void DoTransformTransverse(CDibDoc *pDoc)
{
  DoTransverse(pDoc);

  pDoc->InitDIBData();
  pDoc->UpdateAllViews(NULL, 2);
}

void CDibDoc::DoTransform()
{
  switch (m_transform)
  {
	case 1:  DoFlipH(this); break;
	case 2:  DoFlipV(this); break;
	case 3:  DoTranspose(this); break;
	case 4:  DoTransverse(this); break;
	case 5:  DoRot90(this); break;
	case 6:  DoRot180(this); break;
	case 7:  DoRot270(this); break;
	default: break;
  }
}

void CDibDoc::OnTransformFlipH()
{
  int old_transform = m_transform;
  m_transform = 1;
  switch (old_transform)
  {
	case 1:  break;
	case 2:  DoTransformRot180(this); break;
	case 3:  DoTransformRot90(this); break;
	case 4:  DoTransformRot270(this); break;
	case 5:  DoTransformTransverse(this); break;
	case 6:  DoTransformFlipV(this); break;
	case 7:  DoTransformTranspose(this); break;
	default: DoTransformFlipH(this); break;
  }
}

void CDibDoc::OnTransformFlipV()
{
  int old_transform = m_transform;
  m_transform = 2;
  switch (old_transform)
  {
	case 1:  DoTransformRot180(this); break;
	case 2:  break;
	case 3:  DoTransformRot270(this); break;
	case 4:  DoTransformRot90(this); break;
	case 5:  DoTransformTranspose(this); break;
	case 6:  DoTransformFlipH(this); break;
	case 7:  DoTransformTransverse(this); break;
	default: DoTransformFlipV(this); break;
  }
}

void CDibDoc::OnTransformRot90()
{
  int old_transform = m_transform;
  m_transform = 5;
  switch (old_transform)
  {
	case 1:  DoTransformTransverse(this); break;
	case 2:  DoTransformTranspose(this); break;
	case 3:  DoTransformFlipH(this); break;
	case 4:  DoTransformFlipV(this); break;
	case 5:  break;
	case 6:  DoTransformRot270(this); break;
	case 7:  DoTransformRot180(this); break;
	default: DoTransformRot90(this); break;
  }
}

void CDibDoc::OnTransformRot270()
{
  int old_transform = m_transform;
  m_transform = 7;
  switch (old_transform)
  {
	case 1:  DoTransformTranspose(this); break;
	case 2:  DoTransformTransverse(this); break;
	case 3:  DoTransformFlipV(this); break;
	case 4:  DoTransformFlipH(this); break;
	case 5:  DoTransformRot180(this); break;
	case 6:  DoTransformRot90(this); break;
	case 7:  break;
	default: DoTransformRot270(this); break;
  }
}

void CDibDoc::OnTransformRot180()
{
  int old_transform = m_transform;
  m_transform = 6;
  switch (old_transform)
  {
	case 1:  DoTransformFlipV(this); break;
	case 2:  DoTransformFlipH(this); break;
	case 3:  DoTransformTransverse(this); break;
	case 4:  DoTransformTranspose(this); break;
	case 5:  DoTransformRot90(this); break;
	case 6:  break;
	case 7:  DoTransformRot270(this); break;
	default: DoTransformRot180(this); break;
  }
}

void CDibDoc::OnTransformTranspose()
{
  int old_transform = m_transform;
  m_transform = 3;
  switch (old_transform)
  {
	case 1:  DoTransformRot270(this); break;
	case 2:  DoTransformRot90(this); break;
	case 3:  break;
	case 4:  DoTransformRot180(this); break;
	case 5:  DoTransformFlipH(this); break;
	case 6:  DoTransformTransverse(this); break;
	case 7:  DoTransformFlipV(this); break;
	default: DoTransformTranspose(this); break;
  }
}

void CDibDoc::OnTransformTransverse()
{
  int old_transform = m_transform;
  m_transform = 4;
  switch (old_transform)
  {
	case 1:  DoTransformRot90(this); break;
	case 2:  DoTransformRot270(this); break;
	case 3:  DoTransformRot180(this); break;
	case 4:  break;
	case 5:  DoTransformFlipV(this); break;
	case 6:  DoTransformTranspose(this); break;
	case 7:  DoTransformFlipH(this); break;
	default: DoTransformTransverse(this); break;
  }
}

void CDibDoc::OnTransformOriginal()
{
  int old_transform = m_transform;
  m_transform = 0;
  switch (old_transform)
  {
	case 1:  DoTransformFlipH(this); break;
	case 2:  DoTransformFlipV(this); break;
	case 3:  DoTransformTranspose(this); break;
	case 4:  DoTransformTransverse(this); break;
	case 5:  DoTransformRot270(this); break;
	case 6:  DoTransformRot180(this); break;
	case 7:  DoTransformRot90(this); break;
	case 8:  UpdateAllViews(NULL); break;
	default: break;
  }
}

void CDibDoc::OnTransformWipe()
{
  int old_transform = m_transform;
  m_transform = 8;
  switch (old_transform)
  {
	case 0:  UpdateAllViews(NULL); break;
	case 1:  DoTransformFlipH(this); break;
	case 2:  DoTransformFlipV(this); break;
	case 3:  DoTransformTranspose(this); break;
	case 4:  DoTransformTransverse(this); break;
	case 5:  DoTransformRot270(this); break;
	case 6:  DoTransformRot180(this); break;
	case 7:  DoTransformRot90(this); break;
	default: break;
  }
}
