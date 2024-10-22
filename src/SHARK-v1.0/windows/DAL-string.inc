/*************************************************************************
 *
 * Copyright (c) 2002-2003 MobileWizardry
 * All rights reservered.
 *
 *************************************************************************/

/*
 * @(#)windows/DAL-string.inc
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
_StrInitialize()
{
}

void
_StrTerminate()
{
}

/*************************************************************************
 *
 * Platform Implementation
 *
 *************************************************************************/

void
_StrCopy(uint8 *strDst, uint8 *strSrc)
{
  strcpy((char *)strDst, (char *)strSrc);
}

uint16
_StrLen(uint8 *str)
{
  return (uint16)strlen((char *)str);
}

void
_StrCat(uint8 *str, uint8 *strCat)
{
  strcat((char *)str, (char *)strCat);
}

uint16
_StrAToI(uint8 *str)
{
  return (uint16)atoi((char *)str);
}

void
_StrIToA(uint8 *str, int16 value)
{
  _itoa(value, (char *)str, 10);
}

/********************************* EOF ***********************************/