/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_SndThread.c
 */

#include "../DAL-prv.h"

DWORD WINAPI 
DAL_SndThread(void *data)
{
  Win32_AudioInfo *audio;
  uint32           index;
  uint16           framesize, bufferused;
  DWORD            timeout;

  // grab a handle to our audio engine information
  audio = (Win32_AudioInfo *)data;
  if (audio == NULL) return 0;

  // initialize counters/semaphores
  audio->freeblocks = MAX_BUFFERS;
  InitializeCriticalSection(&audio->wcs);

  // set the thread priority
//SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL); 
  SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

  // this thread stays active until its ready to shutdown the application
  index   = 0;
  timeout = WAIT_TIMEOUT;
  while (timeout == WAIT_TIMEOUT)
  {
    // do we have a wave-out object instance?
    if (audio->active)
    {
      // how many frames do we need to render
      framesize  = AUDIO_FRAME_SIZE;       // MAX RENDER = 2048 frames
      bufferused = AUDIO_FRAME_SIZE;
      if (audio->type   == snd_stereo) bufferused = bufferused << 1;
      if (audio->format == snd_int16)  bufferused = bufferused << 1;

      // obtain buffer from application callback
      if (audio->callback != NULL) audio->callback(audio->id, audio->user, audio->buffer[index], framesize);
      else                         _MemSet(audio->buffer[index], bufferused, 0);
  
      // prepare buffer for playback
      waveOutUnprepareHeader(audio->hwo, &audio->whdr[index], sizeof(WAVEHDR));  
      _MemSet(&audio->whdr[index], sizeof(WAVEHDR), 0);
      audio->whdr[index].lpData         = audio->buffer[index];
      audio->whdr[index].dwBufferLength = bufferused;
      waveOutPrepareHeader(audio->hwo, &audio->whdr[index], sizeof(WAVEHDR));  
      
      // SPECIAL: win32 API doesn't support int8 audio streams (need to convert to uint8)
      if (audio->format == snd_int8)
      {
        uint8 *p;
        int    i;

        p = (uint8 *)audio->buffer[index];  
        i = bufferused;
        do
        {
          *p = *p ^ 0x80; // int8 -> uint8
          p++; 
        }
        while (--i);
      }

      // play buffer
      waveOutWrite(audio->hwo, &audio->whdr[index], sizeof(WAVEHDR));
      EnterCriticalSection(&audio->wcs);
      audio->freeblocks--;
      LeaveCriticalSection(&audio->wcs);
  
      // wait for the next block to become free  
      while ((audio->freeblocks == 0) && (timeout == WAIT_TIMEOUT))
        timeout = WaitForSingleObject(audio->request, 10);

      index++; if (index >= MAX_BUFFERS) index = 0;
    }

    // are we still waiting for a trigger here?
    if (timeout == WAIT_TIMEOUT)
      timeout = WaitForSingleObject(audio->request, 10);
  }

  // clean up counters/semaphores
  DeleteCriticalSection(&audio->wcs);

  return 0;
}

/********************************* EOF ***********************************/
