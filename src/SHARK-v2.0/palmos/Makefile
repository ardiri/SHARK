# ***********************************************************************
#
# Copyright (c) 2002+ MobileWizardry
# All rights reservered.
#
# ***********************************************************************/

#
# @(#)Makefile
#

# compiler commands
CC      = arm-elf-gcc
AR      = arm-elf-ar

# compiler flags
CCFLAGS = -Wall -Wno-multichar -O2 \
	  -mthumb-interwork -fshort-enums -mstructure-size-boundary=8 \
	  -ffixed-r9 -ffixed-r8 -mpic-register=r10 -msingle-pic-base -fPIC

# source / outputs
PACE_OBJS = \
	    PACE/MemSet.o                        \
	    PACE/MemPtrNew.o                     \
	    PACE/MemChunkNew.o                   \
	    PACE/MemChunkFree.o                  \
	    PACE/MemHandleSize.o                 \
	    PACE/MemHandleLock.o                 \
	    PACE/MemHandleUnlock.o               \
	    PACE/MemHeapFreeBytes.o              \
	    PACE/HALMemorySetStorageAreaProtectionState.o \
	    \
	    PACE/DmFindDatabase.o                \
	    PACE/DmCreateDatabase.o              \
	    PACE/DmDeleteDatabase.o              \
	    PACE/DmOpenDatabase.o                \
	    PACE/DmCloseDatabase.o               \
	    PACE/DmFindResource.o                \
	    PACE/DmNewResource.o                 \
	    PACE/DmGetResource.o                 \
	    PACE/DmGetResourceIndex.o            \
	    PACE/DmNumResources.o                \
	    PACE/DmRemoveResource.o              \
	    PACE/DmReleaseResource.o             \
	    PACE/DmWrite.o                       \
	    \
	    PACE/FtrGet.o                        \
	    PACE/FtrPtrNew.o                     \
	    PACE/FtrPtrFree.o                    \
	    PACE/FtrGet.o                        \
	    PACE/SysCurAppDatabase.o             \
	    PACE/SysNotifyRegister.o             \
	    PACE/SysNotifyUnregister.o           \
	    \
	    PACE/SysEventGet.o                   \
	    PACE/SysEventAddToQueue.o            \
	    PACE/SysEventAddUniqueToQueue.o      \
	    PACE/SysHandleEvent.o                \
	    PACE/FrmSetEventHandler.o            \
	    PACE/FrmDispatchEvent.o              \
	    \
	    PACE/FrmGotoForm.o                   \
	    PACE/FrmInitForm.o                   \
	    PACE/FrmDrawForm.o                   \
	    PACE/FrmGetActiveForm.o              \
	    PACE/FrmSetActiveForm.o              \
	    PACE/FrmCloseAllForms.o              \
	    PACE/FrmCustomAlert.o                \
	    PACE/WinSetCoordinateSystem.o        \
	    PACE/WinScreenGetAttribute.o         \
	    PACE/WinScreenMode.o                 \
	    PACE/WinCreateOffscreenWindow.o      \
	    PACE/BmpGetBits.o                    \
	    PACE/WinGetBitmap.o                  \
	    PACE/WinDeleteWindow.o               \
	    PACE/WinCopyRectangle.o              \
	    \
	    PACE/WinSetDrawWindow.o              \
	    PACE/WinGetDisplayExtent.o           \
	    PACE/FrmGetFormBounds.o              \
	    PACE/WinGetDisplayWindow.o           \
	    PACE/WinSetBounds.o                  \
	    PACE/FrmGetWindowHandle.o            \
	    PACE/PINSetInputTriggerState.o       \
	    PACE/WinSetConstraintsSize.o         \
	    PACE/FrmSetDIAPolicyAttr.o           \
	    \
	    PACE/SysTaskDelay.o                  \
	    PACE/SysTicksPerSecond.o             \
	    PACE/TimGetSeconds.o                 \
	    PACE/TimSecondsToDateTime.o          \
	    PACE/PrefSetAppPreferences.o         \
	    PACE/PrefGetAppPreferences.o         \
	    \
	    PACE/KeyCurrentState.o               \
	    PACE/EvtGetPen.o                     \
	    PACE/EvtGetPenNative.o               \
	    \
	    PACE/StrPrintF.o                     \
	    \
	    PACE/SndStreamCreate.o               \
	    PACE/SndStreamStart.o                \
	    PACE/SndStreamStop.o                 \
	    PACE/SndStreamSetVolume.o            \
	    PACE/SndStreamDelete.o               \
	    \
	    PACE/VFSVolumeEnumerate.o            \
	    PACE/VFSDirEntryEnumerate.o          \
	    PACE/VFSDirCreate.o                  \
	    PACE/VFSFileCreate.o                 \
	    PACE/VFSFileDelete.o                 \
	    PACE/VFSFileRename.o                 \
	    PACE/VFSFileGetAttributes.o          \
	    PACE/VFSFileOpen.o                   \
	    PACE/VFSFileRead.o                   \
	    PACE/VFSFileWrite.o                  \
	    PACE/VFSFileSeek.o                   \
	    PACE/VFSFileTell.o                   \
	    PACE/VFSFileEOF.o                    \
	    PACE/VFSFileClose.o                  \
	    \
	    PACE/KALTimerCreate.o                \
	    PACE/KALTimerDelete.o                \
	    PACE/KALTimerSet.o

DAL_OBJ   = DAL-startup.o

LIB_OBJS  = ../common/libraries/SHARK-string.o   \
	    ../common/libraries/SHARK-gfx.o      \
	    ../common/libraries/SHARK-font.o     \
	    ../common/libraries/SHARK-sfx.o

OBJS      = DAL.o       \
	    PACE/PACE.o \
	    $(LIB_OBJS) 

# compile requirements
all: 
	(cd resources; make)

	echo "" > ../SHARK-cfg.h
	rm -f DAL-startup.o
	make startup

	echo "#define LOG_DISABLE 0" > ../SHARK-cfg.h
	rm -f DAL.o
	make SHARK.LIB
#	mv SHARK.a SHARK-no_log.a
	make clean

#	echo "#define LOG_DISABLE 1" > ../SHARK-cfg.h
#	rm -f DAL.o
#	make SHARK.LIB
#	make clean

startup: $(DAL_OBJ) 

SHARK.LIB: $(OBJS) 
	rm -f SHARK.a
	$(AR) cr SHARK.a $(OBJS) $(PACE_OBJS)

# compile rules
.SUFFIXES: .c .o

.c.o:
	$(CC) -c -o $@ $(CCFLAGS) $<

# clean-up funtions
clean:
	rm -f $(OBJS)
	rm -f ../SHARK-cfg.h

distclean:
	make clean
	rm -f SHARK.ro *.a 

# ******************************** E O F *********************************/
