/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-audio.h
 */

#ifndef DAL_AUDIO_H
#define DAL_AUDIO_H

#include "../DAL.h"

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
 *
 * Platform Constants/Structures
 *
 *************************************************************************/

// none

/*************************************************************************
 *
 * Initialization / Termination
 *
 *************************************************************************/

extern boolean     DAL_SndInitialize();
extern void        DAL_SndTerminate();

/*************************************************************************
 *
 * Device Specific
 *
 *************************************************************************/

extern void       DAL_PlaySound(uint32 freq, uint16 duration, uint16 volume);

extern boolean    DAL_SndStreamAvailable();
extern snd_stream DAL_SndStreamCreate(snd_rate rate, snd_format format, snd_type type, _SndStreamBufferCallback func, void *user);
extern void       DAL_SndStreamStart(snd_stream stream_id);
extern void       DAL_SndStreamStop(snd_stream stream_id);
extern void       DAL_SndStreamDelete(snd_stream stream_id);

/*************************************************************************
 *
 * SHARK Internal Use API's
 *
 *************************************************************************/

// none

#ifdef __cplusplus
}
#endif

#endif

/********************************* EOF ***********************************/
