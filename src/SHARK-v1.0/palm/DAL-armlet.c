/*************************************************************************
 *
 * Copyright (c) 2002-2003 Mobile Wizardry
 * All rights reservered.
 *
 *************************************************************************/

/*
 * @(#)palm/DAL-armlet.c
 *
 * -- Aaron Ardiri     (mailto:aaron_ardiri@mobilewizardry.com)
 * -- Charles Kerchner (mailto:chip_kerchner@mobilewizardry.com)
 */

/*************************************************************************
 *
 *     Include Files
 * 
 *************************************************************************/

#include "DAL-prv.h"

/*************************************************************************
 *
 *     Local Functions
 * 
 *************************************************************************/

uint32 ArmletProcessCommand(uint32 command, void **p);

/*************************************************************************
 *
 * FUNCTION:    ArmletEntryPoint
 *
 * DESCRIPTION: ARM code entry point
 *
 * PARAMETERS:  param parameters
 *
 * RETURNED:    none.
 *
 * REVISION HISTORY:
 *
 *************************************************************************/
uint32    
ArmletEntryPoint(const void      *emulStateP,
                 void            *userData68KP,
                 Call68KFuncType *call68KFuncP)
{
  register DAL_GlobalsType *gDALGlobals asm ("r8");
  register void            *gDALGOT     asm ("r10");

  ParameterType  *param;
  uint32          command, i;
  uint32          result = 0;
  void           *p[MAX_PARAM];

  // grab access to our user data (our parameters) 
  param = (ParameterType *)userData68KP;

  // read the variables from the parameter pointer
  gDALGlobals = (DAL_GlobalsType *)__read_unaligned32(&(param->gDALGlobals));
  gDALGOT     =            (void *)__read_unaligned32(&(param->gDALGOT));
  command     =            (uint32)__read_unaligned32(&(param->command));
  for (i=0; i<MAX_PARAM; i++)
    p[i]      =            (void *)__read_unaligned32(&(param->param[i]));

  // we need these for the 68k callback mechanism
  gDALGlobals->emulStateP   = (void *)emulStateP;
  gDALGlobals->call68KFuncP = call68KFuncP;

  // process command
  result = ArmletProcessCommand(command, p);

  // prepare to return the result value
  __write_unaligned32(&(param->result), result);

  return 0;    
}

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
 *     Command Processing
 *
 *************************************************************************/

uint32    
ArmletProcessCommand(uint32 command, void **p)
{
  uint32 result = 0;

  switch (command)
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
         result  = ApplicationHandleEvent((event *)p[0]);
         break;

    case BRIDGE_APPLICATION_TERMINATE:
         ApplicationTerminate();
         break;

    case BRIDGE_SYS_GET_EVENT:
         _SysGetEvent((event *)p[0]);
         break;

    case BRIDGE_SYS_EVENT_AVAILABLE:
         result = _SysEventAvailable();
         break;

    case BRIDGE_PEN_HANDLE_EVENT:
         {
           Boolean penDown;
           coord   screenX;
           coord   screenY;

           penDown = *((uint8 *)p[0]);
           screenX = __read_unaligned16(p[1]);
           screenY = __read_unaligned16(p[2]);
           result = _PenHandleEvent(penDown, screenX, screenY, (event *)p[3]);
         }
         break;

    case BRIDGE_KEY_HANDLE_EVENT:
         {
           uint16 chr;
           uint16 keyCode;
           uint16 modifiers;

           chr       = __read_unaligned16(p[0]);
           keyCode   = __read_unaligned16(p[1]);
           modifiers = __read_unaligned16(p[2]);
           result = _KeyHandleEvent(chr, keyCode, modifiers, (event *)p[3]);
         }
         break;

    case BRIDGE_GFX_REPAINT:
         _GfxRepaint();
         break;

    default: 
         break;
  }

  return result;
}

/********************************* E O F *********************************/
