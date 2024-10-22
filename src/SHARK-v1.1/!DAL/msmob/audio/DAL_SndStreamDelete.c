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
  MSmob_AudioInfo *audio;
  int              i;
  GLOBALS_ACCESS;

  // entry condition, do we have a valid audio stream?
  if (stream_id != 0) return;

  // lets look at the thread handles, see if it is activated
  audio  = (MSmob_AudioInfo *)g->audio.platform;
  if (audio != NULL)
  {
    // we need to ensure that the wave out object is valid
    if (audio->hwo != NULL)
    {
      waveOutReset(audio->hwo);
      for (i=0; i<MAX_BUFFERS; i++)
        waveOutUnprepareHeader(audio->hwo, &audio->whdr[i], sizeof(WAVEHDR));
      waveOutClose(audio->hwo);
      audio->hwo = NULL;

      // terminate the audio playback
      EnterCriticalSection(&audio->wcs);
      audio->freeblocks = MAX_BUFFERS;
      LeaveCriticalSection(&audio->wcs);
    }
  }
}

/********************************* EOF ***********************************/
