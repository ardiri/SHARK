/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_SndStreamDelete.c
 */

#include "../SHARK-prv.h"

void
_SndStreamDelete(snd_stream stream_id)
{
  DAL_SndStreamDelete(stream_id);
}

/********************************* EOF ***********************************/
