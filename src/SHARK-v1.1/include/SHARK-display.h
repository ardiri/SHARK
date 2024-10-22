/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-display.h
 */

#ifndef SHARK_DISPLAY_H
#define SHARK_DISPLAY_H

#include "../SHARK.h"

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
 *
 * Constants / Structures
 *
 *************************************************************************/

#define DISPLAY_RESIZE_UNSUPPORTED  0x00000001  // no resizing support
#define DISPLAY_RESIZE_FIXED        0x00000002  // fixed sizes (160x160, --> 480x320)
#define DISPLAY_RESIZE_ARBITARY     0x00000004  // arbitary

/*************************************************************************
 *
 * Application Programmers Interface
 *
 *************************************************************************/

// device specific
extern void        _LCDGetProperties(void **, coord *width, coord *height, uint8 *depth);
extern void        _LCDSetPalette(uint8 *palette);  // only does something in 8bpp mode
extern void        _LCDRepaintRegion(rectangle *rect);

// device independent
extern void        _LCDRepaint();

#ifdef __cplusplus
}
#endif

#endif

/********************************* EOF ***********************************/
