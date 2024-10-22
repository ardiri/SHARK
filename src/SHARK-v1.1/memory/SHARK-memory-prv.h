/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-memory-prv.h
 */

#ifndef SHARK_MEMORY_PRV_H
#define SHARK_MEMORY_PRV_H

#include "../SHARK.h"

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
 *
 * Module Constants/Structures
 *
 *************************************************************************/

#define MAX_MEM        128

/*************************************************************************
 *
 * Initialization / Termination
 *
 *************************************************************************/

#define _MemInitialize DAL_MemInitialize
#define _MemTerminate  DAL_MemTerminate

/*************************************************************************
 *
 * SHARK Internal Use API's
 *
 *************************************************************************/

extern void       *_MemPtrNewX(uint32 size, boolean storage);

#ifdef __cplusplus
}
#endif

#endif

/********************************* EOF ***********************************/
