/*************************************************************************
 *
 * Copyright 2001+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-string.c
 */

#include "../SHARK.h"
#include "../SHARK-core.h"
#include "../SHARK-string.h"

/*************************************************************************
 * Configuration
 *************************************************************************/

// none

/*************************************************************************
 * Component Variables
 *************************************************************************/

// none

/*************************************************************************
 * Global Variables
 *************************************************************************/

//------------------------------------------------------------------------
//                    --== GLOBALS ARE FORBIDDEN! ==--
//
// DAL may support the use of globals on some platforms, however, its not
// guaranteed that all the destination platforms allow the use of globals
// (variable or static data). use the GlobalsType / _LibSetGlobals() API.
//------------------------------------------------------------------------

// none

/*************************************************************************
 * Platform Dependent Interface
 *************************************************************************/

// none

/*************************************************************************
 * Implementation
 *************************************************************************/

uint32
_StrAToD(char *s)
{
  uint32  result;
  char   *x;

  // default return value
  result = 0;

  // entry-condition 
  if (s == NULL) return result;

  // use temporary variables for processing
  x = s;

  // process the number parsing
  while ((*x >= '0') && (*x <= '9'))
    result = (result * 10) + (*x++ - '0');

  return result;
}

double
_StrAToF(char *s)
{
  double result;
  char  *x;
  int32  whole, div;

  // default return value
  result = 0;

  // entry-condition 
  if (s == NULL) return result;

  // use temporary variables for processing
  x     = s;
  div   = 10;
  result = 0;

  // whats the integer part of this
  whole = _StrAToI(x);

  // lets build the fractional value (if its possible)
  if (_StrChr(x, '.'))
  {
    x = _StrChr(x, '.');
    if (_StrLen(x) > 1)
    {
      x++;

      // process the number parsing
      while ((*x >= '0') && (*x <= '9'))
      {
        result += (*x - '0') / (double)div;
        div = div * 10; x++;
      }
    }
  }

  // bring fractional and decimal together
  result += whole;

  return result;
}

uint32
_StrAToH(char *s)
{
  uint32  result;
  char   *x;

  // default return value
  result = 0;

  // entry-condition 
  if (s == NULL) return result;
 
  // use temporary variables for processing
  x = s;

  // skip over the '0x' if it is there
  if ((_StrLen(s) > 2) && (*x == '0') && (*(x+1) == 'x')) x += 2;

  // process the number parsing
  while ( ((*x          >= '0') &&  (*x          <= '9')) || 
         (((*x & ~0x20) >= 'A') && ((*x & ~0x20) <= 'F')))
  {
    if ((*x >= '0') && (*x <= '9'))
      result = (result << 4) + (*x++ - '0'); // * 16
    else
      result = (result << 4) + (((*x++ & ~0x20) - 'A') + 10); // * 16
  }

  return result;
}

int32
_StrAToI(char *s)
{
  int32   result;
  char   *x;
  boolean negative;

  // default return value
  result = 0;

  // entry-condition 
  if (s == NULL) return result;
  
  // use temporary variables for processing
  negative = false;
  x        = s;

  // process the number parsing
  if ((*x) == '-') { negative = true; x++; }

  while ((*x >= '0') && (*x <= '9'))
    result = (result * 10) + (*x++ - '0');

  // we we need to negate it?
  if (negative)
    result = -result;

  return result;
}

uint32
_StrAToO(char *s)
{
  uint32  result;
  char   *x;

  // default return value
  result = 0;

  // entry-condition 
  if (s == NULL) return result;
  
  // use temporary variables for processing
  x = s;

  // skip over the '0' if it is there
  if (*x == '0') x++;

  // process the number parsing
  while ((*x >= '0') && (*x <= '7'))
    result = (result << 3) + (*x++ - '0');  // * 8

  return result;
}

int32
_StrCaselessCompare(char *s1, char *s2)
{
  return _StrNCaselessCompare(s1, s2, MAX(_StrLen(s1), _StrLen(s2)));
}

char *
_StrCaselessStr(char *s, char *tok)
{
  return (char *)_StrNCaselessStr(s, tok, _StrLen(s));
}

char *
_StrCat(char *s, char *sX)
{
  return (char *)_StrNCat(s, sX, _StrLen(sX));
}

char *
_StrChr(char *s, char chr)
{
  return (char *)_StrNChr((void *)s, chr, _StrLen(s));
}

int32
_StrCompare(char *s1, char *s2)
{
  return _StrNCompare(s1, s2, MAX(_StrLen(s1), _StrLen(s2)));
}

char *
_StrCopy(char *s, char *sX)
{
  return (char *)_StrNCopy(s, sX, _StrLen(sX));
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
  if (value < 0) value = -value;
  for (i=0; ((i<(int32)decimal) && (value != 0)); i++)
  {
    value = value * 10;
    *x++ = '0' + (uint8)value;
    value = value - (int32)value;
  }

  *x++ = 0;
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
_StrLen(char *s)
{
  uint32  result;
  char   *x;

  // default return value
  result = 0;

  // entry-condition 
  if (s == NULL) return result;

  // use temporary variables for processing
  x = s;

  // look for that \0 character
  while (*x++ != '\0') result++;

  return result;
}

int32
_StrNCaselessCompare(char *s1, char *s2, uint32 count)
{
  int32  result;
  char  *x, *y;
  int    i;
  uint32 l1, l2, cnt;

  // default return value
  result = 0;

  // entry-condition 
  if ((s1 == NULL) || (s2 == NULL) || (count == 0)) return result;

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
    result = (*x++ | 0x20) - (*y++ | 0x20);
  } while ((--i) && (result == 0));

  // strings different size?
  if ((result == 0) && (cnt != count)) result = (l1 < l2) ? -1 : 1;

  return result;
}

char *
_StrNCaselessStr(char *s, char *tok, uint32 count)
{
  char *result;
  char *x;
  int   i;

  // default return value
  result = NULL;

  // entry-condition 
  if (s == NULL) return result;
  
  // use temporary variables for processing
  x = s;

  // lets scan the memory buffer
  i = count;
  do
  {
    if (_StrNCaselessCompare(x, tok, _StrLen(tok)) == 0) 
      { result = x; break; }
    x++;
  } while (--i);

  return result;
}

char *
_StrNCat(char *s, char *sX, uint32 count)
{
  char  *ptr;
  uint32 cnt;

  // entry-condition 
  if ((s == NULL) || (sX == NULL) || (count == 0)) return s;

  ptr = s + _StrLen(s);
  cnt = MIN(count, _StrLen(sX));
  _MemMove((void *)ptr, (void *)sX, cnt);
  ptr[cnt] = 0;

  return s;
}

char *
_StrNChr(char *s, char chr, uint32 count)
{
  return (char *)_MemChr((void *)s, chr, (uint32)(MIN(count, _StrLen(s))));
}

int32
_StrNCompare(char *s1, char *s2, uint32 count)
{
  int32  result;
  char  *x, *y;
  int    i;
  uint32 l1, l2, cnt;

  // default return value
  result = 0;

  // entry-condition 
  if ((s1 == NULL) || (s2 == NULL) || (count == 0)) return result;
  
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
    result = *x++ - *y++;
  } while ((--i) && (result == 0));

  // different string lengths
  if ((result == 0) && (cnt != count)) result = (l1 < l2) ? -1 : 1;

  return result;
}

char *
_StrNCopy(char *s, char *sX, uint32 count)
{
  uint32 cnt;

  // entry-condition 
  if ((s == NULL) || (sX == NULL)) return s;

  cnt = MIN(count, _StrLen(sX));
  _MemMove((void *)s, (void *)sX, cnt);
  s[cnt] = 0;  

  return s;
}

char *
_StrNRChr(char *s, char chr, uint32 count)
{
  return (char *)_MemRChr((void *)s, chr, (uint32)(MIN(count, _StrLen(s))));
}

char *
_StrNStr(char *s, char *tok, uint32 count)
{
  char *result;
  char *x;
  int   i;

  // default return value
  result = NULL;

  // entry-condition 
  if ((s == NULL) || (count == 0)) return result;

  // use temporary variables for processing
  x = s;

  // lets scan the memory buffer
  i = count;
  do
  {
    if (_StrNCompare(x, tok, _StrLen(tok)) == 0) 
      { result = x; break; }
    x++;
  } while (--i);

  return result;
}

void
_StrOToA(char *s, uint32 value)
{
  char    *x;
  uint32   div;
  uint8    res;
  boolean  padding;

  // entry-condition 
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

char *
_StrRChr(char *s, char chr)
{
  return (char *)_StrNRChr((void *)s, chr, _StrLen(s));
}

char *
_StrReverse(char *s)
{
  char *p1, *p2, tmp;

  // entry-condition 
  if (s == NULL) return s;

  // use temporary variables for processing
  p1 = s;
  p2 = s + _StrLen(s) - 1;

  // start at <-- and --> swapping until overlap or equality
  while (p2 > p1)
  {
    tmp   = *p1;
    *p1++ = *p2;
    *p2-- = tmp; // reverse the string
  }

  return s;
}

char *
_StrSet(char *s, char chr)
{
  // entry-condition 
  if (s == NULL) return s;

  return (char *)_MemSet((void *)s, chr, _StrLen(s));
}

char *
_StrStr(char *s, char *tok)
{
  return (char *)_StrNStr(s, tok, _StrLen(s));
}

char *
_StrStripMarkup(char *s, uint32 len)
{
  char   *result;
  char   *p, *b, *x; 
  uint32  size, cnt;

  // default return value
  result = NULL;

  // pre-condition (cannot have null pointer)
  if ((s == NULL) && (len > 0)) return result;
  
  // lets first find out how much we will reduce the string
  p    = s; 
  size = len;
  while (p != NULL)
  {
    p = _StrChr(p, '<');
    if (p != NULL)
    {
      x = _StrChr(p, '>');
      if (x != NULL) { size -= (uint32)((long)x - (long)p + 1); p = ++x; }
    }
  }
  size++;

  // allocate a new buffer
  result = (char *)_MemPtrNew(size, false);
  if (result != NULL)
  {
    // find the '<' and corresponding '>' tags, removing between
    p = s;
    b = result; 
    while (p != NULL)
    {
      p = _StrChr(s, '<');
      if (p != NULL)
      {
        cnt = (uint32)((long)p - (long)s);
        _StrNCopy(b, s, cnt); b += cnt;

        x = _StrChr(p, '>');
        if (x != NULL) p = ++x; s = p;
      }
      else
        _StrCopy(b, s);
    }
  }
 
  return result;
}

/********************************* EOF ***********************************/
