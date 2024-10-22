/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_SysTimeLong.c
 */

#include "../DAL-prv.h"

LONGLONG
DAL_SysTimeLong()
{
  LONGLONG      t, tbase;
  SYSTEMTIME    st;
  FILETIME      ft;
  LARGE_INTEGER li;

  // current time
  GetSystemTime(&st);
  SystemTimeToFileTime(&st, &ft);
  li.LowPart  = ft.dwLowDateTime;
  li.HighPart = ft.dwHighDateTime;
  t           = li.QuadPart;

  // 1st Jan 1970
  _MemSet(&st, sizeof(st), 0);
  st.wYear  = 1970;
  st.wMonth = 1;
  st.wDay   = 1;
  SystemTimeToFileTime(&st, &ft);
  li.LowPart  = ft.dwLowDateTime;
  li.HighPart = ft.dwHighDateTime;
  tbase       = li.QuadPart;

  // difference between the two times?
  t -= tbase;

  return t;
}

/********************************* EOF ***********************************/
