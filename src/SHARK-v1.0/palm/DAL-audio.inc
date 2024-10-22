/*************************************************************************
 *
 * Copyright (c) 2002-2003 MobileWizardry
 * All rights reservered.
 *
 *************************************************************************/

/*
 * @(#)palm/DAL-audio.inc
 *
 * -- Aaron Ardiri     (mailto:aaron_ardiri@mobilewizardry.com)
 * -- Charles Kerchner (mailto:chip_kerchner@mobilewizardry.com)
 */

#include "DAL-prv.h"

/*************************************************************************
 *
 * Platform Implementation Structures
 *
 *************************************************************************/

// none

// local functions
static void _AudioInitialize()                                                                __DAL__;
static void _AudioTerminate()                                                                 __DAL__;

/*************************************************************************
 *
 * Initialization / Termination
 *
 *************************************************************************/

void
_AudioInitialize()
{
}

void
_AudioTerminate()
{
}

/*************************************************************************
 *
 * Platform Implementation
 *
 *************************************************************************/

void
_PlaySound(uint32 freq, uint16 duration, uint16 volume)
{
  SndCommandType snd;

  snd.cmd      = sndCmdFrqOn;
  snd.reserved = 0;
  snd.param1   = (int32)freq;
  snd.param2   = (uint16)duration;
  if (volume > 99)
    snd.param3   = sndMaxAmp;
  else
    snd.param3   = (uint16)(((uint32)sndMaxAmp * volume) / 100);

  // play the sound
  SndDoCmd(0, &snd, 0);
}

/********************************* EOF ***********************************/
