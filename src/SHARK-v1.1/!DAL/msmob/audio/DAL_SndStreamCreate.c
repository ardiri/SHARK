/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_SndStreamCreate.c
 */

#include "../DAL-prv.h"

snd_stream 
DAL_SndStreamCreate(snd_rate rate, snd_format format, snd_type type, _SndStreamBufferCallback func, void *user)
{
  MSmob_PlatformInfo *platform;
  MSmob_AudioInfo    *audio;
  snd_stream          stream_id;
  MMRESULT            mmres;
  UINT                i;
  GLOBALS_ACCESS;
  
  // default return value
  stream_id = -1;

  // if we have audio streams already running, stop them
  audio = (MSmob_AudioInfo *)g->audio.platform;
  if (audio != NULL)
  {
    if (audio->active)
    {
      _SndStreamStop(0);

      // sorry, freq+duration no longer available!
      platform = (MSmob_PlatformInfo *)g->platform;
      if (platform != NULL)
        platform->audio.tone_gen = -1;  
    }
    _SndStreamDelete(0);
    audio->active    = false;

    // copy over the stream properties (needed below)
    audio->rate      = rate;
    audio->format    = format;
    audio->type      = type;    

    // prepare streaming audio
    _MemSet(&audio->wfmt, sizeof(WAVEFORMATEX), 0);
    audio->wfmt.wFormatTag      = WAVE_FORMAT_PCM;
    audio->wfmt.nChannels       = (audio->type == snd_mono) ? 1 : 2;
    switch (audio->rate)
    {
      case snd_8khz:  audio->wfmt.nSamplesPerSec = 8000;  break;
      case snd_11khz: audio->wfmt.nSamplesPerSec = 11025; break;
      case snd_22khz: audio->wfmt.nSamplesPerSec = 22050; break;
      case snd_44khz: audio->wfmt.nSamplesPerSec = 44100; break;
      default:                                            break;
    }
    switch (audio->format)
    {
      case snd_int8:
      case snd_uint8: audio->wfmt.wBitsPerSample = 8;     break;
      case snd_int16: audio->wfmt.wBitsPerSample = 16;    break;
      default:                                            break;
    }
    audio->wfmt.nBlockAlign     = audio->wfmt.nChannels * (audio->wfmt.wBitsPerSample / 8);
    audio->wfmt.nAvgBytesPerSec = (uint32)audio->wfmt.nSamplesPerSec * audio->wfmt.nBlockAlign;
    audio->wfmt.cbSize          = 0;
  
    // choose the device wave mapper device
    for (i = 0; i < waveOutGetNumDevs(); i++) 
    {
      mmres = waveOutOpen(&audio->hwo, i, &audio->wfmt, (DWORD)DAL_SndWaveOutProc, (DWORD)audio, CALLBACK_FUNCTION);
      if (mmres == MMSYSERR_NOERROR) 
      {    
        // flag the stream as valid - lets go!
        stream_id        = 0;

        audio->id        = stream_id;
        audio->callback  = func;
        audio->user      = user;
  
        break;  // get out of the device search loop
      }
      else
        audio->hwo       = NULL;
    }
  }

  return stream_id;
}

/********************************* EOF ***********************************/
