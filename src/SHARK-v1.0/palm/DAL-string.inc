/*************************************************************************
 *
 * Copyright (c) 2002-2003 MobileWizardry
 * All rights reservered.
 *
 *************************************************************************/

/*
 * @(#)palm/DAL-string.inc
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
static void _StrInitialize() __DAL__;
static void _StrTerminate()  __DAL__;

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
  StrCopy((int8 *)strDst, (int8 *)strSrc);
}

uint16
_StrLen(uint8 *str)
{
  return (uint16)StrLen((int8 *)str);
}

void
_StrCat(uint8 *str, uint8 *strCat)
{
  StrCat((int8 *)str, (int8 *)strCat);
}

uint16
_StrAToI(uint8 *str)
{
  return (uint16)StrAToI((int8 *)str);
}

void
_StrIToA(uint8 *str, int16 value)
{
  StrIToA((int8 *)str, (int16)value);
}

/********************************* EOF ***********************************/

