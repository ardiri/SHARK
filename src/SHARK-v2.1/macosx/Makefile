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
CCFLAGS	= -O2 -Wall 

# source / outputs
SHK_DIR = .. 
SHK_OBJ = $(SHK_DIR)DAL.o

LIB_DIR = ../common/libraries
LIB_OBJ = $(LIB_DIR)/SHARK-string.o \
	  $(LIB_DIR)/SHARK-gfx.o    \
	  $(LIB_DIR)/SHARK-font.o   \
	  $(LIB_DIR)/SHARK-sfx.o    \
	  $(LIB_DIR)/SHARK-menu.o   \
	  $(LIB_DIR)/SHARK-gui.o 

OBJS	= $(SHK_OBJ) $(LIB_OBJ)

# compile requirements
default: 
	rm -f *.a
	echo "#define LOG_DISABLE 1" > ../SHARK-cfg.h
	make SHARK.a
	mv SHARK.a SHARK-no_log.a
	make clean

	echo "#define LOG_DISABLE 0" > ../SHARK-cfg.h
	make SHARK.a

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
