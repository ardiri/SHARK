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

#define SND_MAX_BUFFERS  3

/*************************************************************************
 *
 * Global Variables
 *
 *************************************************************************/

static HANDLE                    g_snd_request; 
static HANDLE                    g_snd_thread;
static DWORD                     g_snd_thread_id;
static CRITICAL_SECTION          g_snd_wcs;
static uint16                    g_snd_freeblocks;
static HWAVEOUT                  g_snd_hwo;
static WAVEFORMATEX              g_snd_wfmt;
static WAVEHDR                   g_snd_whdr[SND_MAX_BUFFERS]; 

static snd_stream                g_snd_id;
static snd_rate                  g_snd_rate;
static snd_format                g_snd_format;
static snd_type                  g_snd_type;

static void                     *g_snd_buffer[SND_MAX_BUFFERS];
static uint16                    g_snd_size;
static uint16                    g_snd_frames;

static boolean                   g_snd_active; 
static _SndStreamBufferCallback  g_snd_callback; 
static void                     *g_snd_user;

static boolean                   g_snd_stream_defined;

/*************************************************************************
 *
 * Implementation
 *
 *************************************************************************/

static void CALLBACK _SndWaveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, 
                                     DWORD dwParam1, DWORD dwParam2);
static DWORD WINAPI  _SndThread(void *data);

// initialize the streaming audio manager
static boolean
_SndInitialize()
{
  int16   i;
  boolean init_ok;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _LOGCreateEntry(">> _SndInitialize");
#endif

  // assume everything is ok
  init_ok = true;

  // initialize thread and memory blocks required
  g_snd_request = CreateEvent(NULL, TRUE, FALSE, TEXT("AudioDeathEvent"));
  if (g_snd_request == NULL) goto SND_INIT_ABORT;
  g_snd_thread  = (HANDLE)CreateThread(NULL, 0, _SndThread, 
                                       (void *)NULL, 0, &g_snd_thread_id);
  if (g_snd_thread == NULL) goto SND_INIT_ABORT;

SND_INIT_ABORT:

  // default values
  g_snd_stream_defined = false;

  // initialize
  for (i=0; i<SND_MAX_BUFFERS; i++)
    g_snd_buffer[i] = NULL;

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

  // default return value
  result = 0;

  result |= SND_STREAM_MONO  | SND_STREAM_STEREO;
  result |= SND_STREAM_8KHZ  | SND_STREAM_11KHZ |
            SND_STREAM_16KHZ | SND_STREAM_22KHZ | SND_STREAM_44KHZ;

  return result;
}

// start an audio stream
snd_stream 
_SndStreamCreate(snd_rate rate, snd_format format, snd_type type, 
                 _SndStreamBufferCallback func, void *user, uint16 frames)
{
  snd_stream stream_id;
  boolean    init_ok;
  UINT       i;
  MMRESULT   mmres;
  
  // default return value
  stream_id = -1;

  // if we have audio streams already running, stop them
  _SndStreamStop(0);
  _SndStreamDelete(0);
  g_snd_active = false;

  // copy over the stream properties (needed below)
  g_snd_rate   = rate;
  g_snd_format = format;
  g_snd_type   = type;    

  // allocate memory buffers for audio stream
  g_snd_frames = frames;
  g_snd_size   = g_snd_frames;
  if (g_snd_format == snd_int16)  g_snd_size <<= 1;
  if (g_snd_type   == snd_stereo) g_snd_size <<= 1; // increase buffer
  for (i=0; i<SND_MAX_BUFFERS; i++)
  {
    if (g_snd_buffer[i] != NULL) free(g_snd_buffer[i]);
    g_snd_buffer[i] = malloc(g_snd_size); 
    init_ok = (g_snd_buffer[i] != NULL); if (!init_ok) goto SND_CREATE_ABORT;
    memset(g_snd_buffer[i], 0, g_snd_size);
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
  for (i=0; i<waveOutGetNumDevs(); i++)
  {
    mmres = waveOutOpen(&g_snd_hwo, i, &g_snd_wfmt, 
                        (DWORD)_SndWaveOutProc, (DWORD)NULL, CALLBACK_FUNCTION);
    if (mmres == MMSYSERR_NOERROR) 
    {    
      // flag the stream as valid - lets go!
      stream_id       = 0;

      g_snd_id        = stream_id;
      g_snd_callback  = func;
      g_snd_user      = user;

      // we now have a stream defined
      g_snd_stream_defined = true;
      break;
    }
    else
      g_snd_hwo = NULL;
  }

SND_CREATE_ABORT:

  return stream_id;
}

// start an audio stream
void
_SndStreamStart(snd_stream stream_id)
{
  // entry condition, do we have a valid audio stream?
  if (stream_id != 0) return;

  // flag as activated
  g_snd_active = g_snd_stream_defined;
}

// stop an audio stream
void
_SndStreamStop(snd_stream stream_id)
{
  // entry condition, do we have a valid audio stream?
  if (stream_id != 0) return;

  // flag as activated
  g_snd_active = false;
}

// delete an audio stream
void
_SndStreamDelete(snd_stream stream_id)
{
  int16 i;
  
  // entry condition, do we have a valid audio stream?
  if (stream_id != 0) return;

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

  // no steam is defined right now
  g_snd_stream_defined = false;
}

// terminate the streaming audio engine
static void
_SndTerminate()
{
  DWORD timeout;
  int   i;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _LOGCreateEntry(">> _SndTerminate");
#endif

  // terminate the active audio stream
  _SndStreamStop(0);
  _SndStreamDelete(0);

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
}

// waveout callback
static void CALLBACK 
_SndWaveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, 
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
_SndThread(void *data)
{
  uint32 index;
  uint16 framesize, bufferused;
  DWORD  timeout;

  // initialize counters/semaphores
  g_snd_freeblocks = SND_MAX_BUFFERS;
  InitializeCriticalSection(&g_snd_wcs);

  // set the thread priority
  SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

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
      bufferused = framesize;
      if (g_snd_type   == snd_stereo) bufferused = bufferused << 1;
      if (g_snd_format == snd_int16)  bufferused = bufferused << 1;

      // obtain buffer from application callback
      if (g_snd_callback != NULL) 
        g_snd_callback(g_snd_id, g_snd_user, g_snd_buffer[index], framesize);
      else memset(g_snd_buffer[index], 0, bufferused);
 
      // prepare buffer for playback
      waveOutUnprepareHeader(g_snd_hwo, &g_snd_whdr[index], sizeof(WAVEHDR));  
      memset(&g_snd_whdr[index], 0, sizeof(WAVEHDR));
      g_snd_whdr[index].lpData         = g_snd_buffer[index];
      g_snd_whdr[index].dwBufferLength = bufferused;
      waveOutPrepareHeader(g_snd_hwo, &g_snd_whdr[index], sizeof(WAVEHDR));  

      // SPECIAL: msmobile API doesn't support int8 audio streams (remap uint8)
      if (g_snd_format == snd_int8) 
      {
        uint8 *p = (uint8 *)g_snd_buffer[index];
        while (bufferused--) { *p ^= 0x80; p++; }
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

  // clean up counters/semaphores
  DeleteCriticalSection(&g_snd_wcs);

  return 0;
}

/*************************************************************************
 *
 * Implementation - device independent
 *
 *************************************************************************/

#include "../common/SHARK-audio.inc"

/********************************* EOF ***********************************/
