/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_GfxSetPalette.c
 */

#include "../SHARK-prv.h"

// #define GFX_GRAY_INVERT         1       // should grey be inverted?

#define __byte_swap16(n) \
        ( ((((uint16) n) << 8) & 0xFF00) | \
          ((((uint16) n) >> 8) & 0x00FF) )

void
_GfxSetPalette(uint8 *rgb)
{
  uint8 *_p;
  uint8  _r, _g, _b;
  int    i;
  GLOBALS_ACCESS;

  // entry conditions, cannot have NULL pointers
  if ((rgb == NULL) || (g->gfx.palette == NULL)) return;

  // copy over palette entries
  _MemSemaphore(true);
  _MemMove(g->gfx.palette, rgb, (PALETTE_SIZE * 3));
  _MemSemaphore(false);

// PALETTE NOTES: this is an interesting concept to understand
//
//  8bpp = can have 256x256x256 colors to choose from, but max 256 at once
// 12bpp = can have 16x16x16 colors to choose from, all possible at once
// 16bpp = can have 32x64x32 colors to choose from, all possible at once
// 24bpp = the only mode that can match 8bpp custom palette for quality.

  // we need to generate lookup tables for this stuff
  _MemSemaphore(true);
  switch (g->device.display.depth)
  {
    case 1:
         {
           uint8 *p;

           _p  = rgb;
            p  = (uint8 *) g->gfx._indexed;
           for (i=0; i<PALETTE_SIZE; i++)
           {
             uint8  v;
             uint32 _g;
             uint8 min, max;
  
             min = *_p; 
             max = *_p++;
             if (*_p < min) min = *_p;
             if (*_p > max) max = *_p; _p++;
             if (*_p < min) min = *_p;
             if (*_p > max) max = *_p; _p++;
             _g = (max + min) >> 1;

             // map to 0..1 value  (160/256 = 62.5% gray for it to be white)
#ifdef GFX_GRAY_INVERT
             if (_g > 160) v = 1; else v = 0; // inverted (possibly better for b+w screens)
#else
             if (_g > 160) v = 0; else v = 1; // true gray
#endif

             *p++ = (uint8)(v << 7);
             *p++ = (uint8)(v << 6);
             *p++ = (uint8)(v << 5);
             *p++ = (uint8)(v << 4);
             *p++ = (uint8)(v << 3);
             *p++ = (uint8)(v << 2);
             *p++ = (uint8)(v << 1);
             *p++ = (uint8) v;
           }
         }
         break;

    case 2:
         {
           uint8 *p;

           _p  = rgb;
            p  = (uint8 *) g->gfx._indexed;
           for (i=0; i<PALETTE_SIZE; i++)
           {
             uint8  v;
             uint32 _g;
             uint8 min, max;
  
             min = *_p; 
             max = *_p++;
             if (*_p < min) min = *_p;
             if (*_p > max) max = *_p; _p++;
             if (*_p < min) min = *_p;
             if (*_p > max) max = *_p; _p++;
             _g = (max + min) >> 1;

             // map to 0..3 value
#ifdef GFX_GRAY_INVERT
             v = ((uint8)_g >> 6);      // inverted (possibly better for b+w screens)
#else
             v = 3 - (uint8)(_g >> 6);  // true gray
#endif

             *p++ = (uint8)((v & 0x03) << 6);
             *p++ = (uint8)((v & 0x03) << 4);
             *p++ = (uint8)((v & 0x03) << 2);
             *p++ = (uint8) (v & 0x03);
           }
         }
         break;

    case 4:
         {
           uint8 *p;

           _p  = rgb;
            p  = (uint8 *) g->gfx._indexed;
           for (i=0; i<PALETTE_SIZE; i++)
           {
             uint8  v;
             uint32 _g;
             uint8 min, max;
  
             min = *_p; 
             max = *_p++;
             if (*_p < min) min = *_p;
             if (*_p > max) max = *_p; _p++;
             if (*_p < min) min = *_p;
             if (*_p > max) max = *_p; _p++;
             _g = (max + min) >> 1;

             // map to 0..15 value
#ifdef GFX_GRAY_INVERT
             v = ((uint8)_g >> 4);       // inverted (possibly better for b+w screens)
#else
             v = 15 - (uint8)(_g >> 4);  // true gray
#endif

             *p++ = (uint8)((v & 0x0f) << 4);
             *p++ = (uint8) (v & 0x0f);
           }
         }
         break;

    case 8:
         // no look up needed, but we do need to set LCD palette
         _LCDSetPalette(g->gfx.palette);
         break;

    case 12:
         {
           uint16 *p;

           _p = rgb;
            p = g->gfx._direct;
           for (i=0; i<PALETTE_SIZE; i++)
           {
             _r = *_p++;
             _g = *_p++;
             _b = *_p++;
  
             *p = (((uint16)(_r & 0xF0) << 4) |   // 0000111100000000
                    (uint16)(_g & 0xF0)       |   // 0000000011110000
                   ((uint16)(_b & 0xF0) >> 4));   // 0000000000001111
#if ((defined(_M_68K) || defined(_M_ARM)) && defined(__PALMOS__) && (!defined(__TAPWAVE__)))
             *p = __byte_swap16(*p);
#endif
             p++;
           }
         }
         break;  

    case 16:
         {
           uint16 *p;

           _p = rgb;
            p = g->gfx._direct;
           for (i=0; i<PALETTE_SIZE; i++)
           {
             _r = *_p++;
             _g = *_p++;
             _b = *_p++;

/**
//- 565  
             *p = (((uint16)(_r & 0xF8) << 8) |   // 1111100000000000
                   ((uint16)(_g & 0xFC) << 3) |   // 0000011111100000
                   ((uint16)(_b & 0xF8) >> 3));   // 0000000000011111
//- 555  
             *p = (((uint16)(_r & 0xF8) << 7) |   // 0111110000000000
                   ((uint16)(_g & 0xF8) << 3) |   // 0000001111100000
                   ((uint16)(_b & 0xF8) >> 3));   // 0000000000011111
**/
             *p = (((uint16)(_r & 0xF8) << 8) |   // 1111100000000000
                   ((uint16)(_g & 0xFC) << 3) |   // 0000011111100000
                   ((uint16)(_b & 0xF8) >> 3));   // 0000000000011111
#if ((defined(_M_68K) || defined(_M_ARM)) && defined(__PALMOS__) && (!defined(__TAPWAVE__)))
             *p = __byte_swap16(*p);
#endif
             p++;
           }
         }
         break;  

    default:
         break;
  }
  _MemSemaphore(false);
}

/********************************* EOF ***********************************/
