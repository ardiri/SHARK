/*************************************************************************
 *
 * Copyright (c) 2002-2003 MobileWizardry
 * All rights reservered.
 *
 *************************************************************************/

/*
 * @(#)DAL-configure.h
 *
 * -- Aaron Ardiri  (mailto:aaron_ardiri@mobilewizardry.com)
 * -- Chip Kerchner (mailto:chip_kerchner@mobilewizardry.com)
 */

#ifndef __DAL_CONFIG_H__
#define __DAL_CONFIG_H__

/*************************************************************************
 *
 *    Compilation Parameters
 *
 *************************************************************************/

// PALMOS specific DAL configuration
// -DPALM_TITLE_OFF      // no title bar on application
// -DPALM_SILK_MINIMIZED // we do not want the grafitti silk area visible

#define GFX_GRAY_DESATURATION 1     // use (min+max)/2 for grayscale

/*************************************************************************
 *
 *    Build Requirements (multi-segment - palmos specific)
 *
 *************************************************************************/

#if __PALMOS__ && _68k_ 
#define __DAL__ __attribute__ ((section ("mw_dal")))  // code0002.bin
#define __SEG__ __attribute__ ((section ("mw_seg")))  // code0003.bin
#else
#define __DAL__
#define __SEG__
#endif

#endif

/********************************* EOF ***********************************/
