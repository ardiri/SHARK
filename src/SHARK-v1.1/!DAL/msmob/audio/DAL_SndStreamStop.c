/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_SndStreamStop.c
 */

#include "../DAL-prv.h"

void
DAL_SndStreamStop(snd_stream stream_id)
{
  MSmob_AudioInfo *audio;
  GLOBALS_ACCESS;

  // entry condition, do we have a valid audio stream?
  if (stream_id != 0) return;

  // lets look at the thread handles, see if it is activated
  audio  = (MSmob_AudioInfo *)g->audio.platform;
  if (audio != NULL)
  {
    // flag as not-activated
    audio->active = false;
  }
}

/********************************* EOF ***********************************/
