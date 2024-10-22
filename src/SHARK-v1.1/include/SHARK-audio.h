/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-audio.h
 */

#ifndef SHARK_AUDIO_H
#define SHARK_AUDIO_H

#include "../SHARK.h"

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
 *
 * Constants / Structures
 *
 *************************************************************************/

typedef enum { snd_8khz = 0, snd_11khz, snd_22khz, snd_44khz } snd_rate;
typedef enum { snd_int8 = 0, snd_uint8, snd_int16            } snd_format;
typedef enum { snd_mono = 0, snd_stereo                      } snd_type;

typedef int16 snd_stream;

typedef boolean (*_SndStreamBufferCallback)(snd_stream stream_id, void *userdata, void *buffer, uint32 frames);

/*************************************************************************
 *
 * Application Programmers Interface
 *
 *************************************************************************/

// device specific
extern void       _PlaySound(uint32 freq, uint16 duration, uint16 volume);

extern boolean    _SndStreamAvailable();
extern snd_stream _SndStreamCreate(snd_rate rate, snd_format format, snd_type type, _SndStreamBufferCallback func, void *user);
extern void       _SndStreamStart(snd_stream stream_id);
extern void       _SndStreamStop(snd_stream stream_id);
extern void       _SndStreamDelete(snd_stream stream_id);

// device independent
// NONE

#ifdef __cplusplus
}
#endif

#endif

/********************************* EOF ***********************************/
