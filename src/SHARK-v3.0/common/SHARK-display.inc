/*************************************************************************
 *
 * Copyright 2001+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-display.inc
 */

/*************************************************************************
 * Configuration
 *************************************************************************/

// none

/*************************************************************************
 * Component Variables
 *************************************************************************/

typedef struct LCDGlobals
{
  void *struct_prefix;

  lcd_orientation lcd_orientation;
  lcd_dpi         lcd_dpi;

  uint16          lcd_type;                
  uint16          lcd_depth;                
  uint16          lcd_depth_bus;                

  uint8           lcd_a;
  uint8           lcd_r;
  uint8           lcd_g;
  uint8           lcd_b;
  uint32          lcd_a_mask;
  uint32          lcd_r_mask;
  uint32          lcd_g_mask;
  uint32          lcd_b_mask;

  boolean         lcd_inverted;

  void           *lcd_framebuffer;
  uint16          lcd_width;
  uint16          lcd_width_rb;
  uint16          lcd_height;
  uint16          lcd_height_rb;

  boolean         lcd_doublebuffered;
  boolean         lcd_requirelock;
  void           *lcd_framebuffer_dir;

  void          (*fnLCDRepaintRegion)(lcd_region *region);

#if defined(DAL_LCD_GLOBALS)
  DAL_LCD_GLOBALS
#endif

} LCDGlobals;

GLOBALS_DEFINITION;

#define g_lcd_orientation      (LCD_GLOBALS) -> lcd_orientation
#define g_lcd_dpi              (LCD_GLOBALS) -> lcd_dpi

#define g_lcd_type             (LCD_GLOBALS) -> lcd_type
#define g_lcd_depth            (LCD_GLOBALS) -> lcd_depth
#define g_lcd_depth_bus        (LCD_GLOBALS) -> lcd_depth_bus

#define g_lcd_a                (LCD_GLOBALS) -> lcd_a
#define g_lcd_r                (LCD_GLOBALS) -> lcd_r
#define g_lcd_g                (LCD_GLOBALS) -> lcd_g
#define g_lcd_b                (LCD_GLOBALS) -> lcd_b
#define g_lcd_a_mask           (LCD_GLOBALS) -> lcd_a_mask
#define g_lcd_r_mask           (LCD_GLOBALS) -> lcd_r_mask
#define g_lcd_g_mask           (LCD_GLOBALS) -> lcd_g_mask
#define g_lcd_b_mask           (LCD_GLOBALS) -> lcd_b_mask

#define g_lcd_inverted         (LCD_GLOBALS) -> lcd_inverted

#define g_lcd_framebuffer      (LCD_GLOBALS) -> lcd_framebuffer
#define g_lcd_width            (LCD_GLOBALS) -> lcd_width
#define g_lcd_width_rb         (LCD_GLOBALS) -> lcd_width_rb
#define g_lcd_height           (LCD_GLOBALS) -> lcd_height
#define g_lcd_height_rb        (LCD_GLOBALS) -> lcd_height_rb

#define g_lcd_doublebuffered   (LCD_GLOBALS) -> lcd_doublebuffered
#define g_lcd_requirelock      (LCD_GLOBALS) -> lcd_requirelock
#define g_lcd_framebuffer_dir  (LCD_GLOBALS) -> lcd_framebuffer_dir

#define fnLCDRepaintRegion     (LCD_GLOBALS) -> fnLCDRepaintRegion

#if (LOG_DISABLE == 0)
// DPI: constant representation possible values
const uint8 g_lcd_dpi_consts[4] = { 72, 96, 144, 192 };
#endif

/*************************************************************************
 * Platform Dependent Interface
 *************************************************************************/

static boolean DAL_LCDInitialize(uint16, uint16);
static void    DAL_LCDInitialize_RESIZE(uint16, uint16);
static void    DAL_LCDTerminate_RESIZE();
static void    DAL_LCDFlipDisplay();
static void   *DAL_LCDLock();
static void    DAL_LCDResume();
static void    DAL_LCDSuspend();
static void    DAL_LCDUnlock();
static void    DAL_LCDTerminate();

/*************************************************************************
 * Implementation
 *************************************************************************/

static boolean
_LCDInitialize(uint16 width, uint16 height)
{
  boolean init_ok;

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _SHARKLogCreateEntry(">> _LCDInitialize\n");
#endif

  // allocate memory for globals
  LCD_GLOBALS = malloc(sizeof(LCDGlobals));
  if (GLOBALS_UNINITIALIZED) return false;

  // assume everything is ok
  init_ok =  true;
  init_ok &= DAL_LCDInitialize(width, height);

  // did we initialize ok?
  if (init_ok)
  {
    // lets configure the correct blitting engine
    switch (g_lcd_depth_bus)
    {
      case 1:  fnLCDRepaintRegion = _LCDRepaintRegion_1;  break; // T.B.D
      case 2:  fnLCDRepaintRegion = _LCDRepaintRegion_2;  break; // T.B.D
      case 4:  fnLCDRepaintRegion = _LCDRepaintRegion_4;  break; // T.B.D
      case 8:  fnLCDRepaintRegion = _LCDRepaintRegion_8;  break;
      case 12: fnLCDRepaintRegion = _LCDRepaintRegion_12; break; // T.B.D
      case 16: fnLCDRepaintRegion = _LCDRepaintRegion_16; break;
      case 24: fnLCDRepaintRegion = _LCDRepaintRegion_24; break; // T.B.D
      case 32: fnLCDRepaintRegion = _LCDRepaintRegion_32; break;
      default: break;
    }

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  {
    char str_log[256];

    sprintf(str_log, "dpi (dots/in)   :: %d\n", g_lcd_dpi_consts[g_lcd_dpi]);
    _SHARKLogCreateEntry(str_log);

    sprintf(str_log, "display depth   :: %d bus = %d", g_lcd_depth, g_lcd_depth_bus); 
    _SHARKLogCreateEntry(str_log);
    switch (g_lcd_type)
    {
      case lcd_grayscale:
           sprintf(str_log, "display invert  :: %c", g_lcd_inverted ? 'y' : 'n');
           _SHARKLogCreateEntry(str_log);
           break;

      case lcd_direct:
           sprintf(str_log, "display A mask  :: %d 0x%08x", g_lcd_a, (uint)g_lcd_a_mask);
           _SHARKLogCreateEntry(str_log);
           sprintf(str_log, "display R mask  :: %d 0x%08x", g_lcd_r, (uint)g_lcd_r_mask);
           _SHARKLogCreateEntry(str_log);
           sprintf(str_log, "display G mask  :: %d 0x%08x", g_lcd_g, (uint)g_lcd_g_mask);
           _SHARKLogCreateEntry(str_log);
           sprintf(str_log, "display B mask  :: %d 0x%08x", g_lcd_b, (uint)g_lcd_b_mask);
           _SHARKLogCreateEntry(str_log);
           break;

      default:
           break;
    }
  }
#endif

    // configure our LCD framebuffer
    _LCDInitialize_RESIZE(width, height);
  }

  return init_ok;
}

static void
_LCDInitialize_RESIZE(uint16 width, uint16 height)
{
  uint32 size;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // pass to underlying hardware engine
  DAL_LCDInitialize_RESIZE(width, height);

  // allocate the offscreen window (for use with rotation of display)
  size = ((uint32)g_lcd_width_rb * g_lcd_height_rb / (g_lcd_depth_bus >> 3));
  g_lcd_framebuffer = malloc(size);
  if (g_lcd_framebuffer != NULL) memset(g_lcd_framebuffer, 0xff, size);  

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  {
    char str_log[256];

    sprintf(str_log, "\nresolution	:: %3dx%3d\n", g_lcd_width, g_lcd_height);
    _SHARKLogCreateEntry(str_log);
  }
#endif
}

static void
_LCDTerminate_RESIZE()
{
  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // delete frame buffer
  if (g_lcd_framebuffer) free(g_lcd_framebuffer);
  g_lcd_framebuffer = NULL;

  // pass to underlying hardware engine
  DAL_LCDTerminate_RESIZE();
}

void
_LCDGetProperties(void **lcd,
                  uint16 *width, uint16 *height, uint16 *row_bytes,
                  lcd_format *format,
                  lcd_orientation *orientation, lcd_dpi *dpi)
{
  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  if (lcd != NULL) 
  {
    if ((g_lcd_orientation == lcd_rotate0) &&
        (g_lcd_doublebuffered && !g_lcd_requirelock))
      *lcd = (void *)g_lcd_framebuffer_dir;
    else
      *lcd = (void *)g_lcd_framebuffer;
  }
  if (format != NULL)
  {
    // clear the format structure
    memset(format, 0, sizeof(format));

    format -> type      = g_lcd_type;
    format -> depth     = g_lcd_depth;
    format -> depth_bus = g_lcd_depth_bus;

    switch (format -> type)
    {
      case lcd_grayscale:
           format -> data.grayscale.inverted = g_lcd_inverted;
           break;

      case lcd_direct:
           format -> data.direct.alpha   = g_lcd_a;
           format -> data.direct.red     = g_lcd_r;
           format -> data.direct.green   = g_lcd_g;
           format -> data.direct.blue    = g_lcd_b;

           format -> data.direct.a_mask  = g_lcd_a_mask;
           format -> data.direct.r_mask  = g_lcd_r_mask;
           format -> data.direct.g_mask  = g_lcd_g_mask;
           format -> data.direct.b_mask  = g_lcd_b_mask;
           break;

      default:
           break;
    }
  }
  if (orientation != NULL) *orientation = g_lcd_orientation;
  if (dpi         != NULL) *dpi         = g_lcd_dpi;

  // orientation:: landscape
  if ((g_lcd_orientation == lcd_rotate0) ||
      (g_lcd_orientation == lcd_rotate180))
  {
    if (width     != NULL) *width     = g_lcd_width;
    if (height    != NULL) *height    = g_lcd_height;
    if (row_bytes != NULL) *row_bytes = g_lcd_width_rb;
  }
  // orientation:: portrait
  else
  {
    if (width     != NULL) *width     = g_lcd_height;
    if (height    != NULL) *height    = g_lcd_width;
    if (row_bytes != NULL) *row_bytes = g_lcd_height_rb;
  }
}

void
_LCDLock()
{
  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // we only update this pointer if we require locking
  if (g_lcd_requirelock) g_lcd_framebuffer_dir = DAL_LCDLock();
}

void
_LCDRepaintRegion(lcd_region *region)
{
  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  fnLCDRepaintRegion(region);
}

void
_LCDRepaint()
{
  lcd_region region;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // we must query for the width/height as it is affected by orientation
  region.x      = 0;
  region.y      = 0;
  _LCDGetProperties(NULL,&region.width,&region.height,NULL,NULL,NULL,NULL);

  // repaint the region
  _LCDLock();
  _LCDRepaintRegion(&region);
  _LCDUnlock();
}

static void
_LCDResume()
{
  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // pass to underlying hardware engine
  DAL_LCDResume();
}

void
_LCDSetOrientation(lcd_orientation orientation)
{
  event e;

  // ensure orientation is correct
  if ((orientation != lcd_rotate0)   &&
      (orientation != lcd_rotate90)  &&
      (orientation != lcd_rotate180) &&
      (orientation != lcd_rotate270)) orientation = lcd_rotate0;

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // save this for when we need to redraw to the LCD
  g_lcd_orientation = orientation;

  // if the framework has finished initialization, send an event
  if (g_initialized)
  {
    memset(&e, 0, sizeof(event));
    e.eType        = _keyEvent;
    e.data.key.chr = _key_lcd_resize;
    _SHARKHandleEvent(&e);
  }

  // redraw the display
  _LCDRepaint();
}

static void
_LCDSuspend()
{
  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // pass to underlying hardware engine
  DAL_LCDSuspend();
}

static void 
_LCDRepaintRegion_1(lcd_region *region)
{
  // T.B.D
}

static void 
_LCDRepaintRegion_2(lcd_region *region)
{
  // T.B.D
}

static void 
_LCDRepaintRegion_4(lcd_region *region)
{
  // T.B.D
}

static void 
_LCDRepaintRegion_8(lcd_region *region)
{
  uint8  *pS, *pD;
  uint32  rw, rh;
  uint32  rw_h;
  int     i, j;

  // optimization
  if ((g_lcd_orientation == lcd_rotate0) &&
      (g_lcd_doublebuffered && !g_lcd_requirelock)) return;

  // get access to the display buffer pointers
  pS = (uint8 *)g_lcd_framebuffer;
  pD = (uint8 *)g_lcd_framebuffer_dir;

  // entry requirements
  if ((pS == NULL) || (pD == NULL)) return;

  // perform rotation based on orientation
  switch (g_lcd_orientation)
  {
    default:
    case lcd_rotate0:

         // ensure boundary clipping
         if  (region -> x > g_lcd_width)  region -> x = g_lcd_width;
         if  (region -> y > g_lcd_height) region -> y = g_lcd_height;
         if ((region -> x + region -> width)  > g_lcd_width)  
           region -> width  = g_lcd_width - region -> x;
         if ((region -> y + region -> height) > g_lcd_height) 
           region -> height = g_lcd_height - region -> y;

         // increments for each scan line
         rw = g_lcd_width_rb;
 
         // initialize pointers
         pS += (region -> y * rw) + region -> x;
         pD += (region -> y * rw) + region -> x;
  
         i = region -> height;
         while (i--)
         {
           _MemCopy((void *)pD, (void *)pS, (uint32)(region -> width));
           pD += rw;
           pS += rw;
         }
         break;

    case lcd_rotate90:

         // ensure boundary clipping
         if  (region -> y > g_lcd_width)  region -> y = g_lcd_width;
         if  (region -> x > g_lcd_height) region -> x = g_lcd_height;
         if ((region -> y + region -> height) > g_lcd_width)  
           region -> height = g_lcd_width - region -> y;
         if ((region -> x + region -> width)  > g_lcd_height) 
           region -> width  = g_lcd_height - region -> x;

         // how many bytes on each row
         rw   = g_lcd_height_rb;
         rh   = g_lcd_width_rb;
         rw_h = region -> height * rw; // optimization

         // initialize pointers
         pD  += ((rh - region -> y - 1) + (region -> x * rh));
         pS  += (region -> x + (region -> y * rw));

         i = region -> width;
         while (i--)
         {
           j = region -> height;
           while (j--)
           {
             *pD-- = *pS;
              pS += rw;
           }
           pS -= (rw_h - 1);
           pD += (region -> height + rh);
         }
         break;

    case lcd_rotate180:

         // ensure boundary clipping
         if  (region -> x > g_lcd_width)  region -> x = g_lcd_width;
         if  (region -> y > g_lcd_height) region -> y = g_lcd_height;
         if ((region -> x + region -> width)  > g_lcd_width)  
           region -> width  = g_lcd_width - region -> x;
         if ((region -> y + region -> height) > g_lcd_height) 
           region -> height = g_lcd_height - region -> y;

         // increments for each scan line
         rw = g_lcd_width_rb;

         // initialize pointers
         pS += (region -> y * rw) + region -> x;
         pD += (((g_lcd_height - region -> y - 1)) * rw) + 
                 (g_lcd_width  - region -> x) - 1;

         i = region -> height;
         while (i--)
         {
           j = region -> width;
           while (j--)
           {
             *pD-- = *pS++;
           }
           pS += (rw - region -> width);
           pD -= (rw - region -> width);
         }
         break;

    case lcd_rotate270:

         // ensure boundary clipping
         if  (region -> y > g_lcd_width)  region -> y = g_lcd_width;
         if  (region -> x > g_lcd_height) region -> x = g_lcd_height;
         if ((region -> y + region -> height) > g_lcd_width)  
           region -> height = g_lcd_width - region -> y;
         if ((region -> x + region -> width)  > g_lcd_height) 
           region -> width  = g_lcd_height - region -> x;

         // how many bytes on each row
         rw   = g_lcd_height_rb;
         rh   = g_lcd_width_rb;
         rw_h = region -> height * rw;  // optimization

         // initialize pointers
         pD += (region -> y + ((rw - region -> x - 1) * rh));
         pS += (region -> x + (region -> y * rw));

         i = region -> width;
         while (i--)
         {
           j = region -> height;
           while (j--)
           {
             *pD++ = *pS;
              pS += rw;
           }
           pS -= (rw_h - 1);
           pD -= (region -> height + rh);
         }
         break;
  }   
}

static void 
_LCDRepaintRegion_12(lcd_region *region)
{
  // T.B.D
}

static void 
_LCDRepaintRegion_16(lcd_region *region)
{
  uint16 *pS, *pD;
  uint16  rw, rh;
  uint32  rw_h;
  int     i, j;

  // optimization
  if ((g_lcd_orientation == lcd_rotate0) &&
      (g_lcd_doublebuffered && !g_lcd_requirelock)) return;

  // get access to the display buffer pointers
  pS = (uint16 *)g_lcd_framebuffer;
  pD = (uint16 *)g_lcd_framebuffer_dir;

  // entry requirements
  if ((pS == NULL) || (pD == NULL)) return;

  // perform rotation based on orientation
  switch (g_lcd_orientation)
  {
    default:
    case lcd_rotate0:

         // ensure boundary clipping
         if  (region -> x > g_lcd_width)  region -> x = g_lcd_width;
         if  (region -> y > g_lcd_height) region -> y = g_lcd_height;
         if ((region -> x + region -> width)  > g_lcd_width)  
           region -> width  = g_lcd_width - region -> x;
         if ((region -> y + region -> height) > g_lcd_height) 
           region -> height = g_lcd_height - region -> y;

         // increments for each scan line
         rw = (uint16)(g_lcd_width_rb >> 1);

         // initialize pointers
         pS += (region -> y * rw) + region -> x;
         pD += (region -> y * rw) + region -> x;

         i = region -> height;
         while (i--)
         {
           _MemCopy((void *)pD, (void *)pS, (uint32)(region -> width << 1));
           pD += rw;
           pS += rw;
         }
         break;

    case lcd_rotate90:

         // ensure boundary clipping
         if  (region -> y > g_lcd_width)  region -> y = g_lcd_width;
         if  (region -> x > g_lcd_height) region -> x = g_lcd_height;
         if ((region -> y + region -> height) > g_lcd_width)  
           region -> height = g_lcd_width - region -> y;
         if ((region -> x + region -> width)  > g_lcd_height) 
           region -> width  = g_lcd_height - region -> x;

         // how many bytes on each row
         rw   = (uint16)(g_lcd_height_rb >> 1);
         rh   = (uint16)(g_lcd_width_rb  >> 1);
         rw_h = region -> height * rw; // optimization

         // initialize pointers
         pS  += (region -> x + (region -> y * rw));
         pD  += ((rh - region -> y - 1) + (region -> x * rh));

         i = region -> width;
         while (i--)
         {
           j = region -> height;
           while (j--)
           {
             *pD-- = *pS;
              pS += rw;
           }
           pS -= (rw_h - 1);
           pD += (region -> height + rh);
         }
         break;

    case lcd_rotate180:

         // ensure boundary clipping
         if  (region -> x > g_lcd_width)  region -> x = g_lcd_width;
         if  (region -> y > g_lcd_height) region -> y = g_lcd_height;
         if ((region -> x + region -> width)  > g_lcd_width)  
           region -> width  = g_lcd_width - region -> x;
         if ((region -> y + region -> height) > g_lcd_height) 
           region -> height = g_lcd_height - region -> y;

         // increments for each scan line
         rw = (uint16)(g_lcd_width_rb >> 1);

         // initialize pointers
         pS += (region -> x + (region -> y * rw));
         pD += (((g_lcd_height - region -> y - 1)) * rw) + 
                 (g_lcd_width  - region -> x) - 1;

         i = region -> height;
         while (i--)
         {
           j = region -> width;
           while (j--)
           {
             *pD-- = *pS++;
           }
           pD -= (rw - region -> width);
           pS += (rw - region -> width);
         }
         break;

    case lcd_rotate270:

         // ensure boundary clipping
         if  (region -> y > g_lcd_width)  region -> y = g_lcd_width;
         if  (region -> x > g_lcd_height) region -> x = g_lcd_height;
         if ((region -> y + region -> height) > g_lcd_width)  
           region -> height = g_lcd_width - region -> y;
         if ((region -> x + region -> width)  > g_lcd_height) 
           region -> width  = g_lcd_height - region -> x;

         // how many bytes on each row
         rw   = (uint16)(g_lcd_height_rb >> 1);
         rh   = (uint16)(g_lcd_width_rb  >> 1);
         rw_h = region -> height * rw;  // optimization

         // initialize pointers
         pS += (region -> x + (region -> y * rw));
         pD += (region -> y + ((rw - region -> x - 1) * rh));

         i = region -> width;
         while (i--)
         {
           j = region -> height;
           while (j--)
           {
             *pD++ = *pS;
              pS += rw;
           }
           pS -= (rw_h - 1);
           pD -= (region -> height + rh);
         }
         break;
  }   
}

static void 
_LCDRepaintRegion_24(lcd_region *region)
{
  // T.B.D
}

static void 
_LCDRepaintRegion_32(lcd_region *region)
{
  uint32 *pS, *pD;
  uint32  rw, rh;
  uint32  rw_h;
  int     i, j;

  // optimization
  if ((g_lcd_orientation == lcd_rotate0) &&
      (g_lcd_doublebuffered && !g_lcd_requirelock)) return;

  // get access to the display buffer pointers
  pS = (uint32 *)g_lcd_framebuffer;
  pD = (uint32 *)g_lcd_framebuffer_dir;

  // entry requirements
  if ((pS == NULL) || (pD == NULL)) return;

  // perform rotation based on orientation
  switch (g_lcd_orientation)
  {
    default:
    case lcd_rotate0:

         // ensure boundary clipping
         if  (region -> x > g_lcd_width)  region -> x = g_lcd_width;
         if  (region -> y > g_lcd_height) region -> y = g_lcd_height;
         if ((region -> x + region -> width)  > g_lcd_width)  
           region -> width  = g_lcd_width - region -> x;
         if ((region -> y + region -> height) > g_lcd_height) 
           region -> height = g_lcd_height - region -> y;

         // increments for each scan line
         rw = (uint16)(g_lcd_width_rb >> 2);

         // initialize pointers
         pS += (region -> y * rw) + region -> x;
         pD += (region -> y * rw) + region -> x;

         i = region -> height;
         while (i--)
         {
           _MemCopy((void *)pD, (void *)pS, (uint32)(region -> width << 2));
           pD += rw;
           pS += rw;
         }
         break;

    case lcd_rotate90:

         // ensure boundary clipping
         if  (region -> y > g_lcd_width)  region -> y = g_lcd_width;
         if  (region -> x > g_lcd_height) region -> x = g_lcd_height;
         if ((region -> y + region -> height) > g_lcd_width)  
           region -> height = g_lcd_width - region -> y;
         if ((region -> x + region -> width)  > g_lcd_height) 
           region -> width  = g_lcd_height - region -> x;

         // increments for each scan line
         rw   = (uint16)(g_lcd_height_rb >> 2);
         rh   = (uint16)(g_lcd_width_rb  >> 2);
         rw_h = region -> height * rw; // optimization

         // initialize pointers
         pD  += ((rh - region -> y - 1) + (region -> x * rh));
         pS  += (region -> x + (region -> y * rw));

         i = region -> width;
         while (i--)
         {
           j = region -> height;
           while (j--)
           {
             *pD-- = *pS;
              pS += rw;
           }
           pS -= (rw_h - 1);
           pD += (region -> height + rh);
         }
         break;

    case lcd_rotate180:

         // ensure boundary clipping
         if  (region -> x > g_lcd_width)  region -> x = g_lcd_width;
         if  (region -> y > g_lcd_height) region -> y = g_lcd_height;
         if ((region -> x + region -> width)  > g_lcd_width)  
           region -> width  = g_lcd_width - region -> x;
         if ((region -> y + region -> height) > g_lcd_height) 
           region -> height = g_lcd_height - region -> y;

         // increments for each scan line
         rw = (uint16)(g_lcd_width_rb >> 2);

         // initialize pointers
         pS += (region -> y * rw) + region -> x;
         pD += (((g_lcd_height - region -> y - 1)) * rw) + 
                 (g_lcd_width  - region -> x) - 1;

         i = region -> height;
         while (i--)
         {
           j = region -> width;
           while (j--)
           {
             *pD-- = *pS++;
           }
           pS += (rw - region -> width);
           pD -= (rw - region -> width);
         }
         break;

    case lcd_rotate270:

         // ensure boundary clipping
         if  (region -> y > g_lcd_width)  region -> y = g_lcd_width;
         if  (region -> x > g_lcd_height) region -> x = g_lcd_height;
         if ((region -> y + region -> height) > g_lcd_width)  
           region -> height = g_lcd_width - region -> y;
         if ((region -> x + region -> width)  > g_lcd_height) 
           region -> width  = g_lcd_height - region -> x;

         // increments for each scan line
         rw   = (uint16)(g_lcd_height_rb >> 2);
         rh   = (uint16)(g_lcd_width_rb  >> 2);
         rw_h = region -> height * rw;  // optimization

         // initialize pointers
         pD += (region -> y + ((rw - region -> x - 1) * rh));
         pS += (region -> x + (region -> y * rw));
         if (g_lcd_height & 3) pD -= ((4 - (g_lcd_height & 3)) * rh);

         i = region -> width;
         while (i--)
         {
           j = region -> height;
           while (j--)
           {
             *pD++ = *pS;
              pS += rw;
           }
           pS -= (rw_h - 1);
           pD -= (region -> height + rh);
         }
         break;
  }   
}

void
_LCDUnlock()
{
  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  DAL_LCDUnlock();
  DAL_LCDFlipDisplay();

  // we only update this pointer if we require locking
  if (g_lcd_requirelock) g_lcd_framebuffer_dir = NULL;
}

static void
_LCDTerminate()
{
#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _SHARKLogCreateEntry(">> _LCDTerminate");
#endif

  // GLOBALS_PROTECTION:
  if (GLOBALS_UNINITIALIZED) return;

  // release our LCD framebuffer
  _LCDTerminate_RESIZE();

  // pass to underlying hardware engine
  DAL_LCDTerminate();

  // free memory for globals
  free(LCD_GLOBALS); 
  LCD_GLOBALS = NULL;
}

/********************************* EOF ***********************************/
