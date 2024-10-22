/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-platform.h
 */

#ifndef SHARK_PLATFORM_H
#define SHARK_PLATFORM_H

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
extern void        PlatformGetUniqueID(uint8 *str);
extern uint16      PlatformGetUniqueIDCODE();

extern void        PlatformLoadPreferences();
extern void        PlatformSavePreferences();
extern boolean     PlatformLoadPreference(uint16 index, void *buf, uint16 size);
extern void        PlatformSavePreference(uint16 index, void *buf, uint16 size);
extern void        PlatformKillPreference(uint16 index);

extern uint16      PlatformGetResourceSize(uint16 resNum);
extern void       *PlatformGetResource(uint16 resNum);
extern void        PlatformReleaseResource(void *ptr);

// device independent
extern boolean     PlatformResourceExists(uint16 resNum);

#ifdef __cplusplus
}
#endif

#endif

/********************************* EOF ***********************************/
