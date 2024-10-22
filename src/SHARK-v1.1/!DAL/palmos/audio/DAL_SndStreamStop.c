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
#ifdef _M_ARM
  PALM_AudioInfo *audio;
  GLOBALS_ACCESS;

  // entry condition, do we have a valid audio stream?
  if (stream_id != 0) return;

  // lets look at the thread handles, see if it is activated
  audio  = (PALM_AudioInfo *)g->audio.platform;
  if (audio != NULL)
  {
    // flag as no-activated
    audio->active = false;
  }
#endif
}

/********************************* EOF ***********************************/
