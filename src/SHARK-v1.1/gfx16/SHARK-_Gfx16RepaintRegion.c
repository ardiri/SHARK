/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_Gfx16RepaintRegion.c
 */

#include "../SHARK-prv.h"

void
_Gfx16RepaintRegion(rectangle *rect)
{
  uint32  i;
  uint16 *pS;
  GLOBALS_ACCESS;

  switch (g->device.display.depth)
  {
    case 16:
         // nothing to be done, already native access
         break;

    case 12:
         // convert 16bpp --> 12bpp
         if (g->gfx16.bits != NULL)
         {
           uint16 *pD;
           uint16  val;
           void   *bits;
           int     cnt, pix;
           int32   win_delta, scr_delta;
           coord   x1, y1, x2, y2;

           // we need the LCD pointer (12bpp)
           _LCDGetProperties(&bits, NULL, NULL, NULL);

           // whats a "valid" box area for this operation?
           x1 = MAX(0, rect->x);
           y1 = MAX(0, rect->y);
           x2 = MIN(g->device.display.width,  rect->x + rect->width);
           y2 = MIN(g->device.display.height, rect->y + rect->height);
        
           pS   = g->gfx16.bits;
           pD   = (uint16 *)bits;

           // do we still have a valid region?
           if ((x2 > x1) && (y2 > y1))
           {
             pS += (((uint32)y1 * g->device.display.width) + x1);
             pD += (((uint32)y1 * g->device.display.width) + x1);
          
             cnt = y2-y1;
             pix = x2-x1;
        
             win_delta = g->device.display.width - pix;
             scr_delta = g->device.display.width - pix;

             // do each line...
             do
             {
               i = pix;
    
               do
               {
                 val   = *pS++;
                 *pD++ = ((val & 0xf000) >> 4) | ((val & 0x0780) >> 3) | ((val & 0x001e) >> 1);
               } while (--i);

  
               pS += win_delta;
               pD += scr_delta;
             }
             while (--cnt);
           }
         }
         break;

    case 1:
    case 2:
    case 4:
    case 8:
         // unsupported screen depth - do nothing
         break;
  }

  _LCDRepaintRegion(rect);
}

/********************************* EOF ***********************************/
