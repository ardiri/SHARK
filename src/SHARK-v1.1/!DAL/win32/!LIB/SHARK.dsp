# Microsoft Developer Studio Project File - Name="SHARK" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=SHARK - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SHARK.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SHARK.mak" CFG="SHARK - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SHARK - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "SHARK - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SHARK - Win32 Release"

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

!ELSEIF  "$(CFG)" == "SHARK - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "SHARK - Win32 Release"
# Name "SHARK - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\DAL.c
# End Source File
# Begin Source File

SOURCE=..\file\DAL_FileClose.c
# End Source File
# Begin Source File

SOURCE=..\file\DAL_FileCreate.c
# End Source File
# Begin Source File

SOURCE=..\file\DAL_FileDelete.c
# End Source File
# Begin Source File

SOURCE=..\file\DAL_FileEOF.c
# End Source File
# Begin Source File

SOURCE=..\file\DAL_FileInitialize.c
# End Source File
# Begin Source File

SOURCE=..\file\DAL_FileOpen.c
# End Source File
# Begin Source File

SOURCE=..\file\DAL_FileRead.c
# End Source File
# Begin Source File

SOURCE=..\file\DAL_FileSeek.c
# End Source File
# Begin Source File

SOURCE=..\file\DAL_FileTell.c
# End Source File
# Begin Source File

SOURCE=..\file\DAL_FileTerminate.c
# End Source File
# Begin Source File

SOURCE=..\file\DAL_FileWrite.c
# End Source File
# Begin Source File

SOURCE=..\file\DAL_FindFile.c
# End Source File
# Begin Source File

SOURCE=..\input\DAL_InpInitialize.c
# End Source File
# Begin Source File

SOURCE=..\input\DAL_InpTerminate.c
# End Source File
# Begin Source File

SOURCE=..\input\DAL_KeyCurrentState.c
# End Source File
# Begin Source File

SOURCE=..\input\DAL_KeyCurrentStateAnalog.c
# End Source File
# Begin Source File

SOURCE=..\input\DAL_KeyHandleEvent.c
# End Source File
# Begin Source File

SOURCE=..\display\DAL_LCDGetProperties.c
# End Source File
# Begin Source File

SOURCE=..\display\DAL_LCDInitialize.c
# End Source File
# Begin Source File

SOURCE=..\display\DAL_LCDRepaintRegion.c
# End Source File
# Begin Source File

SOURCE=..\display\DAL_LCDSetPalette.c
# End Source File
# Begin Source File

SOURCE=..\display\DAL_LCDTerminate.c
# End Source File
# Begin Source File

SOURCE=..\memory\DAL_MemInitialize.c
# End Source File
# Begin Source File

SOURCE=..\memory\DAL_MemPtrFree.c
# End Source File
# Begin Source File

SOURCE=..\memory\DAL_MemPtrNew.c
# End Source File
# Begin Source File

SOURCE=..\memory\DAL_MemPtrNewX.c
# End Source File
# Begin Source File

SOURCE=..\memory\DAL_MemSemaphore.c
# End Source File
# Begin Source File

SOURCE=..\memory\DAL_MemSetMaximum.c
# End Source File
# Begin Source File

SOURCE=..\memory\DAL_MemTerminate.c
# End Source File
# Begin Source File

SOURCE=..\input\DAL_PenState.c
# End Source File
# Begin Source File

SOURCE=..\platform\DAL_PlatformGetResource.c
# End Source File
# Begin Source File

SOURCE=..\platform\DAL_PlatformGetResourceSize.c
# End Source File
# Begin Source File

SOURCE=..\platform\DAL_PlatformGetUniqueID.c
# End Source File
# Begin Source File

SOURCE=..\platform\DAL_PlatformGetUniqueIDCODE.c
# End Source File
# Begin Source File

SOURCE=..\platform\DAL_PlatformInitialize.c
# End Source File
# Begin Source File

SOURCE=..\platform\DAL_PlatformKillPreference.c
# End Source File
# Begin Source File

SOURCE=..\platform\DAL_PlatformLoadPreference.c
# End Source File
# Begin Source File

SOURCE=..\platform\DAL_PlatformLoadPreferences.c
# End Source File
# Begin Source File

SOURCE=..\platform\DAL_PlatformReleaseResource.c
# End Source File
# Begin Source File

SOURCE=..\platform\DAL_PlatformSavePreference.c
# End Source File
# Begin Source File

SOURCE=..\platform\DAL_PlatformSavePreferences.c
# End Source File
# Begin Source File

SOURCE=..\platform\DAL_PlatformTerminate.c
# End Source File
# Begin Source File

SOURCE=..\audio\DAL_PlaySound.c
# End Source File
# Begin Source File

SOURCE=..\audio\DAL_SndInitialize.c
# End Source File
# Begin Source File

SOURCE=..\audio\DAL_SndStreamAvailable.c
# End Source File
# Begin Source File

SOURCE=..\audio\DAL_SndStreamCreate.c
# End Source File
# Begin Source File

SOURCE=..\audio\DAL_SndStreamDelete.c
# End Source File
# Begin Source File

SOURCE=..\audio\DAL_SndStreamStart.c
# End Source File
# Begin Source File

SOURCE=..\audio\DAL_SndStreamStop.c
# End Source File
# Begin Source File

SOURCE=..\audio\DAL_SndTerminate.c
# End Source File
# Begin Source File

SOURCE=..\audio\DAL_SndThread.c
# End Source File
# Begin Source File

SOURCE=..\audio\DAL_SndToneGenCallback.c
# End Source File
# Begin Source File

SOURCE=..\audio\DAL_SndWaveOutProc.c
# End Source File
# Begin Source File

SOURCE=..\system\DAL_SysDebugMessage.c
# End Source File
# Begin Source File

SOURCE=..\system\DAL_SysDelay.c
# End Source File
# Begin Source File

SOURCE=..\system\DAL_SysGetTicks.c
# End Source File
# Begin Source File

SOURCE=..\system\DAL_SysGetTime.c
# End Source File
# Begin Source File

SOURCE=..\system\DAL_SysInitialize.c
# End Source File
# Begin Source File

SOURCE=..\system\DAL_SysSetFPS.c
# End Source File
# Begin Source File

SOURCE=..\system\DAL_SysTerminate.c
# End Source File
# Begin Source File

SOURCE=..\system\DAL_SysTimeLong.c
# End Source File
# Begin Source File

SOURCE=..\system\DAL_TimerProc.c
# End Source File
# Begin Source File

SOURCE="..\..\..\file\SHARK-_FileClose.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\file\SHARK-_FileCreate.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\file\SHARK-_FileDelete.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\file\SHARK-_FileEOF.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\file\SHARK-_FileOpen.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\file\SHARK-_FileRead.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\file\SHARK-_FileSeek.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\file\SHARK-_FileTell.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\file\SHARK-_FileWrite.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\file\SHARK-_FindFile.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_FntDefineFont.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_FntGetCharsWidth.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_FntGetCharWidth.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_FntGetFont.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_FntGetFontHeight.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_FntGetWordWrap.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_FntReleaseFont.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_FntSetFont.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx16\SHARK-_Gfx16GetProperties.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx16\SHARK-_Gfx16Initialize.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx16\SHARK-_Gfx16Repaint.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx16\SHARK-_Gfx16RepaintRegion.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx16\SHARK-_Gfx16Terminate.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_GfxClearWindow.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_GfxCopyRegion.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_GfxCopyToDisplay.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_GfxCopyToDisplay_12bpp.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_GfxCopyToDisplay_16bpp.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_GfxCopyToDisplay_1bpp.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_GfxCopyToDisplay_2bpp.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_GfxCopyToDisplay_4bpp.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_GfxCopyToDisplay_8bpp.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_GfxCreateWindow.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_GfxCreateWindowFromBuffer.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_GfxCreateWindowFromBufferDisposable.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_GfxDeleteWindow.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_GfxDisplayInfo.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_GfxDrawChar.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_GfxDrawLine.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_GfxDrawRectangle.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_GfxDrawString.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_GfxFillRegion.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_GfxGetClip.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_GfxGetDrawWindow.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_GfxGetPalette.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_GfxGetPaletteIndex.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_GfxGetPixel.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_GfxGetWindowInfo.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_GfxInitialize.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_GfxInvertRegion.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_GfxRepaint.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_GfxRepaintRegion.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_GfxResetClip.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_GfxSetClip.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_GfxSetDrawWindow.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_GfxSetPalette.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_GfxSetPixel.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_GfxSetState.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\gfx\SHARK-_GfxTerminate.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\input\SHARK-_KeyAnalogAvailable.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\input\SHARK-_KeyCurrentState.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\input\SHARK-_KeyCurrentStateAnalog.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\input\SHARK-_KeyGetMask.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\input\SHARK-_KeyHandleEvent.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\input\SHARK-_KeySetMask.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\input\SHARK-_KeySetNumericReporting.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\display\SHARK-_LCDGetProperties.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\display\SHARK-_LCDRepaint.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\display\SHARK-_LCDRepaintRegion.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\display\SHARK-_LCDSetPalette.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\memory\SHARK-_MemChr.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\memory\SHARK-_MemCompare.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\memory\SHARK-_MemCopy.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\memory\SHARK-_MemMove.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\memory\SHARK-_MemPtrFree.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\memory\SHARK-_MemPtrNew.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\memory\SHARK-_MemPtrNewX.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\memory\SHARK-_MemPtrSize.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\memory\SHARK-_MemSemaphore.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\memory\SHARK-_MemSet.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\memory\SHARK-_MemSetMaximum.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\input\SHARK-_PenAvailable.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\input\SHARK-_PenDisplayToWindow.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\input\SHARK-_PenHandleEvent.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\input\SHARK-_PenState.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\audio\SHARK-_PlaySound.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\audio\SHARK-_SndStreamAvailable.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\audio\SHARK-_SndStreamCreate.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\audio\SHARK-_SndStreamDelete.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\audio\SHARK-_SndStreamStart.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\audio\SHARK-_SndStreamStop.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\string\SHARK-_StrAToD.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\string\SHARK-_StrAToF.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\string\SHARK-_StrAToH.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\string\SHARK-_StrAToI.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\string\SHARK-_StrAToO.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\string\SHARK-_StrCaselessCompare.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\string\SHARK-_StrCat.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\string\SHARK-_StrChr.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\string\SHARK-_StrCompare.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\string\SHARK-_StrCopy.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\string\SHARK-_StrDToA.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\string\SHARK-_StrFToA.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\string\SHARK-_StrHToA.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\string\SHARK-_StrInitialize.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\string\SHARK-_StrIToA.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\string\SHARK-_StrLen.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\string\SHARK-_StrNCaselessCompare.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\string\SHARK-_StrNCat.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\string\SHARK-_StrNChr.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\string\SHARK-_StrNCompare.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\string\SHARK-_StrNCopy.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\string\SHARK-_StrNStr.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\string\SHARK-_StrOToA.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\string\SHARK-_StrSet.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\string\SHARK-_StrStr.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\string\SHARK-_StrTerminate.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\system\SHARK-_SysDebugMessage.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\system\SHARK-_SysDelay.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\system\SHARK-_SysEnqueueEvent.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\system\SHARK-_SysEnqueueQuitEvent.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\system\SHARK-_SysEventAvailable.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\system\SHARK-_SysFlushEventQueue.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\system\SHARK-_SysGetEvent.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\system\SHARK-_SysGetFPS.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\system\SHARK-_SysGetGlobals.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\system\SHARK-_SysGetTicks.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\system\SHARK-_SysGetTime.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\system\SHARK-_SysRandom.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\system\SHARK-_SysSetFPS.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\system\SHARK-_SysSetGlobals.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\ui\SHARK-_UIHandleEvent.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\ui\SHARK-_UIInitialize.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\ui\SHARK-_UIMenuAddItem.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\ui\SHARK-_UIMenuAddSeparator.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\ui\SHARK-_UIMenuComplete.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\ui\SHARK-_UIMenuGetHeight.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\ui\SHARK-_UIMenuReset.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\ui\SHARK-_UIMenuSetButton.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\ui\SHARK-_UIMenuSetFont.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\ui\SHARK-_UIMenuSetMenuTriggerRegion.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\ui\SHARK-_UITerminate.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\platform\SHARK-PlatformGetResource.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\platform\SHARK-PlatformGetResourceSize.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\platform\SHARK-PlatformGetUniqueID.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\platform\SHARK-PlatformGetUniqueIDCODE.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\platform\SHARK-PlatformKillPreference.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\platform\SHARK-PlatformLoadPreference.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\platform\SHARK-PlatformLoadPreferences.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\platform\SHARK-PlatformReleaseResource.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\platform\SHARK-PlatformResourceExists.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\platform\SHARK-PlatformSavePreference.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\platform\SHARK-PlatformSavePreferences.c"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# End Target
# End Project
