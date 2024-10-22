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

msdev SHARK.dsw /make "SHARK - Win32 Release" /rebuild
mv Release/SHARK.lib SHARK.lib

msdev SHARK.dsw /make "SHARK - Win32 Debug" /rebuild
mv Debug/SHARK.lib SHARK-debug.lib

REM clean up
rm -rf Release
rm -rf Debug
rm -f SHARK.plg
