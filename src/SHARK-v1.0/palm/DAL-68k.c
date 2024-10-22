/*************************************************************************
 *
 * Copyright (c) 2002-2003 MobileWizardry
 * All rights reservered.
 *
 *************************************************************************/

/*
 * @(#)palm/DAL-68k.c
 *
 * -- Aaron Ardiri     (mailto:aaron_ardiri@mobilewizardry.com)
 * -- Charles Kerchner (mailto:chip_kerchner@mobilewizardry.com)
 */

#include "DAL-prv.h"
#include "application_resource.h"

/*************************************************************************
 *
 *     Global Variables
 *
 *************************************************************************/

static XXX_GlobalsType  gXXXGlobals;
static DAL_GlobalsType  _DALGlobals;
static DAL_GlobalsType *gDALGlobals;

/*************************************************************************
 *
 *     DAL Functions
 *
 *************************************************************************/

#include "DAL-platform.inc"      // platform specific functions
#include "DAL-system.inc"        // system functions
#include "DAL-memory.inc"        // memory functions
#include "DAL-io.inc"            // input/output functions
#include "DAL-string.inc"        // string functions
#include "DAL-gfx.inc"           // graphics functions
#include "DAL-audio.inc"         // audio functions
#include "DAL-file.inc"          // file i/o functions
#include "DAL-globals.inc"       // global

/*************************************************************************
 *
 *     GateWay to DAL from 'abstract' application framework
 *
 *************************************************************************/

static void
DALBridgeInitialize()
{
}

static uint32
DALBridge(UInt16 id, ...)
{
  uint32  result = 0;
  va_list arglist;

  va_start(arglist, id);
  switch (id)
  {
    case BRIDGE_GLOBALS_INITIALIZE:
         GlobalsInitialize();
         break;

    case BRIDGE_GLOBALS_TERMINATE:
         GlobalsTerminate();
         break;

    case BRIDGE_APPLICATION_INITIALIZE:
         result = ApplicationInitialize();
         break;

    case BRIDGE_APPLICATION_GET_UNIQUE_ID:
         result = ApplicationGetUniqueID();
         break;

    case BRIDGE_APPLICATION_GET_NAME:
         result = (uint32)ApplicationGetName();
         break;

    case BRIDGE_APPLICATION_GET_VERSION:
         result = ApplicationGetVersion();
         break;

    case BRIDGE_APPLICATION_GET_PREFS:
         result = (uint32)ApplicationGetPreferences();
         break;

    case BRIDGE_APPLICATION_GET_PREFS_SIZE:
         result = ApplicationGetPreferencesSize();
         break;

    case BRIDGE_APPLICATION_HANDLE_EVENT:
         {
           event *e;
           e      = va_arg(arglist, event *);
           result = ApplicationHandleEvent(e);
         }
         break;

    case BRIDGE_APPLICATION_TERMINATE:
         ApplicationTerminate();
         break;

    case BRIDGE_SYS_GET_EVENT:
         {
           event *e;
           e      = va_arg(arglist, event *);
           _SysGetEvent(e);
         }
         break;

    case BRIDGE_SYS_EVENT_AVAILABLE:
         result = _SysEventAvailable();
         break;

    case BRIDGE_PEN_HANDLE_EVENT:
         {
           event   *e;
           Boolean *penDown;
           Coord   *screenX;
           Coord   *screenY;

           penDown = va_arg(arglist, Boolean *);
           screenX = va_arg(arglist, Coord *);
           screenY = va_arg(arglist, Coord *);
           e       = va_arg(arglist, event *);
           result  = _PenHandleEvent(*penDown, *screenX, *screenY, e);
         }
         break;

    case BRIDGE_KEY_HANDLE_EVENT:
         {
           event  *e;
           UInt16 *chr;
           UInt16 *keyCode;
           UInt16 *modifiers;

           chr       = va_arg(arglist, UInt16 *);
           keyCode   = va_arg(arglist, UInt16 *);
           modifiers = va_arg(arglist, UInt16 *);
           e         = va_arg(arglist, event *);
           result = _KeyHandleEvent(*chr, *keyCode, *modifiers, e);
         } 
         break;

    case BRIDGE_GFX_REPAINT:
         _GfxRepaint();
         break;

    default: 
         break;
  }
  va_end(arglist);

  return result;
}

static void
DALBridgeTerminate()
{
}

/*************************************************************************
 *
 *     The Common DAL Functions (application framework = 68k)
 *
 *************************************************************************/

#include "DAL-common.inc"

/********************************* EOF ***********************************/
