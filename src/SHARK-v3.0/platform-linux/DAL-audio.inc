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

#define SND_DEVICE            "default"

/*************************************************************************
 * Component Variables
 *************************************************************************/

#define SND_GLOBALS           g_snd
#define GLOBALS_UNINITIALIZED (SND_GLOBALS == NULL)
#define GLOBALS_DEFINITION    SndGlobals *g_snd = NULL;

// SNDGlobals::   platform additions
#define DAL_SND_GLOBALS \
  snd_pcm_t      *snd_fd;          \
                                   \
  void           *snd_buffer;      \
  uint32          snd_frames;      \
  uint32          snd_buffer_size; \
                                   \
  boolean         snd_running;     \
  pthread_t       snd_thread;      \
  uint32          snd_thread_delay;

#define g_snd_fd               (SND_GLOBALS) -> snd_fd

#define g_snd_buffer_index     (SND_GLOBALS) -> snd_buffer_index
#define g_snd_buffer           (SND_GLOBALS) -> snd_buffer 
#define g_snd_frames           (SND_GLOBALS) -> snd_frames
#define g_snd_buffer_size      (SND_GLOBALS) -> snd_buffer_size

#define g_snd_running          (SND_GLOBALS) -> snd_running
#define g_snd_thread           (SND_GLOBALS) -> snd_thread
#define g_snd_thread_delay     (SND_GLOBALS) -> snd_thread_delay

/*************************************************************************
 * Platform Independent Implementation
 *************************************************************************/

#include "../common/SHARK-audio.inc"

/*************************************************************************
 * Implementation
 *************************************************************************/

static void *DAL_SndThread(void *);

static boolean
DAL_SndInitialize()
{
  boolean init_ok;
  int     err;

  // default return value
  init_ok = true;

  // clear memory used by memory globals
  memset(SND_GLOBALS, 0, sizeof(SndGlobals));

  // open the audio interface
  err = snd_pcm_open(&g_snd_fd, SND_DEVICE, SND_PCM_STREAM_PLAYBACK, 0);

  // lets determine if there is streaming audio
  g_snd_stream_available = (err == 0);

  // if we have access to the audio interface lets create a thread
  if (g_snd_stream_available)
  {
    // create the snd thread
    err = pthread_create(&g_snd_thread, NULL, 
                   DAL_SndThread, (void *)SND_GLOBALS);
    g_snd_stream_available &= (err == 0);
  }

  return init_ok;
}

static snd_stream
DAL_SndStreamCreate(uint32 frames)
{
  snd_stream result;
  int        format, channels, rate;
  int        err;

  // default return value
  result = STREAM_INVALID;

  channels = (g_snd_type == snd_mono) ? 1 : 2;
  switch (g_snd_rate)
  {
    case snd_8khz:  rate = 8000;    break;
    case snd_11khz: rate = 11025;   break;
    case snd_16khz: rate = 16000;   break;
    case snd_22khz: rate = 22050;   break;
    case snd_44khz: rate = 44100;   break;
    default: goto SND_CREATE_ABORT; break;
  }
  switch (g_snd_format)
  {
    case snd_int8:  format = SND_PCM_FORMAT_S8;     break;
    case snd_uint8: format = SND_PCM_FORMAT_U8;     break;
    case snd_int16: format = SND_PCM_FORMAT_S16_LE; break;
    default: goto SND_CREATE_ABORT; break;
  }

  err = snd_pcm_set_params(g_snd_fd, format, SND_PCM_ACCESS_RW_INTERLEAVED,
                           channels, rate, 1, 50000);
  if (err < 0) goto SND_CREATE_ABORT;

  // we need to store the number of frames required
  g_snd_frames = frames;

  // this is how long we should wait per frames
  g_snd_thread_delay = (1000L / (rate / g_snd_frames));

  // allocate the memory we need for the audio buffer
  g_snd_buffer_size = frames;
  if (g_snd_type   == snd_stereo) g_snd_buffer_size = g_snd_buffer_size << 1; 
  if (g_snd_format == snd_int16)  g_snd_buffer_size = g_snd_buffer_size << 1; 
  g_snd_buffer = (void *)malloc(g_snd_buffer_size);
  if (g_snd_buffer == NULL) goto SND_CREATE_ABORT;

  // if we got this far, our audio interface has been configured right
  result = 0;

SND_CREATE_ABORT:;

  return result;
}

static void
DAL_SndStreamPause()
{
}

static void
DAL_SndStreamResume()
{
}

static void
DAL_SndStreamDelete(snd_stream stream_id)
{
  // if we have an audio buffer allocated, we must delete it
  if (g_snd_buffer != NULL) free(g_snd_buffer);
  g_snd_buffer = NULL;
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

static void *
DAL_SndThread(void *user_data)
{
  uint8              zero;
  snd_pcm_sframes_t  err;

  g_snd_running = true;
  while (g_snd_running)
  {
    // if the sound is active we write to audio interface as normal
    if (g_snd_active)
    {
      // obtain audio buffer from application callback
      if (g_snd_callback != NULL)
        g_snd_callback(0, g_snd_user, g_snd_buffer, g_snd_frames);
      else
      {
        zero = 0;
        if (g_snd_format == snd_uint8) zero = 0x80;
        memset(g_snd_buffer, zero, g_snd_buffer_size);
      }

      // play buffer
      err = snd_pcm_writei(g_snd_fd, g_snd_buffer, g_snd_frames); 
      if (err < 0) snd_pcm_recover(g_snd_fd, err, 0);
      snd_pcm_wait(g_snd_fd, g_snd_thread_delay);
    }

    // lets give the operating system a chance to do something
    pthread_yield();
  }

  return NULL;
}

static void 
DAL_SndTerminate()
{
  // do we have a thread available?
  if (g_snd_stream_available)
  {
    if (g_snd_running)
    {
      // lets bring the thread to a close
      g_snd_running = false;

      // cancel the thread
      pthread_cancel(g_snd_thread);
      pthread_join(g_snd_thread, NULL);
    }

    // free the buffer used for the audio
    if (g_snd_buffer != NULL)
      free(g_snd_buffer);
    g_snd_buffer = NULL;

    // close the audio interface device
    snd_pcm_close(g_snd_fd);
    g_snd_fd = NULL;
  }
}

#undef GLOBALS_DEFINITION
#undef GLOBALS_UNINITIALIZED
#undef SND_GLOBALS

#undef SND_DEVICE

/********************************* EOF ***********************************/
