@echo off
c:
cd c:\_tmp\symbian_s60

rem --
rem -- this conflicts (from cygwin) - must remove *g*
rem -- 

set MAKEFLAGS=

rem --
rem -- need the WIN32 path environment variables
rem -- 

set SDK=Series60

set EPOCROOT=\Symbian\%SDK%\

set PATH_BIN=c:\Bin
set PATH_PERL=c:\perl\bin
set PATH_TOOLS=c:\Symbian\%SDK%\Epoc32\tools
set PATH_GCC=c:\Symbian\%SDK%\Epoc32\gcc\bin
set PATH_JAVA=c:\Progra~1\j2sdk1.4.1\bin
set PATH_WIN=c:\WINDOWS\system32

set PATH=%PATH_BIN%;%PATH_PERL%;%PATH_TOOLS%;%PATH_GCC%;%PATH_JAVA%;%PATH_WIN%

@echo on
cmd /c bldmake bldfiles
cmd /c makmake SHARK armi
cmd /c make -f SHARK.armi
