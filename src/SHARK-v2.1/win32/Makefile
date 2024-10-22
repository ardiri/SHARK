# ***********************************************************************
#
# Copyright (c) 2002+ MobileWizardry
# All rights reservered.
#
# ***********************************************************************/

# @(#)Makefile
#
# -- Aaron Ardiri (mailto:aaron_ardiri@mobilewizardry.com)
#

# compile requirements
default:
	rm -f *.lib
	echo "#define LOG_DISABLE 1" > ../SHARK-cfg.h
	./\!win_build.bat
	mv SHARK.lib SHARK-no_log.lib
	mv SHARK-debug.lib SHARK-no_log-debug.lib

	echo "#define LOG_DISABLE 0" > ../SHARK-cfg.h
	./\!win_build.bat

	rm ../SHARK-cfg.h
