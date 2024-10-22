/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_PlatformGetUniqueID.c
 */

#include "../DAL-prv.h"

void
DAL_PlatformGetUniqueID(uint8 *str)
{
  HKEY  hKey;
  DWORD dwDisposition, i, dwBufLen = 768;
  DWORD dwType = REG_BINARY;
  WCHAR szOwner[384];
  LONG  err;
  _MemSet((void *)szOwner, (uint16)dwBufLen, 0);

  // device owner information
  RegCreateKeyEx(HKEY_CURRENT_USER, _T("\\ControlPanel\\Owner"), 0, NULL, 0, 0, NULL, &hKey, &dwDisposition);
           err = RegQueryValueEx(hKey, _T("Owner"), 0, &dwType, (PBYTE)szOwner, &dwBufLen); // Pocket PC
  if (err) err = RegQueryValueEx(hKey, _T("Name"),  0, &dwType, (PBYTE)szOwner, &dwBufLen); // Smartphone
  for (i=0; i<0x24; i++)
    str[i] = (uint8)(szOwner[i] & 0xff);  // UNICODE --> ASCII

  RegCloseKey(hKey);
}

/********************************* EOF ***********************************/
