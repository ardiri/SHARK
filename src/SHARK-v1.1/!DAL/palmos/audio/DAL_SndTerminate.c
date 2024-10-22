/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_SndTerminate.c
 */

#include "../DAL-prv.h"

void
DAL_SndTerminate()
{
#ifdef _M_ARM
  PALM_AudioInfo *audio;
  GLOBALS_ACCESS;

  // terminate the active audio stream
  _SndStreamStop(0);
  _SndStreamDelete(0);

  // clean up memory usage within audio system
  audio = (PALM_AudioInfo *)g->audio.platform;
  if (audio != NULL)
  {
    if (audio->buffer != NULL) _MemPtrFree(audio->buffer);
    audio->buffer = NULL;
  }

  if (g->audio.platform != NULL) _MemPtrFree(g->audio.platform);
  g->audio.platform = NULL; audio = NULL;
#endif
}

/********************************* EOF ***********************************/
