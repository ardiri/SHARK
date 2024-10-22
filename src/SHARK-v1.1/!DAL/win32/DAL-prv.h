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
 * Conditional Compilation
 *
 *************************************************************************/

   #define PLT_REGISTRY_PREFERENCES    // use registry for preferences
   #define MEM_FORCE_NULL              // NULL from _MemPtrNewX() failure

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
#include <time.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <mmsystem.h>

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
#define IDI_DEFAULT_ICON           101

#define MAX_LOADSTRING             100

#define LOG_FILE                   "application-runtime.txt"

#define OPTIONS_DISPLAY_WIDTH      0x00000001
#define OPTIONS_DISPLAY_HEIGHT     0x00000002
#define OPTIONS_DISPLAY_DEPTH      0x00000004
#define OPTIONS_INPUT_STYLUS       0x00000008
#define OPTIONS_MEMORY_HEAP        0x00000010
#define OPTIONS_MEMORY_STORAGE     0x00000020
#define OPTIONS_MEMORY_WARNINGS    0x00000040

typedef struct Win32_DeviceInfo
{
  struct
  {
    uint32      heap_size;
    uint32      storage_size;
    uint32      warnings;
  } memory;
} Win32_DeviceInfo;

#define MAX_BUFFERS 3

typedef struct Win32_AudioInfo
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

} Win32_AudioInfo;

typedef struct Win32_PlatformInfo
{
  struct
  {
    HANDLE        thread;                 // the audio thread
    DWORD         thread_id;

    snd_stream      tone_gen;             // tone generator
    Win32_SndTonGen data;
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
    FILE         *f;
  } log;

  struct
  {
    boolean       active;
    UINT          timerID;                // global timer
  } system;

  struct
  {
    HWND          LCDWnd;
    HDC           LCDScreen;              // LCD device context
    HINSTANCE     hInst;                  // global application instance
    HACCEL        hAccelTable;            // accelerator table

    boolean       resizing;               // is the window resizing?
  } windows;

} Win32_PlatformInfo;

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

extern void             LOGInitalize();
extern void             LOG_ENTRY(uint8 *str);
extern void             LOGTerminate();

extern void             DAL_ApplicationResizeWindow(boolean);
extern boolean          DAL_ApplicationHandleEvent(event *e);
extern boolean          DAL_ApplicationValidateDRM();

#ifdef __cplusplus
}
#endif

#endif

/********************************* EOF ***********************************/
