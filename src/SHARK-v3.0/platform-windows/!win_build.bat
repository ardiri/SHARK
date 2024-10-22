@echo off
REM ***********************************************************************
REM
REM Copyright (c) 2002+ MobileWizardry
REM All rights reservered.
REM
REM ***********************************************************************

REM @(#)win_build.bat
REM
REM -- Aaron Ardiri (mailto:aaron_ardiri@mobilewizardry.com)
REM

SET VSCOMMONDIR=C:\PROGRA~1\MICROS~2\Common
SET MSDEVDIR=C:\PROGRA~1\MICROS~2\Common\MSDEV98
SET MSVCDIR=C:\PROGRA~1\MICROS~2\VC98

SET VSOSDIR=WIN95
IF "%OS%" == "Windows_NT" SET VSOSDIR=WINNT

SET PATH=%MSDEVDIR%\BIN;%MSVCDIR%\BIN;%VSCOMMONDIR%\TOOLS\%VSOSDIR%;%VSCOMMONDIR%\TOOLS;%PATH%
SET INCLUDE=%MSVCDIR%\ATL\INCLUDE;%MSVCDIR%\INCLUDE;%MSVCDIR%\MFC\INCLUDE;%INCLUDE%
SET LIB=%MSVCDIR%\LIB;%MSVCDIR%\MFC\LIB;%LIB%

SET VSOSDIR=
SET VSCOMMONDIR=

msdev SHARK.dsw /make "SHARK - Win32 Release" /rebuild
mv Release/SHARK.lib SHARK.lib

msdev SHARK.dsw /make "SHARK - Win32 Debug" /rebuild
mv Debug/SHARK.lib SHARK-debug.lib

REM clean up
rm -rf Release
rm -rf Debug
rm -f SHARK.plg 
