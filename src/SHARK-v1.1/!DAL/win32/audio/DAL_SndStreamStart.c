/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_SndStreamStart.c
 */

#include "../DAL-prv.h"

void
DAL_SndStreamStart(snd_stream stream_id)
{
  Win32_AudioInfo *audio;
  GLOBALS_ACCESS;

  // entry condition, do we have a valid audio stream?
  if (stream_id != 0) return;

  // lets look at the thread handles, see if it is activated
  audio  = (Win32_AudioInfo *)g->audio.platform;
  if (audio != NULL)
  {
    // flag as activated
    audio->active = true;
  }
}

/********************************* EOF ***********************************/
