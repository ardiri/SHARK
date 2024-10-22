# ***********************************************************************
# 
# Copyright (c) 2001+ MobileWizardry
# All rights reserved.
#
# ***********************************************************************/

#
# @(#)Makefile
# 

# compiler commands
CC	= gcc
AR	= ar

# compiler flags
CCFLAGS = -O2 -Wall -Wno-multichar -Wno-strict-aliasing $(FLAGS)

# outputs
EXT_DIR = ../extensions
CORE_EXT = $(EXT_DIR)/SHARK-core.o
STR_EXT  = $(EXT_DIR)/SHARK-string.o
FB_EXT   = $(EXT_DIR)/SHARK-fb.o
SFX_EXT  = $(EXT_DIR)/SHARK-sfx.o

EXT     = $(CORE_EXT) $(MEM_EXT) $(FB_EXT) $(STR_EXT) $(SFX_EXT)

SHK_OBJ = DAL.o $(EXT) 
DBG_OBJ = 

OBJS	= $(SHK_OBJ) 

default:
	rm -rf *.a

	# debug builds
	
	echo "#define TARGET_LINUX"             >  ../SHARK-cfg.h
	echo "#define LOG_DISABLE 1"            >> ../SHARK-cfg.h
	FLAGS="-D_DEBUG -g" make libSHARK-debug.a	
	mv libSHARK-debug.a libSHARK-debug.no_log.a
	make clean

	echo "#define TARGET_LINUX"             >  ../SHARK-cfg.h
	echo "#define LOG_DISABLE 0"            >> ../SHARK-cfg.h
	FLAGS="-D_DEBUG -g" make libSHARK-debug.a	
	make clean

	# release builds
	
	echo "#define TARGET_LINUX"             >  ../SHARK-cfg.h
	echo "#define LOG_DISABLE 1"            >> ../SHARK-cfg.h
	FLAGS="" make libSHARK.a	
	mv libSHARK.a libSHARK-no_log.a
	make clean

	echo "#define TARGET_LINUX"             >  ../SHARK-cfg.h
	echo "#define LOG_DISABLE 0"            >> ../SHARK-cfg.h
	FLAGS="" make libSHARK.a	
	make clean

libSHARK.a: $(OBJS)
	$(AR) rc SHARK.a $(OBJS)

libSHARK-debug.a: $(OBJS) $(DBG_OBJ)
	$(AR) rc SHARK-debug.a $(OBJS) $(DBG_OBJ)

clean:
	rm -f $(OBJS) $(DBG_OBJ)
	rm -f ../SHARK-cfg.h

distclean:
	make clean
	rm -f libSHARK*.a

# compile rules
.SUFFIXES: .c .o

.c.o:
	$(CC) -c -o $@ $(CCFLAGS) $<
