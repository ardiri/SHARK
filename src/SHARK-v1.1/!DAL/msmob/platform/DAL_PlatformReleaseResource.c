/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_PlatformReleaseResource.c
 */

#include "../DAL-prv.h"

void
DAL_PlatformReleaseResource(void *ptr)
{
  // its just a pointer, free it :)
  if (ptr != NULL) _MemPtrFree(ptr);
}

/********************************* EOF ***********************************/
