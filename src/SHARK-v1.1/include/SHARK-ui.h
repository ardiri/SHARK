/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-ui.h
 */

#ifndef SHARK_UI_H
#define SHARK_UI_H

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
extern boolean     _UIHandleEvent(event *e);

extern void        _UIMenuReset();
extern coord       _UIMenuGetHeight();
extern void        _UIMenuSetFont(font font_id);
extern void        _UIMenuSetButton(uint8 pos, uint8 *label, uint8 key_code);
extern void        _UIMenuSetMenuTriggerRegion(rectangle *rect);
extern void        _UIMenuAddItem(uint8 *label, uint8 key_code, uint8 *param);
extern void        _UIMenuAddSeparator();
extern void        _UIMenuComplete();

#ifdef __cplusplus
}
#endif

#endif

/********************************* EOF ***********************************/
