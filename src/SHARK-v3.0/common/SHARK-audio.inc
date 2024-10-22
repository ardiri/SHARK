/*************************************************************************
 *
 * Copyright 2001+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-audio.inc
 */

/*************************************************************************
 * Configuration
 *************************************************************************/

// none

/*************************************************************************
 * Component Variables
 *************************************************************************/

typedef struct SndGlobals
{
  void *struct_prefix;

  boolean                   snd_stream_available;
  boolean                   snd_stream_defined;

  snd_rate                  snd_rate;
  snd_format                snd_format;
  snd_type                  snd_type;

  _SndStreamBufferCallback  snd_callback;
  void                     *snd_user;
  boolean                   snd_active;

#if defined(DAL_SND_GLOBALS)
  DAL_SND_GLOBALS
#endif

} SndGlobals;

GLOBALS_DEFINITION;

#define g_snd_stream_available (SND_GLOBALS) -> snd_stream_available
#define g_snd_stream_defined   (SND_GLOBALS) -> snd_stream_defined

#define g_snd_rate             (SND_GLOBALS) -> snd_rate
#define g_snd_format           (SND_GLOBALS) -> snd_format
#define g_snd_type             (SND_GLOBALS) -> snd_type

#define g_snd_callback         (SND_GLOBALS) -> snd_callback
#define g_snd_user             (SND_GLOBALS) -> snd_user
#define g_snd_active           (SND_GLOBALS) -> snd_active

/*************************************************************************
 * Platform Dependent Interface
 *************************************************************************/

static boolean    DAL_SndInitialize();
static snd_stream DAL_SndStreamCreate(uint32);
static void       DAL_SndStreamPause();
static void       DAL_SndStreamResume();
static void       DAL_SndStreamDelete(snd_stream);
static uint32     DAL_SndStreamRatesSupported();
static void       DAL_SndTerminate();

/*************************************************************************
 * Implementation
 *************************************************************************/

#if (LOG_DISABLE == 0)
// audio: constant representation possible values
const uint16 g_snd_rate_consts[]    = { 8000, 11025, 16000, 22050, 44100 };
const uint8  g_snd_channel_consts[] = { 1, 2 };
const uint8  g_snd_format_consts[]  = { 8, 8, 16 };
#endif

static boolean
_SndInitialize()
{
  boolean init_ok;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _SHARKLogCreateEntry(">> _SndInitialize");
#endif

  // allocate memory for globals
  SND_GLOBALS = malloc(sizeof(SndGlobals));
  if (GLOBALS_UNINITIALIZED) return false;

  // assume everything is ok
  init_ok =  true;
  init_ok &= DAL_SndInitialize();

  // did we initialize ok?
  if (init_ok)
  {
    // no stream has been defined yet
    g_snd_stream_defined = false;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  {
    char   str_log[256];
    uint32 rates;

    rates = _SndStreamRatesSupported();
    sprintf(str_log, "\navailable       :: %s", (g_snd_stream_available ? "yes" : "no"));
    _SHARKLogCreateEntry(str_log);
    sprintf(str_log, "rates supported :: ");

    if (rates & SND_STREAM_8KHZ)   strcat(str_log, "8Khz "); 
    if (rates & SND_STREAM_11KHZ)  strcat(str_log, "11Khz "); 
    if (rates & SND_STREAM_16KHZ)  strcat(str_log, "16Khz "); 
    if (rates & SND_STREAM_22KHZ)  strcat(str_log, "22Khz "); 
    if (rates & SND_STREAM_44KHZ)  strcat(str_log, "44Khz "); 
    if (rates & SND_STREAM_MONO)   strcat(str_log, "[mono] "); 
    if (rates & SND_STREAM_STEREO) strcat(str_log, "[stereo] "); 
    if (rates == 0)                strcat(str_log, "- none -");
    strcat(str_log, "\n");

    _SHARKLogCreateEntry(str_log);
  }
#endif
  }

  return init_ok;
}

boolean
_SndStreamAvailable()
{
  boolean result;

  // default return value
  result = false;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;
 
  // status on audio streaming availability
  result = g_snd_stream_available;

  return result;
}

snd_stream
_SndStreamCreate(snd_rate rate, snd_format format, snd_type type,
                 _SndStreamBufferCallback func, void *user, uint16 frames)
{
  snd_stream result;

  // default return value
  result = STREAM_INVALID;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;

  // do we support audio streaming?
  if (g_snd_stream_available)
  {
    // stop any currently running streams
    _SndStreamStop(0);
    _SndStreamDelete(0);
    g_snd_active = false;

    // store the the stream properties 
    g_snd_rate     = rate;
    g_snd_format   = format;
    g_snd_type     = type;
    g_snd_callback = func;
    g_snd_user     = user;

    // create the audio stream and start it
    result = DAL_SndStreamCreate(frames);

    // have we now defined the stream?
    if (result != STREAM_INVALID)
    {
      g_snd_stream_defined = true;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  {
    char str_log[256];

    sprintf(str_log, "\n: _SndStreamCreate()");
    _SHARKLogCreateEntry(str_log);
    sprintf(str_log, "sample rate     :: %d Hz", g_snd_rate_consts[g_snd_rate]);
    _SHARKLogCreateEntry(str_log);
    sprintf(str_log, "channels        :: %d", g_snd_channel_consts[g_snd_type]);
    _SHARKLogCreateEntry(str_log);
    sprintf(str_log, "audio depth     :: %d\n", g_snd_format_consts[g_snd_format]);
    _SHARKLogCreateEntry(str_log);
  }
#endif

    }
  }

  return result;
}

void
_SndStreamDelete(snd_stream stream_id)
{
  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // entry condition
  if (stream_id == STREAM_INVALID) return;

  // this only makes sense if stream is available and defined
  if ((g_snd_stream_available) && (g_snd_stream_defined))
  {
    DAL_SndStreamDelete(stream_id);

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  {
    char str_log[256];

    sprintf(str_log, "\n: _SndStreamDelete()\n");
    _SHARKLogCreateEntry(str_log);
  }
#endif
  }

  // no steam is defined right now
  g_snd_stream_defined = false;
}

uint32
_SndStreamRatesSupported()
{
  uint32 result;

  // default return value
  result = 0;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;

  // do we support audio streaming?
  if (g_snd_stream_available)
  {
    // query underlying hardware
    result = DAL_SndStreamRatesSupported();
  }

  return result;
}

void
_SndStreamStart(snd_stream stream_id)
{
  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // entry condition
  if (stream_id == STREAM_INVALID) return;

  // do we support audio streaming?
  if (g_snd_stream_available)
  {
    // flag as active (only if stream has been defined)
    g_snd_active = g_snd_stream_defined;

    // we may have to do something locally for audio
    DAL_SndStreamResume();
  }
}

void
_SndStreamStop(snd_stream stream_id)
{
  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // entry condition
  if (stream_id == STREAM_INVALID) return;

  // do we support audio streaming?
  if (g_snd_stream_available)
  {
    // flag as in-active
    g_snd_active = false;

    // we may have to do something locally for audio
    DAL_SndStreamPause();
  }
}

static void
_SndTerminate()
{
#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _SHARKLogCreateEntry(">> _SndTerminate");
#endif

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // do we support audio streaming?
  if (g_snd_stream_available)
  {
    // stop and terminate the active audio stream
    _SndStreamStop(0);
    _SndStreamDelete(0);
  }

  DAL_SndTerminate();

  // free memory for globals
  free(SND_GLOBALS);
  SND_GLOBALS = NULL;
}

/********************************* EOF ***********************************/
