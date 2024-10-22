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
  MSmob_PlatformInfo *platform;
  GLOBALS_ACCESS;

  // work with the tone-generator stream
  platform = (MSmob_PlatformInfo *)g->platform;
  if (platform != NULL)
  {
    // does the stream actually exist?
    if ((platform->audio.tone_gen != -1) && (freq <= 11025))
    {
      // initialize variables for freq+duration configuration
      if (volume > 100) volume = 100;
      platform->audio.data.count = 0;  // stop existing audio

      platform->audio.data.index = 0;
      platform->audio.data.count = ((22050L * duration) / 1000);
      platform->audio.data.cycle = (int16)(11025 / freq);
/**
 ** LINEAR: = doesn't give good volume scale for differences
 **
      platform->audio.data.max   = 128 - (127 * volume) / 100;
      platform->audio.data.min   = 128 + (127 * volume) / 100;
 **/
      platform->audio.data.max   = 128 - (uint8)((127L * volume * volume) / 10000L);
      platform->audio.data.min   = 128 + (uint8)((127L * volume * volume) / 10000L);
      platform->audio.data.state = false;
    }
  }
}

/********************************* EOF ***********************************/
