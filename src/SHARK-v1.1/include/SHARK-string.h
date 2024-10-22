/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-string.h
 */

#ifndef SHARK_STRING_H
#define SHARK_STRING_H

#include "../SHARK.h"

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
 *
 * Constants / Structures
 *
 *************************************************************************/

// none

/*************************************************************************
 *
 * Application Programmers Interface
 *
 *************************************************************************/

// device specific
// NONE

// device independent
extern void        _StrSet(uint8 *s, uint8 chr);
extern uint16      _StrLen(uint8 *s);

extern void        _StrCopy(uint8 *s, uint8 *sX);
extern void        _StrCat(uint8 *s, uint8 *sX);
extern uint8 *     _StrStr(uint8 *s, uint8 *tok);
extern uint8 *     _StrChr(uint8 *s, uint8 chr);
extern int16       _StrCompare(uint8 *s1, uint8 *s2);
extern int16       _StrCaselessCompare(uint8 *s1, uint8 *s2);

extern void        _StrNCopy(uint8 *s, uint8 *sX, uint16 count);
extern void        _StrNCat(uint8 *s, uint8 *sX, uint16 count);
extern uint8 *     _StrNStr(uint8 *s, uint8 *tok, uint16 count);
extern uint8 *     _StrNChr(uint8 *s, uint8 chr, uint16 count);
extern int16       _StrNCompare(uint8 *s1, uint8 *s2, uint16 count);
extern int16       _StrNCaselessCompare(uint8 *s1, uint8 *s2, uint16 count);

extern int16       _StrAToI(uint8 *s);
extern void        _StrIToA(uint8 *s, int16 value);
extern uint16      _StrAToD(uint8 *s);
extern void        _StrDToA(uint8 *s, uint16 value);
extern uint16      _StrAToO(uint8 *s);
extern void        _StrOToA(uint8 *s, uint16 value);
extern uint16      _StrAToH(uint8 *s);
extern void        _StrHToA(uint8 *s, uint16 value);
extern float       _StrAToF(uint8 *s);
extern void        _StrFToA(uint8 *s, float value, uint8 decimal);

#ifdef __cplusplus
}
#endif

#endif

/********************************* EOF ***********************************/
