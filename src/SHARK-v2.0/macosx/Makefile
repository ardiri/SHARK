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

# compiler commands
CC	= gcc
AR	= ar

# compiler flags
CCFLAGS	= -arch $(ARCH) -O2 -Wall 

# source / outputs
SHK_DIR = .. 
SHK_OBJ = $(SHK_DIR)DAL.o

LIB_DIR = ../common/libraries
LIB_OBJ = $(LIB_DIR)/SHARK-string.o \
	  $(LIB_DIR)/SHARK-gfx.o    \
	  $(LIB_DIR)/SHARK-font.o   \
	  $(LIB_DIR)/SHARK-sfx.o

OBJS	= $(SHK_OBJ) $(LIB_OBJ)

# compile requirements
default: 
	rm -f *.a
	echo "#define LOG_DISABLE 1" > ../SHARK-cfg.h
	ARCH=ppc make SHARK.a
	mv SHARK.a SHARK-no_log.a.ppc
	make clean

	ARCH=i386 make SHARK.a
	mv SHARK.a SHARK-no_log.a.i386
	make clean

	lipo -create SHARK-no_log.a.ppc SHARK-no_log.a.i386 -output SHARK-no_log.a
	rm -f SHARK*.ppc SHARK*.i386

	echo "#define LOG_DISABLE 0" > ../SHARK-cfg.h
	ARCH=ppc make SHARK.a
	mv SHARK.a SHARK.a.ppc
	make clean

	ARCH=i386 make SHARK.a
	mv SHARK.a SHARK.a.i386
	make clean

	lipo -create SHARK.a.ppc SHARK.a.i386 -output SHARK.a
	rm -f SHARK*.ppc SHARK*.i386

	make clean
	rm ../SHARK-cfg.h

SHARK.a: $(OBJS)
	$(AR) rc SHARK.a *.o

# compile rules
.SUFFIXES: .c .o

.c.o:
	$(CC) -c $(CCFLAGS) $<

# clean up
clean:
	rm -f *.o

distclean:
	make clean
	rm -f SHARK.a
