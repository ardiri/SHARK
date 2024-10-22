/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_SndStreamDelete.c
 */

#include "../DAL-prv.h"

void
DAL_SndStreamDelete(snd_stream stream_id)
{
#ifdef _M_ARM
  PALM_PlatformInfo *platform;
  PALM_AudioInfo    *audio;
  GLOBALS_ACCESS;

  // entry condition, do we have a valid audio stream?
  if (stream_id != 0) return;

  // access device/platform information block
  platform = (PALM_PlatformInfo *)g->platform;
  if (platform != NULL)
  {
    if (platform->audio.streaming) 
    {
      // lets shutdown the streaming audio thread
      audio  = (PALM_AudioInfo *)g->audio.platform;
      if (audio != NULL)
      {
        SndStreamStop(audio->streamRef);
        SndStreamDelete(audio->streamRef);
      }
    }
  }
#endif
}

/********************************* EOF ***********************************/
