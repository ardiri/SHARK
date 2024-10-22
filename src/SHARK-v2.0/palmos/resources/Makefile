# ***********************************************************************
#
# Copyright (c) 2001+ Mobile Wizardry
# All rights reservered.
#
# ***********************************************************************/

#
# @(#)Makefile
#

# compile requirements
default:
	m68k-palmos-gcc -O2 -Wall -palmos4 -o application ../DAL-bootstrap.c
	build-prc SHARK.def application
	par x SHARK.prc code 0 code 1 data 0
	pilrc -q -LE32 SHARK-ARM.rcp
	pilrc -q -ro -creator SHRK -type SHRK -name "" SHARK-68k.rcp SHARK.ro
	cp SHARK.ro ../SHARK.ro 
	make clean

# clean-up funtions
clean:
	rm -f application *.bin *.out

distclean:
	rm -f *.prc
	make clean
