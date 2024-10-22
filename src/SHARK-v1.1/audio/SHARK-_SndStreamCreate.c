/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_SndStreamCreate.c
 */

#include "../SHARK-prv.h"

snd_stream 
_SndStreamCreate(snd_rate rate, snd_format format, snd_type type, _SndStreamBufferCallback func, void *user)
{
  return DAL_SndStreamCreate(rate, format, type, func, user);
}

/********************************* EOF ***********************************/
