/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-memory.h
 */

#ifndef DAL_MEMORY_H
#define DAL_MEMORY_H

#include "../DAL.h"

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
 *
 * Initialization / Termination
 *
 *************************************************************************/

extern boolean     DAL_MemInitialize();
extern void        DAL_MemTerminate();

/*************************************************************************
 *
 * Device Specific
 *
 *************************************************************************/

extern void        DAL_MemSetMaximum(uint16 max);
extern void       *DAL_MemPtrNew(uint16 size, boolean storage);
extern void        DAL_MemSemaphore(boolean active);
extern void        DAL_MemPtrFree(void *p);

/*************************************************************************
 *
 * SHARK Internal Use API's
 *
 *************************************************************************/

extern void       *DAL_MemPtrNewX(uint32 size, boolean storage);

#ifdef __cplusplus
}
#endif

#endif

/********************************* EOF ***********************************/
