/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-system.h
 */

#ifndef DAL_SYSTEM_H
#define DAL_SYSTEM_H

#include "../DAL.h"

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
 *
 * Platform Constants/Structures
 *
 *************************************************************************/

#define QUEUE_SIZE 128

/*************************************************************************
 *
 * Initialization / Termination
 *
 *************************************************************************/

extern boolean     DAL_SysInitialize();
extern void        DAL_SysTerminate();

/*************************************************************************
 *
 * Device Specific
 *
 *************************************************************************/

extern void        DAL_SysSetFPS(uint32 fps);
extern uint32      DAL_SysGetTime();
extern uint32      DAL_SysGetTicks();
extern void        DAL_SysDelay(uint32 millisec);
extern void        DAL_SysDebugMessage(uint8 *msg, boolean dialog);

/*************************************************************************
 *
 * SHARK Internal Use API's
 *
 *************************************************************************/

// none

#ifdef __cplusplus
}
#endif

#endif

/********************************* EOF ***********************************/
