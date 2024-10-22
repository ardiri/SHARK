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
  snd_pcm_t      *snd_h;           \
  int             snd_card;        \
  int             snd_dev;         \
                                   \
  void           *snd_buffer;      \
  uint32          snd_frames;      \
  uint32          snd_buffer_size; \
                                   \
  boolean         snd_running;     \
  pthread_t       snd_thread; 

#define g_snd_h                (SND_GLOBALS) -> snd_h
#define g_snd_card             (SND_GLOBALS) -> snd_card
#define g_snd_dev              (SND_GLOBALS) -> snd_dev

#define g_snd_buffer_index     (SND_GLOBALS) -> snd_buffer_index
#define g_snd_buffer           (SND_GLOBALS) -> snd_buffer 
#define g_snd_frames           (SND_GLOBALS) -> snd_frames
#define g_snd_buffer_size      (SND_GLOBALS) -> snd_buffer_size

#define g_snd_running          (SND_GLOBALS) -> snd_running
#define g_snd_thread           (SND_GLOBALS) -> snd_thread

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

  err = snd_pcm_open_preferred(&g_snd_h, 
                               &g_snd_card, &g_snd_dev, SND_PCM_OPEN_PLAYBACK);

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
  snd_stream               result;
  snd_pcm_channel_info_t   pi;
  snd_pcm_channel_params_t pp;
  snd_pcm_channel_setup_t  ps;
  int                      err;

  // default return value
  result = STREAM_INVALID;

  // we should disable mmap (as we are not going to use it)
  err = snd_pcm_plugin_set_disable(g_snd_h, PLUGIN_DISABLE_MMAP);
  if (err < 0) goto SND_CREATE_ABORT;

  // query the capabilities of the channel
  memset(&pi, 0, sizeof(snd_pcm_channel_info_t));
  pi.channel = SND_PCM_CHANNEL_PLAYBACK;
  err = snd_pcm_plugin_info(g_snd_h, &pi);
  if (err < 0) goto SND_CREATE_ABORT;

  // configure the channel we want to use
  memset(&pp, 0, sizeof(snd_pcm_channel_params_t));
  pp.mode                = SND_PCM_MODE_BLOCK;
  pp.channel             = SND_PCM_CHANNEL_PLAYBACK;
  pp.start_mode          = SND_PCM_START_DATA;
  pp.stop_mode           = SND_PCM_STOP_STOP;
  pp.buf.block.frag_size = pi.max_fragment_size;
  pp.buf.block.frags_max = 1;
  pp.buf.block.frags_min = 1;
  pp.format.interleave   = 1;
  pp.format.voices       = (g_snd_type == snd_mono) ? 1 : 2;
  switch (g_snd_rate)
  {
    case snd_8khz:  pp.format.rate = 8000;    break;
    case snd_11khz: pp.format.rate = 11025;   break;
    case snd_16khz: pp.format.rate = 16000;   break;
    case snd_22khz: pp.format.rate = 22050;   break;
    case snd_44khz: pp.format.rate = 44100;   break;
    default:        goto SND_CREATE_ABORT;    break;
  }
  switch (g_snd_format)
  {
    case snd_int8:  pp.format.format = SND_PCM_SFMT_S8;     break;
    case snd_uint8: pp.format.format = SND_PCM_SFMT_U8;     break;
    case snd_int16: pp.format.format = SND_PCM_SFMT_S16_LE; break;
    default:        goto SND_CREATE_ABORT;                  break;
  }

  err = snd_pcm_plugin_params(g_snd_h, &pp);
  if (err < 0) goto SND_CREATE_ABORT;

  memset(&ps, 0, sizeof(snd_pcm_channel_setup_t));
  ps.channel = SND_PCM_CHANNEL_PLAYBACK;
  err = snd_pcm_plugin_setup(g_snd_h, &ps);
  if (err < 0) goto SND_CREATE_ABORT;

  // lets identify how big our "desired" buffer should be
  g_snd_buffer_size = frames;
  if (g_snd_type   == snd_stereo) g_snd_buffer_size = g_snd_buffer_size << 1;
  if (g_snd_format == snd_int16)  g_snd_buffer_size = g_snd_buffer_size << 1;

  // we need to ensure our buffersize is a multiple of frag_size
  g_snd_buffer_size += (ps.buf.block.frag_size - 
                        (g_snd_buffer_size % ps.buf.block.frag_size));

  // re-calculate how many frames will be required
  g_snd_frames = g_snd_buffer_size;
  if (g_snd_type   == snd_stereo) g_snd_frames = g_snd_frames >> 1;
  if (g_snd_format == snd_int16)  g_snd_frames = g_snd_frames >> 1;

  // allocate the memory we need for the audio buffer
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
  snd_pcm_channel_status_t  status;
  int                       fd;
  fd_set                    fds;
  uint8                     zero;

  // obtain the file descriptor from the channel
  fd = snd_pcm_file_descriptor(g_snd_h, SND_PCM_CHANNEL_PLAYBACK);

  // prepare the file descriptors for writing
  FD_ZERO(&fds);

  g_snd_running = true;
  while (g_snd_running)
  {
    // if the sound is active we write to audio interface as normal
    if (g_snd_active)
    {
      // we must wait until the channel is ready for writing
      fd = snd_pcm_file_descriptor(g_snd_h, SND_PCM_CHANNEL_PLAYBACK);
      FD_SET(fd, &fds);
      select(fd+1, NULL, &fds, NULL, NULL);

      // we ready for writing
      memset(&status, 0, sizeof(status));
      snd_pcm_plugin_status(g_snd_h, &status);

      // do we need to prepare the channel for playback?
      if (status.status == SND_PCM_STATUS_READY    ||
          status.status == SND_PCM_STATUS_UNDERRUN ||
          status.status == SND_PCM_STATUS_OVERRUN)
      {
        snd_pcm_plugin_prepare(g_snd_h, SND_PCM_CHANNEL_PLAYBACK);
      }
      else
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
        snd_pcm_plugin_write(g_snd_h, g_snd_buffer, g_snd_buffer_size); 
      }
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

    // we must flush the channel
    snd_pcm_plugin_flush(g_snd_h, SND_PCM_CHANNEL_PLAYBACK);

    // free the buffer used for the audio
    if (g_snd_buffer != NULL)
      free(g_snd_buffer);
    g_snd_buffer = NULL;

    // close the audio interface device
    snd_pcm_close(g_snd_h);
    g_snd_h = NULL;
  }
}

#undef GLOBALS_DEFINITION
#undef GLOBALS_UNINITIALIZED
#undef SND_GLOBALS

#undef SND_DEVICE

/********************************* EOF ***********************************/
