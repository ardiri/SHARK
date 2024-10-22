/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-audio.c
 */

/*************************************************************************
 *
 * Configuration
 *
 *************************************************************************/

/*************************************************************************
 *
 * Global Variables
 *
 *************************************************************************/

/*************************************************************************
 *
 * Implementation
 *
 *************************************************************************/

// initialize the streaming audio manager
static boolean
_SndInitialize()
{
  boolean init_ok;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _LOGCreateEntry(">> _SndInitialize");
#endif

  // assume everything is ok
  init_ok = true;

  return init_ok;
}

// does this hardware support streaming audio?
boolean
_SndStreamAvailable()
{
  return true;
}

// which audio rates are supported
uint32
_SndStreamRatesSupported()
{                
  uint32 result;
  uint32 platform_id;

  // default return value
  result = 0;

  // lets get the device information
  _SysDeviceInfo(&platform_id, NULL);
  switch (platform_id)
  {
    case DEVICE_SENDO_X:
         result = SND_STREAM_MONO | SND_STREAM_22KHZ;
         break;

    default:
         result = SND_STREAM_MONO | SND_STREAM_8KHZ | SND_STREAM_16KHZ;
         break;
  }

  return result;
}

// start an audio stream
snd_stream 
_SndStreamCreate(snd_rate rate, snd_format format, snd_type type, 
                 _SndStreamBufferCallback func, void *user, uint16 frames)
{
  snd_stream stream_id;
  
  // default return value
  stream_id = -1;

//return stream_id;
  return 0;
}

// start an audio stream
void
_SndStreamStart(snd_stream stream_id)
{
}

// stop an audio stream
void
_SndStreamStop(snd_stream stream_id)
{
}

// delete an audio stream
void
_SndStreamDelete(snd_stream stream_id)
{
}

// terminate the streaming audio engine
static void
_SndTerminate()
{
#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _LOGCreateEntry(">> _SndTerminate");
#endif
}

/*************************************************************************
 *
 * Implementation - device independent
 *
 *************************************************************************/

#include "../common/SHARK-audio.inc"

/********************************* EOF ***********************************/
