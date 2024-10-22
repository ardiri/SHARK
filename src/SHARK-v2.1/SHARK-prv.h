/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-prv.h
 */

#ifndef SHARK_PRV_H
#define SHARK_PRV_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
 * Generic Data Types
 *************************************************************************/

#include "SHARK-cfg.h"

/*************************************************************************
 * Graphics:: Library
 *************************************************************************/

extern boolean     _GfxInitialize(lcd_format *format);
extern void        _GfxTerminate();

/*************************************************************************
 * Font:: Library
 *************************************************************************/

extern boolean     _FntInitialize(lcd_format *format, font_encoding encoding, font_style);
extern void        _FntTerminate();

/*************************************************************************
 * Sound Effects:: Library
 *************************************************************************/

extern boolean     _SfxInitialize(snd_rate rate, snd_format format, snd_type type, uint16 frames, uint16 mix_count);
extern void        _SfxTerminate();

/*************************************************************************
 * User Interface:: Library
 *************************************************************************/

extern boolean     _GUIInitialize(gui_face face);
extern void        _GUITerminate();

/*************************************************************************
 * Menu :: Library
 *************************************************************************/

extern boolean     _MnuInitialize();
extern void        _MnuTerminate();

#ifdef __cplusplus
}
#endif

#endif

/********************************* EOF ***********************************/

