/*************************************************************************
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
  g_pcm_sfx = (_Sfx_PCM_Globals *)g_sfx -> extension;

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
  g_sfx -> extension = (void *)g_pcm_sfx;

  // initialize
  g_pcm_sfx -> tonegen.min   = 0;
  g_pcm_sfx -> tonegen.max   = 0;
  g_pcm_sfx -> tonegen.cycle = 1;
  g_pcm_sfx -> tonegen.index = 0;
  g_pcm_sfx -> tonegen.count = 0;
  g_pcm_sfx -> tonegen.state = false;

SFX_PCM_INIT_ABORT:

  return init_ok;
}

static void
_Sfx_PCM_Terminate()
{
  SFX_GLOBALS_PCM_ACCESS;

  // clean up
  if (g_pcm_sfx != NULL) _MemPtrFree(g_pcm_sfx);
  g_sfx -> extension = NULL;
}

static void
_Sfx_PCM_TonePlay(uint16 freq, uint32 duration, uint8 volume)
{
  uint16 min_freq, cyc_freq;
  uint32 val_u;
   int32 val_s;
  SFX_GLOBALS_PCM_ACCESS;

  cyc_freq = 0;
  min_freq = 0;
  switch (g_sfx -> rate)
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
    g_pcm_sfx -> tonegen.count = 0;  // stop existing audio

    g_pcm_sfx -> tonegen.index = 0;
    g_pcm_sfx -> tonegen.count = ((cyc_freq * duration) / 1000);
    g_pcm_sfx -> tonegen.cycle = (int16)(min_freq / freq);
/**
 ** LINEAR: = doesn't give good volume scale for differences
 **
    g_pcm_sfx -> tonegen.max   = 128 - (127 * volume) / 100;
    g_pcm_sfx -> tonegen.min   = 128 + (127 * volume) / 100;
 **/
    switch (g_sfx -> format)
    {
      case snd_uint8:
           val_u = (uint8)((127L*volume*volume)/10000L);
           
           g_pcm_sfx -> tonegen.max = (uint8)(128 + val_u);
           g_pcm_sfx -> tonegen.min = (uint8)(128 - val_u);
           break;

      case snd_int8:
           val_s = (int8)((127L*volume*volume)/10000L);
           
           g_pcm_sfx -> tonegen.max = (int8) val_s;
           g_pcm_sfx -> tonegen.min = (int8)-val_s;
           break;

      case snd_int16:
           val_s = (int16)((32767L*volume*volume)/10000L);

           g_pcm_sfx -> tonegen.max = (int16) val_s;
           g_pcm_sfx -> tonegen.min = (int16)-val_s;
           break;

      default:
           break;
    }
    g_pcm_sfx -> tonegen.state = false;
  }
}

static boolean
_Sfx_PCM_StreamCallback(void *user, void *buffer, uint32 frames)
{
  boolean  result;
  uint8   *p8,  *_p8;
  uint16  *p16, *_p16;
  uint32   cnt, used;
  uint32   val_u, no_snd_u;
   int32   val_s, no_snd_s;
  int16    i;
  _music  *m;
  _sample *s;
  _SfxGlobals      *g_sfx;
  _Sfx_PCM_Globals *g_pcm_sfx;

  // globals accesss
  g_sfx     = (_SfxGlobals *)user;
  g_pcm_sfx = (_Sfx_PCM_Globals *)g_sfx -> extension;

  // default return value
  result = false;

  // lets start filling our buffer (as appropriate)
  switch (g_sfx -> format)
  {
    case snd_uint8:
         no_snd_u = 0x80;

         p8 = (uint8 *)buffer;
         cnt = frames;
         if (g_sfx -> type == snd_stereo) cnt <<= 1;

         // process the sound buffer
         while (cnt--)
         {
           used  = 0;
           val_u = 0;

           // tone generator
           if (g_pcm_sfx -> tonegen.count > 0)
           {
             // simple square wave, write value
             used++;
             val_u += (g_pcm_sfx -> tonegen.state) 
                       ? g_pcm_sfx -> tonegen.max : g_pcm_sfx -> tonegen.min;

             // next frame of audio
             if ((g_sfx -> type == snd_mono) || ((cnt & 1) == 0))
             {
               // have we completed the amount of frames we need to do?
               if (g_pcm_sfx -> tonegen.index++ > g_pcm_sfx -> tonegen.count)
                 g_pcm_sfx -> tonegen.count = 0;

               // cycle switch (min < ->  max)
               if ((g_pcm_sfx -> tonegen.index % g_pcm_sfx -> tonegen.cycle) == 0)
                 g_pcm_sfx -> tonegen.state = !g_pcm_sfx -> tonegen.state;
             }
           }

           // mix in the music
           m = &g_sfx -> music;
           if (m -> active)
           {
             _p8 = (uint8 *)(m -> data) + m -> offset; m -> offset++;
             if (m -> offset >= m -> length) m -> offset = 0; // loop sample

             used++;
             val_u += (uint8)*_p8;
           }
 
           // mix in the samples (mix_count - 1)
           for (i=0; i < (g_sfx -> mix_count - 1); i++)
           {
             s = g_sfx -> mix_samples[i];
             if (s != NULL)
             {
               if (s -> active)
               {
                 _p8 = (uint8 *)(s -> data) + s -> offset; s -> offset++;
                 if (s -> offset >= s -> length) 
                 {
                   s -> active = false; // terminate sample
                   g_sfx -> mix_samples[i] = NULL;
                 }

                 used++;
                 val_u += (uint8)*_p8;
               }
             }
           }

           if (used == 0) *p8++ = (uint8)no_snd_u;
           else           *p8++ = (uint8)(val_u / used);
         }

         result = true;
         break;

    case snd_int8:
         no_snd_s = 0x00;

         p8 = (uint8 *)buffer;
         cnt = frames;
         if (g_sfx -> type == snd_stereo) cnt <<= 1;

         // process the sound buffer
         while (cnt--)
         {
           used  = 0;
           val_s = 0;

           // tone generator
           if (g_pcm_sfx -> tonegen.count > 0)
           {
             // simple square wave, write value
             used++;
             val_s += (g_pcm_sfx -> tonegen.state) 
                       ? g_pcm_sfx -> tonegen.max : g_pcm_sfx -> tonegen.min;

             // next frame of audio
             if ((g_sfx -> type == snd_mono) || ((cnt & 1) == 0))
             {
               // have we completed the amount of frames we need to do?
               if (g_pcm_sfx -> tonegen.index++ > g_pcm_sfx -> tonegen.count)
                 g_pcm_sfx -> tonegen.count = 0;

               // cycle switch (min < ->  max)
               if ((g_pcm_sfx -> tonegen.index % g_pcm_sfx -> tonegen.cycle) == 0)
                 g_pcm_sfx -> tonegen.state = !g_pcm_sfx -> tonegen.state;
             }
           }
 
           // mix in the music
           m = &g_sfx -> music;
           if (m -> active)
           {
             _p8 = (uint8 *)(m -> data) + m -> offset; m -> offset++;
             if (m -> offset >= m -> length) m -> offset = 0; // loop sample

             used++;
             val_s += (int8)*_p8;
           }
  
           // mix in the samples (mix_count - 1)
           for (i=0; i < (g_sfx -> mix_count - 1); i++)
           {
             s = g_sfx -> mix_samples[i];
             if (s != NULL)
             {
               if (s -> active)
               {
                 _p8 = (uint8 *)(s -> data) + s -> offset; s -> offset++;
                 if (s -> offset >= s -> length) 
                 {
                   s -> active = false; // terminate sample
                   g_sfx -> mix_samples[i] = NULL;
                 }

                 used++;
                 val_s += (int8)*_p8;
               }
             }
           }

           if (used == 0) *p8++ = (int8)no_snd_s;
           else           *p8++ = (int8)(val_s / used);
         }

         result = true;
         break;

    case snd_int16:
         no_snd_s = 0x0000;

         p16 = (uint16 *)buffer;
         cnt = frames;
         if (g_sfx -> type == snd_stereo) cnt <<= 1;

         // process the sound buffer
         while (cnt--)
         {
           used  = 0;
           val_s = 0;

           // tone generator
           if (g_pcm_sfx -> tonegen.count > 0)
           {
             // simple square wave, write value
             used++;
             val_s += (g_pcm_sfx -> tonegen.state) 
                      ? g_pcm_sfx -> tonegen.max : g_pcm_sfx -> tonegen.min;

             // next frame of audio
             if ((g_sfx -> type == snd_mono) || ((cnt & 1) == 0))
             {
               // have we completed the amount of frames we need to do?
               if (g_pcm_sfx -> tonegen.index++ > g_pcm_sfx -> tonegen.count)
                 g_pcm_sfx -> tonegen.count = 0;

               // cycle switch (min < ->  max)
               if ((g_pcm_sfx -> tonegen.index % g_pcm_sfx -> tonegen.cycle) == 0)
                 g_pcm_sfx -> tonegen.state = !g_pcm_sfx -> tonegen.state;
             }
           }

           // mix in the music
           m = &g_sfx -> music;
           if (m -> active)
           {
             _p16 = (uint16 *)(m -> data) + m -> offset; m -> offset++;
             if (m -> offset > m -> length) m -> offset = 0; // loop sample

             used++;
             val_s += (int16)*_p16;
           }

           // mix in the samples (mix_count - 1)
           for (i=0; i < (g_sfx -> mix_count - 1); i++)
           {
             s = g_sfx -> mix_samples[i];
             if (s != NULL)
             {
               if (s -> active)
               {
                 _p16 = (uint16 *)(s -> data) + s -> offset; s -> offset++;
                 if (s -> offset >= s -> length) s -> active = false; // terminate sample
                 
                 used++;
                 val_s += (int16)*_p16;
               }
             }
           }

           if (used == 0) *p16++ = (int16)no_snd_s;
           else           *p16++ = (int16)(val_s / used);
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
