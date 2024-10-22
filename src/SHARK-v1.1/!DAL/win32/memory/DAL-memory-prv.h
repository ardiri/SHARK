/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-memory-prv.h
 */

#include "../DAL-prv.h"

#ifndef DAL_MEMORY_PRV_H
#define DAL_MEMORY_PRV_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
 *
 *    Application Constants/Structures
 *
 *************************************************************************/

typedef struct Win32_MemInfo
{
  struct
  {
    uint32        total;
    uint32        warn;
    uint32        used;

    // usage reporting
    uint32        upper_limit;
  } storage;

  struct
  {
    uint32        total;
    uint32        warn;
    uint32        used;

    // usage reporting
    uint32        upper_limit;
  } heap;

  struct
  {
    // usage reporting
    uint32        max_chunks;
  } memory;

  boolean         warnings;

} Win32_MemInfo;

/*************************************************************************
 *
 * DAL Internal Use API's
 *
 *************************************************************************/

// none

#ifdef __cplusplus
}
#endif

#endif

/********************************* EOF ***********************************/
