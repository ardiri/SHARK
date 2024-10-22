/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_PlatformGetResourceSize.c
 */

#include "../DAL-prv.h"

uint16
DAL_PlatformGetResourceSize(uint16 resNum)
{
  Win32_PlatformInfo *platform;
  uint32              result;
  HRSRC               res;
  uint8               fN[256];
  FILE               *f;
  GLOBALS_ACCESS;

  // default return value
  result = 0;
  res    = NULL;

  // 
  // MODE #1 application resource          (RCDATA)
  //

  // access device/platform information block
  platform = (Win32_PlatformInfo *)g->platform;
  if (platform != NULL)
  {  
    res = FindResource(platform->windows.hInst, MAKEINTRESOURCE(resNum), RT_RCDATA);
    if (res != NULL) 
      result = SizeofResource(platform->windows.hInst, res);
  }
  
  // 
  // MODE#2   ./xxxx{resNum}.bin (relative directory)
  //

  if (result == 0)
  {
    sprintf((char *)fN, ".\\xxxx%04x.bin", resNum);

    f = fopen((char *)fN, "rb");
    if (f != NULL)
    {
      // determine the size of the buffer
      fseek(f, 0, SEEK_END);
      result = ftell(f);
  
      // close the file :)
      fclose(f);
    }
  }

  // 
  // MODE#3   /Common/xxxx{resNum}.bin     (external memory card)
  //

  if (result == 0)
  {
    sprintf((char *)fN, "\\Common\\xxxx%04x.bin", resNum);

    f = fopen((char *)fN, "rb");
    if (f != NULL)
    {
      // determine the size of the buffer
      fseek(f, 0, SEEK_END);
      result = ftell(f);
  
      // close the file :)
      fclose(f);
    }
  }

  // 
  // MODE#4   /Program/{application}/xxxx{resNum}.bin     (external memory card)
  //

  if (result == 0)
  {
    sprintf((char *)fN, "\\Program\\%s\\xxxx%04x.bin", ApplicationGetName(), resNum);

    f = fopen((char *)fN, "rb");
    if (f != NULL)
    {
      // determine the size of the buffer
      fseek(f, 0, SEEK_END);
      result = ftell(f);
  
      // close the file :)
      fclose(f);
    }
  }

  // validation - we cannot handle resources > 64000 bytes
  if (result > 64000)
  {
    result = 0;
    _SysDebugMessage("PlatformGetResourceSize()\n\nERROR: resource is > 64000 bytes.", true);
  }

  return (uint16)(result & 0xffff);
}

/********************************* EOF ***********************************/
