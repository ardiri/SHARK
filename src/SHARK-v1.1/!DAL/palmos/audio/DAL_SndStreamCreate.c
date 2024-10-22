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
#ifdef _M_ARM
  PALM_PlatformInfo *platform;
  PALM_AudioInfo    *audio;
  Err                err;
  SndStreamMode      stream_mode;
  UInt32             stream_rate;
  UInt32             cID, dID;
#endif
  snd_stream         stream_id;
  GLOBALS_ACCESS;
  
  // default return value
  stream_id = -1;

#ifdef _M_ARM
  // access device/platform information block
  platform = (PALM_PlatformInfo *)g->platform;
  if (platform != NULL)
  {
    if (platform->audio.streaming) 
    {
      // if we have audio streams already running, stop them
      audio = (PALM_AudioInfo *)g->audio.platform;
      if (audio != NULL)
      {
        if (audio->active)
        {
          _SndStreamStop(0);
          _SndStreamDelete(0);
        }
        audio->active    = false;
    
        // copy over the stream properties (needed below)
        audio->rate      = rate;
        audio->format    = format;
        audio->type      = type;    
  
        if (audio->buffer != NULL) _MemPtrFree(audio->buffer);
        audio->buffer = NULL;

#ifdef AUD_ONLY_REQUIRED
        // we ONLY need to do this is not 16bit audio
        if ((audio->format == snd_int8) || (audio->format == snd_uint8))
        {      
          // initialize memory
          audio->size = AUDIO_FRAME_SIZE;
          if (audio->type   == snd_stereo) audio->size = audio->size << 1;
          audio->buffer = _MemPtrNew(audio->size, false); 
          if (audio->buffer == NULL) goto SND_CREATE_ABORT;
          _MemSet(audio->buffer, audio->size, 0);
        }
#endif
  
             if (audio->rate == snd_8khz)  stream_rate = 8000;
        else if (audio->rate == snd_11khz) stream_rate = 11025;
        else if (audio->rate == snd_22khz) stream_rate = 22050;
        else if (audio->rate == snd_44khz) stream_rate = 44100;
        else                               stream_rate = 22050;  // assume 22khz?
        stream_mode = (audio->type == snd_mono) ? sndMono : sndStereo;
    
        // lets initialize the streaming audio thread (always 16bit)
        err = SndStreamCreate(&audio->streamRef, sndOutput,
                              stream_rate, sndInt16Little, stream_mode, (void *)DAL_SndThread,
                              (void *)audio, AUDIO_FRAME_SIZE, true);   // 2048 frames
        if (err == errNone)
        {
          // flag the stream as valid - lets go!
          stream_id        = 0;
      
          audio->id        = stream_id;
          audio->callback  = (void *)func;
          audio->user      = user;    
    
          // check for Tungsten T    // 'Palm' = 0x50616c6d :: 'Frg1' = 0x46726731
          // check for Tungsten T|T2 // 'Palm' = 0x50616c6d :: 'Frg2' = 0x46726732
          FtrGet(0x70737973, sysFtrNumOEMCompanyID, &cID);        // 'psys' = 0x70737973, 
          FtrGet(0x70737973, sysFtrNumOEMDeviceID,  &dID);        // 'psys' = 0x70737973
    
          // SndStreamSetVolume::1024 = unity gain, 1:1
          if (((cID == 0x50616c6d) && (dID == 0x46726731)) ||
              ((cID == 0x50616c6d) && (dID == 0x46726732)))
            SndStreamSetVolume(audio->streamRef, 2048);
          else
            SndStreamSetVolume(audio->streamRef, 1024);
          SndStreamStart(audio->streamRef);
        } 
      }
    }
  }

SND_CREATE_ABORT:

#endif

  return stream_id;
}

/********************************* EOF ***********************************/
