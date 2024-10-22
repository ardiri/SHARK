/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_SndWaveOutProc.c
 */

#include "../DAL-prv.h"

void CALLBACK 
DAL_SndWaveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
  MSmob_AudioInfo *audio;

  if (uMsg == WOM_DONE)
  {
    audio = (MSmob_AudioInfo *)dwInstance;

    EnterCriticalSection(&audio->wcs);
    audio->freeblocks++;
    LeaveCriticalSection(&audio->wcs);
  }
}

/********************************* EOF ***********************************/
