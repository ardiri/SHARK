/*************************************************************************
 *
 * Copyright (c) 2002-2003 MobileWizardry
 * All rights reservered.
 *
 *************************************************************************/

/*
 * @(#)series60/DAL-audio.inc
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

/*************************************************************************
 *
 * Initialization / Termination
 *
 *************************************************************************/

void
_AudioInitialize()
{
  register DAL_GlobalsType* gDALGlobals = (DAL_GlobalsType *)Dll::Tls();

  gDALGlobals->iToneGenerator = NULL;
}

void
_AudioTerminate()
{
  register DAL_GlobalsType* gDALGlobals = (DAL_GlobalsType *)Dll::Tls();

  if (gDALGlobals->iToneGenerator) delete gDALGlobals->iToneGenerator;
}

/*************************************************************************
 *
 * Platform Implementation
 *
 *************************************************************************/

void
_PlaySound(uint32 freq, uint16 duration, uint16 volume)
{
  register DAL_GlobalsType* gDALGlobals = (DAL_GlobalsType *)Dll::Tls();

  // play the sound
/*User::Beep((TInt)freq, TTimeIntervalMicroSeconds32(duration * 1000));*/

  // NOTE:
  //
  // the iToneGenerator requires another thread to actually activate the
  // audio callback MatoPrepareComplete() - so, the audio will NOT play
  // until this thread regains control - THIS IS VERY BAD - another way
  // must be implemented to ensure 'right now audio'
  
  if (gDALGlobals->iToneGenerator) delete gDALGlobals->iToneGenerator;
  gDALGlobals->iToneGenerator = ToneGenerator::NewL((TInt)freq, (TInt)(duration * 1000), (TInt)volume);
}

/********************************* EOF ***********************************/
