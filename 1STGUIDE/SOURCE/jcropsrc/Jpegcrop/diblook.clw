; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CMainFrame
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "diblook.h"

ClassCount=5
Class1=CDibDoc
Class2=CDibLookApp
Class3=CAboutDlg
Class4=CDibView
Class5=CMainFrame

ResourceCount=10
Resource1=IDR_DIBTYPE
Resource2=IDR_MAINFRAME
Resource3=IDR_JPGTYPE (Englisch (USA))
Resource4=IDR_DIBTYPE (Englisch (USA))
Resource5=IDD_ABOUTBOX (Englisch (USA))
Resource6=IDR_MAINFRAME (Englisch (USA))
Resource7=IDR_POPUP1 (Englisch (USA))
Resource8=IDD_FRAMEBOX (Englisch (USA))
Resource9=IDD_PREFBOX (Englisch (USA))
Resource10=IDB_MAINFRAME (Englisch (USA))

[CLS:CDibDoc]
Type=0
HeaderFile=dibdoc.h
ImplementationFile=dibdoc.cpp

[CLS:CDibLookApp]
Type=0
HeaderFile=diblook.h
ImplementationFile=diblook.cpp

[CLS:CAboutDlg]
Type=0
HeaderFile=diblook.cpp
ImplementationFile=diblook.cpp

[CLS:CDibView]
Type=0
HeaderFile=dibview.h
ImplementationFile=dibview.cpp
Filter=C
BaseClass=CScrollView
VirtualFilter=VWC

[CLS:CMainFrame]
Type=0
HeaderFile=mainfrm.h
ImplementationFile=mainfrm.cpp

[MNU:IDR_MAINFRAME]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_PRINT_SETUP
Command4=ID_FILE_MRU_FILE1
Command5=ID_APP_EXIT
Command6=ID_VIEW_TOOLBAR
Command7=ID_VIEW_STATUS_BAR
Command8=ID_APP_ABOUT
CommandCount=8

[MNU:IDR_DIBTYPE]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_CLOSE
Command4=ID_FILE_SAVE
Command5=ID_FILE_SAVE_AS
Command6=ID_FILE_PRINT
Command7=ID_FILE_PRINT_PREVIEW
Command8=ID_FILE_PRINT_SETUP
Command9=ID_FILE_MRU_FILE1
Command10=ID_APP_EXIT
Command11=ID_EDIT_UNDO
Command12=ID_EDIT_CUT
Command13=ID_EDIT_COPY
Command14=ID_EDIT_PASTE
Command15=ID_VIEW_TOOLBAR
Command16=ID_VIEW_STATUS_BAR
Command17=ID_WINDOW_NEW
Command18=ID_WINDOW_CASCADE
Command19=ID_WINDOW_TILE_HORZ
Command20=ID_WINDOW_ARRANGE
Command21=ID_APP_ABOUT
CommandCount=21

[ACL:IDR_MAINFRAME]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_FILE_PRINT
Command5=ID_EDIT_UNDO
Command6=ID_EDIT_CUT
Command7=ID_EDIT_COPY
Command8=ID_EDIT_PASTE
Command9=ID_EDIT_UNDO
Command10=ID_EDIT_CUT
Command11=ID_EDIT_COPY
Command12=ID_EDIT_PASTE
Command13=ID_NEXT_PANE
Command14=ID_PREV_PANE
CommandCount=14

[DLG:IDD_ABOUTBOX (Englisch (USA))]
Type=1
Class=?
ControlCount=8
Control1=IDC_STATIC,static,1342177294
Control2=IDC_STATIC,static,1342177283
Control3=IDC_STATIC,static,1342308352
Control4=IDC_STATIC,static,1342308352
Control5=IDC_STATIC,static,1342308352
Control6=IDC_STATIC,static,1342308352
Control7=IDOK,button,1342373889
Control8=IDC_STATIC,static,1342308352

[MNU:IDR_MAINFRAME (Englisch (USA))]
Type=1
Class=?
Command1=ID_FILE_OPEN
Command2=ID_FILE_PREFERENCES
Command3=ID_FILE_PRINT_SETUP
Command4=ID_FILE_MRU_FILE1
Command5=ID_APP_EXIT
Command6=ID_OPTIONS_ENDPOINTSNAP
Command7=ID_OPTIONS_TRIM
Command8=ID_VIEW_TOOLBAR
Command9=ID_VIEW_STATUS_BAR
Command10=ID_VIEW_TITLEBAR
Command11=ID_VIEW_TAB_BAR
Command12=ID_VIEW_BLOCKGRID
Command13=ID_VIEW_CROPMASK
Command14=ID_APP_ABOUT
CommandCount=14

[MNU:IDR_DIBTYPE (Englisch (USA))]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_CLOSE
Command4=ID_FILE_SAVE
Command5=ID_FILE_SAVE_AS
Command6=ID_FILE_PRINT
Command7=ID_FILE_PRINT_PREVIEW
Command8=ID_FILE_PRINT_SETUP
Command9=ID_FILE_MRU_FILE1
Command10=ID_APP_EXIT
Command11=ID_EDIT_UNDO
Command12=ID_EDIT_CUT
Command13=ID_EDIT_COPY
Command14=ID_EDIT_PASTE
Command15=ID_VIEW_TOOLBAR
Command16=ID_VIEW_STATUS_BAR
Command17=ID_WINDOW_NEW
Command18=ID_WINDOW_CASCADE
Command19=ID_WINDOW_TILE_HORZ
Command20=ID_WINDOW_ARRANGE
Command21=ID_APP_ABOUT
CommandCount=21

[ACL:IDR_MAINFRAME (Englisch (USA))]
Type=1
Class=?
Command1=ID_EDIT_SELECT_ALL
Command2=ID_EDIT_COPY
Command3=ID_FILE_NEW
Command4=ID_FILE_OPEN
Command5=ID_FILE_PRINT
Command6=ID_APP_EXIT
Command7=ID_FILE_SAVE_COPY_AS
Command8=ID_EDIT_PASTE
Command9=ID_EDIT_UNDO
Command10=ID_EDIT_CUT
Command11=ID_NEXT_PANE
Command12=ID_PREV_PANE
Command13=ID_EDIT_COPY
Command14=ID_EDIT_PASTE
Command15=ID_FILE_CLOSE
Command16=ID_EDIT_CUT
Command17=ID_EDIT_UNDO
CommandCount=17

[DLG:IDD_PREFBOX (Englisch (USA))]
Type=1
Class=?
ControlCount=19
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_BUTTON1,button,1342242816
Control4=IDC_BUTTON2,button,1342242816
Control5=IDC_STATIC,button,1342308359
Control6=IDC_RADIO4,button,1342177289
Control7=IDC_RADIO5,button,1342177289
Control8=IDC_RADIO6,button,1342177289
Control9=IDC_STATIC,button,1342308359
Control10=IDC_RADIO1,button,1342177289
Control11=IDC_RADIO2,button,1342177289
Control12=IDC_RADIO3,button,1342177289
Control13=IDC_STATIC,button,1342308359
Control14=IDC_RADIO7,button,1342177289
Control15=IDC_RADIO8,button,1342177289
Control16=IDC_CHECK1,button,1342242819
Control17=IDC_CHECK3,button,1342242819
Control18=IDC_CHECK2,button,1342242819
Control19=IDC_CHECK4,button,1342242819

[DLG:IDD_FRAMEBOX (Englisch (USA))]
Type=1
Class=?
ControlCount=15
Control1=IDOK,button,1342242817
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,button,1342177287
Control4=IDC_RADIO1,button,1342177289
Control5=IDC_RADIO2,button,1342177289
Control6=IDC_STATIC,static,1342308352
Control7=IDC_EDIT1,edit,1350631552
Control8=IDC_STATIC,static,1342308352
Control9=IDC_EDIT2,edit,1350631552
Control10=IDC_STATIC,static,1342308352
Control11=IDC_EDIT3,edit,1350631552
Control12=IDC_STATIC,static,1342308352
Control13=IDC_EDIT4,edit,1350631552
Control14=IDC_CHECK1,button,1342242819
Control15=IDC_EDIT5,edit,1350631552

[MNU:IDR_JPGTYPE (Englisch (USA))]
Type=1
Class=?
Command1=ID_FILE_OPEN
Command2=ID_FILE_CLOSE
Command3=ID_FILE_SAVE_COPY_AS
Command4=ID_FILE_PREFERENCES
Command5=ID_FILE_PRINT
Command6=ID_FILE_PRINT_PREVIEW
Command7=ID_FILE_PRINT_SETUP
Command8=ID_FILE_MRU_FILE1
Command9=ID_APP_EXIT
Command10=ID_OPTIONS_SAVEZOOM
Command11=ID_OPTIONS_ENDPOINTSNAP
Command12=ID_OPTIONS_TRIM
Command13=ID_OPTIONS_GRAYSCALE
Command14=ID_EDIT_UNDO
Command15=ID_EDIT_SELECT_ALL
Command16=ID_TRANSFORM_ROT_90
Command17=ID_TRANSFORM_ROT_270
Command18=ID_TRANSFORM_FLIP_H
Command19=ID_TRANSFORM_FLIP_V
Command20=ID_TRANSFORM_ROT_180
Command21=ID_TRANSFORM_TRANSPOSE
Command22=ID_TRANSFORM_TRANSVERSE
Command23=ID_TRANSFORM_ORIGINAL
Command24=ID_TRANSFORM_WIPE
Command25=ID_VIEW_TOOLBAR
Command26=ID_VIEW_STATUS_BAR
Command27=ID_VIEW_TITLEBAR
Command28=ID_VIEW_TAB_BAR
Command29=ID_VIEW_BLOCKGRID
Command30=ID_VIEW_CROPMASK
Command31=ID_VIEW_ZOOM1
Command32=ID_VIEW_ZOOM2
Command33=ID_VIEW_ZOOM3
Command34=ID_VIEW_ZOOM4
Command35=ID_VIEW_ZOOM5
Command36=ID_VIEW_ZOOM6
Command37=ID_VIEW_ZOOM7
Command38=ID_VIEW_ZOOM8
Command39=ID_VIEW_ZOOM9
Command40=ID_VIEW_ZOOM10
Command41=ID_VIEW_ZOOM11
Command42=ID_VIEW_ZOOM12
Command43=ID_VIEW_ZOOM13
Command44=ID_VIEW_ZOOM14
Command45=ID_VIEW_ZOOM15
Command46=ID_VIEW_ZOOM16
Command47=ID_WINDOW_NEW
Command48=ID_WINDOW_CASCADE
Command49=ID_WINDOW_TILE_HORZ
Command50=ID_WINDOW_ARRANGE
Command51=ID_APP_ABOUT
CommandCount=51

[MNU:IDR_POPUP1 (Englisch (USA))]
Type=1
Class=?
Command1=ID_POPUP1_MOVEFRAME
Command2=ID_POPUP1_DEFINEFRAME
Command3=ID_POPUP1_SAVEFRAMEAS
CommandCount=3

[TB:IDB_MAINFRAME (Englisch (USA))]
Type=1
Class=?
Command1=ID_BUTTON32826
Command2=ID_BUTTON32827
Command3=ID_BUTTON32828
Command4=ID_BUTTON32829
Command5=ID_BUTTON32830
Command6=ID_BUTTON32831
Command7=ID_BUTTON32832
Command8=ID_BUTTON32833
Command9=ID_BUTTON32834
Command10=ID_BUTTON32835
Command11=ID_BUTTON32836
Command12=ID_BUTTON32837
Command13=ID_BUTTON32838
Command14=ID_BUTTON32839
Command15=ID_BUTTON32840
Command16=ID_BUTTON32841
Command17=ID_BUTTON32842
Command18=ID_BUTTON32843
CommandCount=18

