/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-types.h
 */

#ifndef SHARK_TYPES_H
#define SHARK_TYPES_H

#include "../SHARK.h"

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
 *
 * Constants / Structures
 *
 *************************************************************************/

// common constants
#define false 0
#define true  (!false)
#ifndef NULL
#define NULL  (void *)0
#endif

// generic "integer" - best for loop counters, generics
typedef unsigned int       uint;

// specific sized integers
typedef   signed char      int8;
typedef unsigned char      uint8;
typedef   signed short     int16;
typedef unsigned short     uint16;
typedef   signed long      int32;
typedef unsigned long      uint32;

// miscellaneous
typedef unsigned char      boolean;
typedef   signed short     coord;

typedef struct rectangle
{
  coord x;
  coord y;
  coord width;
  coord height;
} rectangle;

// helper macros
#define ABS(a)   ((a) < 0 ? -(a) : (a))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

/*************************************************************************
 *
 * Application Programmers Interface
 *
 *************************************************************************/

// device specific
// none

// device independent
// none

#ifdef __cplusplus
}
#endif

#endif

/********************************* EOF ***********************************/
