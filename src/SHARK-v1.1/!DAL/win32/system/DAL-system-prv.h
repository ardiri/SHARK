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

/*************************************************************************
 *
 * DAL Internal Use API's
 *
 *************************************************************************/

extern LONGLONG      DAL_SysTimeLong();
extern void CALLBACK DAL_TimerProc(UINT id, UINT msg, DWORD dwUser, DWORD dw1, DWORD dw2);

#ifdef __cplusplus
}
#endif

#endif

/********************************* EOF ***********************************/
