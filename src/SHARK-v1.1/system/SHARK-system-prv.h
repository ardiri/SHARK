/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-system-prv.h
 */

#ifndef SHARK_SYSTEM_PRV_H
#define SHARK_SYSTEM_PRV_H

#include "../SHARK.h"

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
 *
 * Module Constants/Structures
 *
 *************************************************************************/

// none

/*************************************************************************
 *
 * Initialization / Termination
 *
 *************************************************************************/

#define _SysInitialize DAL_SysInitialize
#define _SysTerminate  DAL_SysTerminate

/*************************************************************************
 *
 * SHARK Internal Use API's
 *
 *************************************************************************/

extern boolean     _SysEventAvailable();
extern void        _SysGetEvent(event *);
extern void        _SysFlushEventQueue();

#ifdef __cplusplus
}
#endif

#endif

/********************************* EOF ***********************************/
