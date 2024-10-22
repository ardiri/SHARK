/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_PlatformKillPreference.c
 */

#include "../DAL-prv.h"

void
DAL_PlatformKillPreference(uint16 index)
{
#ifdef PLT_REGISTRY_PREFERENCES
  uint8  keyName[64];
  uint16 *keyNameU;
#else
  uint8  fileName[64];
  uint16 *buf;
#endif

#ifdef PLT_REGISTRY_PREFERENCES
  sprintf(keyName, "Software\\Mobile Wizardry\\%s\\Preferences\\SLOT-%d", ApplicationGetName(), index);

  // delete the registry key
  keyNameU = DAL_SysCreateUnicodeString(keyName);
  RegDeleteKey(HKEY_CURRENT_USER, keyNameU);
  _MemPtrFree(keyNameU);
#else
  sprintf(fileName, "%s-preferences-SLOT-%d.dat", ApplicationGetName(), index);

  // remove the file 
  buf = DAL_SysCreateUnicodeString(fileName);
  DeleteFile((const uint16 *)buf);
  _MemPtrFree(buf);
#endif
}

/********************************* EOF ***********************************/
