/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-display.h
 */

#ifndef DAL_DISPLAY_H
#define DAL_DISPLAY_H

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

extern boolean     DAL_LCDInitialize();
extern void        DAL_LCDTerminate();

/*************************************************************************
 *
 * Device Specific
 *
 *************************************************************************/

extern void        DAL_LCDGetProperties(void **bits, coord *width, coord *height, uint8 *depth);
extern void        DAL_LCDSetPalette(uint8 *palette);
extern void        DAL_LCDRepaintRegion(rectangle *rect);

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
