# ***********************************************************************
#
# Copyright (c) 2001+ MobileWizardry
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
CCFLAGS = -O2 -Wall -D__PALMOS__ -D_M_ARM                        \
	  -nostartfiles -mshort-load-bytes -ffixed-r9 -ffixed-r8 \
	  -mpic-register=r10 -msingle-pic-base -fPIC 

# source / outputs
SHK_DIR = ../../..
SHK_OBJ = $(SHK_DIR)/audio/SHARK-_PlaySound.o                         \
	  $(SHK_DIR)/audio/SHARK-_SndStreamAvailable.o                \
	  $(SHK_DIR)/audio/SHARK-_SndStreamCreate.o                   \
	  $(SHK_DIR)/audio/SHARK-_SndStreamDelete.o                   \
	  $(SHK_DIR)/audio/SHARK-_SndStreamStart.o                    \
	  $(SHK_DIR)/audio/SHARK-_SndStreamStop.o                     \
	  $(SHK_DIR)/display/SHARK-_LCDGetProperties.o                \
	  $(SHK_DIR)/display/SHARK-_LCDRepaint.o                      \
	  $(SHK_DIR)/display/SHARK-_LCDRepaintRegion.o                \
	  $(SHK_DIR)/display/SHARK-_LCDSetPalette.o                   \
	  $(SHK_DIR)/file/SHARK-_FileClose.o                          \
	  $(SHK_DIR)/file/SHARK-_FileCreate.o                         \
	  $(SHK_DIR)/file/SHARK-_FileDelete.o                         \
	  $(SHK_DIR)/file/SHARK-_FileEOF.o                            \
	  $(SHK_DIR)/file/SHARK-_FileOpen.o                           \
	  $(SHK_DIR)/file/SHARK-_FileRead.o                           \
	  $(SHK_DIR)/file/SHARK-_FileSeek.o                           \
	  $(SHK_DIR)/file/SHARK-_FileTell.o                           \
	  $(SHK_DIR)/file/SHARK-_FileWrite.o                          \
	  $(SHK_DIR)/file/SHARK-_FindFile.o                           \
	  $(SHK_DIR)/gfx/SHARK-_FntDefineFont.o                       \
	  $(SHK_DIR)/gfx/SHARK-_FntGetCharWidth.o                     \
	  $(SHK_DIR)/gfx/SHARK-_FntGetCharsWidth.o                    \
	  $(SHK_DIR)/gfx/SHARK-_FntGetFont.o                          \
	  $(SHK_DIR)/gfx/SHARK-_FntGetFontHeight.o                    \
	  $(SHK_DIR)/gfx/SHARK-_FntGetWordWrap.o                      \
	  $(SHK_DIR)/gfx/SHARK-_FntReleaseFont.o                      \
	  $(SHK_DIR)/gfx/SHARK-_FntSetFont.o                          \
	  $(SHK_DIR)/gfx/SHARK-_GfxClearWindow.o                      \
	  $(SHK_DIR)/gfx/SHARK-_GfxCopyRegion.o                       \
	  $(SHK_DIR)/gfx/SHARK-_GfxCopyToDisplay.o                    \
	  $(SHK_DIR)/gfx/SHARK-_GfxCopyToDisplay_12bpp.o              \
	  $(SHK_DIR)/gfx/SHARK-_GfxCopyToDisplay_16bpp.o              \
	  $(SHK_DIR)/gfx/SHARK-_GfxCopyToDisplay_1bpp.o               \
	  $(SHK_DIR)/gfx/SHARK-_GfxCopyToDisplay_2bpp.o               \
	  $(SHK_DIR)/gfx/SHARK-_GfxCopyToDisplay_4bpp.o               \
	  $(SHK_DIR)/gfx/SHARK-_GfxCopyToDisplay_8bpp.o               \
	  $(SHK_DIR)/gfx/SHARK-_GfxCreateWindow.o                     \
	  $(SHK_DIR)/gfx/SHARK-_GfxCreateWindowFromBuffer.o           \
	  $(SHK_DIR)/gfx/SHARK-_GfxCreateWindowFromBufferDisposable.o \
	  $(SHK_DIR)/gfx/SHARK-_GfxDeleteWindow.o                     \
	  $(SHK_DIR)/gfx/SHARK-_GfxDisplayInfo.o                      \
	  $(SHK_DIR)/gfx/SHARK-_GfxDrawChar.o                         \
	  $(SHK_DIR)/gfx/SHARK-_GfxDrawLine.o                         \
	  $(SHK_DIR)/gfx/SHARK-_GfxDrawRectangle.o                    \
	  $(SHK_DIR)/gfx/SHARK-_GfxDrawString.o                       \
	  $(SHK_DIR)/gfx/SHARK-_GfxFillRegion.o                       \
	  $(SHK_DIR)/gfx/SHARK-_GfxGetClip.o                          \
	  $(SHK_DIR)/gfx/SHARK-_GfxGetDrawWindow.o                    \
	  $(SHK_DIR)/gfx/SHARK-_GfxGetPalette.o                       \
	  $(SHK_DIR)/gfx/SHARK-_GfxGetPaletteIndex.o                  \
	  $(SHK_DIR)/gfx/SHARK-_GfxGetPixel.o                         \
	  $(SHK_DIR)/gfx/SHARK-_GfxGetWindowInfo.o                    \
	  $(SHK_DIR)/gfx/SHARK-_GfxInitialize.o                       \
	  $(SHK_DIR)/gfx/SHARK-_GfxInvertRegion.o                     \
	  $(SHK_DIR)/gfx/SHARK-_GfxRepaint.o                          \
	  $(SHK_DIR)/gfx/SHARK-_GfxRepaintRegion.o                    \
	  $(SHK_DIR)/gfx/SHARK-_GfxResetClip.o                        \
	  $(SHK_DIR)/gfx/SHARK-_GfxSetClip.o                          \
	  $(SHK_DIR)/gfx/SHARK-_GfxSetDrawWindow.o                    \
	  $(SHK_DIR)/gfx/SHARK-_GfxSetPalette.o                       \
	  $(SHK_DIR)/gfx/SHARK-_GfxSetPixel.o                         \
	  $(SHK_DIR)/gfx/SHARK-_GfxSetState.o                         \
	  $(SHK_DIR)/gfx/SHARK-_GfxTerminate.o                        \
	  $(SHK_DIR)/gfx16/SHARK-_Gfx16GetProperties.o                \
	  $(SHK_DIR)/gfx16/SHARK-_Gfx16Initialize.o                   \
	  $(SHK_DIR)/gfx16/SHARK-_Gfx16Repaint.o                      \
	  $(SHK_DIR)/gfx16/SHARK-_Gfx16RepaintRegion.o                \
	  $(SHK_DIR)/gfx16/SHARK-_Gfx16Terminate.o                    \
	  $(SHK_DIR)/input/SHARK-_KeyAnalogAvailable.o                \
	  $(SHK_DIR)/input/SHARK-_KeyCurrentState.o                   \
	  $(SHK_DIR)/input/SHARK-_KeyCurrentStateAnalog.o             \
	  $(SHK_DIR)/input/SHARK-_KeyGetMask.o                        \
	  $(SHK_DIR)/input/SHARK-_KeyHandleEvent.o                    \
	  $(SHK_DIR)/input/SHARK-_KeySetMask.o                        \
	  $(SHK_DIR)/input/SHARK-_KeySetNumericReporting.o            \
	  $(SHK_DIR)/input/SHARK-_PenAvailable.o                      \
	  $(SHK_DIR)/input/SHARK-_PenDisplayToWindow.o                \
	  $(SHK_DIR)/input/SHARK-_PenHandleEvent.o                    \
	  $(SHK_DIR)/input/SHARK-_PenState.o                          \
	  $(SHK_DIR)/memory/SHARK-_MemChr.o                           \
	  $(SHK_DIR)/memory/SHARK-_MemCompare.o                       \
	  $(SHK_DIR)/memory/SHARK-_MemCopy.o                          \
	  $(SHK_DIR)/memory/SHARK-_MemMove.o                          \
	  $(SHK_DIR)/memory/SHARK-_MemPtrFree.o                       \
	  $(SHK_DIR)/memory/SHARK-_MemPtrNew.o                        \
	  $(SHK_DIR)/memory/SHARK-_MemPtrNewX.o                       \
	  $(SHK_DIR)/memory/SHARK-_MemPtrSize.o                       \
	  $(SHK_DIR)/memory/SHARK-_MemSemaphore.o                     \
	  $(SHK_DIR)/memory/SHARK-_MemSet.o                           \
	  $(SHK_DIR)/memory/SHARK-_MemSetMaximum.o                    \
	  $(SHK_DIR)/platform/SHARK-PlatformGetResource.o             \
	  $(SHK_DIR)/platform/SHARK-PlatformGetResourceSize.o         \
	  $(SHK_DIR)/platform/SHARK-PlatformGetUniqueID.o             \
	  $(SHK_DIR)/platform/SHARK-PlatformGetUniqueIDCODE.o         \
	  $(SHK_DIR)/platform/SHARK-PlatformLoadPreference.o          \
	  $(SHK_DIR)/platform/SHARK-PlatformLoadPreferences.o         \
	  $(SHK_DIR)/platform/SHARK-PlatformKillPreference.o          \
	  $(SHK_DIR)/platform/SHARK-PlatformReleaseResource.o         \
	  $(SHK_DIR)/platform/SHARK-PlatformResourceExists.o          \
	  $(SHK_DIR)/platform/SHARK-PlatformSavePreference.o          \
	  $(SHK_DIR)/platform/SHARK-PlatformSavePreferences.o         \
	  $(SHK_DIR)/string/SHARK-_StrAToD.o                          \
	  $(SHK_DIR)/string/SHARK-_StrAToF.o                          \
	  $(SHK_DIR)/string/SHARK-_StrAToH.o                          \
	  $(SHK_DIR)/string/SHARK-_StrAToI.o                          \
	  $(SHK_DIR)/string/SHARK-_StrAToO.o                          \
	  $(SHK_DIR)/string/SHARK-_StrCaselessCompare.o               \
	  $(SHK_DIR)/string/SHARK-_StrCat.o                           \
	  $(SHK_DIR)/string/SHARK-_StrChr.o                           \
	  $(SHK_DIR)/string/SHARK-_StrCompare.o                       \
	  $(SHK_DIR)/string/SHARK-_StrCopy.o                          \
	  $(SHK_DIR)/string/SHARK-_StrDToA.o                          \
	  $(SHK_DIR)/string/SHARK-_StrFToA.o                          \
	  $(SHK_DIR)/string/SHARK-_StrHToA.o                          \
	  $(SHK_DIR)/string/SHARK-_StrIToA.o                          \
	  $(SHK_DIR)/string/SHARK-_StrInitialize.o                    \
	  $(SHK_DIR)/string/SHARK-_StrLen.o                           \
	  $(SHK_DIR)/string/SHARK-_StrNCaselessCompare.o              \
	  $(SHK_DIR)/string/SHARK-_StrNCat.o                          \
	  $(SHK_DIR)/string/SHARK-_StrNChr.o                          \
	  $(SHK_DIR)/string/SHARK-_StrNCompare.o                      \
	  $(SHK_DIR)/string/SHARK-_StrNCopy.o                         \
	  $(SHK_DIR)/string/SHARK-_StrNStr.o                          \
	  $(SHK_DIR)/string/SHARK-_StrOToA.o                          \
	  $(SHK_DIR)/string/SHARK-_StrSet.o                           \
	  $(SHK_DIR)/string/SHARK-_StrStr.o                           \
	  $(SHK_DIR)/string/SHARK-_StrTerminate.o                     \
	  $(SHK_DIR)/system/SHARK-_SysDebugMessage.o                  \
	  $(SHK_DIR)/system/SHARK-_SysDelay.o                         \
	  $(SHK_DIR)/system/SHARK-_SysEnqueueEvent.o                  \
	  $(SHK_DIR)/system/SHARK-_SysEnqueueQuitEvent.o              \
	  $(SHK_DIR)/system/SHARK-_SysEventAvailable.o                \
	  $(SHK_DIR)/system/SHARK-_SysFlushEventQueue.o               \
	  $(SHK_DIR)/system/SHARK-_SysGetEvent.o                      \
	  $(SHK_DIR)/system/SHARK-_SysGetFPS.o                        \
	  $(SHK_DIR)/system/SHARK-_SysGetGlobals.o                    \
	  $(SHK_DIR)/system/SHARK-_SysGetTicks.o                      \
	  $(SHK_DIR)/system/SHARK-_SysGetTime.o                       \
	  $(SHK_DIR)/system/SHARK-_SysRandom.o                        \
	  $(SHK_DIR)/system/SHARK-_SysSetFPS.o                        \
	  $(SHK_DIR)/system/SHARK-_SysSetGlobals.o                    \
	  $(SHK_DIR)/ui/SHARK-_UIHandleEvent.o                        \
	  $(SHK_DIR)/ui/SHARK-_UIInitialize.o                         \
	  $(SHK_DIR)/ui/SHARK-_UIMenuAddItem.o                        \
	  $(SHK_DIR)/ui/SHARK-_UIMenuAddSeparator.o                   \
	  $(SHK_DIR)/ui/SHARK-_UIMenuComplete.o                       \
	  $(SHK_DIR)/ui/SHARK-_UIMenuGetHeight.o                      \
	  $(SHK_DIR)/ui/SHARK-_UIMenuReset.o                          \
	  $(SHK_DIR)/ui/SHARK-_UIMenuSetButton.o                      \
	  $(SHK_DIR)/ui/SHARK-_UIMenuSetFont.o                        \
	  $(SHK_DIR)/ui/SHARK-_UIMenuSetMenuTriggerRegion.o           \
	  $(SHK_DIR)/ui/SHARK-_UITerminate.o                          

DAL_DIR = ..
DAL_OBJ = $(DAL_DIR)/DAL.o                                            \
	  $(DAL_DIR)/DAL-ARM-palmos.o                                 \
	  $(DAL_DIR)/audio/DAL_PlaySound.o                            \
	  $(DAL_DIR)/audio/DAL_SndInitialize.o                        \
	  $(DAL_DIR)/audio/DAL_SndStreamAvailable.o                   \
	  $(DAL_DIR)/audio/DAL_SndStreamCreate.o                      \
	  $(DAL_DIR)/audio/DAL_SndStreamDelete.o                      \
	  $(DAL_DIR)/audio/DAL_SndStreamStart.o                       \
	  $(DAL_DIR)/audio/DAL_SndStreamStop.o                        \
	  $(DAL_DIR)/audio/DAL_SndTerminate.o                         \
	  $(DAL_DIR)/audio/DAL_SndThread.o                            \
	  $(DAL_DIR)/display/DAL_LCDGetProperties.o                   \
	  $(DAL_DIR)/display/DAL_LCDInitialize.o                      \
	  $(DAL_DIR)/display/DAL_LCDRepaintRegion.o                   \
	  $(DAL_DIR)/display/DAL_LCDSetPalette.o                      \
	  $(DAL_DIR)/display/DAL_LCDTerminate.o                       \
	  $(DAL_DIR)/file/DAL_FileClose.o                             \
	  $(DAL_DIR)/file/DAL_FileCreate.o                            \
	  $(DAL_DIR)/file/DAL_FileDelete.o                            \
	  $(DAL_DIR)/file/DAL_FileEOF.o                               \
	  $(DAL_DIR)/file/DAL_FileInitialize.o                        \
	  $(DAL_DIR)/file/DAL_FileOpen.o                              \
	  $(DAL_DIR)/file/DAL_FileRead.o                              \
	  $(DAL_DIR)/file/DAL_FileSeek.o                              \
	  $(DAL_DIR)/file/DAL_FileTell.o                              \
	  $(DAL_DIR)/file/DAL_FileTerminate.o                         \
	  $(DAL_DIR)/file/DAL_FileWrite.o                             \
	  $(DAL_DIR)/file/DAL_FindFile.o                              \
	  $(DAL_DIR)/input/DAL_InpInitialize.o                        \
	  $(DAL_DIR)/input/DAL_InpTerminate.o                         \
	  $(DAL_DIR)/input/DAL_KeyCurrentState.o                      \
	  $(DAL_DIR)/input/DAL_KeyCurrentStateAnalog.o                \
	  $(DAL_DIR)/input/DAL_KeyHandleEvent.o                       \
	  $(DAL_DIR)/input/DAL_PenState.o                             \
	  $(DAL_DIR)/memory/DAL_MemInitialize.o                       \
	  $(DAL_DIR)/memory/DAL_MemPtrFree.o                          \
	  $(DAL_DIR)/memory/DAL_MemPtrNew.o                           \
	  $(DAL_DIR)/memory/DAL_MemPtrNewX.o                          \
	  $(DAL_DIR)/memory/DAL_MemSemaphore.o                        \
	  $(DAL_DIR)/memory/DAL_MemSetMaximum.o                       \
	  $(DAL_DIR)/memory/DAL_MemTerminate.o                        \
	  $(DAL_DIR)/platform/DAL_PlatformGetResource.o               \
	  $(DAL_DIR)/platform/DAL_PlatformGetResourceSize.o           \
	  $(DAL_DIR)/platform/DAL_PlatformGetUniqueID.o               \
	  $(DAL_DIR)/platform/DAL_PlatformGetUniqueIDCODE.o           \
	  $(DAL_DIR)/platform/DAL_PlatformInitialize.o                \
	  $(DAL_DIR)/platform/DAL_PlatformLoadPreference.o            \
	  $(DAL_DIR)/platform/DAL_PlatformLoadPreferences.o           \
	  $(DAL_DIR)/platform/DAL_PlatformKillPreference.o            \
	  $(DAL_DIR)/platform/DAL_PlatformReleaseResource.o           \
	  $(DAL_DIR)/platform/DAL_PlatformSavePreference.o            \
	  $(DAL_DIR)/platform/DAL_PlatformSavePreferences.o           \
	  $(DAL_DIR)/platform/DAL_PlatformTerminate.o                 \
	  $(DAL_DIR)/system/DAL_SysDebugMessage.o                     \
	  $(DAL_DIR)/system/DAL_SysDelay.o                            \
	  $(DAL_DIR)/system/DAL_SysGetTicks.o                         \
	  $(DAL_DIR)/system/DAL_SysGetTime.o                          \
	  $(DAL_DIR)/system/DAL_SysInitialize.o                       \
	  $(DAL_DIR)/system/DAL_SysSetFPS.o                           \
	  $(DAL_DIR)/system/DAL_SysTerminate.o
OBJS    = $(DAL_OBJ) $(SHK_OBJ)

# compile requirements
all: SHARK.LIB arm_loader 
	(cd resources; make)
#	make clean

arm_loader: $(DAL_DIR)/DAL-ARM.o
	cp $(DAL_DIR)/DAL-ARM.o .

SHARK.LIB: $(OBJS) 
	rm -f SHARK.a
	$(AR) cr SHARK.a $(OBJS)

# compile rules
.SUFFIXES: .c .o

.c.o:
	$(CC) -c -o $@ $(CCFLAGS) $<

# clean-up funtions
clean:
	rm -f $(OBJS) $(DAL_DIR)/DAL-ARM.o

distclean:
	make clean
	rm -f SHARK.ro *.a DAL-ARM.o

# ******************************** E O F *********************************/
