# ***********************************************************************
#
# Copyright (c) 2001+
# All rights reservered.
#
# ***********************************************************************/

#
# @(#)Makefile
#

# compile requirements
default:
	pilrc -q -ro -creator SHRK -type SHRK -name "SHARK RESOURCES" \
	  application.rcp SHARK.ro
	cp SHARK.ro ..

# clean-up funtions
clean:
	rm -f *.bin

distclean:
	rm -f *.ro
	make clean
