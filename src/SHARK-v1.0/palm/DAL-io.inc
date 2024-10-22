/*************************************************************************
 *
 * Copyright (c) 2002-2003 MobileWizardry
 * All rights reservered.
 *
 *************************************************************************/

/*
 * @(#)palm/DAL-io.inc
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

typedef struct _IO_DeviceInfo
{
#ifndef _arm_
  struct
  {
    Boolean    device;                      // is the gamepad driver present
    uint16     libRef;                      // library reference for gamepad
    uint32     gamePadUserCount;
  } gamePad;
#endif

  struct
  {
    Boolean    device;                      // 4-button zire/zire21?
  } zire;

  Boolean      hasNativeDPAD;
  Boolean      hasNoStdKeys;
} _IO_DeviceInfo;

// local functions
static void    _IOInitialize()                                                          __DAL__;
static boolean _KeyHandleEvent(uint16 chr, uint16 keyCode, uint16 modifiers, event *e)  __DAL__;
static boolean _PenHandleEvent(Boolean penDown, Coord screenX, Coord screenY, event *e) __DAL__;
static void    _IOTerminate()                                                           __DAL__;

/*************************************************************************
 *
 * Initialization / Termination
 *
 *************************************************************************/

void
_IOInitialize()
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  _IO_DeviceInfo *deviceP;
#ifndef _arm_
  Err             err;
  uint32          deviceID;
#endif
  uint32          version;

  gDALGlobals->io.info = (void *)MemPtrNew(sizeof(_IO_DeviceInfo));
  MemSet(gDALGlobals->io.info, sizeof(_IO_DeviceInfo), 0);
  deviceP = (_IO_DeviceInfo *)gDALGlobals->io.info;

  // initialize defaults
  gDALGlobals->io.keyMask = ~KeyAllMask;

  // has native dpad configurations (put all possible checks here)?
  deviceP->hasNativeDPAD  = false;
  deviceP->hasNoStdKeys   = false;
  deviceP->hasNativeDPAD |= (FtrGet(navFtrCreator, navFtrVersion, &version)  != ftrErrNoSuchFeature);
  deviceP->hasNoStdKeys  |= (FtrGet(twFtrCreator, twFtrAPIVersion, &version) != ftrErrNoSuchFeature);

  // let get the hardware flags
  version = 0;
  FtrGet(sysFtrCreator, sysFtrNumUIHardwareFlags, &version);
  deviceP->hasNativeDPAD |= (version & sysFtrNumUIHardwareHas5Way);

  // TREO 600: first GM workaround (didn't have hardware flags)
  {
    uint32 com, dev;

    FtrGet(sysFtrCreator, sysFtrNumOEMCompanyID, &com);
    FtrGet(sysFtrCreator, sysFtrNumOEMDeviceID,  &dev);

    // hspr = 0x68737072, H101 = 0x48313031
    deviceP->hasNativeDPAD |=  ((com == 0x68737072) && (dev == 0x48313031));
  }
  
#ifndef _arm_
  // gamepad driver: initialize
  err = SysLibFind(GPD_LIB_NAME,&deviceP->gamePad.libRef);
  if (err == sysErrLibNotFound) 
    err = SysLibLoad(sysFileTLibrary,GPD_LIB_CREATOR,&deviceP->gamePad.libRef);
  deviceP->gamePad.device = (err == errNone);
  if (deviceP->gamePad.device) GPDOpen(deviceP->gamePad.libRef);
#endif

#ifndef _arm_
  FtrGet(sysFtrCreator, sysFtrNumOEMDeviceID, &deviceID); 
  deviceP->zire.device = ((deviceID == 'Cubs') || (deviceID == 'Zi21'));
#endif
}

void
_IOTerminate()
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  _IO_DeviceInfo *deviceP;
#ifndef _arm_
  Err             err;
#endif

  deviceP = (_IO_DeviceInfo *)gDALGlobals->io.info;

#ifndef _arm_
  // gamepad driver: shutdown
  if (deviceP->gamePad.device)
  {
    err = GPDClose(deviceP->gamePad.libRef, &deviceP->gamePad.gamePadUserCount);
    if (deviceP->gamePad.gamePadUserCount == 0)
      SysLibRemove(deviceP->gamePad.libRef);
  }
#endif

  // clean up
  MemPtrFree(deviceP);
  gDALGlobals->io.info = NULL;
}

/*************************************************************************
 *
 * Platform Implementation
 *
 *************************************************************************/

void
_KeySetMask(uint32 mask)
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  gDALGlobals->io.keyMask = mask;
}

uint32
_KeyGetMask()
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  return gDALGlobals->io.keyMask;
}

uint32
_KeyCurrentState()
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  _IO_DeviceInfo *deviceP;
#ifndef _arm_
  Err             err;
#endif
  uint32          palm_state = KeyCurrentState();
  uint32          result     = 0;

  deviceP = (_IO_DeviceInfo *)gDALGlobals->io.info;

  // are there no standard keys available (it can happen!) 
  if (deviceP->hasNoStdKeys)
    palm_state = palm_state & 0xffffff06;   // drop the lower byte [want pageup/down]

  // SPECIAL (zire 4 button device)
  if (deviceP->zire.device)
  {
    if ((palm_state & (keyBitHard1 | keyBitHard2)) == (keyBitHard1 | keyBitHard2))
    {
      palm_state &= ~(keyBitHard1 | keyBitHard2);
      result |= KeyPadSelect;
    }
    else if (palm_state & keyBitHard1) 
    {
      palm_state &= ~keyBitHard1;
      result |= KeyPadLeft;
    }
    else if (palm_state & keyBitHard2) 
    {
      palm_state &= ~keyBitHard2;
      result |= KeyPadRight;
    }
  }

  // special combination keys
  if ((palm_state & keyBitHard1) &&
      (palm_state & keyBitHard4))     result |= KeyOption1;
  if ((palm_state & keyBitHard2) &&
      (palm_state & keyBitHard3))     result |= KeyOption2;

  // standard keys
  if (deviceP->hasNativeDPAD)
  {
    if ((palm_state & keyBitHard1) ||
        (palm_state & keyBitHard2) ||
        (palm_state & keyBitHard3) ||
        (palm_state & keyBitHard4))   result |= KeyAction;
    if  (palm_state & keyBitPageUp)   result |= KeyScrollUp;
    if  (palm_state & keyBitPageDown) result |= KeyScrollDown;
  }
  else
  {
    if  (palm_state & keyBitHard1)    result |= KeyAction;
    if  (palm_state & keyBitHard2)    result |= KeyPadLeft;    // simulate 'dpad'
    if  (palm_state & keyBitHard3)    result |= KeyPadRight;   // simulate 'dpad'
    if  (palm_state & keyBitHard4)    result |= KeyAction;
    if  (palm_state & keyBitPageUp)   result |= KeyScrollUp;
    if  (palm_state & keyBitPageDown) result |= KeyScrollDown;
  }

#ifndef _arm_
  // handera keys
  if (palm_state & keyBitJogUp)     result |= KeyScrollUp;
  if (palm_state & keyBitJogDown)   result |= KeyScrollDown;
  if (palm_state & keyBitJogPush)   result |= KeyScrollSelect;
#endif

  // palmos 5.0 keys [dpad]
  if (palm_state & keyBitPageUp)    result |= KeyPadUp;
  if (palm_state & keyBitPageDown)  result |= KeyPadDown;
  if (palm_state & keyBitNavLeft)   result |= KeyPadLeft;
  if (palm_state & keyBitNavRight)  result |= KeyPadRight;
  if (palm_state & keyBitNavSelect) result |= KeyPadSelect;

  // palmos 5.0 keys [handspring/tapwave d-pad]
  if (palm_state & keyBitRockerUp)     result |= KeyPadUp;
  if (palm_state & keyBitRockerDown)   result |= KeyPadDown;
  if (palm_state & keyBitRockerLeft)   result |= KeyPadLeft;
  if (palm_state & keyBitRockerRight)  result |= KeyPadRight;
  if (palm_state & keyBitRockerSelect) result |= KeyPadSelect;

  // tapwave unit
  if ((palm_state & keyBitTriggerA) ||
      (palm_state & keyBitTriggerB) ||
      (palm_state & keyBitActionC)  ||
      (palm_state & keyBitActionD))   result |= KeyAction;
  if  (palm_state & keyBitActionA)    result |= KeyOption1;
  if  (palm_state & keyBitActionB)    result |= KeyOption2;

#ifndef _arm_
  // gamepad
  if (deviceP->gamePad.device)
  {
    uint8 gamePadKeyStatus;

    // read the state of the gamepad
    err = GPDReadInstant(deviceP->gamePad.libRef, &gamePadKeyStatus);
    if (err == errNone)
    {
      // process
      if  (gamePadKeyStatus & GAMEPAD_DOWN)        result |= KeyPadDown;
      if  (gamePadKeyStatus & GAMEPAD_UP)          result |= KeyPadUp;
      if  (gamePadKeyStatus & GAMEPAD_LEFT)        result |= KeyPadLeft;
      if  (gamePadKeyStatus & GAMEPAD_RIGHT)       result |= KeyPadRight;
      if ((gamePadKeyStatus & GAMEPAD_LEFTFIRE) ||
          (gamePadKeyStatus & GAMEPAD_RIGHTFIRE))  result |= KeyPadSelect;
    }
  }
#endif

  return result;
}

boolean
_PenAvailable()
{
  return true;   // all PALM devices have stylus :)
}

void
_PenState(coord *penX, coord *penY, boolean *penDown)
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  switch (gDALGlobals->device_id)
  {
    case DEVICE_PALM_HDD:
         EvtGetPenNative(WinGetDisplayWindow(), (Coord *)penX, (Coord *)penY, (Boolean *)penDown);
         break;

    case DEVICE_PALM_SONY:
         EvtGetPen((Coord *)penX, (Coord *)penY, (Boolean *)penDown);

         if (gDALGlobals->gfx.density == kDensityDouble)
         {
           *penX = *penX << 1;
           *penY = *penY << 1;
         }
         break;

    default:
         EvtGetPen((Coord *)penX, (Coord *)penY, (Boolean *)penDown);
         break;
  }

#ifdef PALM_TIMING_CHECK
  *penY -= (gDALGlobals->gfx.density * 11) / kDensityLow;
#endif
}

#define _EvtKeydownIsVirtual(modifiers) (((modifiers) & virtualKeyMask) != 0)
#define _EvtKeydownIsCommand(modifiers) (((modifiers) & commandKeyMask) != 0)

#define _TxtCharIsHardKey(modifiers, chr)            \
  (                                                  \
    _EvtKeydownIsCommand(modifiers) &&               \
    (((chr) >= hardKeyMin) && ((chr) <= hardKeyMax)) \
  )

#define _IsFiveWayNavEvent(chr, keyCode)                                             \
  (                                                                                  \
   (((chr) == vchrPageUp) || ((chr) == vchrPageDown) || ((chr) == vchrNavChange)) && \
   (((keyCode) & (navBitsAll | navChangeBitsAll)) != 0)                              \
  )

static boolean
_KeyHandleEvent(uint16 chr, uint16 keyCode, uint16 modifiers, event *e)
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  _IO_DeviceInfo *deviceP;
  boolean processed = false;

  deviceP = (_IO_DeviceInfo *)gDALGlobals->io.info;

  // special case #1: hard keys
  if (_EvtKeydownIsCommand(modifiers) &&
      _TxtCharIsHardKey(modifiers, chr))
  {
    uint32 keyState = KeyCurrentState();

    // SPECIAL (zire 4 button device)
    if (deviceP->zire.device)
    {
      switch (chr)
      {
        case vchrHard1:
             if ((gDALGlobals->io.keyMask & KeyPadLeft) == 0)
             {
               e->eType        = _keyEvent;
               e->data.key.chr = _key_padleft;
  
               processed = true;
             }
  
             // special - both keys?
             if ((keyState & keyBitHard2) &&
                 ((gDALGlobals->io.keyMask & KeyPadSelect) == 0))
             {
               e->eType        = _keyEvent;
               e->data.key.chr = _key_padselect;
               processed = true;
             }
             break;
  
        case vchrHard2:
             if ((gDALGlobals->io.keyMask & KeyPadRight) == 0)
             {
               e->eType        = _keyEvent;
               e->data.key.chr = _key_padright;
  
               processed = true;
             }
  
             // special - both keys?
             if ((keyState & keyBitHard1) &&
                 ((gDALGlobals->io.keyMask & KeyPadSelect) == 0))
             {
               e->eType        = _keyEvent;
               e->data.key.chr = _key_padselect;
               processed = true;
             }
             break;

        default:
             break;
      }
    }
    else

    switch (chr)
    {
      case vchrHard1:
           if ((gDALGlobals->io.keyMask & KeyAction) == 0)
           {
             e->eType        = _keyEvent;
             e->data.key.chr = _key_action;

             processed = true;
           }

           // special - both keys?
           if ((keyState & keyBitHard4) &&
               ((gDALGlobals->io.keyMask & KeyOption1) == 0))
           {
             e->eType        = _keyEvent;
             e->data.key.chr = _key_option1;
             processed = true;
           }
           break;

      case vchrHard2:
           if (deviceP->hasNativeDPAD)
           {
             if ((gDALGlobals->io.keyMask & KeyAction) == 0)
             {
               e->eType        = _keyEvent;
               e->data.key.chr = _key_action;
  
               processed = true;
             }
           }
           else
           {
             if ((gDALGlobals->io.keyMask & KeyPadLeft) == 0)
             {
               e->eType        = _keyEvent;
               e->data.key.chr = _key_padleft;
  
               processed = true;
             }
           }

           // special - both keys?
           if ((keyState & keyBitHard3) &&
               ((gDALGlobals->io.keyMask & KeyOption2) == 0))
           {
             e->eType        = _keyEvent;
             e->data.key.chr = _key_option2;
             processed = true;
           }
           break;

      case vchrHard3:
           if (deviceP->hasNativeDPAD)
           {
             if ((gDALGlobals->io.keyMask & KeyAction) == 0)
             {
               e->eType        = _keyEvent;
               e->data.key.chr = _key_action;
  
               processed = true;
             }
           }
           else
           {
             if ((gDALGlobals->io.keyMask & KeyPadRight) == 0)
             {
               e->eType        = _keyEvent;
               e->data.key.chr = _key_padright;
  
               processed = true;
             }
           }

           // special - both keys?
           if ((keyState & keyBitHard2) &&
               ((gDALGlobals->io.keyMask & KeyOption2) == 0))
           {
             e->eType        = _keyEvent;
             e->data.key.chr = _key_option2;
             processed = true;
           }
           break;

      case vchrHard4:
           if ((gDALGlobals->io.keyMask & KeyAction) == 0)
           {
             e->eType        = _keyEvent;
             e->data.key.chr = _key_action;

             processed = true;
           }

           // special - both keys?
           if ((keyState & keyBitHard1) &&
               ((gDALGlobals->io.keyMask & KeyOption1) == 0))
           {
             e->eType        = _keyEvent;
             e->data.key.chr = _key_option1;

             processed = true;
           }
           break;

      default:
           break;
    }
  }
  else

  // special case #2: palmos 5.0+ [5-way dpad]
  if (_EvtKeydownIsVirtual(modifiers) && 
      _IsFiveWayNavEvent(chr, keyCode))
  {
//  if (keyCode & (navBitUp | navChangeUp))
    if (keyCode & navBitUp)
    {
      if ((gDALGlobals->io.keyMask & KeyPadUp) == 0)
      {
        e->eType        = _keyEvent;
        e->data.key.chr = _key_padup;

        processed = true;
      }
    }
//  if (keyCode & (navBitDown | navChangeDown))
    if (keyCode & navBitDown)
    {
      if ((gDALGlobals->io.keyMask & KeyPadDown) == 0)
      {
        e->eType        = _keyEvent;
        e->data.key.chr = _key_paddown;

        processed = true;
      }
    }
//  if (keyCode & (navBitLeft | navChangeLeft))
    if (keyCode & navBitLeft)
    {
      if ((gDALGlobals->io.keyMask & KeyPadLeft) == 0)
      {
        e->eType        = _keyEvent;
        e->data.key.chr = _key_padleft;

        processed = true;
      }
    }
//  if (keyCode & (navBitRight | navChangeRight))
    if (keyCode & navBitRight)
    {
      if ((gDALGlobals->io.keyMask & KeyPadRight) == 0)
      {
        e->eType        = _keyEvent;
        e->data.key.chr = _key_padright;

        processed = true;
      }
    }
//  if (keyCode & (navBitSelect | navChangeSelect))
    if (keyCode & navBitSelect)
    {
      if ((gDALGlobals->io.keyMask & KeyPadSelect) == 0)
      {
        e->eType        = _keyEvent;
        e->data.key.chr = _key_padselect;

        processed = true;
      }
    }
  }
  else

  // all other cases
  {
    switch (chr)
    {
      case vchrPageUp:                   // normal page up event
      case vchrJogUp:                    // jog dial up: sony
#ifndef _arm_
      case vchrPrevField:                // jog dial up: handera
      case vchrTrgJogUp:
#endif
      case vchrRockerUp:                 // new OS5 event
      case vchrThumbWheelUp:             // new OS5 event

           if ((gDALGlobals->io.keyMask & KeyScrollUp) == 0)
           {
             e->eType        = _keyEvent;
             e->data.key.chr = _key_scrollup;
           }
           processed = true;
           break;

      case vchrPageDown:                 // normal page down event
      case vchrJogDown:                  // jog dial down: sony
#ifndef _arm_
      case vchrNextField:                // jog dial down: handera
      case vchrTrgJogDown:
#endif
      case vchrRockerDown:               // new OS5 event
      case vchrThumbWheelDown:           // new OS5 event

           if ((gDALGlobals->io.keyMask & KeyScrollDown) == 0)
           {
             e->eType        = _keyEvent;
             e->data.key.chr = _key_scrolldown;
           }
           processed = true;
           break;

      case vchrRockerLeft:               // new OS5 event
           if ((gDALGlobals->io.keyMask & KeyPadLeft) == 0)
           {
             e->eType        = _keyEvent;
             e->data.key.chr = _key_padleft;
    
             processed = true;
           }
           break;

      case vchrRockerRight:              // new OS5 event
           if ((gDALGlobals->io.keyMask & KeyPadRight) == 0)
           {
             e->eType        = _keyEvent;
             e->data.key.chr = _key_padright;
    
             processed = true;
           }
           break;

      case vchrJogPress:                 // jog dial select: sony
#ifndef _arm_
      case vchrTrgJogPush:               // jog dial select: handera
#endif
      case vchrRockerCenter:             // new OS5 event
      case vchrThumbWheelPush:           // new OS5 event
      case vchrTriggerA:
      case vchrTriggerB:
      case vchrActionC:
      case vchrActionD:

           if ((gDALGlobals->io.keyMask & KeyScrollSelect) == 0)
           {
             e->eType        = _keyEvent;
             e->data.key.chr = _key_scrollselect;
           }
           processed = true;
           break;

      case vchrActionA:
           if ((gDALGlobals->io.keyMask & KeyOption1) == 0)
           {
             e->eType        = _keyEvent;
             e->data.key.chr = _key_option1;
           }
           processed = true;
           break;

      case vchrActionB:
           if ((gDALGlobals->io.keyMask & KeyOption2) == 0)
           {
             e->eType        = _keyEvent;
             e->data.key.chr = _key_option2;
           }
           processed = true;
           break;

      default:

           if ((chr >= 0x20) && (chr < MAX_FONT_CHAR))
           {
             e->eType        = _keyEvent;
             e->data.key.chr = chr;

             processed = true;
           }
           break;
    }
  }

  return processed;
}

static boolean
_PenHandleEvent(Boolean penDown, Coord screenX, Coord screenY, event *e)
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif

  e->eType   = _penEvent;
  e->penDown = penDown;

  switch (gDALGlobals->device_id)
  {
    case DEVICE_PALM_HDD:
    case DEVICE_PALM_SONY:

         switch (gDALGlobals->gfx.density)
         {
           case kDensityDouble:
                e->screenX = e->data.pen.x = screenX * 2;
                e->screenY = e->data.pen.y = screenY * 2;
                break;

           case kDensityOneAndAHalf:
                e->screenX = e->data.pen.x = (screenX * 3) >> 1;
                e->screenY = e->data.pen.y = (screenY * 3) >> 1;
                break;

           case kDensityLow:
           default:
                e->screenX = e->data.pen.x = screenX;
                e->screenY = e->data.pen.y = screenY;
                break;
         }
         break;

    default:
         e->screenX = e->data.pen.x = screenX;
         e->screenY = e->data.pen.y = screenY;
         break;
  }

  // only valid if within range of drawable area :)
  return ((e->screenY > gDALGlobals->gfx.titleHeight)  &&
          (e->screenX > 0)                             &&
          (e->screenX < (coord)gDALGlobals->gfx.width) &&
          (e->screenY < (coord)(gDALGlobals->gfx.height + gDALGlobals->gfx.titleHeight)));
}

boolean
_PenDisplayToWindow(coord *penX, coord *penY)
{
#ifdef _arm_
  register DAL_GlobalsType *gDALGlobals asm ("r8");
#endif
  // must adjust penY by title height
  *penY = *penY - gDALGlobals->gfx.titleHeight;

#ifdef PALM_TIMING_CHECK
  *penY -= (gDALGlobals->gfx.density * 11) / kDensityLow;
#endif

  // only valid if within range of drawable area :)
  return ((*penX > 0) &&
          (*penY > 0) &&
          (*penX < (coord)gDALGlobals->gfx.width) &&
          (*penY < (coord)gDALGlobals->gfx.height));
}

/********************************* EOF ***********************************/
