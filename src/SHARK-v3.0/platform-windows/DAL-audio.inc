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

#define SND_MAX_BUFFERS  4

/*************************************************************************
 * Component Variables
 *************************************************************************/

#define SND_GLOBALS           g_snd
#define GLOBALS_UNINITIALIZED (SND_GLOBALS == NULL)
#define GLOBALS_DEFINITION    SndGlobals *g_snd = NULL;

// SNDGlobals::   platform additions
#define DAL_SND_GLOBALS \
  HWAVEOUT          snd_hwo;                     \
  WAVEFORMATEX      snd_wfmt;                    \
  WAVEHDR           snd_whdr[SND_MAX_BUFFERS];   \
  uint16            snd_freeblocks;              \
                                                 \
  void             *snd_buffer[SND_MAX_BUFFERS]; \
  uint32            snd_buffer_size;             \
  uint32            snd_frames;                  \
                                                 \
  HANDLE            snd_request;                 \
  HANDLE            snd_thread;                  \
  DWORD             snd_thread_id;               \
  CRITICAL_SECTION  snd_wcs;

#define g_snd_hwo              (SND_GLOBALS) -> snd_hwo
#define g_snd_wfmt             (SND_GLOBALS) -> snd_wfmt
#define g_snd_whdr             (SND_GLOBALS) -> snd_whdr
#define g_snd_freeblocks       (SND_GLOBALS) -> snd_freeblocks

#define g_snd_buffer           (SND_GLOBALS) -> snd_buffer
#define g_snd_buffer_size      (SND_GLOBALS) -> snd_buffer_size
#define g_snd_frames           (SND_GLOBALS) -> snd_frames

#define g_snd_request          (SND_GLOBALS) -> snd_request
#define g_snd_thread           (SND_GLOBALS) -> snd_thread
#define g_snd_thread_id        (SND_GLOBALS) -> snd_thread_id
#define g_snd_wcs              (SND_GLOBALS) -> snd_wcs

/*************************************************************************
 * Platform Independent Implementation
 *************************************************************************/

#include "../common/SHARK-audio.inc"

/*************************************************************************
 * Implementation
 *************************************************************************/

static void  CALLBACK DAL_SndWaveOutProc(HWAVEOUT, UINT, DWORD, DWORD, DWORD);
static DWORD WINAPI   DAL_SndThread(void *);

static boolean
DAL_SndInitialize()
{
  boolean init_ok;

  // default return value
  init_ok = true;

  // clear memory used by memory globals
  memset(SND_GLOBALS, 0, sizeof(SndGlobals));

  // initialize counters/semaphores
  g_snd_freeblocks = SND_MAX_BUFFERS;
  InitializeCriticalSection(&g_snd_wcs);

  // initialize thread and memory blocks required
  g_snd_request = CreateEvent(NULL, TRUE, FALSE, TEXT("AudioDeathEvent"));
  if (g_snd_request == NULL) goto SND_INIT_ABORT;
  g_snd_thread  = (HANDLE)CreateThread(NULL, 0, DAL_SndThread,
                                       (void *)NULL, 0, &g_snd_thread_id);
  if (g_snd_thread == NULL) goto SND_INIT_ABORT;

  // mark the audio state as being available
  g_snd_stream_available = true;
 
SND_INIT_ABORT:;

  return init_ok;
}

static snd_stream
DAL_SndStreamCreate(uint32 frames)
{
  snd_stream result;
  int        i;
  MMRESULT   mmres;

  // default return value
  result = STREAM_INVALID;

  // allocate memory buffers for audio stream
  g_snd_frames      = frames;
  g_snd_buffer_size = g_snd_frames;
  if (g_snd_format == snd_int16)  g_snd_buffer_size <<= 1;
  if (g_snd_type   == snd_stereo) g_snd_buffer_size <<= 1; // increase buffer
  for (i=0; i<SND_MAX_BUFFERS; i++)
  {
    if (g_snd_buffer[i] != NULL) free(g_snd_buffer[i]);
    g_snd_buffer[i] = malloc(g_snd_buffer_size);
    if (g_snd_buffer[i] == NULL) goto SND_CREATE_ABORT;
    memset(g_snd_buffer[i], 0, g_snd_buffer_size);
  }

  // prepare streaming audio
  memset(&g_snd_wfmt, 0, sizeof(WAVEFORMATEX));
  g_snd_wfmt.wFormatTag      = WAVE_FORMAT_PCM;
  g_snd_wfmt.nChannels       = (g_snd_type == snd_mono) ? 1 : 2;
  switch (g_snd_rate)
  {
    case snd_8khz:  g_snd_wfmt.nSamplesPerSec = 8000;  break;
    case snd_11khz: g_snd_wfmt.nSamplesPerSec = 11025; break;
    case snd_16khz: g_snd_wfmt.nSamplesPerSec = 16000; break;
    case snd_22khz: g_snd_wfmt.nSamplesPerSec = 22050; break;
    case snd_44khz: g_snd_wfmt.nSamplesPerSec = 44100; break;
    default:                                           break;
  }
  switch (g_snd_format)
  {
    case snd_int8:
    case snd_uint8: g_snd_wfmt.wBitsPerSample = 8;     break;
    case snd_int16: g_snd_wfmt.wBitsPerSample = 16;    break;
    default:                                           break;
  }
  g_snd_wfmt.nBlockAlign     = g_snd_wfmt.nChannels *
                               (g_snd_wfmt.wBitsPerSample / 8);
  g_snd_wfmt.nAvgBytesPerSec = (uint32)g_snd_wfmt.nSamplesPerSec *
                               g_snd_wfmt.nBlockAlign;
  g_snd_wfmt.cbSize          = 0;

  // choose the device wave mapper device
  mmres = 
    waveOutOpen(&g_snd_hwo, WAVE_MAPPER, &g_snd_wfmt,
                (DWORD)DAL_SndWaveOutProc, (DWORD)NULL, CALLBACK_FUNCTION);
  if (mmres == MMSYSERR_NOERROR)
  {
    // flag the stream as valid - lets go!
    result = 0;
  }
  else 
    g_snd_hwo = NULL;

SND_CREATE_ABORT:;

  return result;
}

static void
DAL_SndStreamPause()
{
  // we simply dont do anything here
}

static void
DAL_SndStreamResume()
{
  // we simply dont do anything here
}

static void
DAL_SndStreamDelete(snd_stream stream_id)
{
  int16 i;

  // we need to ensure that the wave out object is valid
  if (g_snd_hwo != NULL)
  {
    waveOutReset(g_snd_hwo);
    for (i=0; i<SND_MAX_BUFFERS; i++)
      waveOutUnprepareHeader(g_snd_hwo, &g_snd_whdr[i], sizeof(WAVEHDR));
    waveOutClose(g_snd_hwo);
    g_snd_hwo = NULL;

    // terminate the audio playback
    EnterCriticalSection(&g_snd_wcs);
    g_snd_freeblocks = SND_MAX_BUFFERS;
    LeaveCriticalSection(&g_snd_wcs);
  }
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
DAL_SndTerminate()
{
  DWORD timeout;
  int   i;

  // terminate the audio thread
  if ((g_snd_request != NULL) && (g_snd_thread != NULL))
  {
    SetEvent(g_snd_request);
    timeout = WaitForSingleObject(g_snd_thread, 1000);
    if (timeout == WAIT_TIMEOUT)
      TerminateThread(g_snd_thread, 0);
  }

  // release the event/thread handles we used
  if (g_snd_request != NULL) CloseHandle(g_snd_request);
  if (g_snd_thread  != NULL) CloseHandle(g_snd_thread);
  g_snd_thread  = NULL;
  g_snd_request = NULL;

  // clean up memory usage within audio system
  for (i=0; i<SND_MAX_BUFFERS; i++)
  {
    if (g_snd_buffer[i] != NULL) free(g_snd_buffer[i]);
    g_snd_buffer[i] = NULL;
  }

  // clean up counters/semaphores
  DeleteCriticalSection(&g_snd_wcs);
}

// waveout callback
static void CALLBACK
DAL_SndWaveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance,
                   DWORD dwParam1, DWORD dwParam2)
{
  if (uMsg == WOM_DONE)
  {
    EnterCriticalSection(&g_snd_wcs);
    g_snd_freeblocks++;
    LeaveCriticalSection(&g_snd_wcs);
  }
}

// audio streaming thread implementation
static DWORD WINAPI
DAL_SndThread(void *data)
{
  uint32 index;
  uint16 framesize, cnt;
  uint8  zero;
  DWORD  timeout;

  // set the thread priority
  SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);

  // this thread stays active until its ready to shutdown the application
  index   = 0;
  timeout = WAIT_TIMEOUT;
  while (timeout == WAIT_TIMEOUT)
  {
    // do we have a wave-out object instance?
    if (g_snd_active)
    {
      // how many frames do we need to render
      framesize  = g_snd_frames;

      // obtain buffer from application callback
      if (g_snd_callback != NULL)
      {
        g_snd_callback(0, g_snd_user, g_snd_buffer[index], framesize);
      }
      else 
      {
        zero = 0;
        if (g_snd_format == snd_uint8) zero = 0x80; 
        memset(g_snd_buffer[index], zero, g_snd_buffer_size);
      }

      // prepare buffer for playback
      waveOutUnprepareHeader(g_snd_hwo, &g_snd_whdr[index], sizeof(WAVEHDR));
      memset(&g_snd_whdr[index], 0, sizeof(WAVEHDR));
      g_snd_whdr[index].lpData         = g_snd_buffer[index];
      g_snd_whdr[index].dwBufferLength = g_snd_buffer_size;
      waveOutPrepareHeader(g_snd_hwo, &g_snd_whdr[index], sizeof(WAVEHDR));

      // SPECIAL: windows API doesn't support int8 audio streams (remap uint8)
      if (g_snd_format == snd_int8)
      {
        uint8 *p = (uint8 *)g_snd_buffer[index];
        cnt = g_snd_buffer_size;
        while (cnt--) *p = *p++ ^ 0x80;
      }

      // play buffer
      waveOutWrite(g_snd_hwo, &g_snd_whdr[index], sizeof(WAVEHDR));
      EnterCriticalSection(&g_snd_wcs);
      g_snd_freeblocks--;
      LeaveCriticalSection(&g_snd_wcs);

      // wait for the next block to become free
      while ((g_snd_freeblocks == 0) && (timeout == WAIT_TIMEOUT))
        timeout = WaitForSingleObject(g_snd_request, 10);

      index++; if (index >= SND_MAX_BUFFERS) index = 0;
    }

    // are we still waiting for a trigger here?
    if (timeout == WAIT_TIMEOUT)
      timeout = WaitForSingleObject(g_snd_request, 10);
  }

  return 0;
}

#undef GLOBALS_DEFINITION
#undef GLOBALS_UNINITIALIZED
#undef SND_GLOBALS

#undef SND_MAX_BUFFERS

/********************************* EOF ***********************************/
