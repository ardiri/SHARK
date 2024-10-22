/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_KeyCurrentState.c
 */

#include "../DAL-prv.h"

uint32
DAL_KeyCurrentState()
{
  PALM_PlatformInfo *platform;
  uint32             result     = 0;
  UInt32             keyState;
  GLOBALS_ACCESS;

  keyState = KeyCurrentState();

  // access device/platform information block
  platform = (PALM_PlatformInfo *)g->platform;
  if (platform != NULL)
  {
    // tapwave zodiac - lets zap the normal keys (interference)
    if (platform->input.tapwave)
      keyState = keyState & 0xffffff00;   
  
#ifdef INP_SUPPORT_POWER
    // filter the power button from processing
    if (platform->input.power)
      keyState = keyState & (~keyBitHard4);
#endif

    // special combination keys
    if ((keyState & keyBitHard1) &&
        (keyState & keyBitHard4))     result |= KeyOption1;
    if ((keyState & keyBitHard2) &&
        (keyState & keyBitHard3))     result |= KeyOption2;
    if (result != 0) goto KEY_ABORT;

    // normal application buttons (with d-pad available)
    if (platform->input.dpad)
    {
      if ((keyState & keyBitHard1) ||
          (keyState & keyBitHard2) ||
          (keyState & keyBitHard3) ||
          (keyState & keyBitHard4))    result |= KeyAction;
      if  (keyState & keyBitPageUp)    result |= KeyPadUp;
      if  (keyState & keyBitPageDown)  result |= KeyPadDown;
    }
    else
    // normal application buttons
    {
      if (keyState & keyBitHard1)      result |= KeyAction;
      if (keyState & keyBitHard2)      result |= KeyPadLeft;
      if (keyState & keyBitHard3)      result |= KeyPadRight;
      if (keyState & keyBitHard4)      result |= KeyAction;
      if (keyState & keyBitPageUp)     result |= KeyPadUp;
      if (keyState & keyBitPageDown)   result |= KeyPadDown;
    }
  
    // handera keys
    if (keyState & keyBitJogUp)        result |= KeyPadUp;
    if (keyState & keyBitJogDown)      result |= KeyPadDown;
    if (keyState & keyBitJogPush)      result |= KeyPadSelect;
  
    // palmos 5.0 keys [dpad]
    if (keyState & keyBitPageUp)       result |= KeyPadUp;
    if (keyState & keyBitPageDown)     result |= KeyPadDown;
    if (keyState & keyBitNavLeft)      result |= KeyPadLeft;
    if (keyState & keyBitNavRight)     result |= KeyPadRight;
    if (!platform->input.tapwave)
    {
      // special: ignore this on tapwave device
      if (keyState & keyBitNavSelect)  result |= KeyPadSelect;
    }
  
    // palmos 5.0 keys [handspring]
    if (keyState & keyBitRockerUp)     result |= KeyPadUp;
    if (keyState & keyBitRockerDown)   result |= KeyPadDown;
    if (keyState & keyBitRockerLeft)   result |= KeyPadLeft;
    if (keyState & keyBitRockerRight)  result |= KeyPadRight;
    if (!platform->input.tapwave)
    {
      // special: ignore this on tapwave device
      if (keyState & keyBitRockerSelect) result |= KeyPadSelect;
    }

    // tapwave
    if (platform->input.tapwave)
    {
      if (keyState & keyBitTriggerA)   result |= KeyOption1;
      if (keyState & keyBitTriggerB)   result |= KeyOption2;
      if ((keyState & keyBitActionA)  ||
          (keyState & keyBitActionB)  ||
          (keyState & keyBitActionC)  ||
          (keyState & keyBitActionD))  result |= KeyAction;
      if (keyState & keyBitFunction)   result |= KeyPause;
    }
  }

KEY_ABORT:

  return result;
}

/********************************* EOF ***********************************/
