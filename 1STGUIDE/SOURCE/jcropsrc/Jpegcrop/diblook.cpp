// diblook.cpp : Defines the class behaviors for the application.
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

#include "mainfrm.h"
#include "dibdoc.h"
#include "dibview.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDibLookApp

BEGIN_MESSAGE_MAP(CDibLookApp, CWinApp)
	//{{AFX_MSG_MAP(CDibLookApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_FILE_PREFERENCES, OnFilePreferences)
	ON_COMMAND(ID_VIEW_BLOCKGRID, OnViewBlockGrid)
	ON_UPDATE_COMMAND_UI(ID_VIEW_BLOCKGRID, OnUpdateViewBlockGrid)
	ON_COMMAND(ID_VIEW_CROPMASK, OnViewCropMask)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CROPMASK, OnUpdateViewCropMask)
	ON_COMMAND(ID_OPTIONS_ENDPOINTSNAP, OnOptionsEndpointSnap)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_ENDPOINTSNAP, OnUpdateOptionsEndpointSnap)
	ON_COMMAND(ID_OPTIONS_TRIM, OnOptionsTrim)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_TRIM, OnUpdateOptionsTrim)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDibLookApp construction
// Place all significant initialization in InitInstance

CDibLookApp::CDibLookApp()
{
	m_show_blockgrid = 0;
	m_show_cropmask = 0;
	m_endpoint_snap = 1;
	m_trim = 0;
	m_optimize_coding = 2;
	m_copyoption = 2;
	m_dither = 1;
	m_processing_mode = 0;
	m_copyfiletime = 1;
	m_zoom_reload = 1;
	m_progressive = 0;
	m_width = 208;
	m_height = 208;
	m_xoffset = -1;
	m_yoffset = -1;
	m_cropspec = 0;
	m_actual = 0;
	m_infname[0] = 0;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CDibLookApp object

CDibLookApp NEAR theApp;

void load_inf(void)
{
	FILE *fp;

	if ((fp = fopen(theApp.m_infname, "r")) == NULL)
		return;
	fscanf(fp, "OptimizeCoding = %d\n", &theApp.m_optimize_coding);
	if (theApp.m_optimize_coding < 0 || theApp.m_optimize_coding > 2)
		theApp.m_optimize_coding = 0;
	fscanf(fp, "CopyOption = %d\n", &theApp.m_copyoption);
	if (theApp.m_copyoption < 0 || theApp.m_copyoption > 2)
		theApp.m_copyoption = 2;
	fscanf(fp, "Dither = %d\n", &theApp.m_dither);
	if (theApp.m_dither < 0 || theApp.m_dither > 1)
		theApp.m_dither = 0;
	fscanf(fp, "ProcessingMode = %d\n", &theApp.m_processing_mode);
	if (theApp.m_processing_mode < 0 || theApp.m_processing_mode > 1)
		theApp.m_processing_mode = 0;
	fscanf(fp, "CopyFileTime = %d\n", &theApp.m_copyfiletime);
	if (theApp.m_copyfiletime < 0 || theApp.m_copyfiletime > 1)
		theApp.m_copyfiletime = 1;
	fscanf(fp, "ZoomReload = %d\n", &theApp.m_zoom_reload);
	if (theApp.m_zoom_reload < 0 || theApp.m_zoom_reload > 1)
		theApp.m_zoom_reload = 0;
	fscanf(fp, "Progressive = %d\n", &theApp.m_progressive);
	if (theApp.m_progressive < 0 || theApp.m_progressive > 1)
		theApp.m_progressive = 0;
	fclose(fp);
}

void save_inf(void)
{
	FILE *fp;

	if ((fp = fopen(theApp.m_infname, "w")) == NULL)
		return;
	fprintf(fp, "OptimizeCoding = %d\n", theApp.m_optimize_coding);
	fprintf(fp, "CopyOption = %d\n", theApp.m_copyoption);
	fprintf(fp, "Dither = %d\n", theApp.m_dither);
	fprintf(fp, "ProcessingMode = %d\n", theApp.m_processing_mode);
	fprintf(fp, "CopyFileTime = %d\n", theApp.m_copyfiletime);
	fprintf(fp, "ZoomReload = %d\n", theApp.m_zoom_reload);
	fprintf(fp, "Progressive = %d\n", theApp.m_progressive);
	fclose(fp);
}

/////////////////////////////////////////////////////////////////////////////
// CDibLookApp initialization

BOOL CDibLookApp::InitInstance()
{
	// Standard initialization
	//  (if you are not using these features and wish to reduce the size
	// of your final executable, you should remove the following initialization

	Enable3dControls();     // Use 3d controls in dialogs
	LoadStdProfileSettings(8);  // Load standard INI file options (including MRU)

	// Register document templates which serve as connection between
	//  documents and views.  Views are contained in the specified view

	AddDocTemplate(new CMultiDocTemplate(IDR_JPGTYPE,
			RUNTIME_CLASS(CDibDoc),
			RUNTIME_CLASS(CMDIChildWnd),        // standard MDI child frame
			RUNTIME_CLASS(CDibView)));

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();
	m_pMainWnd = pMainFrame;

	GetCurrentDirectory(sizeof(m_infname), m_infname);
	if (strlen(m_infname) && m_infname[strlen(m_infname) - 1] != '\\')
		strcat(m_infname, "\\");
	strcat(m_infname, "jpegcrop.inf");
	load_inf();

	// enable file manager drag/drop and DDE Execute open
	m_pMainWnd->DragAcceptFiles();

	EnableShellOpen();
	RegisterShellFileTypes(TRUE);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	if (cmdInfo.m_nShellCommand != CCommandLineInfo::FileOpen) {
		cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;
	}

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg() : CDialog(CAboutDlg::IDD)
		{
			//{{AFX_DATA_INIT(CAboutDlg)
			//}}AFX_DATA_INIT
		}

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
		enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CDibLookApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

void CDibLookApp::OnViewBlockGrid()
{
  m_show_blockgrid ^= 1;

  POSITION pos = GetFirstDocTemplatePosition();
  CDocTemplate *pDoct = GetNextDocTemplate( pos );
  if (pDoct == NULL) return;

  pos = pDoct->GetFirstDocPosition();
  while (pos != NULL)
  {
	  CDocument *pDoc = pDoct->GetNextDoc( pos );
	  pDoc->UpdateAllViews(NULL);
  }
}

void CDibLookApp::OnUpdateViewBlockGrid(CCmdUI *pCmdUI)
{
  pCmdUI->SetCheck(m_show_blockgrid);
}

void CDibLookApp::OnViewCropMask()
{
  m_show_cropmask ^= 1;

  POSITION pos = GetFirstDocTemplatePosition();
  CDocTemplate *pDoct = GetNextDocTemplate( pos );
  if (pDoct == NULL) return;

  pos = pDoct->GetFirstDocPosition();
  while (pos != NULL)
  {
	  CDocument *pDoc = pDoct->GetNextDoc( pos );
	  pDoc->UpdateAllViews(NULL);
  }
}

void CDibLookApp::OnUpdateViewCropMask(CCmdUI *pCmdUI)
{
  pCmdUI->SetCheck(m_show_cropmask);
}

void CDibLookApp::OnOptionsEndpointSnap()
{
  m_endpoint_snap ^= 1;
}

void CDibLookApp::OnUpdateOptionsEndpointSnap(CCmdUI *pCmdUI)
{
  pCmdUI->SetCheck(m_endpoint_snap);
}

void CDibLookApp::OnOptionsTrim()
{
  m_trim ^= 1;

  CDibDoc *pActiveDoc = NULL;

  CMDIFrameWnd *pFrame = (CMDIFrameWnd *) m_pMainWnd;
  if (pFrame)
  {
	// Get the active MDI child window.
	CMDIChildWnd *pChild = (CMDIChildWnd *) pFrame->GetActiveFrame();
	// or CMDIChildWnd *pChild = pFrame->MDIGetActive();
	if (pChild)
	  pActiveDoc = (CDibDoc *) pChild->GetActiveDocument();
  }

  POSITION pos = GetFirstDocTemplatePosition();
  CDocTemplate *pDoct = GetNextDocTemplate( pos );
  if (pDoct == NULL) return;

  pos = pDoct->GetFirstDocPosition();
  while (pos != NULL)
  {
	  CDocument *pDoc = pDoct->GetNextDoc( pos );
	  pDoc->UpdateAllViews(NULL, pDoc == pActiveDoc ? 1 : 3);
  }
}

void CDibLookApp::OnUpdateOptionsTrim(CCmdUI *pCmdUI)
{
  pCmdUI->SetCheck(m_trim);
}

/////////////////////////////////////////////////////////////////////////////
// CDibLookApp commands

class CPrefDlg : public CDialog
{
public:
	CPrefDlg() : CDialog(CPrefDlg::IDD)
		{
			//{{AFX_DATA_INIT(CPrefDlg)
			//}}AFX_DATA_INIT
		}
// Dialog Data
	//{{AFX_DATA(CPrefDlg)
		enum { IDD = IDD_PREFBOX };
	//}}AFX_DATA
	int m_optimize_coding;
	int m_copyoption;
	int m_processing_mode;
	int m_dither;
	int m_copyfiletime;
	int m_zoom_reload;
	int m_progressive;
// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnButton1();
	virtual void OnButton2();
	//{{AFX_MSG(CPrefDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

void CPrefDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefDlg)
	//}}AFX_DATA_MAP
}

BOOL CPrefDlg::OnInitDialog()
{
  if (m_optimize_coding == 0)
	((CButton *)GetDlgItem(IDC_RADIO4))->SetCheck(TRUE);
  else if (m_optimize_coding == 1)
	((CButton *)GetDlgItem(IDC_RADIO5))->SetCheck(TRUE);
  else
	((CButton *)GetDlgItem(IDC_RADIO6))->SetCheck(TRUE);
  if (m_copyoption == 0)
	((CButton *)GetDlgItem(IDC_RADIO1))->SetCheck(TRUE);
  else if (m_copyoption == 1)
	((CButton *)GetDlgItem(IDC_RADIO2))->SetCheck(TRUE);
  else
	((CButton *)GetDlgItem(IDC_RADIO3))->SetCheck(TRUE);
  if (m_processing_mode == 0)
	((CButton *)GetDlgItem(IDC_RADIO7))->SetCheck(TRUE);
  else
	((CButton *)GetDlgItem(IDC_RADIO8))->SetCheck(TRUE);
  if (m_dither)
	((CButton *)GetDlgItem(IDC_CHECK1))->SetCheck(TRUE);
  if (m_copyfiletime)
	((CButton *)GetDlgItem(IDC_CHECK2))->SetCheck(TRUE);
  if (m_zoom_reload)
	((CButton *)GetDlgItem(IDC_CHECK3))->SetCheck(TRUE);
  if (m_progressive)
	((CButton *)GetDlgItem(IDC_CHECK4))->SetCheck(TRUE);
  CDialog::OnInitDialog();
  return TRUE;
}

void CPrefDlg::OnOK()
{
  CDialog::OnOK();
  if (((CButton *)GetDlgItem(IDC_RADIO4))->GetCheck())
	((CDibLookApp *)AfxGetApp())->m_optimize_coding = 0;
  else if (((CButton *)GetDlgItem(IDC_RADIO5))->GetCheck())
	((CDibLookApp *)AfxGetApp())->m_optimize_coding = 1;
  else
	((CDibLookApp *)AfxGetApp())->m_optimize_coding = 2;
  if (((CButton *)GetDlgItem(IDC_RADIO1))->GetCheck())
	((CDibLookApp *)AfxGetApp())->m_copyoption = 0;
  else if (((CButton *)GetDlgItem(IDC_RADIO2))->GetCheck())
	((CDibLookApp *)AfxGetApp())->m_copyoption = 1;
  else
	((CDibLookApp *)AfxGetApp())->m_copyoption = 2;
  if (((CButton *)GetDlgItem(IDC_RADIO7))->GetCheck())
	((CDibLookApp *)AfxGetApp())->m_processing_mode = 0;
  else
	((CDibLookApp *)AfxGetApp())->m_processing_mode = 1;
  if (((CButton *)GetDlgItem(IDC_CHECK1))->GetCheck())
	((CDibLookApp *)AfxGetApp())->m_dither = 1;
  else
	((CDibLookApp *)AfxGetApp())->m_dither = 0;
  if (((CButton *)GetDlgItem(IDC_CHECK2))->GetCheck())
	((CDibLookApp *)AfxGetApp())->m_copyfiletime = 1;
  else
	((CDibLookApp *)AfxGetApp())->m_copyfiletime = 0;
  if (((CButton *)GetDlgItem(IDC_CHECK3))->GetCheck())
	((CDibLookApp *)AfxGetApp())->m_zoom_reload = 1;
  else
	((CDibLookApp *)AfxGetApp())->m_zoom_reload = 0;
  if (((CButton *)GetDlgItem(IDC_CHECK4))->GetCheck())
	((CDibLookApp *)AfxGetApp())->m_progressive = 1;
  else
	((CDibLookApp *)AfxGetApp())->m_progressive = 0;
}

void CPrefDlg::OnButton1()
{
  if (((CButton *)GetDlgItem(IDC_RADIO4))->GetCheck())
	((CDibLookApp *)AfxGetApp())->m_optimize_coding = 0;
  else if (((CButton *)GetDlgItem(IDC_RADIO5))->GetCheck())
	((CDibLookApp *)AfxGetApp())->m_optimize_coding = 1;
  else
	((CDibLookApp *)AfxGetApp())->m_optimize_coding = 2;
  if (((CButton *)GetDlgItem(IDC_RADIO1))->GetCheck())
	((CDibLookApp *)AfxGetApp())->m_copyoption = 0;
  else if (((CButton *)GetDlgItem(IDC_RADIO2))->GetCheck())
	((CDibLookApp *)AfxGetApp())->m_copyoption = 1;
  else
	((CDibLookApp *)AfxGetApp())->m_copyoption = 2;
  if (((CButton *)GetDlgItem(IDC_RADIO7))->GetCheck())
	((CDibLookApp *)AfxGetApp())->m_processing_mode = 0;
  else
	((CDibLookApp *)AfxGetApp())->m_processing_mode = 1;
  if (((CButton *)GetDlgItem(IDC_CHECK1))->GetCheck())
	((CDibLookApp *)AfxGetApp())->m_dither = 1;
  else
	((CDibLookApp *)AfxGetApp())->m_dither = 0;
  if (((CButton *)GetDlgItem(IDC_CHECK2))->GetCheck())
	((CDibLookApp *)AfxGetApp())->m_copyfiletime = 1;
  else
	((CDibLookApp *)AfxGetApp())->m_copyfiletime = 0;
  if (((CButton *)GetDlgItem(IDC_CHECK3))->GetCheck())
	((CDibLookApp *)AfxGetApp())->m_zoom_reload = 1;
  else
	((CDibLookApp *)AfxGetApp())->m_zoom_reload = 0;
  if (((CButton *)GetDlgItem(IDC_CHECK4))->GetCheck())
	((CDibLookApp *)AfxGetApp())->m_progressive = 1;
  else
	((CDibLookApp *)AfxGetApp())->m_progressive = 0;

  save_inf();
}

void CPrefDlg::OnButton2()
{
  load_inf();

  m_optimize_coding = ((CDibLookApp *)AfxGetApp())->m_optimize_coding;
  m_copyoption = ((CDibLookApp *)AfxGetApp())->m_copyoption;
  m_dither = ((CDibLookApp *)AfxGetApp())->m_dither;
  m_processing_mode = ((CDibLookApp *)AfxGetApp())->m_processing_mode;
  m_copyfiletime = ((CDibLookApp *)AfxGetApp())->m_copyfiletime;
  m_zoom_reload = ((CDibLookApp *)AfxGetApp())->m_zoom_reload;
  m_progressive = ((CDibLookApp *)AfxGetApp())->m_progressive;

  ((CButton *)GetDlgItem(IDC_RADIO1))->SetCheck(FALSE);
  ((CButton *)GetDlgItem(IDC_RADIO2))->SetCheck(FALSE);
  ((CButton *)GetDlgItem(IDC_RADIO3))->SetCheck(FALSE);
  ((CButton *)GetDlgItem(IDC_RADIO4))->SetCheck(FALSE);
  ((CButton *)GetDlgItem(IDC_RADIO5))->SetCheck(FALSE);
  ((CButton *)GetDlgItem(IDC_RADIO6))->SetCheck(FALSE);
  ((CButton *)GetDlgItem(IDC_RADIO7))->SetCheck(FALSE);
  ((CButton *)GetDlgItem(IDC_RADIO8))->SetCheck(FALSE);
  ((CButton *)GetDlgItem(IDC_CHECK1))->SetCheck(FALSE);
  ((CButton *)GetDlgItem(IDC_CHECK2))->SetCheck(FALSE);
  ((CButton *)GetDlgItem(IDC_CHECK3))->SetCheck(FALSE);
  ((CButton *)GetDlgItem(IDC_CHECK4))->SetCheck(FALSE);

  if (m_optimize_coding == 0)
	((CButton *)GetDlgItem(IDC_RADIO4))->SetCheck(TRUE);
  else if (m_optimize_coding == 1)
	((CButton *)GetDlgItem(IDC_RADIO5))->SetCheck(TRUE);
  else
	((CButton *)GetDlgItem(IDC_RADIO6))->SetCheck(TRUE);
  if (m_copyoption == 0)
	((CButton *)GetDlgItem(IDC_RADIO1))->SetCheck(TRUE);
  else if (m_copyoption == 1)
	((CButton *)GetDlgItem(IDC_RADIO2))->SetCheck(TRUE);
  else
	((CButton *)GetDlgItem(IDC_RADIO3))->SetCheck(TRUE);
  if (m_processing_mode == 0)
	((CButton *)GetDlgItem(IDC_RADIO7))->SetCheck(TRUE);
  else
	((CButton *)GetDlgItem(IDC_RADIO8))->SetCheck(TRUE);
  if (m_dither)
	((CButton *)GetDlgItem(IDC_CHECK1))->SetCheck(TRUE);
  if (m_copyfiletime)
	((CButton *)GetDlgItem(IDC_CHECK2))->SetCheck(TRUE);
  if (m_zoom_reload)
	((CButton *)GetDlgItem(IDC_CHECK3))->SetCheck(TRUE);
  if (m_progressive)
	((CButton *)GetDlgItem(IDC_CHECK4))->SetCheck(TRUE);
}

BEGIN_MESSAGE_MAP(CPrefDlg, CDialog)
	//{{AFX_MSG_MAP(CPrefDlg)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_BN_CLICKED(IDC_BUTTON2, OnButton2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CDibLookApp::OnFilePreferences()
{
	CPrefDlg prefDlg;
	prefDlg.m_optimize_coding = m_optimize_coding;
	prefDlg.m_copyoption = m_copyoption;
	prefDlg.m_dither = m_dither;
	prefDlg.m_processing_mode = m_processing_mode;
	prefDlg.m_copyfiletime = m_copyfiletime;
	prefDlg.m_zoom_reload = m_zoom_reload;
	prefDlg.m_progressive = m_progressive;
	prefDlg.DoModal();
}
