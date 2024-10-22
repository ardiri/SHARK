/*************************************************************************
 *
 * Copyright (c) 2002-2003 MobileWizardry
 * All rights reservered.
 *
 *************************************************************************/

/*
 * @(#)windows/resource.h
 *
 * -- Aaron Ardiri     (mailto:aaron_ardiri@mobilewizardry.com)
 * -- Charles Kerchner (mailto:chip_kerchner@mobilewizardry.com)
 */

#ifndef __RESOURCE_H__
#define __RESOURCE_H__

/*************************************************************************
 *
 *     Platform Defines
 *
 *************************************************************************/

#ifndef UNDER_CE
#define UNDER_CE _WIN32_WCE
#endif

#ifndef _WIN32
#define _WIN32
#endif

/*************************************************************************
 *
 *     Header File Includes
 *
 *************************************************************************/

#ifdef _WIN32_WCE
  #include <aygshell.h>
  #define I_IMAGENONE                   (-2)
#else
  #include <windows.h>
#endif

#ifdef RC_INVOKED
  #ifndef _INC_WINDOWS
    #define _INC_WINDOWS
    #include "winuser.h"           // extract from windows header
    #include "winver.h"
  #endif
#endif

#ifdef IDC_STATIC
  #undef IDC_STATIC
#endif
#define IDC_STATIC                      (-1)

/*************************************************************************
 *
 *     Resource Constants
 *
 *************************************************************************/

#define IDS_DEFAULT_APP_NAME_ID         1
#define IDC_DEFAULT_CLASS_ID            2

#define IDS_GENERIC_QUIT                3
#define IDR_GENERIC_MENUBAR             4
#define IDM_GENERIC_QUIT                5

#define IDI_DEFAULT_ICON                101

#endif
