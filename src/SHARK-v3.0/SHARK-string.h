/*************************************************************************
 *
 * Copyright 2001+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-string.h
 */

#ifndef SHARK_STRING_H
#define SHARK_STRING_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
 * SHARK: string
 *************************************************************************
 *
 * STRING MANIPULATION LIBRARY
 * ---------------------------
 *
 * A string is a simple memory buffer containing characters that is
 * terminated with the use of a NULL character. Strings are typically
 * used to present binary information in readable form to the user.
 * The string library is based on libc but has been rewritten to ensure 
 * portability and ensure API completeness across different platforms.
 *
 *************************************************************************/

//------------------------------------------------------------------------
// constants and enumerations
//------------------------------------------------------------------------

// none

//------------------------------------------------------------------------
// structures and datatypes 
//------------------------------------------------------------------------

// none

//------------------------------------------------------------------------
// application programmer interface
//------------------------------------------------------------------------

#define IS_ALPHA(a)    ((((a) & ~0x20) >= 'A') && (((a) & ~0x20) <= 'Z'))
#define IS_ALPHANUM(a) (IS_DIGIT(a) || IS_ALPHA(a))
#define IS_DIGIT(a)    (((a) >= '0') && ((a) <= '9'))
#define IS_HEXDIGIT(a) (IS_DIGIT(a) || \
                       ((((a) & ~0x20) >= 'A') && (((a) & ~0x20) <= 'F')))
#define IS_OPERATOR(a) (((a) == '-') || ((a) == '+') || \
                        ((a) == '/') || ((a) == '*'))
#define IS_SPACE(a)    (((a) == ' ') || ((a) == 160) || \
                        ((a) == '\t') || ((a) == '\n'))

uint32
_StrAToD(char *s);

double
_StrAToF(char *s);

uint32
_StrAToH(char *s);

int32
_StrAToI(char *s);

uint32
_StrAToO(char *s);

int32
_StrCaselessCompare(char *s1, char *s2);

char *
_StrCaselessStr(char *s, char *tok);

char *
_StrCat(char *s, char *sX);

char *
_StrChr(char *s, char chr);

int32
_StrCompare(char *s1, char *s2);

char *
_StrCopy(char *s, char *sX);

void
_StrDToA(char *s, uint32 value);

void
_StrFToA(char *s, double value, uint32 decimal);

void
_StrHToA(char *s, uint32 value);

void
_StrIToA(char *s, int32 value);

uint32
_StrLen(char *s);

int32
_StrNCaselessCompare(char *s1, char *s2, uint32 count);

char *
_StrNCaselessStr(char *s, char *tok, uint32 count);

char *
_StrNCat(char *s, char *sX, uint32 count);

char *
_StrNChr(char *s, char chr, uint32 count);

int32
_StrNCompare(char *s1, char *s2, uint32 count);

char *
_StrNCopy(char *s, char *sX, uint32 count);

char *
_StrNRChr(char *s, char chr, uint32 count);

char *
_StrNStr(char *s, char *tok, uint32 count);

void
_StrOToA(char *s, uint32 value);

char *
_StrRChr(char *s, char chr);

char *
_StrReverse(char *s);

char *
_StrSet(char *s, char chr);

char *
_StrStr(char *s, char *tok);

char *
_StrStripMarkup(char *s, uint32 len);

#endif

/********************************* EOF ***********************************/
