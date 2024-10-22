/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-string.c
 */

#include "../../SHARK.h"

/*************************************************************************
 *
 * Configuration
 *
 *************************************************************************/

/*************************************************************************
 *
 * Global Variables
 *
 *************************************************************************/

//------------------------------------------------------------------------
//                    --== GLOBALS ARE FORBIDDEN! ==--
//
// DAL may support the use of globals on some platforms, however, its not
// guaranteed that all the destination platforms allow the use of globals
// (variable or static data). use the GlobalsType / _LibSetGlobals() API.
//------------------------------------------------------------------------

/*************************************************************************
 *
 * Implementation
 *
 *************************************************************************/

void
_StrSet(char *s, char chr)
{
  // pre-condition (cannot have null pointer)
  if (s == NULL) return;

  _MemSet((void *)s, chr, _StrLen(s));
}

uint32
_StrLen(char *s)
{
  char   *x;
  uint32  len;

  // default return value
  len = 0;

  // pre-condition (cannot have null pointer)
  if (s != NULL)
  {
    // use temporary variables for processing
    x = s;

    // look for that \0 character
    while (*x++ != '\0') len++;
  }

  return len;
}

char *
_StrReverse(char *s)
{
  char  *x;
  char  *p1, *p2, tmp;

  // default return value
  x = s;

  // pre-condition (cannot have null pointer)
  if (s != NULL)
  {
    // use temporary variables for processing
    p1 = s;
    p2 = s + _StrLen(x) - 1;

    // start at <-- and --> swapping until overlap or equality
    while (p2 > p1)
    {
      tmp   = *p1;
      *p1++ = *p2;
      *p2-- = tmp; // reverse the string
    }
  }

  return x;
}

void
_StrCopy(char *s, char *sX)
{
  _StrNCopy(s, sX, _StrLen(sX));
}

void
_StrCat(char *s, char *sX)
{
  _StrNCat(s, sX, _StrLen(sX));
}

char *
_StrChr(char *s, char chr)
{
  return (char *)_StrNChr((void *)s, chr, _StrLen(s));
}

char *
_StrRChr(char *s, char chr)
{
  return (char *)_StrNRChr((void *)s, chr, _StrLen(s));
}

char *
_StrStr(char *s, char *tok)
{
  return (char *)_StrNStr(s, tok, _StrLen(s));
}

char *
_StrCaselessStr(char *s, char *tok)
{
  return (char *)_StrNCaselessStr(s, tok, _StrLen(s));
}

int32
_StrCompare(char *s1, char *s2)
{
  return _StrNCompare(s1, s2, MAX(_StrLen(s1), _StrLen(s2)));
}

int32
_StrCaselessCompare(char *s1, char *s2)
{
  return _StrNCaselessCompare(s1, s2, MAX(_StrLen(s1), _StrLen(s2)));
}

void
_StrNCopy(char *s, char *sX, uint32 count)
{
  uint32 cnt;

  // pre-condition (cannot have null pointer)
  if ((s == NULL) || (sX == NULL)) return;

  cnt = MIN(count, _StrLen(sX));
  _MemMove((void *)s, (void *)sX, cnt);
  s[cnt] = 0;  
}

void
_StrNCat(char *s, char *sX, uint32 count)
{
  char  *ptr;
  uint32 cnt;

  // pre-condition (cannot have null pointer)
  if ((s == NULL) || (sX == NULL)) return;

  ptr = s + _StrLen(s);
  cnt = MIN(count, _StrLen(sX));
  _MemMove((void *)ptr, (void *)sX, cnt);
  ptr[cnt] = 0;
}

char *
_StrNChr(char *s, char chr, uint32 count)
{
  return (char *)_MemChr((void *)s, chr, (uint32)(MIN(count, _StrLen(s))));
}

char *
_StrNRChr(char *s, char chr, uint32 count)
{
  return (char *)_MemRChr((void *)s, chr, (uint32)(MIN(count, _StrLen(s))));
}

char *
_StrNStr(char *s, char *tok, uint32 count)
{
  char *pos;
  char *x;
  int   i;

  // default return value
  pos = NULL;

  // pre-condition (cannot have null pointer)
  if (s != NULL)
  {
    // use temporary variables for processing
    x = s;

    // lets scan the memory buffer
    i = count;
    do
    {
      if (_StrNCompare(x, tok, _StrLen(tok)) == 0) 
        { pos = x; goto STR_STR_DONE; }
      x++;
    } while (--i);

STR_STR_DONE:
    ;
  }

  return pos;
}

char *
_StrNCaselessStr(char *s, char *tok, uint32 count)
{
  char *pos;
  char *x;
  int   i;

  // default return value
  pos = NULL;

  // pre-condition (cannot have null pointer)
  if (s != NULL)
  {
    // use temporary variables for processing
    x = s;

    // lets scan the memory buffer
    i = count;
    do
    {
      if (_StrNCaselessCompare(x, tok, _StrLen(tok)) == 0) 
        { pos = x; goto STR_STR_CASELESS_DONE; }
      x++;
    } while (--i);

STR_STR_CASELESS_DONE:
    ;
  }

  return pos;
}

int32
_StrNCompare(char *s1, char *s2, uint32 count)
{
  char  *x, *y;
  int32  diff;
  int    i;
  uint32 l1, l2, cnt;

  // default return value
  diff = 0;

  // pre-condition (cannot have null pointer)
  if ((s1 != NULL) && (s2 != NULL) && (count > 0))
  {
    // use temporary variables for processing
    x = s1;
    y = s2;

    l1  = _StrLen(s1);
    l2  = _StrLen(s2);
    cnt = count;

    cnt = MIN(cnt, l1);
    cnt = MIN(cnt, l2);

    // calculate differences 
    i = cnt;
    do
    {
      diff = *x++ - *y++;
    } while ((--i) && (diff == 0));

    if ((diff == 0) && (cnt != count)) diff = (l1 < l2) ? -1 : 1;
  }

  return diff;
}

int32
_StrNCaselessCompare(char *s1, char *s2, uint32 count)
{
  char  *x, *y;
  int16  diff;
  int    i;
  uint32 l1, l2, cnt;

  // default return value
  diff = 0;

  // pre-condition (cannot have null pointer)
  if ((s1 != NULL) && (s2 != NULL) && (count > 0))
  {
    // use temporary variables for processing
    x = s1;
    y = s2;

    l1  = _StrLen(s1);
    l2  = _StrLen(s2);
    cnt = count;

    cnt = MIN(cnt, l1);
    cnt = MIN(cnt, l2);

    // calculate differences 
    i = cnt;
    do
    {
      diff = (*x++ | 0x20) - (*y++ | 0x20);
    } while ((--i) && (diff == 0));

    if ((diff == 0) && (cnt != count)) diff = (l1 < l2) ? -1 : 1;
  }

  return diff;
}

int32
_StrAToI(char *s)
{
  char   *x;
  int32   value;
  boolean negative;

  // default return value
  value = 0;

  // pre-condition (cannot have null pointer)
  if (s != NULL)
  {
    // use temporary variables for processing
    negative = false;
    x        = s;

    // process the number parsing
    if ((*x) == '-') { negative = true; x++; }

    while ((*x >= '0') && (*x <= '9'))
      value = (value * 10) + (*x++ - '0');

    // we we need to negate it?
    if (negative)
      value = -value;
  }

  return value;
}

void
_StrIToA(char *s, int32 value)
{
  char   *x;
  int32   div;
  int8    res;
  boolean padding;

  // pre-condition (cannot have null pointer)
  if (s == NULL) return;

  // use temporary variables for processing
  x       = s;
  div     = 1000000000; // max is 2147483648
  padding = false;

  // number text generation
  if (value == 0) { *x++ = '0'; }
  else
  {
    // negative? need to think about this
    if (value <  0) { *x++ = '-'; value = -value; }

    while (div > 0)
    {
      res = (int8)((value / div) % 10);

      if (res != 0) { *x++ = '0' + res; padding = true; }
      else if (padding) *x++ = '0';

      div = div / 10;
    }
  }
  *x++ = 0;
}

uint32
_StrAToD(char *s)
{
  char   *x;
  uint32  value;

  // default return value
  value = 0;

  // pre-condition (cannot have null pointer)
  if (s != NULL)
  {
    // use temporary variables for processing
    x = s;

    // process the number parsing
    while ((*x >= '0') && (*x <= '9'))
      value = (value * 10) + (*x++ - '0');
  }

  return value;
}

void
_StrDToA(char *s, uint32 value)
{
  char   *x;
  uint32  div;
  uint8   res;
  boolean padding;

  // pre-condition (cannot have null pointer)
  if (s == NULL) return;

  // use temporary variables for processing
  x       = s;
  div     = 1000000000; // max is 2147483648
  padding = false;

  // number text generation
  if (value == 0) { *x++ = '0'; }
  else
  {
    while (div > 0)
    {
      res = (uint8)((value / div) % 10);

      if (res != 0) { *x++ = '0' + res; padding = true; }
      else if (padding) *x++ = '0';

      div = div / 10;
    }
  }
  *x++ = 0;
}

uint32
_StrAToO(char *s)
{
  char   *x;
  uint32  value;

  // default return value
  value = 0;

  // pre-condition (cannot have null pointer)
  if (s != NULL)
  {
    // use temporary variables for processing
    x = s;

    // skip over the '0' if it is there
    if (*x == '0') x++;

    // process the number parsing
    while ((*x >= '0') && (*x <= '7'))
      value = (value << 3) + (*x++ - '0');  // * 8
  }

  return value;
}

void
_StrOToA(char *s, uint32 value)
{
  char    *x;
  uint32   div;
  uint8    res;
  boolean  padding;

  // pre-condition (cannot have null pointer)
  if (s == NULL) return;

  // use temporary variables for processing
  x       = s;
  div     = 2L << 29; // 8^10 = 1073741824
  padding = false;

  // number text generation
  *x++ = '0';

  while (div > 0)
  {
    res = (uint8)((value / div) & 0x07); // mod 8

    if (res != 0) { *x++ = '0' + res; padding = true; }
    else if (padding) *x++ = '0';

    div = div >> 3; // div 8
  }
  *x++ = 0;
}

uint32
_StrAToH(char *s)
{
  char   *x;
  uint32  value;

  // default return value
  value = 0;

  // pre-condition (cannot have null pointer)
  if (s != NULL)
  {
    // use temporary variables for processing
    x = s;

    // skip over the '0x' if it is there
    if ((_StrLen(s) > 2) && (*x == '0') && (*(x+1) == 'x')) x += 2;

    // process the number parsing
    while (((*x >= '0') && (*x <= '9')) || (((*x & ~0x20) >= 'A') && ((*x & ~0x20) <= 'A')))
    {
      if ((*x >= '0') && (*x <= '9'))
        value = (value << 4) + (*x++ - '0'); // * 16
      else
        value = (value << 4) + (((*x++ & ~0x20) - 'A') + 10); // * 16
    }
  }

  return value;
}

void
_StrHToA(char *s, uint32 value)
{
  char    *x;
  uint32   div;
  uint8    res;
  boolean  padding;

  // pre-condition (cannot have null pointer)
  if (s == NULL) return;

  // use temporary variables for processing
  x       = s;
  div     = 2L << 27; // 16^7 = 268435456
  padding = true;

  // number text generation
  *x++ = '0';
  *x++ = 'x';

  while (div > 0)
  {
    res = (uint8)((value / div) & 0x0f); // mod 16

    if (res != 0)
    {
      if (res < 10) *x++ = '0' + res;
      else          *x++ = 'A' + (res-10);
      padding = true;
    }
    else if (padding) *x++ = '0';

    div = div >> 4; // div 16;
  }
  *x++ = 0;
}

double
_StrAToF(char *s)
{
  char  *x;
  int32  whole, div;
  double value;

  // default return value
  value = 0;

  // pre-condition (cannot have null pointer)
  if (s != NULL)
  {
    // use temporary variables for processing
    x     = s;
    div   = 10;
    value = 0;

    // whats the integer part of this
    whole = _StrAToI(x);

    // lets build the fractional value (if its possible)
    if (_StrChr(x, '.'))
    {
      x = _StrChr(x, '.');
      if (_StrLen(x) > 2)
      {
        x++;

        // process the number parsing
        while ((*x >= '0') && (*x <= '9'))
        {
          value += (*x - '0') / (double)div;
          div = div * 10; x++;
        }
      }
    }

    // bring fractional and decimal together
    value += whole;
  }

  return value;
}

void
_StrFToA(char *s, double value, uint32 decimal)
{
  char *x;
  int32 i;

  // pre-condition (cannot have null pointer)
  if (s == NULL) return;

  // use temporary variables for processing
  x   = s;

  // number text generation
  _StrIToA(x, (int32)value);
  value = value - (int32)value;
  x += _StrLen(x);

  // decimal point
  if (value != 0) *x++ = '.';

  // fractional value
  for (i=0; ((i<(int32)decimal) && (value != 0)); i++)
  {
    value = value * 10;
    *x++ = '0' + (uint8)value;
    value = value - (int32)value;
  }

  *x++ = 0;
}

/********************************* EOF ***********************************/
