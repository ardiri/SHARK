/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-prv.h
 */

#include "../DAL.h"

#ifndef DAL_DEVICE_PRV_H
#define DAL_DEVICE_PRV_H

/*************************************************************************
 *
 * NOTES
 *
 *************************************************************************

INP_HANDLE_KEY_UP_DN_MASK
register keyup/keydown to generate the keystate mask value, was introduced
as a possible bug-fix for the asus a716 device (erratic key codes), however
it didn't work 100% and has created a side effect on other devices.

STATUS: disable support for this mechanism

GAPI_SUPPORT
use GAPI instead of the normal GDI routines for the display buffer. 

STATUS: testing (MDM) and may need to support 5:5:5 displays

 *************************************************************************/

/*************************************************************************
 *
 * Conditional Compilation
 *
 *************************************************************************/

   #define PLT_REGISTRY_PREFERENCES    // use registry for preferences
   #define INP_BUTTONS_AS_ACTIONS      // should all APP buttons be KeyAction
// #define INP_HANDLE_KEY_UP_DN_MASK   // handle KeyCurrentState via keyup/down
   #define GAPI_SUPPORT                // use GAPI for graphics engine

/*************************************************************************
 *
 * Globals Access Optimizations
 *
 *************************************************************************/

#undef GLOBALS_ACCESS
#define GLOBALS_ACCESS 

extern DAL_Globals *g;

/*************************************************************************
 *
 * Header Defines
 *
 *************************************************************************/

// system includes
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <aygshell.h>
#include <commctrl.h>
#include <mmsystem.h>
#include <projects.h>

#include    "audio/DAL-audio-prv.h"
#include  "display/DAL-display-prv.h"
#include     "file/DAL-file-prv.h"
#include    "input/DAL-input-prv.h"
#include   "memory/DAL-memory-prv.h"
#include "platform/DAL-platform-prv.h"
#include   "system/DAL-system-prv.h"

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
 *
 * Application Constants/Structures
 *
 *************************************************************************/

#define IDS_DEFAULT_APP_NAME_ID    1
#define IDC_DEFAULT_CLASS_ID       2
#define MAX_LOADSTRING             100

#define MAX_BUFFERS 3

typedef struct MSmob_AudioInfo
{
  HANDLE           request;               // thread death request

  CRITICAL_SECTION wcs;
  uint16           freeblocks;            // thread semaphore/audio blocks

  HWAVEOUT         hwo;
  WAVEFORMATEX     wfmt;
  WAVEHDR          whdr[MAX_BUFFERS];     // wave-output handles

  snd_stream       id;
  snd_rate         rate;
  snd_format       format;
  snd_type         type;                  // stream information

  uint16           size;
  void            *buffer[MAX_BUFFERS];   // audio buffers

  boolean          active; 
  _SndStreamBufferCallback callback; 
  void            *user;                  // streaming audio callbacks

} MSmob_AudioInfo;

typedef struct MSmob_PlatformInfo
{
  struct
  {
    HANDLE        thread;                 // the audio thread

    snd_stream    tone_gen;               // tone generator
    MSmob_SndTonGen data;
  } audio;

  struct
  {
    HDC           dc;                     // window dc handle
    struct
    {
      BITMAPINFOHEADER bmiHeader;
      DWORD            bmiColors[4];      // bitmap information
    } bmi;

    HBITMAP       bmp;                    // bitmap handle
    void         *bits;                   // bitmap buffer pointer
  } display;

  struct
  {
    boolean       active;
    UINT          timerID;                // global timer
  } system;

#ifdef INP_HANDLE_KEY_UP_DN_MASK
  struct
  {
    uint32        appKeyState;
  } input;
#endif

  struct
  {
    HWND          LCDWnd;
    HDC           LCDScreen;              // LCD device context
    HINSTANCE     hInst;                  // global application instance
    HACCEL        hAccelTable;            // accelerator table

    SHACTIVATEINFO s_sai;
  } windows;

#ifdef GAPI_SUPPORT
  struct
  {
    HINSTANCE     libRef;
    int           (*GXOpenDisplay)(HWND hWnd, DWORD dwFlags);
    int           (*GXCloseDisplay)(void);
    void         *(*GXBeginDraw)(void);
    int           (*GXSuspend)(void);
    int           (*GXResume)(void);
    int           (*GXEndDraw)(void);
  } gapi;
#endif

} MSmob_PlatformInfo;

/*************************************************************************
 *
 * Platform Abstractions
 *
 *************************************************************************/

// these macro's define system specific API calls (memory related) that
// are required for application management - and, > 64k limits of SHARK.

#define SYSTEM_MemPtrNew     malloc
#define SYSTEM_MemPtrFree    free
#define SYSTEM_MemSet(p,v,c) memset(p,c,v)

/*************************************************************************
 *
 * Application Functions
 *
 *************************************************************************/

// device abstraction layer
extern boolean DAL_Initialize(void);
extern void    DAL_Configuration(void);
extern void    DAL_Terminate(void);
extern boolean GlobalsInitialize(void);
extern void    GlobalsRepaint(void);
extern void    GlobalsTerminate(void);

// device application layer
extern int WINAPI       WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);
extern boolean          InitInstance(HINSTANCE hInstance, int nCmdShow);
extern LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

extern void             InitApplication(void);
extern void             EventLoop(void);
extern void             EndApplication(void);

extern boolean          DAL_ApplicationHandleEvent(event *e);
extern boolean          DAL_ApplicationValidateDRM();

#ifdef __cplusplus
}
#endif

#endif

/********************************* EOF ***********************************/
