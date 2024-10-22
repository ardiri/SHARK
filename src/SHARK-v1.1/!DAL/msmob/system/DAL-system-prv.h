/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-system-prv.h
 */

#include "../DAL-prv.h"

#ifndef DAL_SYSTEM_PRV_H
#define DAL_SYSTEM_PRV_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
 *
 *    Application Constants/Structures
 *
 *************************************************************************/

#define SECS_TO_FT_MULT       10000000
#define DEFAULT_TICKS_PER_SEC 1000
#define DEFAULT_TIMER_ID      1

/*************************************************************************
 *
 * DAL Internal Use API's
 *
 *************************************************************************/

extern LONGLONG      DAL_SysTimeLong();
extern uint16       *DAL_SysCreateUnicodeString(uint8 *);

#ifdef __cplusplus
}
#endif

#endif

/********************************* EOF ***********************************/
