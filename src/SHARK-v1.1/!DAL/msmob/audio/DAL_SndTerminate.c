/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_SndTerminate.c
 */

#include "../DAL-prv.h"

void
DAL_SndTerminate()
{
  MSmob_PlatformInfo *platform;
  MSmob_AudioInfo    *audio;
  DWORD               timeout;
  int                 i;
  GLOBALS_ACCESS;

  // terminate the active audio stream
  _SndStreamStop(0);
  _SndStreamDelete(0);

  // terinate the audio thread
  audio    = (MSmob_AudioInfo *)g->audio.platform;
  platform = (MSmob_PlatformInfo *)g->platform;
  if ((platform != NULL) && (audio != NULL))
  {
    if ((audio->request != NULL) && (platform->audio.thread != NULL))
    {
      SetEvent(audio->request);
      timeout = WaitForSingleObject(platform->audio.thread, 1000);
      if (timeout == WAIT_TIMEOUT)
        TerminateThread(platform->audio.thread, 0);
    }

    if (audio->request != NULL)         CloseHandle(audio->request); 
    if (platform->audio.thread != NULL) CloseHandle(platform->audio.thread);
    platform->audio.thread = NULL;
    audio->request         = NULL;
  }

  // clean up memory usage within audio system
  audio = (MSmob_AudioInfo *)g->audio.platform;
  if (audio != NULL)
  {
    for (i=0; i<MAX_BUFFERS; i++)
    {
      if (audio->buffer[i] != NULL) _MemPtrFree(audio->buffer[i]);
      audio->buffer[i] = NULL;
    }
  }

  if (g->audio.platform != NULL) _MemPtrFree(g->audio.platform);
  g->audio.platform = NULL; audio = NULL;
}

/********************************* EOF ***********************************/
