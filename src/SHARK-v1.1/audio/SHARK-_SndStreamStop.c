/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_SndStreamStop.c
 */

#include "../SHARK-prv.h"

void
_SndStreamStop(snd_stream stream_id)
{
  DAL_SndStreamStop(stream_id);
}

/********************************* EOF ***********************************/
