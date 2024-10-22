/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-input-prv.h
 */

#ifndef SHARK_INPUT_PRV_H
#define SHARK_INPUT_PRV_H

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

#define _InpInitialize DAL_InpInitialize
#define _InpTerminate  DAL_InpTerminate

/*************************************************************************
 *
 * SHARK Internal Use API's
 *
 *************************************************************************/

extern boolean     _KeyHandleEvent(uint16 key, event *_e);
extern boolean     _PenHandleEvent(event *_e);

#ifdef __cplusplus
}
#endif

#endif

/********************************* EOF ***********************************/
