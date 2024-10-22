/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_LCDInitialize.c
 */

#include "../DAL-prv.h"

boolean
DAL_LCDInitialize()
{
  Win32_PlatformInfo *platform;
  BITMAPINFO         *pbmi;
  uint32              i, j, size;
  uint16             *p;
  uint8               val, _r, _g, _b;
  boolean             init_ok;
  GLOBALS_ACCESS;

  // LOGGING EVENT
  LOG_ENTRY(">> LCDInitialize");

  // assume everything is ok
  init_ok = true;

  // verify the device + platform structures are valid
  platform = (Win32_PlatformInfo *)g->platform;
  init_ok = ((platform != NULL)); if (!init_ok) goto LCD_INIT_ABORT;

  pbmi = (BITMAPINFO *)&platform->display.bmi.bmiHeader;
  pbmi->bmiHeader.biClrImportant  = 0;
  pbmi->bmiHeader.biClrUsed       = 0;
  pbmi->bmiHeader.biCompression   = BI_BITFIELDS;
  pbmi->bmiHeader.biPlanes        = 1;
  pbmi->bmiHeader.biBitCount      = 16;
  pbmi->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
  pbmi->bmiHeader.biHeight        = -(LONG)(g->device.display.height);
  pbmi->bmiHeader.biWidth         =  (LONG)(g->device.display.width);
  pbmi->bmiHeader.biSizeImage     = ((((((LONG)(g->device.display.width) * pbmi->bmiHeader.biBitCount) + 31) & ~(31)) / 8) * (LONG)(g->device.display.height));
  pbmi->bmiHeader.biXPelsPerMeter = 0;
  pbmi->bmiHeader.biYPelsPerMeter = 0;

  // configure the bitmap to use the currently active palette (important)
  platform->display.bmi.bmiColors[0] = 0x0000f800;
  platform->display.bmi.bmiColors[1] = 0x000007e0;
  platform->display.bmi.bmiColors[2] = 0x0000001f;

  platform->display.dc  = CreateCompatibleDC(platform->windows.LCDScreen);
  platform->display.bmp = CreateDIBSection(platform->display.dc, pbmi, DIB_RGB_COLORS, ((void **)(&platform->display.bits)), NULL, 0);
  SelectObject(platform->display.dc, platform->display.bmp);

  // initialize window to white (or, report error)
  if ((platform->display.dc == NULL) || (platform->display.bmp == NULL) || (platform->display.bits == NULL))
  {
    _SysDebugMessage("DAL_LCDInitialize()\n\nERROR: unable to create base window", true);
    init_ok = false; goto LCD_INIT_ABORT;
  }

  // fill the window with white color
  size = g->device.display.width * g->device.display.height * 2; // 16bpp
  SYSTEM_MemSet(platform->display.bits, size, 0xff);

  // lets allocate memory for our device window
  if (g->device.display.depth != 16)
  {
    size = (g->device.display.width * g->device.display.height / (16 / g->device.display.depth)) * 2;
    g->device.display.bits = (void *)_MemPtrNewX(size, false);
    if (g->device.display.bits == NULL)
    {
      _SysDebugMessage("GlobalsInitialize()\n\nERROR: unable to create device window", true);
      init_ok = false; goto LCD_INIT_ABORT;
    }
    SYSTEM_MemSet(g->device.display.bits, size, (g->device.display.depth > 8) ? 0xff : 0x00);
  }
  else
    g->device.display.bits = NULL; // reuse the same buffer

  // allocate palette references (only for indexed graphics)
  if (g->device.display.depth <= 8)
  {
    g->device.display.palette = (void *)_MemPtrNew((uint16)(256 * sizeof(uint16) * (8 / g->device.display.depth)), false);
    if (g->device.display.palette == NULL)
    {
      _SysDebugMessage("DAL_LCDInitialize()\n\nERROR: unable to create palette lookup table", true);
      init_ok = false; goto LCD_INIT_ABORT;
    }
  }

  // generate lookup tables
  switch (g->device.display.depth)
  {
    case 1:
         p = g->device.display.palette;
         for (j=0; j<256; j++)
         {
           val = (uint8)(j & 0xff);
           i   = 8;
           do
           {
                  if ((val & 0x80) == 0x80) { _r = _g = _b = 0;   }
             else                           { _r = _g = _b = 255; }  // simplicity

             *p++ = (((uint16)(_r & 0xF8) << 8) |   // 1111100000000000
                     ((uint16)(_g & 0xFC) << 3) |   // 0000011111100000
                     ((uint16)(_b & 0xF8) >> 3));   // 0000000000011111   5-6-5 display

             val <<= 1;
           } while (--i);
         }
         break;

    case 2:
         p = g->device.display.palette;
         for (j=0; j<256; j++)
         {
           val = (uint8)(j & 0xff);
           i   = 4;
           do
           {
                  if ((val & 0xC0) == 0xC0) { _r = _g = _b = 0;   }
             else if ((val & 0x80) == 0x80) { _r = _g = _b = 64;  }
             else if ((val & 0x40) == 0x40) { _r = _g = _b = 192; }
             else                           { _r = _g = _b = 255; } // simplicity

             *p++ = (((uint16)(_r & 0xF8) << 8) |   // 1111100000000000
                     ((uint16)(_g & 0xFC) << 3) |   // 0000011111100000
                     ((uint16)(_b & 0xF8) >> 3));   // 0000000000011111

             val <<= 2;
           } while (--i);
         }
         break;

    case 4:
         p = g->device.display.palette;
         for (j=0; j<256; j++)
         {
           val = (uint8)(j & 0xff);
           i   = 2;
           do
           {
                  if ((val & 0xF0) == 0xF0) { _r = _g = _b = 0;   }
             else if ((val & 0xE0) == 0xE0) { _r = _g = _b = 17;  }
             else if ((val & 0xD0) == 0xD0) { _r = _g = _b = 34;  }
             else if ((val & 0xC0) == 0xC0) { _r = _g = _b = 51;  }
             else if ((val & 0xB0) == 0xB0) { _r = _g = _b = 68;  }
             else if ((val & 0xA0) == 0xA0) { _r = _g = _b = 85;  }
             else if ((val & 0x90) == 0x90) { _r = _g = _b = 102; }
             else if ((val & 0x80) == 0x80) { _r = _g = _b = 119; }
             else if ((val & 0x70) == 0x70) { _r = _g = _b = 136; }
             else if ((val & 0x60) == 0x60) { _r = _g = _b = 153; }
             else if ((val & 0x50) == 0x50) { _r = _g = _b = 170; }
             else if ((val & 0x40) == 0x40) { _r = _g = _b = 187; }
             else if ((val & 0x30) == 0x30) { _r = _g = _b = 204; }
             else if ((val & 0x20) == 0x20) { _r = _g = _b = 221; }
             else if ((val & 0x10) == 0x10) { _r = _g = _b = 238; }
             else                           { _r = _g = _b = 255; } // simplicity

             *p++ = (((uint16)(_r & 0xF8) << 8) |   // 1111100000000000
                     ((uint16)(_g & 0xFC) << 3) |   // 0000011111100000
                     ((uint16)(_b & 0xF8) >> 3));   // 0000000000011111

             val <<= 4;
           } while (--i);
         }
         break;

    case 8:
         {
#include "../../../SHARK-gfx-palette.inc"

           DAL_LCDSetPalette(sys_palette);
         }
         break;

    default:
         break;
  }  
  
  // everything ok?
  goto LCD_INIT_SUCCESS;

LCD_INIT_ABORT:

  // clean up memory usage within memory management system
  if (g->device.display.bits != NULL)    _MemPtrFree(g->device.display.bits);
  if (g->device.display.palette != NULL) _MemPtrFree(g->device.display.palette);
  if (platform->display.bmp != NULL)     DeleteObject(platform->display.bmp);
  if (platform->display.dc != NULL)      DeleteDC(platform->display.dc);
  g->device.display.bits    = NULL;
  g->device.display.palette = NULL;
  platform->display.bmp     = NULL;
  platform->display.dc      = NULL;

LCD_INIT_SUCCESS:

  return init_ok;
}

/********************************* EOF ***********************************/
