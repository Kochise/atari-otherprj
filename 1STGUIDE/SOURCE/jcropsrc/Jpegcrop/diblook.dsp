# Microsoft Developer Studio Project File - Name="DibLook" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=DibLook - Win32 Release
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "diblook.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "diblook.mak" CFG="DibLook - Win32 Release"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "DibLook - Win32 Release" (basierend auf  "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe
# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 1
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G6 /MT /W3 /GX /Ox /Oa /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FD /c
# SUBTRACT CPP /Fr /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 ..\jpeg-9b\Release\jpeg.lib /nologo /subsystem:windows /machine:I386 /out:".\Release/jpegcrop.exe"
# SUBTRACT LINK32 /pdb:none
# Begin Target

# Name "DibLook - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\dibapi.cpp
# End Source File
# Begin Source File

SOURCE=.\dibdoc.cpp
# End Source File
# Begin Source File

SOURCE=.\diblmac.r
# End Source File
# Begin Source File

SOURCE=.\diblook.cpp
# End Source File
# Begin Source File

SOURCE=.\diblook.rc
# End Source File
# Begin Source File

SOURCE=.\dibtran.cpp
# End Source File
# Begin Source File

SOURCE=.\dibview.cpp
# End Source File
# Begin Source File

SOURCE=.\docrop.cpp
# End Source File
# Begin Source File

SOURCE=.\jpegdib.cpp
# End Source File
# Begin Source File

SOURCE=.\mainfrm.cpp
# End Source File
# Begin Source File

SOURCE=.\OXTabClientWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\stdafx.cpp
# ADD CPP /Yc
# End Source File
# Begin Source File

SOURCE=.\transupp.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;inl;fi;fd"
# Begin Source File

SOURCE=.\dibapi.h
# End Source File
# Begin Source File

SOURCE=.\dibdoc.h
# End Source File
# Begin Source File

SOURCE=.\diblook.h
# End Source File
# Begin Source File

SOURCE=.\dibview.h
# End Source File
# Begin Source File

SOURCE=.\docrop.h
# End Source File
# Begin Source File

SOURCE=.\jconfig.h
# End Source File
# Begin Source File

SOURCE=.\jerror.h
# End Source File
# Begin Source File

SOURCE=.\jinclude.h
# End Source File
# Begin Source File

SOURCE=.\jmorecfg.h
# End Source File
# Begin Source File

SOURCE=.\jpegdib.h
# End Source File
# Begin Source File

SOURCE=.\jpegint.h
# End Source File
# Begin Source File

SOURCE=.\jpeglib.h
# End Source File
# Begin Source File

SOURCE=.\mainfrm.h
# End Source File
# Begin Source File

SOURCE=.\OXTabClientWnd.h
# End Source File
# Begin Source File

SOURCE=.\stdafx.h
# End Source File
# Begin Source File

SOURCE=.\transupp.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=".\res\Abendrot00-96pF.bmp"
# End Source File
# Begin Source File

SOURCE=.\res\jpegcrop.ico
# End Source File
# Begin Source File

SOURCE=.\res\jpegdoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ToolbarDraw.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\res\jpegcrop.exe.manifest
# End Source File
# End Target
# End Project
