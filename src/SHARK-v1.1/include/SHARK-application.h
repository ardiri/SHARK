/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-application.h
 */

#ifndef SHARK_APPLICATION_H
#define SHARK_APPLICATION_H

#include "../SHARK.h"

#ifdef __cplusplus
extern "C" {
#endif

extern uint32      ApplicationGetUniqueID();
extern uint8      *ApplicationGetName();
extern uint32      ApplicationGetVersion();

extern void       *ApplicationGetPreferences();
extern uint16      ApplicationGetPreferencesSize();
extern uint32      ApplicationGetDisplayProperties();

extern boolean     ApplicationInitialize();
extern boolean     ApplicationHandleEvent(event *e);
extern void        ApplicationTerminate();

#ifdef __cplusplus
}
#endif

#endif

/********************************* EOF ***********************************/
