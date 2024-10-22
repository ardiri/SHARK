# ***********************************************************************
#
# Copyright (c) 2002+ MobileWizardry
# All rights reservered.
#
# ***********************************************************************/

# @(#)Makefile
#
# -- Aaron Ardiri (mailto:aaron_ardiri@mobilewizardry.com)

TMP_DIR=/cygdrive/c/_tmp/
SYM_DIR=/cygdrive/c/Symbian/Series60/
REL_DIR=$(SYM_DIR)Epoc32/Release/ARMI/UREL/

# compile requirements
default:
	# clean up setup
	rm -rf $(TMP_DIR)
 
	# prepare compilation directory for building
	mkdir $(TMP_DIR)
	mkdir $(TMP_DIR)symbian_s60/
	mkdir $(TMP_DIR)common/
	mkdir $(TMP_DIR)common/libraries
  
	echo "#define LOG_DISABLE 1" > ../SHARK-cfg.h

	cp -r * $(TMP_DIR)symbian_s60
	cp -r ../SHARK* $(TMP_DIR)
	cp    ../common/SHARK* $(TMP_DIR)common
	cp -r ../common/libraries/* $(TMP_DIR)common/libraries

	# build! (call batch file to do the work)
	cmd /c c:\\_tmp\\symbian_s60\\!e32_build.bat 

	# make a copy of our LIB file
	cp $(REL_DIR)SHARK.LIB SHARK-no_log.LIB

	rm ../SHARK-cfg.h

	rm -rf $(TMP_DIR)
	rm -rf $(SYM_DIR)Epoc32/BUILD
	rm -f  $(REL_DIR)SHARK.*

	# prepare compilation directory for building
	mkdir $(TMP_DIR)
	mkdir $(TMP_DIR)symbian_s60/
	mkdir $(TMP_DIR)common/
	mkdir $(TMP_DIR)common/libraries
  
	echo "#define LOG_DISABLE 0" > ../SHARK-cfg.h

	cp -r * $(TMP_DIR)symbian_s60
	cp -r ../SHARK* $(TMP_DIR)
	cp    ../common/SHARK* $(TMP_DIR)common
	cp -r ../common/libraries/* $(TMP_DIR)common/libraries

	# build! (call batch file to do the work)
	cmd /c c:\\_tmp\\symbian_s60\\!e32_build.bat 

	# make a copy of our LIB file
	cp $(REL_DIR)SHARK.LIB SHARK.LIB

	rm ../SHARK-cfg.h

	rm -rf $(TMP_DIR)
	rm -rf $(SYM_DIR)Epoc32/BUILD
	rm -f  $(REL_DIR)SHARK.*
