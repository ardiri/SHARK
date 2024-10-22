/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-platform.h
 */

#ifndef DAL_PLATFORM_H
#define DAL_PLATFORM_H

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

extern boolean     DAL_PlatformInitialize();
extern void        DAL_PlatformTerminate();

/*************************************************************************
 *
 * Device Specific
 *
 *************************************************************************/

extern void        DAL_PlatformGetUniqueID(uint8 *str);
extern uint16      DAL_PlatformGetUniqueIDCODE();

extern void        DAL_PlatformLoadPreferences();
extern void        DAL_PlatformSavePreferences();
extern boolean     DAL_PlatformLoadPreference(uint16 index, void *buf, uint16 size);
extern void        DAL_PlatformSavePreference(uint16 index, void *buf, uint16 size);
extern void        DAL_PlatformKillPreference(uint16 index);

extern uint16      DAL_PlatformGetResourceSize(uint16 resNum);
extern void       *DAL_PlatformGetResource(uint16 resNum);
extern void        DAL_PlatformReleaseResource(void *ptr);

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
