# ***********************************************************************
# 
# Copyright (c) 2001+ MobileWizardry
# All rights reserved.
#
# ***********************************************************************/

#
# @(#)Makefile
# 

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

clean:
	rm -f $(OBJS) $(DBG_OBJ)
	rm -f ../SHARK-cfg.h

distclean:
	make clean
	rm -f libSHARK*.a

