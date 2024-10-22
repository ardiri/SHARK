/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_SndInitialize.c
 */

#include "../DAL-prv.h"

boolean
DAL_SndInitialize()
{
  MSmob_PlatformInfo *platform;
  MSmob_AudioInfo    *audio;
  boolean             init_ok;
  DWORD               timeout;
  int                 i;
  GLOBALS_ACCESS;

  // assume everythign is ok
  init_ok = true;

  g->audio.platform = (void *)_MemPtrNew(sizeof(MSmob_AudioInfo), false);
  init_ok = (g->audio.platform != NULL); if (!init_ok) goto SND_INIT_ABORT;
  _MemSet(g->audio.platform, sizeof(MSmob_AudioInfo), 0);

  // create the audio thread, default to 'tone-gen' engine
  audio    = (MSmob_AudioInfo *)g->audio.platform;
  platform = (MSmob_PlatformInfo *)g->platform;
  if ((platform != NULL) && (audio != NULL))
  {
    audio->request = CreateEvent(NULL, TRUE, FALSE, TEXT("AudioDeathEvent"));
    if (audio->request == NULL)         goto SND_INIT_ABORT;
    platform->audio.thread  = (HANDLE)CreateThread(NULL, 0, DAL_SndThread, (void *)audio, 0, NULL);
    if (platform->audio.thread == NULL) goto SND_INIT_ABORT;

    // initialize memory
    audio->size = AUDIO_BUFFER_SIZE;
    for (i=0; i<MAX_BUFFERS; i++)
    {
      audio->buffer[i] = _MemPtrNew(audio->size, false); 
      init_ok = (audio->buffer[i] != NULL); if (!init_ok) goto SND_INIT_ABORT;
      _MemSet(audio->buffer[i], audio->size, 0);
    }
  }

  // create the tone-generator stream
  platform = (MSmob_PlatformInfo *)g->platform;
  if (platform != NULL)
  {
    platform->audio.tone_gen = _SndStreamCreate(snd_22khz, snd_uint8, snd_mono, DAL_SndToneGenCallback, &platform->audio.data);
    if (platform->audio.tone_gen != -1) _SndStreamStart(platform->audio.tone_gen); 
  }

  // everything ok?
  goto SND_INIT_SUCCESS;

SND_INIT_ABORT:

  // terminate the audio thread
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

SND_INIT_SUCCESS:

  return init_ok;
}

/********************************* EOF ***********************************/
