/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-ui-prv.h
 */

#ifndef SHARK_UI_PRV_H
#define SHARK_UI_PRV_H

#include "../SHARK.h"

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
 *
 * Module Constants/Structures
 *
 *************************************************************************/

#define MAX_MENUS     13
#define MENU_SPACE_X  10
#define MENU_SPACE_Y  3
#define MENU_ARROWS_X (2 + g->ui.menu.spacing)

// #define SHARK_UI_DOUBLE_TAP         // user must double tap with pen

/*************************************************************************
 *
 * Initialization / Termination
 *
 *************************************************************************/

extern boolean     _UIInitialize();
extern void        _UITerminate();

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
