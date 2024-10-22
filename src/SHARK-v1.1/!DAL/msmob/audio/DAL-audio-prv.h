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

typedef struct
{
  uint8    min;
  uint8    max;
  uint16   cycle;
  uint32   index;
  uint32   count;
  boolean  state;
} MSmob_SndTonGen;

#define AUDIO_FRAME_SIZE  2048
#define AUDIO_BUFFER_SIZE (AUDIO_FRAME_SIZE * 4)   // *2 = stereo
                                                   // *2 = 16bit

/*************************************************************************
 *
 * DAL Internal Use API's
 *
 *************************************************************************/

extern void CALLBACK DAL_SndWaveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
extern DWORD WINAPI DAL_SndThread(void *data);

extern boolean DAL_SndToneGenCallback(snd_stream stream_id, void *userdata, void *buffer, uint32 frames);

#ifdef __cplusplus
}
#endif

#endif

/********************************* EOF ***********************************/
