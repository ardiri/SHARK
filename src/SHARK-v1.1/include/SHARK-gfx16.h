/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-gfx16.h
 */

#ifndef SHARK_GFX16_H
#define SHARK_GFX16_H

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
// none

// device independent
extern void        _Gfx16GetProperties(void **, coord *width, coord *height, uint8 *depth);
extern void        _Gfx16Repaint();
extern void        _Gfx16RepaintRegion(rectangle *rect);

#ifdef __cplusplus
}
#endif

#endif

/********************************* EOF ***********************************/
