/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-system.h
 */

#ifndef SHARK_SYSTEM_H
#define SHARK_SYSTEM_H

#include "../SHARK.h"

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
 *
 * Constants / Structures
 *
 *************************************************************************/

typedef enum
{
  _nilEvent = 0,
  _appStartEvent,
  _appStopEvent,
  _penEvent,
  _keyEvent
} sys_event;

struct _GenericEvent { uint32 _scratch_;  };
struct _PenEvent     { coord  x; coord y; };
struct _KeyEvent     { uint8  chr;        };

typedef struct event
{
   int16     eType;
   boolean   penDown;
   uint8     padding;
   coord     screenX;
   coord     screenY;

   union
   {
     struct _GenericEvent generic;
     struct _PenEvent     pen;
     struct _KeyEvent     key;
   } data;

} event;

/*************************************************************************
 *
 * Application Programmers Interface
 *
 *************************************************************************/

// device specific
extern void        _SysSetFPS(uint32 fps);
extern uint32      _SysGetTime();
extern uint32      _SysGetTicks();
extern void        _SysDelay(uint32 millisec);
extern void        _SysDebugMessage(uint8 *msg, boolean dialog);

// device independent
extern uint32      _SysGetFPS();
extern void        _SysEnqueueEvent(event *e);
extern void        _SysEnqueueQuitEvent();
extern uint16      _SysRandom(uint32 seed);
extern void        _SysSetGlobals(void *gbls);
extern void       *_SysGetGlobals();

#ifdef __cplusplus
}
#endif

#endif

/********************************* EOF ***********************************/
