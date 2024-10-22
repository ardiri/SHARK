/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_SndToneGenCallback.c
 */

#include "../DAL-prv.h"

boolean 
DAL_SndToneGenCallback(snd_stream stream_id, void *userdata, void *buffer, uint32 frames)
{
  Win32_SndTonGen *param;
  uint32           i;
  uint8           *p;
  boolean          result;

  // user data reference
  param = (Win32_SndTonGen *)userdata;

  // default return value (if we have something, lets use it)
  result = (param->count > 0);

  p = (uint8 *)buffer;
  i = frames;
  do
  {
    // do we have something to play?
    if (param->count > 0)
    {
      // cycle switch?
      if ((param->index % param->cycle) == 0)
        param->state = !param->state;

      // write to the buffer
      *p++ = (param->state) ? param->max : param->min;

      // audio complete?
      if (param->index++ > param->count) param->count = 0;
    }

    // zero sound
    else
      *p++ = 128;

  } while (--i);

  return result;
}

/********************************* EOF ***********************************/
