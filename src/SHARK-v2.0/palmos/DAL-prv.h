/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-prv.h
 */

// -----------------------------------------------------------------------
// system includes 
// -----------------------------------------------------------------------

// SHARK includes
#include "../SHARK.h"
#include "../SHARK-prv.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define malloc(x)     MemChunkNew(0, x, 0x1100)  // memNewChunkFlagPreLock    
                                                 // memNewChunkFlagAllowLarge 
#define free(x)       MemChunkFree(x)
#define memset(p,v,s) MemSet(p,s,v)
#define sprintf       StrPrintF

#include "PACE/PACE.h"

#include "resources/resource.h"

/*************************************************************************
 *
 * Global Variables
 *
 *************************************************************************/

typedef struct
{
  // common shared information
  boolean  initialized;

  uint8    lcd_a;
  uint8    lcd_r;
  uint8    lcd_g;
  uint8    lcd_b;
  uint8    lcd_r_mask;
  uint8    lcd_g_mask;
  uint8    lcd_b_mask;

  coord    width;
  coord    width_rb;
  coord    height;
  coord    height_rb;

  char     path[256];
  uint16   path_vol;

  application_config config;

  // module global areas
  void    *lcd;
  void    *mem;
  void    *lib;
  void    *sys;
  void    *snd;
  void    *file;
  void    *pref;

  // platform specific information (needed at runtime)
  struct
  {
    UInt32 osVer;
    UInt32 comID;
    UInt32 devID;

    UInt32 pin_version;
    UInt32 scr_density;
    
    UInt32 mem_dynheap_available;
    UInt32 mem_storage_available;
  } ftr;

  struct
  {
    void   *_68k;
    void   *_emu;     // PACE callback interface

    boolean snd_api;  // use PACE for SndStream*
    boolean dia_api;  // use PACE for PIN*
  } PACE;

} GlobalsType;

register GlobalsType *g   asm ("r8");
register void        *GOT asm ("r10");

/********************************* EOF ***********************************/
