/*************************************************************************
 *
 * Copyright 2001+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-sfx.c
 */

#include "../SHARK.h"
#include "../SHARK-sfx.h"

/*************************************************************************
 * Configuration
 *************************************************************************/

#define SFX_LIBRARY_ID  0x5F736678 // '_sfx'

#define MAX_PARALLEL    16

/*************************************************************************
 * Component Variables
 *************************************************************************/

typedef struct _music
{
  boolean  in_use;
  void    *data;

  boolean  active;
  uint32   length;
  uint32   offset;
} _music;

typedef struct _sample
{
  boolean  in_use;
  uint32   id;
  void    *data;
  uint32   length;

  struct
  {
    boolean  active;
    uint32   offset;
  } idx[MAX_PARALLEL];
} _sample;

/*************************************************************************
 * Global Variables
 *************************************************************************/

typedef struct _SfxGlobals
{
  snd_rate      rate;
  snd_format    format;
  snd_type      type;
  snd_stream    stream_id;

  uint16        mix_count;
  _sample     **mix_samples;

  _music        music;
  uint32        sample_count;
  _sample       samples[MAX_SAMPLES];

  struct
  {
    uint8       music;
    uint8       sfx;        // 0 = off .. 10 = max
  } volume;

  void         *extension;

  boolean     (*fnSfxInitialize)(snd_rate rate, snd_format format, snd_type type, uint16 frames);
  void        (*fnSfxTerminate)(void);

  void        (*fnTonePlay)(uint16 freq, uint32 duration, uint8 volume);
  boolean     (*fnStreamCallback)(void *user, void *buffer, uint32 frames);

} _SfxGlobals;

#define SFX_GLOBALS_ACCESS \
  _SfxGlobals *g_sfx = (_SfxGlobals *)_LibGetGlobals(SFX_LIBRARY_ID);
#define GLOBALS_UNINITIALIZED (g_sfx == NULL)

//------------------------------------------------------------------------
//                    --== GLOBALS ARE FORBIDDEN! ==--
//
// DAL may support the use of globals on some platforms, however, its not
// guaranteed that all the destination platforms allow the use of globals
// (variable or static data). use the GlobalsType / _LibGetGlobals() API.
//------------------------------------------------------------------------

/*************************************************************************
 * Platform Dependent Interface
 *************************************************************************/

static void    _SfxInitialize_RESIZE();
static boolean _SfxStreamCallback(snd_stream, void *, void *, uint32);
static boolean _SfxDriverConfiguration(snd_rate, snd_format, snd_type, uint16, uint16);
static boolean _SfxHandleEvent(event *);
static void    _SfxTerminate_RESIZE();

/*************************************************************************
 * Implementation
 *************************************************************************/

boolean
_SfxInitialize(snd_rate rate, snd_format format, snd_type type, uint16 frames, uint16 mix_count)
{
  boolean      init_ok;
  SFX_GLOBALS_ACCESS;

  // assume everything is ok
  init_ok = true;

  // GLOBALS_PROTECTION:
  if (!GLOBALS_UNINITIALIZED) return false;

  // register the library
  if (_LibRegister(SFX_LIBRARY_ID, "_SfxLibrary", (void *)_SfxHandleEvent))
  {
    // allocate globals memory for the library
    g_sfx = (_SfxGlobals *)_MemPtrNew(sizeof(_SfxGlobals), false);
    init_ok &= (g_sfx != NULL); if (!init_ok) goto SFX_INIT_ABORT;
    _LibSetGlobals(SFX_LIBRARY_ID, (void *)g_sfx);

    // allocate the pointer buffer for the current mixing samples
    g_sfx -> mix_samples  = 
      (_sample **)_MemPtrNew((mix_count-1) * sizeof(_sample *), false);
    init_ok = (g_sfx -> mix_samples != NULL);
    if (!init_ok) goto SFX_INIT_ABORT;

    // initialize the _Sfx driver
    init_ok = _SfxDriverConfiguration(rate, format, type, frames, mix_count);
    if (!init_ok) goto SFX_INIT_ABORT;

    // lets start the streaming audio
    _SndStreamStart(g_sfx -> stream_id);

    // initialize _Sfx internally
    _SfxInitialize_RESIZE();
  }
  else
    init_ok = false;

SFX_INIT_ABORT:

  return init_ok;
}

void
_SfxTerminate()
{
  SFX_GLOBALS_ACCESS;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // terminate _Sfx internally
  _SfxTerminate_RESIZE();

  // lets stop the streaming audio
  _SndStreamStop(g_sfx -> stream_id);
  _SndStreamDelete(g_sfx -> stream_id);
  g_sfx -> stream_id = STREAM_INVALID;

  // shutdown the sub-Sfx layer
  if (g_sfx -> fnSfxTerminate != NULL)
    g_sfx -> fnSfxTerminate();

  // free the pointer buffer for the current mixing samples
  if (g_sfx -> mix_samples != NULL)
    _MemPtrFree(g_sfx -> mix_samples);
  g_sfx -> mix_samples = NULL;

  // free the globals used
  _MemPtrFree(g_sfx); g_sfx = NULL;
  _LibSetGlobals(SFX_LIBRARY_ID, (void *)g_sfx);

  // shutdown the library
  _LibShutdown(SFX_LIBRARY_ID);
}

boolean
_SfxDefineMusic(sfx_type type, void *data, uint32 length)
{
  boolean result;
  SFX_GLOBALS_ACCESS;

  // default return value
  result = false;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;

  // entry condition (must call _SfxInitialize)
  if ((data == NULL) || (length == 0)) return result;

  // entry condition
  if (g_sfx -> music.in_use) return result;

  // save the globals pointer value
  g_sfx -> music.active = false;
  g_sfx -> music.data   = data;
  g_sfx -> music.length = length;
  g_sfx -> music.offset = 0;
  g_sfx -> music.in_use = true; // now we are using it

  // special internal handling
  if (g_sfx -> format == snd_int16)
    g_sfx -> music.length = g_sfx -> music.length >> 1;  // frame count

  return result;
}

void
_SfxRemoveMusic()
{
  SFX_GLOBALS_ACCESS;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // entry condition
  if (!g_sfx -> music.in_use) return;

  g_sfx -> music.in_use = false;
}

boolean
_SfxDefineSample(uint32 id, void *data, uint32 length)
{
  boolean result;
  int32   index, i;
  SFX_GLOBALS_ACCESS;

  // default return value
  result = false;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;

  // entry condition 
  if ((data == NULL) || (length == 0)) return result;

  // is there any room?
  if (g_sfx -> sample_count < MAX_SAMPLES)
  {
    // we need to find the first available spot
    index = 0;
    while (g_sfx -> samples[index].in_use) index++;

    // found free slot; lets initialize it
    g_sfx -> samples[index].id     = id;
    g_sfx -> samples[index].data   = data;
    g_sfx -> samples[index].length = length;
    for (i=0; i<MAX_PARALLEL; i++)
    {
      g_sfx -> samples[index].idx[i].active = false;
      g_sfx -> samples[index].idx[i].offset = 0;
    }
    g_sfx -> samples[index].in_use = true; // now we are using it

    // special internal handling
    if (g_sfx -> format == snd_int16)
      g_sfx -> samples[index].length = g_sfx -> samples[index].length >> 1;  

    g_sfx -> sample_count++;
    result = true;
  }

  return result;
}

void
_SfxRemoveSample(uint32 id)
{
  int32 index;
  SFX_GLOBALS_ACCESS;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // lets find out matching index and ensure its defined
  index = 0;
  while ((index < MAX_SAMPLES) && (g_sfx -> samples[index].id != id)) index++;
  if ((index >= MAX_SAMPLES) || (!g_sfx -> samples[index].in_use)) return;

  // save the globals pointer value
  g_sfx -> samples[index].in_use = false;
}

void
_SfxVolume(vol_command command, uint8 *music, uint8 *sfx)
{
  SFX_GLOBALS_ACCESS;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  switch (command)
  {
    case vol_set:
         if (music != NULL) g_sfx -> volume.music = *music;
         if (sfx != NULL)   g_sfx -> volume.sfx   = *sfx;
         break;

    default:
    case vol_get:
         if (music != NULL) *music = g_sfx -> volume.music;
         if (sfx != NULL)   *sfx   = g_sfx -> volume.sfx;
         break;
  }
}

void
_SfxToneCommand(uint16 freq, uint32 duration, uint8 volume)
{
  SFX_GLOBALS_ACCESS;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // ensure the volume is correct
  volume = MIN(100, volume);

  // call the appropriate audio function
  if (g_sfx -> fnTonePlay != NULL)
    g_sfx -> fnTonePlay(freq, duration, volume);
}

void
_SfxSampleCommand(uint32 id, sfx_command command, void *data)
{
  int32    index, i, j, idx;
  _sample *s;
  SFX_GLOBALS_ACCESS;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // lets find out matching sample id and ensure its defined
  index = 0;
  while ((index < MAX_SAMPLES) && (g_sfx -> samples[index].id != id)) index++;
  if ((index == MAX_SAMPLES) || (!g_sfx -> samples[index].in_use)) return;

  switch (command)
  {
    case sfx_play:

         // has the sample already started? (is so, next)
         i = 0;
         while ((i < (g_sfx -> mix_count - 1)) && 
                (g_sfx -> mix_samples[i] != &g_sfx -> samples[index])) i++;
         if (i == (g_sfx -> mix_count - 1))
         {
           // lets find the first available spot (if there is any)
           i = 0;
           while ((i < (g_sfx -> mix_count - 1)) && 
                  (g_sfx -> mix_samples[i] != NULL)) i++;
           if (i < (g_sfx -> mix_count - 1))
           {
             s = &g_sfx -> samples[index];

             // ensure we have a clean setup
             for (j=0; j<MAX_PARALLEL; j++)
             {
               s -> idx[j].offset = 0;
               s -> idx[j].active = false;
             }

             // start playing the sample
             idx = 0;
             s -> idx[idx].offset = 0;
             s -> idx[idx].active = true;

             // insert the sample into the system
             g_sfx -> mix_samples[i] = &g_sfx -> samples[index];
           }
         }
         else
         {
           s = &g_sfx -> samples[index];

           // the sample is playing; so lets start a parallel one
           idx = -1;
           for (j=0; j<MAX_PARALLEL; j++)
           { 
             if (!s -> idx[j].active) { idx = j; break; }
             else if (s -> idx[j].offset == 0) break; // avoid many
           }

           // start playing the sample
           if (idx != -1)
           {
             s -> idx[idx].offset = 0;
             s -> idx[idx].active = true;
           }
         }
         break;

    case sfx_stop:

         // the sample must be playing - so, we must find it
         i = 0;
         while ((i < (g_sfx -> mix_count - 1)) && 
                (g_sfx -> mix_samples[i] != &g_sfx -> samples[index])) i++;
         if (i < (g_sfx -> mix_count - 1))
         {
           // remove the sample from the system
           g_sfx -> mix_samples[i] = NULL;
         
           // stop the sample
           s = &g_sfx -> samples[index];
           for (j=0; j<MAX_PARALLEL; j++)
           {
             s -> idx[j].offset = 0;
             s -> idx[j].active = false;
           }
         }
         break;

    default:
         break;
  }
}

void
_SfxMusicCommand(sfx_command command, void *data)
{
  SFX_GLOBALS_ACCESS;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // entry condition 
  if (!g_sfx -> music.in_use) return;

  switch (command)
  {
    case sfx_play:
         g_sfx -> music.offset = 0;
         g_sfx -> music.active = true;
         break;

    case sfx_seek:
         g_sfx -> music.offset = (uint32)((long)data);

         // special internal handling
         if (g_sfx -> format == snd_int16)
           g_sfx -> music.offset = g_sfx -> music.offset >> 1;  // frame

         break;

    case sfx_pause:
         g_sfx -> music.active = !g_sfx -> music.active;
         break;

    case sfx_stop:
         g_sfx -> music.active = false;
         g_sfx -> music.offset = 0;
         break;

    default:
         break;
  }
}

static void
_SfxInitialize_RESIZE()
{
}

static void
_SfxTerminate_RESIZE()
{
}

static boolean
_SfxStreamCallback(snd_stream stream_id, void *user, void *buffer, uint32 frames)
{
  boolean  result;
  int32    i, j;
  boolean  active;
  _sample *s;
  _SfxGlobals *g_sfx = (_SfxGlobals *)user;

  // default return value
  result = false;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;

  // call the appropriate audio callback
  if (g_sfx -> fnStreamCallback != NULL)
    result = g_sfx -> fnStreamCallback((void *)g_sfx, buffer, frames);

  // did any samples finish - we should remove them from system
  for (i=0; i < (g_sfx -> mix_count-1); i++)
  {
    s = g_sfx -> mix_samples[i];
    if (s != NULL)
    {
      // if at least one is active; keep it in there
      active = false;
      for (j=0; j<MAX_PARALLEL; j++)
        active |= (s -> idx[j].active);
      if (!active) g_sfx -> mix_samples[i] = NULL;
    }
  }
 
  return result;
}

#include "SHARK-sfx-pcmmix.inc"

static boolean
_SfxDriverConfiguration(snd_rate rate, snd_format format, snd_type type, uint16 frames, uint16 mix_count)
{
  boolean     result;
  SFX_GLOBALS_ACCESS;

  // default return value
  result = false;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;

  // our first check is basic - do we have streaming audio?
  result = _SndStreamAvailable();
  if (!result) goto SFX_CONFIG_QUIT;

  // we need to know how many samples we'll be mixing at once
  g_sfx -> rate         = rate;
  g_sfx -> format       = format;
  g_sfx -> type         = type;
  g_sfx -> mix_count    = mix_count;
  g_sfx -> sample_count = 0;

  // configure our sub-sfx layer
  g_sfx -> fnSfxInitialize  = (void *)_Sfx_PCM_Initialize;
  g_sfx -> fnSfxTerminate   = (void *)_Sfx_PCM_Terminate;
  g_sfx -> fnStreamCallback = (void *)_Sfx_PCM_StreamCallback;
  g_sfx -> fnTonePlay       = (void *)_Sfx_PCM_TonePlay;

  // initialize the sub-sfx layer
  result = false;
  if (g_sfx -> fnSfxInitialize != NULL)
    result = g_sfx -> fnSfxInitialize(rate, format, type, frames);
  if (!result) goto SFX_CONFIG_QUIT;

  // defaults
  g_sfx -> volume.music = 6;
  g_sfx -> volume.sfx   = 6;  // just a little over half way

  // lets create our audio stream
  g_sfx -> stream_id = _SndStreamCreate(rate, format, type, 
                                        _SfxStreamCallback, g_sfx, frames);
  result = (g_sfx -> stream_id != STREAM_INVALID);
  if (!result) goto SFX_CONFIG_QUIT;

  result = true;
  
SFX_CONFIG_QUIT:

  return result;
}

static boolean
_SfxHandleEvent(event *e)
{
  boolean result;
  SFX_GLOBALS_ACCESS;

  // default return value
  result = false;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return result;

  switch (e -> eType)
  {
    case _keyEvent:
         switch (e -> data.key.chr)
         {
           case _key_lcd_resize:
                _SfxTerminate_RESIZE();
                _SfxInitialize_RESIZE();

                result = true;
                break;

           default:
                break;
         }
         break;

    default:
         break;
  }

  return result;
}

#undef GLOBALS_UNINITIALIZED
#undef SFX_GLOBALS_ACCESS

#undef MAX_PARALLEL 

#undef SFX_LIBRARY_ID

/********************************* EOF ***********************************/
