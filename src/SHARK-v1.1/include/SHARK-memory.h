/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-memory.h
 */

#ifndef SHARK_MEMORY_H
#define SHARK_MEMORY_H

#include "../SHARK.h"

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
 *
 * Constants / Structures
 *
 *************************************************************************/

// none

/*************************************************************************
 *
 * Application Programmers Interface
 *
 *************************************************************************/

// device specific
extern void        _MemSetMaximum(uint16 max);
extern void       *_MemPtrNew(uint16 size, boolean storage);
extern void        _MemSemaphore(boolean active);
extern void        _MemPtrFree(void *p);

// device independent
extern uint16      _MemPtrSize(void *p);
extern void        _MemSet(void *p, uint16 count, uint8 value);
extern void        _MemMove(void *pDst, void *pSrc, uint16 count);
extern void        _MemCopy(void *pDst, void *pSrc, uint16 count);
extern uint16      _MemCompare(void *p1, void *p2, uint16 count);
extern void *      _MemChr(void *p, uint8 chr, uint16 count);

#ifdef __cplusplus
}
#endif

#endif

/********************************* EOF ***********************************/
