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

#define VOLUME_ADJUST 8

Err 
DAL_SndThread(void *userData, SndStreamRef stream, void *buffer, UInt32 frameCount)
{
#ifdef _M_ARM
  PALM_AudioInfo *audio;
  int8           *pS;
  int16          *pD;
  int             i;
  uint16          framesize, bufferused;

  // grab a handle to our audio engine information
  audio = (PALM_AudioInfo *)userData;
  if ((audio == NULL) || (audio->callback == NULL)) goto AUDIO_SILENT;

  // how many frames do we need to render
  framesize  = frameCount;       // MAX RENDER = 2048 frames
  bufferused = frameCount;
  if (audio->type   == snd_stereo) bufferused = bufferused << 1;
  if (audio->format == snd_int16)  bufferused = bufferused << 1;

  // convert it dynamically to 16bit (or, use directly)
  switch (audio->format)
  {
    case snd_int8:
        
         // we may not have enough memory allocated for this operation
         if (frameCount > AUDIO_FRAME_SIZE) goto AUDIO_SILENT;

         // fill the audio callback buffer appropriately (internally)
         audio->callback(audio->id, audio->user, audio->buffer, framesize);

         // convert from 8bit to 16bit on the fly
         pD = (int16 *)buffer;
         pS = (int8 *)audio->buffer;
         i  = bufferused;
         do
         {
           *pD++ = (int16)*pS++ << VOLUME_ADJUST; // int8 -> int16
         }
         while (--i);
         break;

    case snd_uint8:
        
         // we may not have enough memory allocated for this operation
         if (frameCount > AUDIO_FRAME_SIZE) goto AUDIO_SILENT;

         // fill the audio callback buffer appropriately (internally)
         audio->callback(audio->id, audio->user, audio->buffer, framesize);

         // convert from 8bit to 16bit on the fly
         pD = (int16 *)buffer;
         pS = (int8 *)audio->buffer;
         i  = bufferused;
         do
         {
           *pD++ = (int16)(((uint8)*pS++) - 0x80) << VOLUME_ADJUST; // uint8 -> int16
         }
         while (--i);
         break;

    case snd_int16:

         // fill the audio callback buffer appropriately (direct to buffer)
         audio->callback(audio->id, audio->user, buffer, framesize);
         break;

    default:
         break;
  }

  goto AUDIO_DONE;

AUDIO_SILENT:

  // silent audio please
  _MemSet(buffer, (frameCount << 1), 0);  // remember, 16bit

AUDIO_DONE:

#endif

  return 0;
}

/********************************* EOF ***********************************/
