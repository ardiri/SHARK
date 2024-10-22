/*************************************************************************
 *
 * Copyright (c) 2002-2003 MobileWizardry
 * All rights reservered.
 *
 *************************************************************************/

/*
 * @(#)series60/DAL-platform.inc
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
PlatformInitialize()
{
}

void
PlatformTerminate()
{
}

/*************************************************************************
 *
 * Platform Implementation
 *
 *************************************************************************/

uint16
PlatformGetResourceSize(uint16 resNum)
{
  uint16  result = 0;
  uint8   fileName[256];
  uint8   uniqID[5] = {};
  FILE   *f;

  // 
  // MODE#1   /System/Apps/app-{crid}/xxxx{resNum}.bin
  //

  // convert uniqueID to 4 chars
  uniqID[0] = (ApplicationGetUniqueID() >> 24) & 0xFF;
  uniqID[1] = (ApplicationGetUniqueID() >> 16) & 0xFF;
  uniqID[2] = (ApplicationGetUniqueID() >> 8 ) & 0xFF;
  uniqID[3] = (ApplicationGetUniqueID()      ) & 0xFF;
  uniqID[4] = 0;

  // generate the filename of where the resource will be stored
    sprintf((char *)fileName, "C:\\System\\Apps\\app-%s\\xxxx%04x.bin", uniqID, resNum);  // ram
  f = fopen((char *)fileName, "rb");
  if (f == NULL)
  {
    sprintf((char *)fileName, "E:\\System\\Apps\\app-%s\\xxxx%04x.bin", uniqID, resNum);  // storage card
    f = fopen((char *)fileName, "rb");
  }

  // 
  // MODE#2   /Common/xxxx{resNum}.bin
  //

  if (f == NULL)
  {
      sprintf((char *)fileName, "C:\\Common\\xxxx%04x.bin", resNum);                      // ram
    f = fopen((char *)fileName, "rb");
    if (f == NULL)
    {
      sprintf((char *)fileName, "E:\\Common\\xxxx%04x.bin", resNum);                      // storage card
      f = fopen((char *)fileName, "rb");
    }
  }

  // try to open the resource file (cross fingers)
  if (f != NULL)
  {
    // determine the size of the buffer
    fseek(f, 0, SEEK_END);
    result = (uint16)ftell(f);
    fseek(f, 0, SEEK_SET);

    // close the file :)
    fclose(f);
  }

  return result;
}

void *
PlatformGetResource(uint16 resNum)
{
  void   *result = NULL;
  uint8   fileName[256];
  uint8   uniqID[5] = {};
  FILE   *f;
  uint16  size;

  // 
  // MODE#1   /System/Apps/app-{crid}/xxxx{resNum}.bin
  //

  // convert uniqueID to 4 chars
  uniqID[0] = (ApplicationGetUniqueID() >> 24) & 0xFF;
  uniqID[1] = (ApplicationGetUniqueID() >> 16) & 0xFF;
  uniqID[2] = (ApplicationGetUniqueID() >> 8 ) & 0xFF;
  uniqID[3] = (ApplicationGetUniqueID()      ) & 0xFF;
  uniqID[4] = 0;

  // generate the filename of where the resource will be stored
    sprintf((char *)fileName, "C:\\System\\Apps\\app-%s\\xxxx%04x.bin", uniqID, resNum);  // ram
  f = fopen((char *)fileName, "rb");
  if (f == NULL)
  {
    sprintf((char *)fileName, "E:\\System\\Apps\\app-%s\\xxxx%04x.bin", uniqID, resNum);  // storage card
    f = fopen((char *)fileName, "rb");
  }

  // 
  // MODE#2   Common/xxxx{resNum}.bin
  //

  if (f == NULL)
  {
      sprintf((char *)fileName, "C:\\Common\\xxxx%04x.bin", resNum);                      // ram
    f = fopen((char *)fileName, "rb");
    if (f == NULL)
    {
      sprintf((char *)fileName, "E:\\Common\\xxxx%04x.bin", resNum);                      // storage card
      f = fopen((char *)fileName, "rb");
    }
  }

  // try to open the resource file (cross fingers)
  if (f != NULL)
  {
    // determine the size of the buffer
    fseek(f, 0, SEEK_END);
    size = (uint16)ftell(f);
    fseek(f, 0, SEEK_SET);

    // allocate buffer large enough for it, and, read it in
    result = malloc(size);
    if (result != NULL)
      fread(result, 1, size, f);

    // close the file :)
    fclose(f);
  }

  return result;
}

void
PlatformReleaseResource(void *ptr)
{
  // its just a pointer, free it :)
  if (ptr != NULL) free(ptr);
}

/********************************* EOF ***********************************/
