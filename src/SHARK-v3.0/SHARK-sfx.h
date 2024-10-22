/*************************************************************************
 *
 * Copyright 2001+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-sfx.h
 */

#ifndef SHARK_SFX_H
#define SHARK_SFX_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
 * SHARK: sound engine
 *************************************************************************
 *
 * SFX ENGINE
 * ----------
 *
 * A SHARK library provides basic audio support for playing tones, 
 * samples and music that utilizes the same audio freq, rate, size.
 *
 *************************************************************************/

//------------------------------------------------------------------------
// constants and enumerations
//------------------------------------------------------------------------

#define MAX_SAMPLES 63

typedef enum sfx_type
{
  sfx_pcm = 0
} sfx_type;

typedef enum sfx_command
{
  sfx_play = 0,
  sfx_stop,
  sfx_pause,
  sfx_seek
} sfx_command;

typedef enum vol_command
{
  vol_get = 0,
  vol_set
} vol_command;

//------------------------------------------------------------------------
// structures and datatypes 
//------------------------------------------------------------------------

// none

//------------------------------------------------------------------------
// application programmer interface
//------------------------------------------------------------------------

extern boolean
_SfxInitialize(snd_rate rate, snd_format format, snd_type type, uint16 frames, uint16 mix_count);

extern boolean
_SfxDefineMusic(sfx_type type, void *data, uint32 length);

extern void
_SfxRemoveMusic();

extern boolean
_SfxDefineSample(uint32 id, void *data, uint32 length);

extern void
_SfxRemoveSample(uint32 id);

extern void
_SfxVolume(vol_command command, uint8 *music, uint8 *sfx);

extern void
_SfxToneCommand(uint16 freq, uint32 duration, uint8 volume);

extern void
_SfxSampleCommand(uint32 id, sfx_command command, void *data);

extern void
_SfxMusicCommand(sfx_command command, void *data);

extern void
_SfxTerminate();

#endif

/********************************* EOF ***********************************/
