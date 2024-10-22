# ***********************************************************************
#
# Copyright (c) 2001+ MobileWizardry
# All rights reservered.
#
# ***********************************************************************/

#
# @(#)Makefile
#

# compile requirements
default:
	make clean

	echo "#define TARGET_WINDOWS"           >  ../SHARK-cfg.h
	echo "#define LOG_DISABLE 1"             >> ../SHARK-cfg.h

	./\!win_build.bat
	mv SHARK.lib SHARK.no_log.lib
	mv SHARK-debug.lib SHARK-debug.no_log.lib

	echo "#define TARGET_WINDOWS"           >  ../SHARK-cfg.h
	echo "#define LOG_DISABLE 0"             >> ../SHARK-cfg.h
	./\!win_build.bat

	rm ../SHARK-cfg.h

clean:
	rm -f *.lib
