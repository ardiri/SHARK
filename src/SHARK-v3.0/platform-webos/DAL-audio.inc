/*************************************************************************
 *
 * Copyright 2001+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-audio.inc
 */

/*************************************************************************
 * Configuration
 *************************************************************************/

// none

/*************************************************************************
 * Component Variables
 *************************************************************************/

#define SND_GLOBALS           g_snd
#define GLOBALS_UNINITIALIZED (SND_GLOBALS == NULL)
#define GLOBALS_DEFINITION    SndGlobals *g_snd = NULL;

// SNDGlobals::   platform additions
#define DAL_SND_GLOBALS \
  SDL_AudioSpec   snd_spec;

#define g_snd_spec             (SND_GLOBALS) -> snd_spec

/*************************************************************************
 * Platform Independent Implementation
 *************************************************************************/

#include "../common/SHARK-audio.inc"

/*************************************************************************
 * Implementation
 *************************************************************************/

static void DAL_SndCallback(void *, uint8 *, int);

static boolean
DAL_SndInitialize()
{
  boolean init_ok;

  // default return value
  init_ok = true;

  // clear memory used by memory globals
  memset(SND_GLOBALS, 0, sizeof(SndGlobals));

  // lets determine if there is streaming audio
  g_snd_stream_available = true;

  return init_ok;
}

static snd_stream
DAL_SndStreamCreate(uint32 frames)
{
  snd_stream result;
  int        err;

  // default return value
  result = STREAM_INVALID;

  // if we got this far, our audio interface has been configured right
  result = 0;

  // configure our audio specification
  memset(&g_snd_spec, 0, sizeof(SDL_AudioSpec));

  g_snd_spec.channels = (g_snd_type == snd_mono) ? 1 : 2;
  switch (g_snd_rate)
  {
    case snd_8khz:  g_snd_spec.freq = 8000;    break;
    case snd_11khz: g_snd_spec.freq = 11025;   break;
    case snd_16khz: g_snd_spec.freq = 16000;   break;
    case snd_22khz: g_snd_spec.freq = 22050;   break;
    case snd_44khz: g_snd_spec.freq = 44100;   break;
    default: goto SND_CREATE_ABORT; break;
  }
  switch (g_snd_format)
  {
    case snd_int8:  g_snd_spec.format = AUDIO_S8;  break;
    case snd_uint8: g_snd_spec.format = AUDIO_U8;  break;
    case snd_int16: g_snd_spec.format = AUDIO_S16; break;
    default: goto SND_CREATE_ABORT; break;
  }
  g_snd_spec.samples = frames;

  // configure our callback
  g_snd_spec.callback = DAL_SndCallback;
  g_snd_spec.userdata = NULL;

  // open the audio device
  err = SDL_OpenAudio(&g_snd_spec, NULL);  
  if (err < 0) goto SND_CREATE_ABORT;

  // lets start the audio playback
  SDL_PauseAudio(false);

  // lets inform the other applications we are active
  PDL_NotifyMusicPlaying(PDL_TRUE);

SND_CREATE_ABORT:;

  return result;
}

static void
DAL_SndStreamPause()
{
  SDL_PauseAudio(true);
}

static void
DAL_SndStreamResume()
{
  SDL_PauseAudio(false);
}

static void
DAL_SndStreamDelete(snd_stream stream_id)
{
  // lets inform the other applications we are no longer active
  PDL_NotifyMusicPlaying(PDL_FALSE);

  // shutdown the SDL layer
  SDL_CloseAudio();
}

static uint32
DAL_SndStreamRatesSupported()
{
  uint32 result;

  // default return value
  result = 0;

  // what rates/configurations are available
  result |= SND_STREAM_MONO  | SND_STREAM_STEREO;
  result |= SND_STREAM_8KHZ  | SND_STREAM_11KHZ |
            SND_STREAM_16KHZ | SND_STREAM_22KHZ | SND_STREAM_44KHZ;

  return result;
}

static void
DAL_SndCallback(void *user_data, uint8 *stream, int len)
{
  uint8 zero;

  if (g_snd_active && (stream != NULL))
  {
    if (g_snd_callback != NULL)
    {
      if (g_snd_type   == snd_stereo) len = len >> 1;
      if (g_snd_format == snd_int16)  len = len >> 1;
      g_snd_callback(0, g_snd_user, stream, len);
    }
    else
    {
      zero = 0;
      if (g_snd_format == snd_uint8) zero = 0x80;
      memset(stream, zero, len);
    }
  }
}

static void
DAL_SndTerminate()
{
}

#undef GLOBALS_DEFINITION
#undef GLOBALS_UNINITIALIZED
#undef SND_GLOBALS

/********************************* EOF ***********************************/
