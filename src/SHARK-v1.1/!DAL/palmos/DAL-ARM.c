/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-ARM.c
 */

#include "DAL-ARM-platform_loader.h"
#include "DAL-prv.h"

/*************************************************************************
 *
 * Armlet Entry Point
 *
 *************************************************************************/

void  *XXX_MemPtrNew(const void *, Call68KFuncType *, uint32);
void   XXX_MemPtrFree(const void *, Call68KFuncType *, void *);

uint32    
ArmletMain(const void      *emulStateP,
           void            *userData68KP,
           Call68KFuncType *call68KFuncP)
{
  register void *GOT asm ("r10");
  uint32   i, size;
  uint8   *p;
  uint32   result;
  GLOBALS_ACCESS;

  // default value
  result = 0;

  // initalize global variables
  g = (DAL_Globals *)XXX_MemPtrNew(emulStateP, call68KFuncP, sizeof(DAL_Globals));
  if (g != NULL)
  {
    size = sizeof(DAL_Globals);
    for (i=0, p=(uint8 *)g; i<size; i++) *p++ = 0;

    // we need to initialize a few things
    GOT                    = (void *)userData68KP;
    g->platform_special[0] = (void *)emulStateP;
    g->platform_special[1] = (void *)call68KFuncP;

    // run the application - direct link to previous application code
    result = PilotMain(sysAppLaunchCmdNormalLaunch, (void *)NULL, 0);

    // clean up globals variables
    XXX_MemPtrFree(emulStateP, call68KFuncP, g);
  }

  return result;    
}

#define PACE_PARAMS_INIT()            \
  uint8 params[] = {
  
#define PACE_PARAMS_ADD8(param)       \
  (  (UInt8)(param)        & 0xff), 0,

#define PACE_PARAMS_ADD16(param)      \
  (((UInt16)(param) >> 8)  & 0xff),   \
  ( (UInt16)(param)        & 0xff),

#define PACE_PARAMS_ADD32(param)      \
  (((uint32)(param) >> 24) & 0xff),   \
  (((uint32)(param) >> 16) & 0xff),   \
  (((uint32)(param) >> 8)  & 0xff),   \
  ( (uint32)(param)        & 0xff),

#define PACE_PARAMS_DONE()            \
  };

#define PACE_EXEC(trap, flag)         \
  call68KFuncP((void *)emulStateP,    \
    PceNativeTrapNo(trap), &params, sizeof(params) | (flag));

#define sysTrapMemPtrNew      0xA013

void *
XXX_MemPtrNew(const void *emulStateP, Call68KFuncType *call68KFuncP, uint32 size)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(size)
  PACE_PARAMS_DONE()
  return (void *)PACE_EXEC(sysTrapMemPtrNew, kPceNativeWantA0)
}

#define sysTrapMemChunkFree   0xA012

void
XXX_MemPtrFree(const void *emulStateP, Call68KFuncType *call68KFuncP, void *p)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(p)
  PACE_PARAMS_DONE()
  PACE_EXEC(sysTrapMemChunkFree, 0)
}

/********************************* EOF ***********************************/
