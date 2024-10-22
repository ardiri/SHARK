/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-ARM-platform_loader.h
 */

#ifndef DAL_LOADER_H
#define DAL_LOADER_H

/*************************************************************************
 *
 * Header Defines
 *
 *************************************************************************/

#ifdef _M_68K
#include "DAL-68K-platform.h"
#endif
#ifdef _M_ARM
#include "DAL-ARM-platform.h"
#endif
#ifdef _M_X86
#include "DAL-X86-platform.h"
#endif

#include "resource.h"

/*************************************************************************
 *
 * Application Constants/Structures/Typedefs
 *
 *************************************************************************/

#define kPceNativeWantA0     (0x10000000)
#define kPceNativeTrapNoMask (0x00000FFF)

#define PceNativeTrapNo(sysTrapNo) (sysTrapNo & kPceNativeTrapNoMask)

typedef unsigned long Call68KFuncType(const void *emulStateP, unsigned long trapOrFunction, const void *argsOnStackP, unsigned long argsSizeAndwantA0);
typedef unsigned long NativeFuncType(const void *emulStateP, void *userData68KP, Call68KFuncType *call68KFuncP);

#ifdef _M_68K
#define sysTrapPceNativeCall 0xA45A
UInt32 PceNativeCall(NativeFuncType *nativeFuncP, void *userDataP)
  SYS_TRAP(sysTrapPceNativeCall);
#endif

#define __byte_swap16(n) \
        ( ((((UInt16) n) << 8) & 0xFF00) | \
          ((((UInt16) n) >> 8) & 0x00FF) )

#define __byte_swap32(n) \
        ( ((((UInt32) n) << 24) & 0xFF000000) |	\
          ((((UInt32) n) <<  8) & 0x00FF0000) |	\
          ((((UInt32) n) >>  8) & 0x0000FF00) |	\
          ((((UInt32) n) >> 24) & 0x000000FF) )
 
#define __read_unaligned32(addr)  \
        ( ((((UInt8 *)(addr))[0]) << 24) | \
          ((((UInt8 *)(addr))[1]) << 16) | \
          ((((UInt8 *)(addr))[2]) <<  8) | \
          ((((UInt8 *)(addr))[3])) )

#define __write_unaligned32(addr, value) \
	      ( ((UInt8 *)(addr))[0] = (UInt8)((UInt32)(value) >> 24), \
	      	((UInt8 *)(addr))[1] = (UInt8)((UInt32)(value) >> 16), \
	  	    ((UInt8 *)(addr))[2] = (UInt8)((UInt32)(value) >>  8), \
	  	    ((UInt8 *)(addr))[3] = (UInt8)((UInt32)(value)) )

#define __read_byte32(addr)  \
        ( ((((UInt8 *)(addr))[3]) << 24) | \
          ((((UInt8 *)(addr))[2]) << 16) | \
          ((((UInt8 *)(addr))[1]) <<  8) | \
          ((((UInt8 *)(addr))[0])) )

#define __read_byte32_16(addr)  \
        ( ((((UInt16 *)(addr))[1]) << 16) | \
          ((((UInt16 *)(addr))[0])) )

#define __write_byte32(addr, value) \
        ( ((UInt8 *)(addr))[3] = (UInt8)((UInt32)(value) >> 24), \
	        ((UInt8 *)(addr))[2] = (UInt8)((UInt32)(value) >> 16), \
	        ((UInt8 *)(addr))[1] = (UInt8)((UInt32)(value) >>  8), \
	        ((UInt8 *)(addr))[0] = (UInt8)((UInt32)(value)) )

#define __write_byte32_16(addr, value) \
        ( ((UInt16 *)(addr))[1] = (UInt16)((UInt32)(value) >> 16), \
          ((UInt16 *)(addr))[0] = (UInt16)((UInt32)(value)) )

#define __read_unaligned16(addr)  \
        ( ((((UInt8 *)(addr))[0]) <<  8) | \
          ((((UInt8 *)(addr))[1])) )

#define __read_byte16(addr)  \
        ( ((((UInt8 *)(addr))[1]) <<  8) | \
          ((((UInt8 *)(addr))[0])) )

#define __write_unaligned16(addr, value) \
        ( ((UInt8 *)(addr))[0] = (UInt8)((UInt16)(value) >>  8), \
          ((UInt8 *)(addr))[1] = (UInt8)((UInt16)(value)) )

#define __write_byte16(addr, value) \
        ( ((UInt8 *)(addr))[1] = (UInt8)((UInt16)(value) >>  8), \
          ((UInt8 *)(addr))[0] = (UInt8)((UInt16)(value)) )

/*************************************************************************
 *
 * Application Functions
 *
 *************************************************************************/

extern Boolean DAL_LoadArmResource();
extern void    DAL_UnloadArmResource();
extern UInt32  PilotMain(UInt16 cmd, MemPtr cmdPB, UInt16 launchFlags);

#endif

/********************************* EOF ***********************************/
