@echo off
REM ***********************************************************************
REM
REM Copyright (c) 2002+ MobileWizardry
REM All rights reservered.
REM
REM ***********************************************************************

REM @(#)msmob_build.bat
REM
REM -- Aaron Ardiri (mailto:aaron_ardiri@mobilewizardry.com)
REM

set OSVERSION=WCE300
set TARGETCPU=ARM
set PLATFORM="MS Pocket PC"
set WCEROOT=C:\Program Files\Microsoft eMbedded Tools
set SDKROOT=C:\Program Files\Windows CE Tools

set SDK=%SDKROOT%\%OSVERSION%\%PLATFORM%

set PATH=%WCEROOT%\Common\EVC\bin;%WCEROOT%/EVC/%OSVERSION%/bin:%PATH%
set INCLUDE=%SDK%/include;%SDK%/MFC/include;%SDK%/ATL/include
set LIB=%SDK%/lib/arm;%SDK%/MFC/lib/arm;%SDK%/ATL/lib/arm
set CC=clarm
set CFG=none

evc SHARK.vcp /make "SHARK - Win32 (WCE ARM) Release" /useenv /ceconfig="Pocket PC" /rebuild
mv ARMRel/SHARK.lib SHARK.lib

evc SHARK.vcp /make "SHARK - Win32 (WCE ARM) Debug" /useenv /ceconfig="Pocket PC" /rebuild
mv ARMDbg/SHARK.LIB SHARK-debug.lib

REM clean up

rm -rf ARMRel
rm -rf ARMDbg
rm -f SHARK.vcl
