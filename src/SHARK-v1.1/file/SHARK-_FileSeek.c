/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_FileSeek.c
 */

#include "../SHARK-prv.h"

void
_FileSeek(file *f, uint32 ofs, file_position position)
{
  DAL_FileSeek(f, ofs, position);
}

/********************************* EOF ***********************************/
