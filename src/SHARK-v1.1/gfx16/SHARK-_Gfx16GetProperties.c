/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_Gfx16GetProperties.c
 */

#include "../SHARK-prv.h"

void
_Gfx16GetProperties(void **bits, coord *width, coord *height, uint8 *depth)
{
  uint32 i, size;
  GLOBALS_ACCESS;

  // lets get the normal values first
  _LCDGetProperties(bits, width, height, depth);

  switch (*depth)
  {
    case 16:
         break;

    case 12: 
         // if we haven't allocated a buffer yet, lets do it now
         if (g->gfx16.bits == NULL)
         {
           size = (uint32)(*width) * (*height);
           g->gfx16.bits = _MemPtrNewX((uint32)(size << 1), false); // 16bpp buffer
           if (g->gfx16.bits != NULL)
           {
             i = size;
             do
             {
               *((uint16 *)g->gfx16.bits) = 0;
             } while (--i);

             *bits  = g->gfx16.bits;
             *depth = 16;
           }
         }
         else
         {
           *bits  = g->gfx16.bits;
           *depth = 16;
         }
         break;

    case 1:
    case 2:
    case 4:
    case 8:
         // unsupported screen depth - application should check against 16bpp
         _SysDebugMessage("_Gfx16GetProperties() - unsupported graphic display depth", false);
         break;
  }
}

/********************************* EOF ***********************************/
