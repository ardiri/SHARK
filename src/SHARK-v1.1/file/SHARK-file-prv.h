/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-file-prv.h
 */

#ifndef SHARK_FILE_PRV_H
#define SHARK_FILE_PRV_H

#include "../SHARK.h"

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
 *
 * Module Constants/Structures
 *
 *************************************************************************/

typedef struct _file
{
  uint32  location;
  int8    fileName[256];
  void   *fp;
} _file;

/*************************************************************************
 *
 * Initialization / Termination
 *
 *************************************************************************/

#define _FileInitialize DAL_FileInitialize
#define _FileTerminate  DAL_FileTerminate

/*************************************************************************
 *
 * SHARK Internal Use API's
 *
 *************************************************************************/

// NONE

#ifdef __cplusplus
}
#endif

#endif

/********************************* EOF ***********************************/
