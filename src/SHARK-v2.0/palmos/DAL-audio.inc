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

typedef struct
{
  boolean                   snd_stream_available;
  boolean                   snd_stream_defined;

  snd_stream                snd_id;
  snd_rate                  snd_rate;
  snd_format                snd_format;
  snd_type                  snd_type;

  boolean                   snd_active; 
  SndStreamRef              snd_stream_ref;
  _SndStreamBufferCallback  snd_callback; 
  void                     *snd_user;

} SndGlobals;

#define g_snd_stream_available ((SndGlobals *)g -> snd) -> snd_stream_available
#define g_snd_stream_defined   ((SndGlobals *)g -> snd) -> snd_stream_defined

#define g_snd_id               ((SndGlobals *)g -> snd) -> snd_id
#define g_snd_rate             ((SndGlobals *)g -> snd) -> snd_rate
#define g_snd_format           ((SndGlobals *)g -> snd) -> snd_format
#define g_snd_type             ((SndGlobals *)g -> snd) -> snd_type

#define g_snd_active           ((SndGlobals *)g -> snd) -> snd_active
#define g_snd_stream_ref       ((SndGlobals *)g -> snd) -> snd_stream_ref
#define g_snd_callback         ((SndGlobals *)g -> snd) -> snd_callback
#define g_snd_user             ((SndGlobals *)g -> snd) -> snd_user

#define GLOBALS_UNINITIALIZED  (g -> snd == NULL)

/*************************************************************************
 *
 * Implementation
 *
 *************************************************************************/

static Err DAL_SndThread(void *, SndStreamRef, void *, UInt32);

// initialize the streaming audio manager
static boolean
_SndInitialize()
{
  boolean     init_ok;
  Err         err;
  UInt32      version;
  SndGlobals *g_lib;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _LOGCreateEntry(">> _SndInitialize");
#endif

  // allocate memory for audio globals
  g_lib = (SndGlobals *)malloc(sizeof(SndGlobals));
  if (g_lib != NULL) g -> snd = (void *)g_lib;

  // assume everything is ok
  init_ok  = true;
  init_ok &= (g_lib != NULL);

  // default values
  if (init_ok)
  {
    memset(g_lib, 0, sizeof(SndGlobals));

    g_snd_stream_defined   = false;
    g_snd_stream_available = false;

    // check for sound manager feature 
    err = FtrGet(sysFileCSoundMgr, sndFtrIDVersion, &version);
    g_snd_stream_available |= (err == errNone);

    // Sony NX/NZ/TG: shipped with 5.0 
    // --> support http://www.aibohack.com/clie/modclieaud.htm
    // Palm T|T:      shipped with 5.0
    // --> support http://kb.palmsource.com/ .. p_faqid=378
    g_snd_stream_available |= ((g -> ftr.comID == 'Palm') && 
                               (g -> ftr.devID == 'Frg1')); 

    // SndStream* are OS 5.1+ API (r9 table) must use PACE for 5.0
    g -> PACE.snd_api = g_snd_stream_available &&
                        ((g -> ftr.osVer & 0x0ff00000) == 0x05000000); 
  }

  return init_ok;
}

// does this hardware support streaming audio?
boolean
_SndStreamAvailable()
{
  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return false;

  return g_snd_stream_available;
}

// which audio rates are supported
uint32
_SndStreamRatesSupported()
{
  uint32 result;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return 0;

  // default return value
  result = 0;

  // what rates/configurations are available
  if (g_snd_stream_available)
  {
    result |= SND_STREAM_MONO  | SND_STREAM_STEREO;
    result |= SND_STREAM_8KHZ  | SND_STREAM_11KHZ | 
              SND_STREAM_16KHZ | SND_STREAM_22KHZ | SND_STREAM_44KHZ;
  }
  
  return result;
}

// start an audio stream
snd_stream 
_SndStreamCreate(snd_rate rate, snd_format format, snd_type type, 
                 _SndStreamBufferCallback func, void *user, uint16 frames)
{
  snd_stream     stream_id;
  UInt32         s_rate;
  SndSampleType  s_format;
  SndStreamWidth s_mode;
  Err            err;
  
  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return -1;

  // default return value
  stream_id = -1;

  // we can only do this is we support audio streaming
  if (g_snd_stream_available)
  {
    // if we have audio streams already running, stop them
    _SndStreamStop(0);
    _SndStreamDelete(0);
    g_snd_active = false;

    // copy over the stream properties (needed below)
    g_snd_rate   = rate;
    g_snd_format = format;
    g_snd_type   = type;    
  
    // prepare streaming audio
    s_mode = (g_snd_type == snd_mono) ? sndMono : sndStereo;
    switch (g_snd_rate)
    {
      case snd_8khz:  s_rate = 8000;  break;
      case snd_11khz: s_rate = 11025; break;
      case snd_16khz: s_rate = 16000; break;
      case snd_22khz: s_rate = 22050; break;
      case snd_44khz: s_rate = 44100; break;
      default:                        goto SND_CREATE_ABORT; 
    }
    switch (g_snd_format)
    {
      case snd_int8:  s_format = sndInt8;        break;
      case snd_uint8: s_format = sndUInt8;       break;
      case snd_int16: s_format = sndInt16Little; break;
      default:                        goto SND_CREATE_ABORT; 
    }

    // shutdown 
    if (g -> PACE.snd_api) 
      err = SndStreamCreate_PACE(&g_snd_stream_ref, sndOutput,
                                s_rate, s_format, s_mode, (void *)DAL_SndThread,
                                (void *)g -> snd, frames, true);
    else
          err = SndStreamCreate(&g_snd_stream_ref, sndOutput,
                                s_rate, s_format, s_mode, (void *)DAL_SndThread,
                                (void *)g -> snd, frames, true);
    if (err == errNone)
    {
      // flag the stream as valid - lets go!
      stream_id       = 0;
        
      g_snd_id        = stream_id;
      g_snd_callback  = func;
      g_snd_user      = user;    
      
      // set the unity volume 2:1 and start the stream
      if (g -> PACE.snd_api) 
      {
        SndStreamSetVolume_PACE(g_snd_stream_ref, 2048);
            SndStreamStart_PACE(g_snd_stream_ref);
      }
      else
      {
        SndStreamSetVolume(g_snd_stream_ref, 2048);
            SndStreamStart(g_snd_stream_ref);
      }
  
      g_snd_stream_defined = true;
    }
  }

SND_CREATE_ABORT:

  return stream_id;
}

// start an audio stream
void
_SndStreamStart(snd_stream stream_id)
{
  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // entry condition, do we have a valid audio stream?
  if (stream_id != 0) return;

  // flag as activated
  g_snd_active = g_snd_stream_defined;
}

// stop an audio stream
void
_SndStreamStop(snd_stream stream_id)
{
  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // entry condition, do we have a valid audio stream?
  if (stream_id != 0) return;

  // flag as activated
  g_snd_active = false;
}

// delete an audio stream
void
_SndStreamDelete(snd_stream stream_id)
{
  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // entry condition, do we have a valid audio stream?
  if (stream_id != 0) return;

  // we can only do this is we support audio streaming
  if ((g_snd_stream_available) && (g_snd_stream_defined))
  {
    // shutdown 
    if (g -> PACE.snd_api) 
    {
      SndStreamStop_PACE(g_snd_stream_ref);
      SndStreamDelete_PACE(g_snd_stream_ref);
    }
    else
    {
      SndStreamStop(g_snd_stream_ref);
      SndStreamDelete(g_snd_stream_ref);
    }
  }

  // no steam is defined right now
  g_snd_stream_defined = false;
}

// terminate the streaming audio engine
static void
_SndTerminate()
{
#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _LOGCreateEntry(">> _SndTerminate");
#endif

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // terminate the active audio stream
  _SndStreamStop(0);
  _SndStreamDelete(0);

  // free memory used by audio globals
  if (g -> snd != NULL) free(g -> snd);
  g -> snd = NULL;
}

static Err 
DAL_SndThread(void *user, SndStreamRef stream, void *buffer, UInt32 frameCount)
{
  uint32 size;
  SndGlobals *lib = (SndGlobals *)user;

  // call our callback engine
  if ((lib -> snd_active) && 
      (lib -> snd_callback != NULL))
  {
    lib -> snd_callback(lib -> snd_id, lib -> snd_user, buffer, frameCount);
  }
  else
  {
    size = frameCount;
    if (lib -> snd_type   == snd_stereo) size = size << 1;
    if (lib -> snd_format == snd_int16)  size = size << 1;

    memset(buffer, 0, size); // zero memory buffer
  }

  return errNone;
}

/*************************************************************************
 *
 * Implementation - device independent
 *
 *************************************************************************/

#include "../common/SHARK-audio.inc"

/********************************* EOF ***********************************/
