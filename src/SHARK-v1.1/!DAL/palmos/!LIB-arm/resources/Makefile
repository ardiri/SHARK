# ***********************************************************************
#
# Copyright (c) 2001+
# All rights reservered.
#
# ***********************************************************************/

#
# @(#)Makefile
#

CCFLAGS = -O2 -palmos4

# compile requirements
default:
	m68k-palmos-gcc $(CCFLAGS) -D_M_68K ../../DAL-ARM-platform_loader.c
	build-prc application.def a.out
	par x application.prc code 0
	par x application.prc code 1
	par x application.prc data 0
	rm application.prc  
	pilrc -q -ro -creator SHRK -type SHRK -name "SHARK RESOURCES" \
	  application.rcp SHARK.ro
	cp SHARK.ro ..
	make clean

# clean-up funtions
clean:
	rm -f *.bin *.out

distclean:
	rm -f *.ro
	make clean
