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
  PALM_PlatformInfo *platform;
  boolean            init_ok;
#ifdef _M_ARM
  PALM_AudioInfo    *audio;
  UInt32             version;  
  UInt32             cID, dID;
  Err                err;
#endif
  GLOBALS_ACCESS;

  // assume everythign is ok
  init_ok = true;

  // access device/platform information block
  platform = (PALM_PlatformInfo *)g->platform;
  if (platform != NULL)
  {
    // default return value
    platform->audio.streaming = false;

#ifdef _M_ARM
    // check for sound manager feature
    err = FtrGet(0x736e646d, sndFtrIDVersion, &version);    // 'sndm' = 0x736e646d
    platform->audio.streaming |= (err == errNone);
    
    // check for Tungsten T // 'Palm' = 0x50616c6d :: 'Frg1' = 0x46726731
    FtrGet(0x70737973, sysFtrNumOEMCompanyID, &cID);        // 'psys' = 0x70737973, 
    FtrGet(0x70737973, sysFtrNumOEMDeviceID,  &dID);        // 'psys' = 0x70737973
    platform->audio.streaming |= ((cID == 0x50616c6d) && (dID == 0x46726731)); 
#endif
  }

#ifdef _M_ARM
  g->audio.platform = (event *)_MemPtrNew(sizeof(PALM_AudioInfo), false);
  init_ok = (g->audio.platform != NULL); if (!init_ok) goto SND_INIT_ABORT;
  _MemSet(g->audio.platform, sizeof(PALM_AudioInfo), 0);

  // create the audio thread, default to 'tone-gen' engine
  audio    = (PALM_AudioInfo *)g->audio.platform;
  platform = (PALM_PlatformInfo *)g->platform;
  if ((platform != NULL) && (audio != NULL))
  {
#ifndef AUD_ONLY_REQUIRED
    // initialize memory
    audio->size = AUDIO_BUFFER_SIZE;
    audio->buffer = _MemPtrNew(audio->size, false); 
    init_ok = (audio->buffer != NULL); if (!init_ok) goto SND_INIT_ABORT;
    _MemSet(audio->buffer, audio->size, 0);
#else
    audio->buffer = NULL;
#endif
  }

  // everything ok?
  goto SND_INIT_SUCCESS;

SND_INIT_ABORT:

  // clean up memory usage within audio system
  audio = (PALM_AudioInfo *)g->audio.platform;
  if (audio != NULL)
  {
    if (audio->buffer != NULL) _MemPtrFree(audio->buffer);
    audio->buffer = NULL;
  }

  if (g->audio.platform != NULL) _MemPtrFree(g->audio.platform);
  g->audio.platform = NULL; audio = NULL;

SND_INIT_SUCCESS:

#endif

  return init_ok;
}

/********************************* EOF ***********************************/
