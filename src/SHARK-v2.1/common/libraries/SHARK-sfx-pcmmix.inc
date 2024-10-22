/*e************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-sfx-pcmmix.c
 */

/*************************************************************************
 *
 * Configuration
 *
 *************************************************************************/

// none

/*************************************************************************
 *
 * Global Variables
 *
 *************************************************************************/

typedef struct _Sfx_PCM_Globals
{
  struct
  {
    uint16  min;
    uint16  max;
    uint16  cycle;
    uint32  index;
    uint32  count;
    boolean state;
  } tonegen;

} _Sfx_PCM_Globals;

#define SFX_GLOBALS_PCM_ACCESS  \
  _SfxGlobals       *g_sfx;     \
  _Sfx_PCM_Globals  *g_pcm_sfx; \
  g_sfx     = (_SfxGlobals *)_LibGetGlobals(SFX_LIBRARY_ID); \
  g_pcm_sfx = (_Sfx_PCM_Globals *)g_sfx->extension;

//------------------------------------------------------------------------
//                    --== GLOBALS ARE FORBIDDEN! ==--
//
// DAL may support the use of globals on some platforms, however, its not
// guaranteed that all the destination platforms allow the use of globals
// (variable or static data). use the GlobalsType / _LibGetGlobals() API.
//------------------------------------------------------------------------

/*************************************************************************
 *
 * Generic _Sfx routines
 *
 *************************************************************************/

static boolean
_Sfx_PCM_Initialize(snd_rate rate, snd_format format, snd_type type, uint16 frames)
{
  boolean           init_ok;
  _Sfx_PCM_Globals *g_pcm_sfx;
  SFX_GLOBALS_ACCESS;

  // assume everything is ok
  init_ok = true;

  // allocate globals memory for the library
  g_pcm_sfx = (_Sfx_PCM_Globals *)_MemPtrNew(sizeof(_Sfx_PCM_Globals), false);
  init_ok &= (g_pcm_sfx != NULL); if (!init_ok) goto SFX_PCM_INIT_ABORT;
  g_sfx->extension = (void *)g_pcm_sfx;

  // initialize
  g_pcm_sfx->tonegen.min   = 0;
  g_pcm_sfx->tonegen.max   = 0;
  g_pcm_sfx->tonegen.cycle = 1;
  g_pcm_sfx->tonegen.index = 0;
  g_pcm_sfx->tonegen.count = 0;
  g_pcm_sfx->tonegen.state = false;

SFX_PCM_INIT_ABORT:

  return init_ok;
}

static void
_Sfx_PCM_Terminate()
{
  SFX_GLOBALS_PCM_ACCESS;

  // clean up
  if (g_pcm_sfx != NULL) _MemPtrFree(g_pcm_sfx);
  g_sfx->extension = NULL;
}

static void
_Sfx_PCM_TonePlay(uint16 freq, uint32 duration, uint8 volume)
{
  uint16 min_freq, cyc_freq;
  SFX_GLOBALS_PCM_ACCESS;

  cyc_freq = 0;
  min_freq = 0;
  switch (g_sfx->rate)
  {
    case snd_8khz:  cyc_freq =  8000; break;
    case snd_11khz: cyc_freq = 11025; break;
    case snd_16khz: cyc_freq = 16000; break;
    case snd_22khz: cyc_freq = 22050; break;
    case snd_44khz: cyc_freq = 44100; break;
    default: break;
  }
  min_freq = cyc_freq >> 1;

  // can we actually do this?
  if ((freq > 0) && (freq <= min_freq))
  {
    // initialize variables for freq+duration configuration
    g_pcm_sfx->tonegen.count = 0;  // stop existing audio

    g_pcm_sfx->tonegen.index = 0;
    g_pcm_sfx->tonegen.count = ((cyc_freq * duration) / 1000);
    g_pcm_sfx->tonegen.cycle = (int16)(min_freq / freq);
/**
 ** LINEAR: = doesn't give good volume scale for differences
 **
    g_pcm_sfx->tonegen.max   = 128 - (127 * volume) / 100;
    g_pcm_sfx->tonegen.min   = 128 + (127 * volume) / 100;
 **/
    switch (g_sfx->format)
    {
      case snd_int8:
      case snd_uint8:
           g_pcm_sfx->tonegen.max = 128 + (uint8)((127L*volume*volume)/10000L);
           g_pcm_sfx->tonegen.min = 128 - (uint8)((127L*volume*volume)/10000L);
           break;

      case snd_int16:
           g_pcm_sfx->tonegen.max = (int16)((32767L*volume*volume)/10000L);
           g_pcm_sfx->tonegen.min = -g_pcm_sfx->tonegen.max;
           break;

      default:
           break;
    }
    g_pcm_sfx->tonegen.state = false;
  }
}

static boolean
_Sfx_PCM_StreamCallback(void *user, void *buffer, uint32 frames)
{
  boolean  result;
  uint8   *p8,  *_p8;
  uint16  *p16, *_p16;
  uint32   cnt, i, val, no_snd;
  _music  *m;
  _sample *s;
  _SfxGlobals      *g_sfx;
  _Sfx_PCM_Globals *g_pcm_sfx;

  // globals accesss
  g_sfx     = (_SfxGlobals *)user;
  g_pcm_sfx = (_Sfx_PCM_Globals *)g_sfx->extension;

  // default return value
  result = false;

  // lets start filling our buffer (as appropriate)
  no_snd = 0x00;
  switch (g_sfx->format)
  {
    case snd_uint8:
         no_snd = 0x00;

    case snd_int8:
         no_snd ^= 0x80;   // take into account signed values

         p8 = (uint8 *)buffer;
         cnt = frames;
         if (g_sfx->type == snd_stereo) cnt <<= 1;

         // process the sound buffer
         while (cnt--)
         {
           val = 0;

           // tone generator
           if (g_pcm_sfx->tonegen.count > 0)
           {
             // simple square wave, write value
             val += (g_pcm_sfx->tonegen.state) 
                       ? g_pcm_sfx->tonegen.max : g_pcm_sfx->tonegen.min;

             // next frame of audio
             if ((g_sfx->type == snd_mono) || ((cnt & 1) == 0))
             {
               // have we completed the amount of frames we need to do?
               if (g_pcm_sfx->tonegen.index++ > g_pcm_sfx->tonegen.count)
                 g_pcm_sfx->tonegen.count = 0;

               // cycle switch (min <-> max)
               if ((g_pcm_sfx->tonegen.index % g_pcm_sfx->tonegen.cycle) == 0)
                 g_pcm_sfx->tonegen.state = !g_pcm_sfx->tonegen.state;
             }
           }
           else
             val += no_snd;

           // mix in the music
           m = &g_sfx->music;
           if (m->active)
           {
             _p8 = (uint8 *)(m->data) + m->offset; m->offset++;
             if (m->offset >= m->length) m->offset = 0; // loop sample

             val += *_p8;
           }
           else
             val += no_snd;

           // mix in the samples (mix_count - 1)
           i = g_sfx->mix_count-1;
           while (i--)
           {
             s = g_sfx->mix_samples[i];
             if (s == NULL)
               val += no_snd;
             else
             {
               if ((s->active) && (s->offset < s->length))
               {
                 _p8 = (uint8 *)(s->data) + s->offset; s->offset++;
                 val += *_p8;
               }
               else
               {
                 g_sfx->mix_samples[i] = NULL;
                 val += no_snd;
               }
             }
           }

           *p8++ = (int8)(val / (g_sfx->mix_count + 1));
         }

         result = true;
         break;

    case snd_int16:
         no_snd = 0x8000;   // take into account signed

         p16 = (uint16 *)buffer;
         cnt = frames;
         if (g_sfx->type == snd_stereo) cnt <<= 1;

         // process the sound buffer
         while (cnt--)
         {
           val = 0;

           // tone generator
           if (g_pcm_sfx->tonegen.count > 0)
           {
             // simple square wave, write value
             val += (g_pcm_sfx->tonegen.state) 
                      ? g_pcm_sfx->tonegen.max : g_pcm_sfx->tonegen.min;

             // next frame of audio
             if ((g_sfx->type == snd_mono) || ((cnt & 1) == 0))
             {
               // have we completed the amount of frames we need to do?
               if (g_pcm_sfx->tonegen.index++ > g_pcm_sfx->tonegen.count)
                 g_pcm_sfx->tonegen.count = 0;

               // cycle switch (min <-> max)
               if ((g_pcm_sfx->tonegen.index % g_pcm_sfx->tonegen.cycle) == 0)
                 g_pcm_sfx->tonegen.state = !g_pcm_sfx->tonegen.state;
             }
           }
           else
             val += no_snd;

           // mix in the music
           m = &g_sfx->music;
           if (m->active)
           {
             _p16 = (uint16 *)(m->data) + m->offset; m->offset++;
             if (m->offset > m->length) m->offset = 0; // loop sample

             val += *_p16;
           }
           else
             val += no_snd;

           // mix in the samples (mix_count - 1)
           i = g_sfx->mix_count-1;
           while (i--)
           {
             s = g_sfx->mix_samples[i];
             if (s == NULL)
               val += no_snd;
             else
             {
               if ((s->active) && (s->offset < s->length))
               {
                 _p16 = (uint16 *)(s->data) + s->offset; s->offset++;
                 val += *_p16;
               }
               else
               {
                 g_sfx->mix_samples[i] = NULL;
                 val += no_snd;
               }
             }
           }

           *p16++ = (int16)(val / (g_sfx->mix_count + 1));
         }

         result = true;
         break;

    // unknown format: should never happen
    default:
         break;
  }

  return result;
}

/********************************* EOF ***********************************/
