/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-input.h
 */

#ifndef DAL_INPUT_H
#define DAL_INPUT_H

#include "../DAL.h"

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
 *
 * Platform Constants/Structures
 *
 *************************************************************************/

// none

/*************************************************************************
 *
 * Initialization / Termination
 *
 *************************************************************************/

extern boolean     DAL_InpInitialize();
extern void        DAL_InpTerminate();

/*************************************************************************
 *
 * Device Specific
 *
 *************************************************************************/

extern uint32      DAL_KeyCurrentState();
extern void        DAL_KeyCurrentStateAnalog(uint8 *x, uint8 *y);
extern boolean     DAL_PenAvailable();
extern void        DAL_PenState(coord *penX, coord *penY, boolean *penDown);

/*************************************************************************
 *
 * SHARK Internal Use API's
 *
 *************************************************************************/

extern boolean     DAL_KeyHandleEvent(uint16 key, event *_e);

#ifdef __cplusplus
}
#endif

#endif

/********************************* EOF ***********************************/
