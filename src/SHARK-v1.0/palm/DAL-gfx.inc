/*************************************************************************
 *
 * Copyright (c) 2002-2003 MobileWizardry
 * All rights reservered.
 *
 *************************************************************************/

/*
 * @(#)palm/DAL-gfx.inc
 *
 * -- Aaron Ardiri     (mailto:aaron_ardiri@mobilewizardry.com)
 * -- Charles Kerchner (mailto:chip_kerchner@mobilewizardry.com)
 */

#include "DAL-prv.h"

/*************************************************************************
 *
 * Platform Implementation Structures
 *
 *************************************************************************/

typedef struct _Gfx_DeviceInfo
{
  struct
  {
    Boolean        device;
    uint16         libRef;

    struct
    {
      Boolean      device;
      uint16       libRef;
    } silk;

  } sonyHR;                           // running on sony hi-res device?

#ifndef _arm_
  struct
  {
    Boolean        device;

    struct
    {
      Boolean      device;
      Boolean      maximized_entry;   // was the silk screen maximized on entry?
    } silk;
  } handeraQVGA;                      // running on handera QVGA device?
#endif

  struct
  {
    Boolean        device;

    struct
    {
      Boolean      device;
      Boolean      maximized_entry;   // was the silk screen maximized on entry?
    } silk;

    struct
    {
      Boolean      device;
    } hawkeye;

  } palmHDD;                          // running on palm HDD device?

  gfx_window      *win;

} _Gfx_DeviceInfo;

typedef struct _PalmWindow
{
  BitmapType   *bmp;
  BitmapTypeV3 *bmpV3;
  WinHandle    win;
  Coord        width;
  Coord        height;
  uint8        *bits;
} _PalmWindow;

typedef struct _FontInfo
{
  _PalmWindow  **winFonts;
  boolean      *fontHacked;
  coord        *fontHeight;
  uint8        **fontWidth;
  coord        **fontOffset;          // font resources
} _FontInfo;

// local functions
static void _GfxInitialize()                                                                      __DAL__;
static void _GfxCopyToDisplay16bpp(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)    __DAL__;
static void _GfxCopyToDisplay16bppDSA(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y) __DAL__;
static void _GfxCopyToDisplay8bppDSA(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)  __DAL__;
static void _GfxCopyToDisplay8bpp(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)     __DAL__;
static void _GfxCopyToDisplay4bpp(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)     __DAL__;
static void _GfxTerminate()                                                                       __DAL__;

static gfx_window  *_GfxCreateWindowX(coord width, coord height, uint16 depth)                    __DAL__;

/*************************************************************************
 *
 * Initialization / Termination
 *
 *************************************************************************/

void
_GfxInitialize()
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  uint32          *depthPtr, depthsToTry[] = { 16, 8, 4, 1 }; // 1bpp = sanity
  uint32           depthOffset;
  Err              err;
  uint32           romVersion;
  uint32           version;
  _Gfx_DeviceInfo *deviceP;
  _FontInfo       *fontP;
  int              i;
  boolean          m68k_cpu;
  uint32           cpuID;

#include "DAL-gfx-palette.h"

  gDALGlobals->gfx.info = (void *)MemPtrNew(sizeof(_Gfx_DeviceInfo));
  MemSet(gDALGlobals->gfx.info, sizeof(_Gfx_DeviceInfo), 0);
  deviceP = (_Gfx_DeviceInfo *)gDALGlobals->gfx.info;

  gDALGlobals->gfx.font = (void *)MemPtrNew(sizeof(_FontInfo));
  MemSet(gDALGlobals->gfx.font, sizeof(_FontInfo), 0);
  fontP = (_FontInfo *)gDALGlobals->gfx.font;

  // get the rom version from the device
  FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);

  //
  // device detection and library loading
  //

  err = SysLibFind(sonySysLibNameHR, &deviceP->sonyHR.libRef);
  if (err != errNone)
    err = SysLibLoad(sysFileTLibrary, sonySysFileCHRLib, &deviceP->sonyHR.libRef);
  if (err == errNone)
    err = HROpen(deviceP->sonyHR.libRef);
  deviceP->sonyHR.device = (err == errNone);
  if (deviceP->sonyHR.device) goto DEV_ABORT_DETECT;

#ifndef _arm_
  deviceP->handeraQVGA.device =
    (FtrGet(TRGSysFtrID, TRGVgaFtrNum, &version) != ftrErrNoSuchFeature);
  if (deviceP->handeraQVGA.device) goto DEV_ABORT_DETECT;
#endif

  FtrGet(sysFtrCreator, sysFtrNumWinVersion, &version);
  deviceP->palmHDD.device = (version >= 4);
  if (deviceP->palmHDD.device) goto DEV_ABORT_DETECT;

DEV_ABORT_DETECT:

  //
  // silk screen identifications
  //

  // silk screen library available?
  err = SysLibFind(sonySysLibNameSilk, &deviceP->sonyHR.silk.libRef);
  if (err != errNone)
    err = SysLibLoad(sysFileTLibrary, sonySysFileCSilkLib, &deviceP->sonyHR.silk.libRef);
  if (err == errNone)
    err = SilkLibOpen(deviceP->sonyHR.silk.libRef);
  deviceP->sonyHR.silk.device = (err == errNone);
  if (deviceP->sonyHR.silk.device) goto SLK_ABORT_DETECT;

#ifndef _arm_
  // silk screen library available?
  deviceP->handeraQVGA.silk.device =
    (FtrGet(TRGSysFtrID, TRGSilkFtrNum, &version) != ftrErrNoSuchFeature);
  if (deviceP->handeraQVGA.silk.device) goto SLK_ABORT_DETECT;
#endif

  // silk screen library available?
  deviceP->palmHDD.silk.device =
    (FtrGet(pinCreator, pinFtrAPIVersion, &version) != ftrErrNoSuchFeature);

  // hawkeye device?
  deviceP->palmHDD.hawkeye.device = 
    (FtrGet(statLibCreator, statFtrNumVersion, &version) != ftrErrNoSuchFeature);

SLK_ABORT_DETECT:

  //
  // device gfx initialization
  //

  gDALGlobals->gfx.dsa = 1;  // assume we can do DSA :P
  depthOffset          = 1;  // 16bpp unless told otherwise 

  // any device with silk area, must try to be 16bpp first to prevent palette fuck up
  if (   deviceP->sonyHR.silk.device  
      || deviceP->palmHDD.silk.device 
      || deviceP->palmHDD.hawkeye.device
#ifndef _arm_
      || deviceP->handeraQVGA.silk.device
#endif
     ) depthOffset = 0;

  gDALGlobals->gfx._4bpp[0]    = (uint8 *)MemPtrNew(sizeof(uint8) * PALETTE_SIZE);
  gDALGlobals->gfx._4bpp[1]    = (uint8 *)MemPtrNew(sizeof(uint8) * PALETTE_SIZE);
  gDALGlobals->gfx._16bpp      = (uint16 *)MemPtrNew(sizeof(uint16) * PALETTE_SIZE);
  gDALGlobals->gfx.palette     = (uint8 *)MemPtrNew(sizeof(uint8) * PALETTE_SIZE * 4);
  gDALGlobals->gfx.paletteDiff = (uint32 *)MemPtrNew(sizeof(uint32) * PALETTE_SIZE);
  MemSet(gDALGlobals->gfx._4bpp[0],    sizeof(uint8)  * PALETTE_SIZE, 0);
  MemSet(gDALGlobals->gfx._4bpp[1],    sizeof(uint8)  * PALETTE_SIZE, 0);
  MemSet(gDALGlobals->gfx._16bpp,      sizeof(uint16) * PALETTE_SIZE, 0);
  MemSet(gDALGlobals->gfx.palette,     sizeof(uint8)  * PALETTE_SIZE * 4, 0);
  MemSet(gDALGlobals->gfx.paletteDiff, sizeof(uint32) * PALETTE_SIZE, 0);

  // sony: change into the appropriate display depth
  if (deviceP->sonyHR.device)
  {
    Coord width, height;

    gDALGlobals->device_id   = DEVICE_PALM_SONY;
    gDALGlobals->gfx.width   = 320;
    gDALGlobals->gfx.height  = 320;
    gDALGlobals->gfx.density = kDensityDouble;

    // title height is always 10% pixels of screen
#ifdef PALM_TITLE_OFF
    gDALGlobals->gfx.titleHeight = 0;
#else
    gDALGlobals->gfx.titleHeight = 32;
#endif

    // ok, if we have the silk screen, lets play!
    if (deviceP->sonyHR.silk.device)
    {
      // lets go full screen!
      SilkLibEnableResize(deviceP->sonyHR.silk.libRef);
#ifdef PALM_SILK_MINIMIZED
      SilkLibResizeDispWin(deviceP->sonyHR.silk.libRef, silkResizeToStatus);
#else
      SilkLibResizeDispWin(deviceP->sonyHR.silk.libRef, silkResizeNormal);
#endif
      SilkLibDisableResize(deviceP->sonyHR.silk.libRef);

      // if not square, assume virtual grafitti = must be in 16bpp
      HRWinGetDisplayExtent(deviceP->sonyHR.libRef, &width, &height);
      if (width != height) depthOffset = 0;
      gDALGlobals->gfx.dsa &= (depthOffset != 0);  // special: 16bpp = no DSA
    }
    else
    {
      width  = 320;
      height = 320;
    }

    // CHANGE DISPLAY MODE
    depthPtr = &depthsToTry[depthOffset];
    while (HRWinScreenMode(deviceP->sonyHR.libRef,
                           winScreenModeSet,
                           &gDALGlobals->gfx.width, &gDALGlobals->gfx.height,
                           depthPtr, NULL))
      depthPtr++;

    gDALGlobals->gfx.depth  = *depthPtr;
    gDALGlobals->gfx.width  = width;
    gDALGlobals->gfx.height = height;

#ifndef PALM_SILK_MINIMIZED
    if (gDALGlobals->gfx.height > gDALGlobals->gfx.width)
      gDALGlobals->gfx.height = gDALGlobals->gfx.width;      // square screen!
    if (gDALGlobals->gfx.height < gDALGlobals->gfx.width)
      gDALGlobals->gfx.width  = gDALGlobals->gfx.height;     // square screen!
#endif
  }

#ifndef _arm_
  // handera: change into the appropriate display depth
  else
  if (deviceP->handeraQVGA.device)
  {
    gDALGlobals->device_id   = DEVICE_PALM_HANDERA;
    gDALGlobals->gfx.width   = 240;
    gDALGlobals->gfx.height  = 240;
    gDALGlobals->gfx.density = kDensityOneAndAHalf;

    // title height is always 10% pixels of screen
#ifdef PALM_TITLE_OFF
    gDALGlobals->gfx.titleHeight = 0;
#else
    gDALGlobals->gfx.titleHeight = 24;
#endif

    // we want 1:1
    VgaSetScreenMode(screenMode1To1, rotateModeNone);

    // ok, if we have the silk screen, lets play!
    if (deviceP->handeraQVGA.silk.device)
    {
      // lets keep track of this
      deviceP->handeraQVGA.silk.maximized_entry = SilkWindowMaximized();

#ifdef PALM_SILK_MINIMIZED
      SilkMinimizeWindow();
      gDALGlobals->gfx.height = gDALGlobals->gfx.height+64;
#else
      SilkMaximizeWindow();
#endif
    }

    // if not square, assume virtual grafitti = must be in 16bpp
    depthOffset = 0;  // all QVGA = virtual grafitti
    gDALGlobals->gfx.dsa &= (depthOffset != 0);  // special: 16bpp = no DSA

    // CHANGE DISPLAY MODE
    depthPtr = &depthsToTry[depthOffset];
    while (WinScreenMode(winScreenModeSet, NULL, NULL, depthPtr, NULL))
      depthPtr++;

    gDALGlobals->gfx.depth = *depthPtr;
  }
#endif

  // high density device: change into the appropriate display depth
  else
  if (deviceP->palmHDD.device)
  {
    gDALGlobals->device_id = DEVICE_PALM_HDD;

    // get the current display information
    WinScreenGetAttribute(winScreenWidth,  &gDALGlobals->gfx.width);
    WinScreenGetAttribute(winScreenHeight, &gDALGlobals->gfx.height);

    // if not square, assume virtual grafitti = must be in 16bpp
    if (gDALGlobals->gfx.width != gDALGlobals->gfx.height) depthOffset = 0;
    gDALGlobals->gfx.dsa &= (depthOffset != 0);  // special: 16bpp = no DSA

#ifndef PALM_SILK_MINIMIZED
    if (gDALGlobals->gfx.height > gDALGlobals->gfx.width)
      gDALGlobals->gfx.height = gDALGlobals->gfx.width;      // square screen!
    if (gDALGlobals->gfx.height < gDALGlobals->gfx.width)
      gDALGlobals->gfx.width  = gDALGlobals->gfx.height;     // square screen!
#endif

    // title height is always 10% pixels of screen
#ifdef PALM_TITLE_OFF
    gDALGlobals->gfx.titleHeight = 0;
#else
    gDALGlobals->gfx.titleHeight = gDALGlobals->gfx.height / 10;
#endif

    // which depth do we have?
    switch (gDALGlobals->gfx.width)
    {
      case 160: gDALGlobals->gfx.density = kDensityLow;         break;
      case 240: gDALGlobals->gfx.density = kDensityOneAndAHalf; break;
      case 320: gDALGlobals->gfx.density = kDensityDouble;      break;
      default:  gDALGlobals->gfx.width   = 160;
                gDALGlobals->gfx.height  = 160;
                gDALGlobals->gfx.density = kDensityLow;         break;
    }

    // ok, if we have the silk screen, lets play!
    if (deviceP->palmHDD.silk.device)
    {
      // lets keep track of this
      deviceP->palmHDD.silk.maximized_entry = (PINGetInputAreaState() == pinInputAreaClosed);

#ifdef PALM_SILK_MINIMIZED
      {
        uint32 data; // width, height
        StatGetAttribute(statAttrDimension, &data);
        gDALGlobals->gfx.height -= (data & 0xffff);   // lower bits = height
      }
#endif
      gDALGlobals->gfx.height -= gDALGlobals->gfx.titleHeight;
    }

    // CHANGE DISPLAY MODE
    depthPtr = &depthsToTry[depthOffset];
    while (WinScreenMode(winScreenModeSet, NULL, NULL, depthPtr, NULL))
      depthPtr++;

    gDALGlobals->gfx.depth = *depthPtr;
  }

  // generic: change into the appropriate display depth
  else
  {
    gDALGlobals->device_id   = DEVICE_PALM;
    gDALGlobals->gfx.width   = 160;
    gDALGlobals->gfx.height  = 160;
    gDALGlobals->gfx.density = kDensityLow;

    // title height is always 10% pixels of screen
#ifdef PALM_TITLE_OFF
    gDALGlobals->gfx.titleHeight = 0;
#else
    gDALGlobals->gfx.titleHeight = 16;
#endif

    depthPtr = &depthsToTry[depthOffset];
    while (WinScreenMode(winScreenModeSet, NULL, NULL, depthPtr, NULL))
      depthPtr++;

    gDALGlobals->gfx.depth = *depthPtr;
  }

  // adjust for title bar
  gDALGlobals->gfx.height = gDALGlobals->gfx.height - gDALGlobals->gfx.titleHeight;

  //
  // gfx initialization
  //

  // lets make sure we are running on a "m68k chip" :) - blt routines work
  FtrGet(sysFtrCreator, sysFtrNumProcessorID, &cpuID);
  m68k_cpu = ((cpuID == sysFtrNumProcessor328) ||
              (cpuID == sysFtrNumProcessorEZ)  ||
              (cpuID == sysFtrNumProcessorVZ)  ||
              (cpuID == sysFtrNumProcessorSuperVZ));

  WinSetDrawWindow(WinGetDisplayWindow());

  gDALGlobals->gfx.window =
    _GfxCreateWindow(gDALGlobals->gfx.width, gDALGlobals->gfx.height);
  _GfxSetDrawWindow(gDALGlobals->gfx.window);

  // prepare blitter function pointer (if needed)
  gDALGlobals->gfx.fnCopyRegion = NULL;
  deviceP->win                  = NULL;
  switch (gDALGlobals->gfx.depth)
  {
    case 16:
         deviceP->win = _GfxCreateWindowX(gDALGlobals->gfx.width, gDALGlobals->gfx.height, 16);
         if (gDALGlobals->gfx.dsa)
           gDALGlobals->gfx.fnCopyRegion = _GfxCopyToDisplay16bppDSA;
         else
           gDALGlobals->gfx.fnCopyRegion = _GfxCopyToDisplay16bpp;
         break;

    case 8:
         if ((romVersion < palmos5) || (m68k_cpu))
           gDALGlobals->gfx.fnCopyRegion = _GfxCopyToDisplay8bpp;
         else
         if (gDALGlobals->gfx.dsa)
           gDALGlobals->gfx.fnCopyRegion = _GfxCopyToDisplay8bppDSA;
         break;

    case 4:
         if ((romVersion < palmos5) || (m68k_cpu))
           gDALGlobals->gfx.fnCopyRegion = _GfxCopyToDisplay4bpp;
         else
         if (gDALGlobals->gfx.dsa)
           gDALGlobals->gfx.fnCopyRegion = _GfxCopyToDisplay4bpp;
         break;

    default:
         break;
  }

  // initialize the "font" resources
  fontP->winFonts   =
    (_PalmWindow **)MemPtrNew(sizeof(_PalmWindow *) * MAX_FONT);
  fontP->fontHacked = (boolean *)MemPtrNew(sizeof(boolean) * MAX_FONT);
  fontP->fontHeight = (coord *)MemPtrNew(sizeof(coord) * MAX_FONT);
  fontP->fontWidth  = (uint8 **)MemPtrNew(sizeof(uint8 *) * MAX_FONT);
  fontP->fontOffset = (coord **)MemPtrNew(sizeof(coord *) * MAX_FONT);
  for (i=0; i<MAX_FONT; i++)
  {
    fontP->winFonts[i]   = NULL;
    fontP->fontWidth[i]  = NULL;
    fontP->fontOffset[i] = NULL;
  }

  // default to system palette
  _GfxSetPalette(sys_palette);
}

void
_GfxTerminate()
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  _Gfx_DeviceInfo *deviceP;
  _FontInfo       *fontP;
  int              i;

  deviceP = (_Gfx_DeviceInfo *)gDALGlobals->gfx.info;
  fontP   = (_FontInfo *)gDALGlobals->gfx.font;

  //
  // gfx shutdown
  //

  _GfxSetDrawWindow(NULL);
  _GfxDeleteWindow(gDALGlobals->gfx.window);
  switch (gDALGlobals->gfx.depth)
  {
    case 16:
         _GfxDeleteWindow(deviceP->win);
         break;

    default:
         break;
  }

  MemPtrFree(gDALGlobals->gfx._4bpp[0]);
  MemPtrFree(gDALGlobals->gfx._4bpp[1]);
  MemPtrFree(gDALGlobals->gfx._16bpp);
  MemPtrFree(gDALGlobals->gfx.palette);
  MemPtrFree(gDALGlobals->gfx.paletteDiff);
  gDALGlobals->gfx._4bpp[0]    = NULL;
  gDALGlobals->gfx._4bpp[1]    = NULL;
  gDALGlobals->gfx._16bpp      = NULL;
  gDALGlobals->gfx.palette     = NULL;
  gDALGlobals->gfx.paletteDiff = NULL;

  // clean up "font" resources
  for (i=0; i<MAX_FONT; i++)
    _FntReleaseFont(i);
  MemPtrFree(fontP->winFonts);
  MemPtrFree(fontP->fontHacked);
  MemPtrFree(fontP->fontHeight);
  MemPtrFree(fontP->fontWidth);
  MemPtrFree(fontP->fontOffset);

  //
  // device gfx shutdown
  //

  if (deviceP->sonyHR.device)
  {
    HRWinScreenMode(deviceP->sonyHR.libRef,
                    winScreenModeSetToDefaults, NULL, NULL, NULL, NULL);
  }
  else
    WinScreenMode(winScreenModeSetToDefaults, NULL, NULL, NULL, NULL);

  //
  // device detection and library unloading
  //

  if (deviceP->sonyHR.device)
  {
    HRClose(deviceP->sonyHR.libRef);
  }

#ifndef _arm_
  if (deviceP->handeraQVGA.silk.device)
  {
    // let restore the old silkscreen setting (when we came in)
    if (deviceP->handeraQVGA.silk.maximized_entry)
      SilkMaximizeWindow();
  }
#endif

  // ok, if we have the silk screen, lets play!
  if (deviceP->palmHDD.silk.device)
  {
    // let restore the old silkscreen setting (when we came in)
    if (deviceP->palmHDD.silk.maximized_entry)
      PINSetInputAreaState(pinInputAreaClosed);

    PINSetInputTriggerState(pinInputTriggerEnabled);
  }

  // shutdown the silk library if it exists
  if (deviceP->sonyHR.silk.device)
  {
    // restore back to normal and shutdown
    SilkLibEnableResize(deviceP->sonyHR.silk.libRef);
    SilkLibResizeDispWin(deviceP->sonyHR.silk.libRef, silkResizeNormal);
    SilkLibDisableResize(deviceP->sonyHR.silk.libRef);
    SilkLibClose(deviceP->sonyHR.silk.libRef);
  }

  MemPtrFree(gDALGlobals->gfx.info);
  MemPtrFree(gDALGlobals->gfx.font);

  // restore the system palette
  WinPalette(winPaletteSetToDefault, 0, 0, NULL);
}

/*************************************************************************
 *
 * Platform Implementation
 *
 *************************************************************************/

gfx_window *
_GfxCreateWindowX(coord width, coord height, uint16 depth)
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  _Gfx_DeviceInfo *deviceP;
  _PalmWindow     *window;
  Err              err;

  deviceP = (_Gfx_DeviceInfo *)gDALGlobals->gfx.info;

  window = (_PalmWindow *)MemPtrNew(sizeof(_PalmWindow));
  if (window != NULL)
  {
    MemSet(window, sizeof(_PalmWindow), 0);

    // sony device
    if (deviceP->sonyHR.device)
    {
      window->bmp =
        HRBmpCreate(deviceP->sonyHR.libRef,
                    (Coord)width, (Coord)height, depth, NULL, &err);
      if (err == errNone)
        window->win =
          HRWinCreateBitmapWindow(deviceP->sonyHR.libRef, window->bmp, &err);
    }
  
#ifndef _arm_
    // handera device
    else
    if (deviceP->handeraQVGA.device)
    {
      uint16 rowBytes;
      uint32 size;
  
      // we need to do some checks here, just in case we need larger 64k bitmap
      rowBytes    = width + (width%2);  // 2byte boundary
      size        = sizeof(BitmapType) + ((uint32)rowBytes*(uint32)height);
  
      // < 64k window, can use the normal technique for bitmap allocation
      if (size < 64000)
      {
        window->bmp =
          BmpCreate((Coord)width, (Coord)height, depth, NULL, &err);
        if (err == errNone)
          window->win = WinCreateBitmapWindow(window->bmp, &err);
      }
  
      // > 64k version, *hack hack hack*
      else
      {
        window->bmp = BmpCreate(16, 16, depth, NULL, &err); // small window
        if (err == errNone)
        {
          window->win = WinCreateBitmapWindow(window->bmp, &err);
  
          // we have to tweak/play around with the bitmap now - fun fun :)
          if (err == errNone)
          {
            WinHandle winHandle;
  
            // we do not need this bitmap anymore :) (it was only a dummy)
            BmpDelete(window->bmp);
  
            window->bmp =
              (BitmapType *)MemChunkNew(0, size, 0x1200); // memNewChunkFlagNonMovable 0x0200
                                                          // memNewChunkFlagAllowLarge 0x1000

            // initialize the bitmap
            MemSet(window->bmp, size, 0);
            window->bmp->width     = width;
            window->bmp->height    = height;
            window->bmp->rowBytes  = rowBytes;
            window->bmp->pixelSize = depth;
            window->bmp->version   = BitmapVersionTwo;
  
            // fix the window structure accordingly
            window->win->bitmapP                = (void *)window->bmp;
            window->win->windowBounds.extent.x  = width;
            window->win->windowBounds.extent.y  = height;
            window->win->displayWidthV20        = width;
            window->win->displayHeightV20       = height;
            window->win->displayAddrV20         = (void *)window->bmp;
            window->win->windowFlags.freeBitmap = false;
  
            // make sure the clipping is set appropriately
            winHandle = WinSetDrawWindow(window->win);
            WinResetClip();
            WinSetDrawWindow(winHandle);
          }
        }
      }
    }
#endif
  
    // generic device
    else
    {
      window->bmp =
        BmpCreate((Coord)width, (Coord)height, depth, NULL, &err);
  
      if (err == errNone)
      {
        // high density device
        if (deviceP->palmHDD.device)
        {
          window->bmpV3 =
            BmpCreateBitmapV3(window->bmp, gDALGlobals->gfx.density,
                              BmpGetBits(window->bmp), NULL);
  
          err = (window->bmpV3 == NULL);
          if (err == errNone)
            window->win =
              WinCreateBitmapWindow((BitmapType *)window->bmpV3, &err);
        }
        else
          window->win = WinCreateBitmapWindow(window->bmp, &err);
      }
    }
    window->width  = width;
    window->height = height;
  
    // any errors?
    if (err != errNone)
    {
      if (window->bmp != NULL)   BmpDelete(window->bmp); 
      if (window->bmpV3 != NULL) BmpDelete((BitmapType *)window->bmpV3);
      if (window->win != NULL)   WinDeleteWindow(window->win, false);
  
      window->bmp   = NULL;
      window->bmpV3 = NULL;
      window->win   = NULL;

      MemPtrFree(window); window = NULL;
    }
    else
      window->bits = (uint8 *)BmpGetBits(WinGetBitmap(window->win));
  }

  return (gfx_window *)window;
}

gfx_window *
_GfxCreateWindow(coord width, coord height) 
{
  return _GfxCreateWindowX(width, height, GFX_WINDOW_BPP);
}

gfx_window *
_GfxCreateWindowFromBuffer(coord width, coord height, uint8 *bits)
{
  _PalmWindow *window;

  window = (_PalmWindow *)MemPtrNew(sizeof(_PalmWindow));
  if (window != NULL)
  {
    MemSet(window, sizeof(_PalmWindow), 0);

    window->width  = width;
    window->height = height;
    window->bits   = bits;
  }

  return (gfx_window *)window;
}

void
_GfxDeleteWindow(gfx_window *win)
{
  _PalmWindow *window = (_PalmWindow *)win;

  if (win != NULL)
  {
    if (window->bmp != NULL)   BmpDelete(window->bmp);
    if (window->bmpV3 != NULL) BmpDelete((BitmapType *)window->bmpV3);
    if (window->win != NULL)   WinDeleteWindow(window->win, false);

    MemPtrFree(window);
  }
}

void
_GfxCopyToDisplay(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  _Gfx_DeviceInfo   *deviceP;
  _PalmWindow       *window = (_PalmWindow *)win;
  RectangleType      _rect;

#ifdef PALM_TIMING_CHECK
  scr_y += (gDALGlobals->gfx.density * 11) / kDensityLow;
#endif

  // do we have a function pointer for customized blitting?
  if (gDALGlobals->gfx.fnCopyRegion == NULL)
  {
    deviceP = (_Gfx_DeviceInfo *)gDALGlobals->gfx.info;

    _rect.topLeft.x = rect->x;
    _rect.topLeft.y = rect->y;
    _rect.extent.x  = rect->width;
    _rect.extent.y  = rect->height;

    // sony devices
    if (deviceP->sonyHR.device)
      HRWinCopyRectangle(deviceP->sonyHR.libRef,
                         window->win, WinGetDisplayWindow(),
                         &_rect, (Coord)scr_x, (Coord)(scr_y+gDALGlobals->gfx.titleHeight), winPaint);

    // palm hires devices
    else
    if (deviceP->palmHDD.device)
    {
      WinSetCoordinateSystem(kCoordinatesNative);
      WinCopyRectangle(window->win, WinGetDisplayWindow(),
                       &_rect, (Coord)scr_x, (Coord)(scr_y+gDALGlobals->gfx.titleHeight), winPaint);
      WinSetCoordinateSystem(kCoordinatesStandard);
    }

    // generic devices
    else
      WinCopyRectangle(window->win, WinGetDisplayWindow(),
                       &_rect, (Coord)scr_x, (Coord)(scr_y+gDALGlobals->gfx.titleHeight), winPaint);
  }

  // customized blitter (function pointer)
  else
    gDALGlobals->gfx.fnCopyRegion(win, rect, scr_x, scr_y);
}

/*************************************************************************
 *
 * Library Routines
 *
 *************************************************************************/

#include "DAL-gfx-lib.inc"
#include "DAL-gfx-fonts.inc"

/*************************************************************************
 *
 * Custom Device Blitters
 *
 *************************************************************************/

void 
_GfxCopyToDisplay16bpp(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  _Gfx_DeviceInfo   *deviceP;
  _PalmWindow       *window = (_PalmWindow *)win;
  _PalmWindow       *winLCD;
  RectangleType      _rect;

  deviceP = (_Gfx_DeviceInfo *)gDALGlobals->gfx.info;

  //
  // translation: 8bpp -> 16bpp
  //

  winLCD = (_PalmWindow *)deviceP->win;
  {
    uint16 *ptrLCD, *pal;
    uint8  *ptrWin;
    int     cnt, pix, i;
    int32   win_delta, scr_delta;
#ifdef _68k_
    uint    win1, win2;
#else
    uint32  win1, win2;
#endif

    // determine the "pointer" value, and how many bytes to blit
    pal     = gDALGlobals->gfx._16bpp;
    ptrWin  =  (uint8 *)window->bits;
    ptrLCD  = (uint16 *)winLCD->bits;
    ptrWin += (((uint32)rect->y * window->width) + rect->x);
    ptrLCD += (((uint32)rect->y * winLCD->width) + rect->x);

    win_delta = window->width - rect->width;
    scr_delta = winLCD->width - rect->width;

    cnt = rect->height;
    pix = rect->width;

    // do each line...
    do
    {
      i = pix;

      // must have multiple of 8 for unrolling
#ifdef _68k_
      if (i & 0x07)
      {
        uint win2 = (i & 0x07);
        do
        {
		      win1 = *ptrWin++;
          *ptrLCD++ = *(pal + win1);
        } while (--win2);
      }

      // chunks of 8 writes (faster)
      if (i >>= 3)
      {
        do 
        {
		      win1 = ptrWin[0];
		      win2 = ptrWin[1];
          ptrLCD[0] = *(pal + win1);
		      win1 = ptrWin[2];
          ptrLCD[1] = *(pal + win2);
		      win2 = ptrWin[3];
          ptrLCD[2] = *(pal + win1);
		      win1 = ptrWin[4];
          ptrLCD[3] = *(pal + win2);
		      win2 = ptrWin[5];
          ptrLCD[4] = *(pal + win1);
		      win1 = ptrWin[6];
          ptrLCD[5] = *(pal + win2);
		      win2 = ptrWin[7];
          ptrLCD[6] = *(pal + win1);
          ptrWin += 8;
          ptrLCD[7] = *(pal + win2);
          ptrLCD += 8;
        }
        while (--i);
      }
#else
      win2 = (4 - ((uint32)(ptrWin))) & 0x3;
      if (win2)
      {
        i -= win2;
        do
        {
          win1 = *ptrWin++;
          *ptrLCD++ = *(pal + win1);
        }
        while (--win2);
      }

      // chunks of 8 writes (faster)
      win2 = i & 0x7;
      if (i >>= 3)
      {
        do 
        {
		      win1 = ((uint32 *)(ptrWin))[0];
          ((uint32 *)(ptrLCD))[0] = (*(uint16 *)((uint8 *)(pal) + ((win1 >> 7) & 0x1fe)) << 16)  +
            *(uint16 *)((uint8 *)(pal) + ((win1 & 0xff) << 1));
          ((uint32 *)(ptrLCD))[1] = (*(uint16 *)((uint8 *)(pal) + ((win1 >> 23) & 0x1fe)) << 16) +
            *(uint16 *)((uint8 *)(pal) + ((win1 >> 15) & 0x1fe));
		      win1 = ((uint32 *)(ptrWin))[1];
          ptrWin += 8;
          ((uint32 *)(ptrLCD))[2] = (*(uint16 *)((uint8 *)(pal) + ((win1 >> 7) & 0x1fe))  << 16) +
            *(uint16 *)((uint8 *)(pal) + ((win1 & 0xff) << 1));
          ((uint32 *)(ptrLCD))[3] = (*(uint16 *)((uint8 *)(pal) + ((win1 >> 23) & 0x1fe)) << 16) +
            *(uint16 *)((uint8 *)(pal) + ((win1 >> 15) & 0x1fe));
          ptrLCD += 8;
        }
        while (--i);
      }

      if (win2)
      {
        do
        {
          win1 = *ptrWin++;
          *ptrLCD++ = *(pal + win1);
        }
        while (--win2);
      }
#endif

      ptrWin += win_delta;
      ptrLCD += scr_delta;
    }
    while (--cnt);
  }

  //
  // copy:        16bpp -> LCD
  //

  _rect.topLeft.x = rect->x;
  _rect.topLeft.y = rect->y;
  _rect.extent.x  = rect->width;
  _rect.extent.y  = rect->height;

  // sony devices
  if (deviceP->sonyHR.device)
    HRWinCopyRectangle(deviceP->sonyHR.libRef,
                       winLCD->win, WinGetDisplayWindow(),
                       &_rect, (Coord)scr_x, (Coord)(scr_y+gDALGlobals->gfx.titleHeight), winPaint);

  // palm hires devices
  else
  if (deviceP->palmHDD.device)
  {
    WinSetCoordinateSystem(kCoordinatesNative);
    WinCopyRectangle(winLCD->win, WinGetDisplayWindow(),
                     &_rect, (Coord)scr_x, (Coord)(scr_y+gDALGlobals->gfx.titleHeight), winPaint);
    WinSetCoordinateSystem(kCoordinatesStandard);
  }

  // generic devices
  else
    WinCopyRectangle(winLCD->win, WinGetDisplayWindow(),
                     &_rect, (Coord)scr_x, (Coord)(scr_y+gDALGlobals->gfx.titleHeight), winPaint);
}

void
_GfxCopyToDisplay16bppDSA(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  _PalmWindow *window = (_PalmWindow *)win;
  uint16      *ptrLCD = (uint16 *)BmpGetBits(WinGetBitmap(WinGetDisplayWindow()));
  uint16      *pal    = gDALGlobals->gfx._16bpp;
  coord        x1, y1, x2, y2;

  // adjust for title bar
  ptrLCD += (gDALGlobals->gfx.titleHeight * gDALGlobals->gfx.width);

  // whats a "valid" box area for this operation?
  x1 = MAX(0, rect->x);
  y1 = MAX(0, rect->y);
  x2 = MIN(window->width,  rect->x + rect->width);
  y2 = MIN(window->height, rect->y + rect->height);

  // whats a "valid" box area for this operation? - destination window
  if (scr_x < 0) { x1 = x1 - scr_x; scr_x = 0; }
  if (scr_y < 0) { y1 = y1 - scr_y; scr_y = 0; }
  if ((scr_x + (x2 - x1)) > gDALGlobals->gfx.width)  { x2 = x1 + (gDALGlobals->gfx.width  - scr_x); }
  if ((scr_y + (y2 - y1)) > gDALGlobals->gfx.height) { y2 = y1 + (gDALGlobals->gfx.height - scr_y); }

  // do we still have a valid region?
  if ((x2 > x1) && (y2 > y1))
  {
    uint8 *ptrWin;
    int    cnt;
    int32  win_delta, scr_delta;

    // determine the "pointer" value, and how many bytes to blit
    ptrWin  = window->bits + (((uint32)y1 * window->width) + x1);
    ptrLCD += (((uint32)scr_y * gDALGlobals->gfx.width) + scr_x);
    cnt     = (x2 - x1);

    win_delta = window->width - cnt;
    scr_delta = gDALGlobals->gfx.width - cnt;

    y2 -= y1;
    // do each line...
    do
    {
      int16 cnt2 = cnt;

      do
      {
        *ptrLCD++ = *(pal + *ptrWin++);
      }
      while (--cnt2);

      ptrWin += win_delta;
      ptrLCD += scr_delta;
    }
    while (--y2);
  }
}

void
_GfxCopyToDisplay8bppDSA(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  _PalmWindow *window = (_PalmWindow *)win;
  uint8       *ptrLCD = BmpGetBits(WinGetBitmap(WinGetDisplayWindow()));
  coord        x1, y1, x2, y2;

  // adjust for title bar
  ptrLCD += (gDALGlobals->gfx.titleHeight * gDALGlobals->gfx.width);

  // whats a "valid" box area for this operation?
  x1 = MAX(0, rect->x);
  y1 = MAX(0, rect->y);
  x2 = MIN(window->width,  rect->x + rect->width);
  y2 = MIN(window->height, rect->y + rect->height);

  // whats a "valid" box area for this operation? - destination window
  if (scr_x < 0) { x1 = x1 - scr_x; scr_x = 0; }
  if (scr_y < 0) { y1 = y1 - scr_y; scr_y = 0; }
  if ((scr_x + (x2 - x1)) > gDALGlobals->gfx.width)  { x2 = x1 + (gDALGlobals->gfx.width  - scr_x); }
  if ((scr_y + (y2 - y1)) > gDALGlobals->gfx.height) { y2 = y1 + (gDALGlobals->gfx.height - scr_y); }

  // do we still have a valid region?
  if ((x2 > x1) && (y2 > y1))
  {
    uint8 *ptrWin;
    int    cnt;
    int32  win_delta, scr_delta;

    // determine the "pointer" value, and how many bytes to blit
    ptrWin  = window->bits + (((uint32)y1 * window->width) + x1);
    ptrLCD += (((uint32)scr_y * gDALGlobals->gfx.width) + scr_x);
    cnt     = (x2 - x1);

    win_delta = window->width - cnt;
    scr_delta = gDALGlobals->gfx.width - cnt;

    y2 -= y1;
    // do each line...
    do
    {
      int16 cnt2 = cnt;

      do
      {
        *ptrLCD++ = *ptrWin++;
      }
      while (--cnt2);

      ptrWin += win_delta;
      ptrLCD += scr_delta;
    }
    while (--y2);
  }
}

void
_GfxCopyToDisplay8bpp(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  _PalmWindow *window = (_PalmWindow *)win;
  uint8       *ptrLCD = BmpGetBits(WinGetBitmap(WinGetDisplayWindow()));
  coord        x1, y1, x2, y2;

  // adjust for title bar
  ptrLCD = ptrLCD + gDALGlobals->gfx.titleHeight * gDALGlobals->gfx.width;

  // whats a "valid" box area for this operation?
  x1 = MAX(0, rect->x);
  y1 = MAX(0, rect->y);
  x2 = MIN(window->width,  rect->x + rect->width);
  y2 = MIN(window->height, rect->y + rect->height);

  // whats a "valid" box area for this operation? - destination window
  if (scr_x < 0) { x1 = x1 - scr_x; scr_x = 0; }
  if (scr_y < 0) { y1 = y1 - scr_y; scr_y = 0; }
  if ((scr_x + (x2 - x1)) > gDALGlobals->gfx.width)  { x2 = x1 + (gDALGlobals->gfx.width  - scr_x); }
  if ((scr_y + (y2 - y1)) > gDALGlobals->gfx.height) { y2 = y1 + (gDALGlobals->gfx.height - scr_y); }

  // do we still have a valid region?
  if ((x2 > x1) && (y2 > y1))
  {
    uint8 *ptrWin;
    int    cnt;
    int32  win_delta, scr_delta;

    // determine the "pointer" value, and how many bytes to blit
    ptrWin  = window->bits + (((uint32)y1 * window->width) + x1);
    ptrLCD += (((uint32)scr_y * gDALGlobals->gfx.width) + scr_x);
    cnt     = (x2 - x1);

    win_delta = window->width - cnt;
    scr_delta = gDALGlobals->gfx.width - cnt;

    y2 -= y1;
    // do each line...
    do
    {
      int16 cnt2 = cnt;

      // copy bytes if we're dword unaligned
      if ((uint8)((uint32)(ptrLCD)) & 0x01)
      {
        *ptrLCD++ = *ptrWin++;
        cnt2--;
      }

      // if we're even aligned, copy dwords
      if (((uint8)((uint32)(ptrWin)) & 0x01) == 0)
      {
        int16 dwords = cnt2 >> 2;

        if (dwords)
        {
          int16 cnt3;

          cnt2 &= 0x03;

          cnt3 = (dwords & 0x03);
          if (cnt3)
          {
            dwords -= cnt3;
            do
            {
              *((uint32 *)(ptrLCD))++ = *((uint32 *)(ptrWin))++;
            }
            while (--cnt3);
          }

          if (dwords >>= 2)
          {
            do
            {
              *((uint32 *)(ptrLCD))++ = *((uint32 *)(ptrWin))++;
              *((uint32 *)(ptrLCD))++ = *((uint32 *)(ptrWin))++;
              *((uint32 *)(ptrLCD))++ = *((uint32 *)(ptrWin))++;
              *((uint32 *)(ptrLCD))++ = *((uint32 *)(ptrWin))++;
            }
            while (--dwords);
          }
        }
      }

      // copy remaining bytes
      if (cnt2)
      {
        do
        {
          *ptrLCD++ = *ptrWin++;
        }
        while (--cnt2);
      }

      ptrWin += win_delta;
      ptrLCD += scr_delta;
    }
    while (--y2);
  }
}

void
_GfxCopyToDisplay4bpp(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y)
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  _PalmWindow *window = (_PalmWindow *)win;
  uint8       *ptrLCD = BmpGetBits(WinGetBitmap(WinGetDisplayWindow()));
  coord        x1, y1, x2, y2;

  // adjust for title bar
  ptrLCD = ptrLCD + gDALGlobals->gfx.titleHeight * (gDALGlobals->gfx.width >> 1);

  // whats a "valid" box area for this operation?
  x1 = MAX(0, rect->x);
  y1 = MAX(0, rect->y);
  x2 = MIN(window->width,  rect->x + rect->width);
  y2 = MIN(window->height, rect->y + rect->height);

  // whats a "valid" box area for this operation? - destination window
  if (scr_x < 0) { x1 = x1 - scr_x; scr_x = 0; }
  if (scr_y < 0) { y1 = y1 - scr_y; scr_y = 0; }
  if ((scr_x + (x2 - x1)) > gDALGlobals->gfx.width)  { x2 = x1 + (gDALGlobals->gfx.width  - scr_x); }
  if ((scr_y + (y2 - y1)) > gDALGlobals->gfx.height) { y2 = y1 + (gDALGlobals->gfx.height - scr_y); }

  // do we still have a valid region?
  if ((x2 > x1) && (y2 > y1))
  {
    uint8 *ptrWin;
    int    cnt;
    uint8 *gray0, *gray1;
    int32  win_delta, scr_delta;

    // determine the "pointer" value, and how many bytes to blit
    ptrWin  = window->bits + (((uint32)y1 * window->width) + x1);
    ptrLCD += ((((uint32)scr_y * gDALGlobals->gfx.width) + scr_x) >> 1);  // NOTE: 4bpp
    cnt     = (x2 - x1);

    win_delta = window->width - cnt;
    scr_delta = ((gDALGlobals->gfx.width - cnt) >> 1);
    if ((cnt & 0x01) && !(x1 & 0x01)) scr_delta++;

    gray0 = &gDALGlobals->gfx._4bpp[0][0];
    gray1 = &gDALGlobals->gfx._4bpp[1][0];

    y2 -= y1;
    // do each line...
    do
    {
      uint16 cnt2 = cnt;

      // prefix pixel
      if (x1 & 0x01)
      {
        *ptrLCD = (*ptrLCD & 0xf0) | gray1[*ptrWin++]; ptrLCD++;
        cnt2--;
      }

      // pixels aligned on byte boundary
      cnt2 >>= 1;
      if (cnt2)
      {
        do
        {
          uint8 hi = *ptrWin++;
          uint8 lo = *ptrWin++;

          *ptrLCD++ = gray0[hi] | gray1[lo];
        }
        while (--cnt2);
      }

      // postfix pixel
      if (x2 & 0x01)
      {
        *ptrLCD = (*ptrLCD & 0x0f) | gray0[*ptrWin++];
      }

      ptrWin += win_delta;
      ptrLCD += scr_delta;
    }
    while (--y2);
  }
}

/********************************* EOF ***********************************/
