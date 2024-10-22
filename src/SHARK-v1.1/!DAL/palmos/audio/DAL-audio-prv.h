/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-audio-prv.h
 */

#include "../DAL-prv.h"

#ifndef DAL_AUDIO_PRV_H
#define DAL_AUDIO_PRV_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
 *
 *    Application Constants/Structures
 *
 *************************************************************************/

#define AUDIO_FRAME_SIZE  2048
#define AUDIO_BUFFER_SIZE (AUDIO_FRAME_SIZE * 4)   // *2 = stereo
                                                   // *2 = 16bit

/*************************************************************************
 *
 * DAL Internal Use API's
 *
 *************************************************************************/

Err DAL_SndThread(void *, SndStreamRef, void *, UInt32);

#ifdef __cplusplus
}
#endif

#endif

/********************************* EOF ***********************************/
