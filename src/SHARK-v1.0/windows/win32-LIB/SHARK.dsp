# Microsoft Developer Studio Project File - Name="SHARK" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=SHARK - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SHARK.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SHARK.mak" CFG="SHARK - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SHARK - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe
# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Target

# Name "SHARK - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE="..\DAL-audio.inc"
# End Source File
# Begin Source File

SOURCE="..\DAL-file.inc"
# End Source File
# Begin Source File

SOURCE="..\DAL-gfx-fonts.inc"
# End Source File
# Begin Source File

SOURCE="..\DAL-gfx-lib.inc"
# End Source File
# Begin Source File

SOURCE="..\DAL-gfx.inc"
# End Source File
# Begin Source File

SOURCE="..\DAL-io.inc"
# End Source File
# Begin Source File

SOURCE="..\DAL-memory.inc"
# End Source File
# Begin Source File

SOURCE="..\DAL-platform.inc"
# End Source File
# Begin Source File

SOURCE="..\DAL-string.inc"
# End Source File
# Begin Source File

SOURCE="..\DAL-system.inc"
# End Source File
# Begin Source File

SOURCE=..\DAL.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\application_resource.h
# End Source File
# Begin Source File

SOURCE="..\..\DAL-configure.h"
# End Source File
# Begin Source File

SOURCE="..\DAL-gfx-palette.h"
# End Source File
# Begin Source File

SOURCE="..\DAL-prv.h"
# End Source File
# Begin Source File

SOURCE=..\..\DAL.h
# End Source File
# Begin Source File

SOURCE=..\resource.h
# End Source File
# Begin Source File

SOURCE=..\StdAfx.h
# End Source File
# End Group
# End Target
# End Project
