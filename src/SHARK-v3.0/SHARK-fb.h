/*************************************************************************
 *
 * Copyright 2001+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-fb.h
 */

#ifndef SHARK_FB_H
#define SHARK_FB_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
 * SHARK: frame buffer
 *************************************************************************
 *
 * FRAME BUFFER
 * -------------
 *
 * A SHARK library providing elementary two-dimensional graphics 
 * functionality using a frame buffer (memory region) implementation.
 *
 * The library implements either an index based paletted graphics or 
 * direct color implementation, acting as a common layer between the 
 * application and the underlying hardware display engine.
 *
 * FrameBuffer modes:
 *
 * - 8bit index based paletted (256 colors)
 * - 16bit direct color (444, 555, 565 xRGB)
 *
 *************************************************************************/

//------------------------------------------------------------------------
// constants and enumerations
//------------------------------------------------------------------------

typedef enum fb_type
{
  fb_palette,
  fb_direct
} fb_type;

//------------------------------------------------------------------------
// structures and datatypes 
//------------------------------------------------------------------------

typedef struct fb_format
{
  fb_type  type;

  uint16   depth;
  union
  {
    struct
    {
      uint8  red;
      uint8  green;
      uint8  blue;
    } direct;
  } data;
} fb_format;

typedef uint32 color;

#define fb_region lcd_region

//------------------------------------------------------------------------
// application programmer interface
//------------------------------------------------------------------------

#define FB444GetColor(r,g,b,s) \
  (((((color)(r) & 0xf0) << 4) |      /* 0000111100000000: R */ \
    (((color)(g) & 0xf0)     ) |      /* 0000000011110000: G */ \
    (((color)(b) & 0xf0) >> 4)) << s) /* 0000000000001111: B */  

#define FBColorGet444(c,r,g,b,s) \
  {                                                                  \
    color x = (c) >> (s);                                            \
    (r) = (uint8)((((((x & 0x0f00) + 0x0100)     ) - 1) >> 4) - 7);  \
    (g) = (uint8)((((((x & 0x00f0) + 0x0010) << 4) - 1) >> 4) - 7);  \
    (b) = (uint8)((((((x & 0x000f) + 0x0001) << 8) - 1) >> 4) - 7);  \
                                                                     \
    if (((uint16)x & 0x000f) == 0x0000) (b) = 0;                     \
    if (((uint16)x & 0x000f) == 0x000f) (b) = 255;                   \
    if (((uint16)x & 0x00f0) == 0x0000) (g) = 0;                     \
    if (((uint16)x & 0x00f0) == 0x00f0) (g) = 255;                   \
    if (((uint16)x & 0x0f00) == 0x0000) (r) = 0;                     \
    if (((uint16)x & 0x0f00) == 0x0f00) (r) = 255;                   \
  }

#define FB555GetColor(r,g,b,s) \
  (((((color)(r) & 0xf8) << 7) |      /* 0111110000000000: R */ \
    (((color)(g) & 0xf8) << 2) |      /* 0000001111100000: G */ \
    (((color)(b) & 0xf8) >> 3)) << s) /* 0000000000011111: B */  

#define FBColorGet555(c,r,g,b,s) \
  {                                                                  \
    color x = (c) >> (s);                                            \
    (r) = (uint8)((((((x & 0x7c00) + 0x0400) >> 2) - 1) >> 5) - 3);  \
    (g) = (uint8)((((((x & 0x03c0) + 0x0020) << 3) - 1) >> 5) - 3);  \
    (b) = (uint8)((((((x & 0x001f) + 0x0001) << 8) - 1) >> 5) - 3);  \
                                                                     \
    if (((uint16)x & 0x001f) == 0x0000) (b) = 0;                     \
    if (((uint16)x & 0x001f) == 0x001f) (b) = 255;                   \
    if (((uint16)x & 0x03c0) == 0x0000) (g) = 0;                     \
    if (((uint16)x & 0x03c0) == 0x03c0) (g) = 255;                   \
    if (((uint16)x & 0x7c00) == 0x0000) (r) = 0;                     \
    if (((uint16)x & 0x7c00) == 0x7c00) (r) = 255;                   \
  }

#define FB565GetColor(r,g,b,s) \
  (((((color)(r) & 0xf8) << 8) |      /* 1111100000000000: R */ \
    (((color)(g) & 0xfc) << 3) |      /* 0000011111100000: G */ \
    (((color)(b) & 0xf8) >> 3)) << s) /* 0000000000011111: B */

#define FBColorGet565(c,r,g,b,s) \
  {                                                                  \
    color x = (c) >> (s);                                            \
    (r) = (uint8)((((((x & 0xf800) + 0x0800) >> 3) - 1) >> 5) - 3);  \
    (g) = (uint8)((((((x & 0x07e0) + 0x0020) << 3) - 1) >> 6) - 1);  \
    (b) = (uint8)((((((x & 0x001f) + 0x0001) << 8) - 1) >> 5) - 3);  \
                                                                     \
    if (((uint16)x & 0x001f) == 0x0000) (b) = 0;                     \
    if (((uint16)x & 0x001f) == 0x001f) (b) = 255;                   \
    if (((uint16)x & 0x07e0) == 0x0000) (g) = 0;                     \
    if (((uint16)x & 0x07e0) == 0x07e0) (g) = 255;                   \
    if (((uint16)x & 0xf800) == 0x0000) (r) = 0;                     \
    if (((uint16)x & 0xf800) == 0xf800) (r) = 255;                   \
  }

#define FB888GetColor(r,g,b,s) \
  ((((color)(r) << 16) | ((color)(g) <<  8) | ((color)(b))) << s)   

#define FBColorGet888(c,r,g,b,s) \
  {                                                                  \
    color x = (c) >> (s);                                            \
    (r) = (uint8)((x & 0x0f00) >> 16);                               \
    (g) = (uint8)((x & 0x00f0) >>  8);                               \
    (b) = (uint8)((x & 0x000f)      );                               \
  }

extern boolean
_FBInitialize(fb_format *format);

extern void
_FBClear();

extern void
_FBGetPalette(uint8 *rgb);

extern color
_FBGetPaletteIndex(uint8 r, uint8 g, uint8 b);

extern void
_FBGetProperties(void **bits, uint16 *width, uint16 *height, uint16 *rowBytes);

extern void
_FBGetRGBFromIndex(color c, uint8 *r, uint8 *g, uint8 *b);

extern void
_FBRepaint();

extern void
_FBRepaintRegion(fb_region *region);

extern void
_FBSetPalette(uint8 *rgb);

extern void
_FBTerminate();

#endif

/********************************* EOF ***********************************/
