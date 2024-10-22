/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_PlaySound.c
 */

#include "../DAL-prv.h"

void
DAL_PlaySound(uint32 freq, uint16 duration, uint16 volume)
{
  PALM_PlatformInfo *platform;
  SndCommandType     snd;
  GLOBALS_ACCESS;

  // access device/platform information block
  platform = (PALM_PlatformInfo *)g->platform;
  if (platform != NULL)
  {
    snd.cmd      = sndCmdFrqOn;
    snd.reserved = 0;
    snd.param1   = (int32)freq;
    snd.param2   = (uint16)duration;
    if (volume > 99) snd.param3   = platform->audio.volume;
    else             snd.param3   = (uint16)(((uint32)platform->audio.volume * volume) / 100);
  
    // play the sound
    SndDoCmd(0, &snd, 0);
  }
}

/********************************* EOF ***********************************/
