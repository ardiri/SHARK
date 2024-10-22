/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK.h
 */

#ifndef SHARK_H
#define SHARK_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
 * Generic Data Types
 *************************************************************************/

// common constants
#define false 0
#define true  (!false)
#ifndef NULL
#define NULL  (void *)0
#endif

// generic "integer" - best for loop counters, generics
typedef unsigned int            uint;

// specific sized integers
typedef   signed char           int8;
typedef unsigned char           uint8;
typedef   signed short          int16;
typedef unsigned short          uint16;
typedef   signed long           int32;
typedef unsigned long           uint32;

// miscellaneous
typedef unsigned char           boolean;

typedef enum
{ iterator_start = 0, iterator_active, iterator_stop = 255} iterator;

// helper macros
#define ABS(a)   ((a) < 0 ?  -(a) : (a))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define SIGN(a)  ((a) < 0   ?  -1 : 1)

// reading complex integer values from 8bit pointer
#define READ_16_LE(p)     (((uint16)((uint8 *)(p))[1] <<  8) |             \
                            (uint16)((uint8 *)(p))[0])

#define READ_16_BE(p)     (((uint16)((uint8 *)(p))[0] <<  8) |             \
                            (uint16)((uint8 *)(p))[1])

#define WRITE_16_LE(p, v) (((uint8 *)(p))[1] = (uint8)((uint16)(v) >>  8), \
                           ((uint8 *)(p))[0] = (uint8)((uint16)(v)) )

#define WRITE_16_BE(p, v) (((uint8 *)(p))[0] = (uint8)((uint16)(v) >>  8), \
                           ((uint8 *)(p))[1] = (uint8)((uint16)(v)) )

#define READ_32_LE(p)     (((uint32)((uint8 *)(p))[3] << 24) |             \
                           ((uint32)((uint8 *)(p))[2] << 16) |             \
                           ((uint32)((uint8 *)(p))[1] <<  8) |             \
                            (uint32)((uint8 *)(p))[0])

#define READ_32_BE(p)     (((uint32)((uint8 *)(p))[0] << 24) |             \
                           ((uint32)((uint8 *)(p))[1] << 16) |             \
                           ((uint32)((uint8 *)(p))[2] <<  8) |             \
                            (uint32)((uint8 *)(p))[3])

#define WRITE_32_LE(p, v) (((uint8 *)(p))[3] = (uint8)((uint32)(v) >> 24), \
                           ((uint8 *)(p))[2] = (uint8)((uint32)(v) >> 16), \
                           ((uint8 *)(p))[1] = (uint8)((uint32)(v) >>  8), \
                           ((uint8 *)(p))[0] = (uint8)((uint32)(v)) )

#define WRITE_32_BE(p, v) (((uint8 *)(p))[0] = (uint8)((uint32)(v) >> 24), \
                           ((uint8 *)(p))[1] = (uint8)((uint32)(v) >> 16), \
                           ((uint8 *)(p))[2] = (uint8)((uint32)(v) >>  8), \
                           ((uint8 *)(p))[3] = (uint8)((uint32)(v)) )

#define SWAP_16(n)        (((((uint16) n) << 8) & 0xFF00) |                \
                           ((((uint16) n) >> 8) & 0x00FF))

#define SWAP_32(n)        (((((uint32) n) << 24) & 0xFF000000) |           \
                           ((((uint32) n) <<  8) & 0x00FF0000) |           \
                           ((((uint32) n) >>  8) & 0x0000FF00) |           \
                           ((((uint32) n) >> 24) & 0x000000FF))

/*************************************************************************
 * System
 *************************************************************************/

typedef int16      coord;
typedef enum
{
  _nilEvent = 0,
  _appStartEvent,        // application should do globals initialization
  _appGoEvent,           // application should do application startup
  _appStopEvent,         // application should do application shutdown
  _penEvent,
  _keyEvent,
  _userEventBase = 0x1f
} sys_event;

struct _GenericEvent { uint32 _padding_;                       };
struct _PenEvent     { coord  x; coord y;                      };
struct _KeyEvent     { uint8  chr; uint8 chr_x[3]; uint8 size; };

typedef struct event
{
  int16     eType;
  boolean   penDown;
  uint8     _padding_;
  coord     screenX;
  coord     screenY;

  union
  {
    struct _GenericEvent generic;
    struct _PenEvent     pen;
    struct _KeyEvent     key;
  } data;

} event;

typedef struct datetime
{
  int16  year;
  uint   month:4;
  uint   day  :5;
  uint   hour :5;
  uint   min  :6;
  uint   sec  :6;
} datetime;

#define PLATFORM_UNKNOWN       0x00000000
#define PLATFORM_WIN32         0x01000000
#define PLATFORM_MACOSX        0x02000000
#define PLATFORM_PALM          0x03000000
#define PLATFORM_POCKETPC      0x04000000
#define PLATFORM_SMARTPHONE    0x05000000
#define PLATFORM_SYMBIAN_S60   0x06000000
#define PLATFORM_SYMBIAN_UIQ   0x07000000
#define PLATFORM_GIZMONDO      0x08000000
#define PLATFORM_SONY_PSP      0x09000000

#define PLATFORM_GENERIC(d)    ((d) & 0xFF000000)
#define PLATFORM_PROVIDER(d)   ((d) & 0x00FF0000)

#define PROVIDER_APPLE         0x00010000
#define PROVIDER_MICROSOFT     0x00020000
#define PROVIDER_MOTOROLA      0x00030000
#define PROVIDER_NOKIA         0x00040000
#define PROVIDER_SAMSUNG       0x00050000
#define PROVIDER_SENDO         0x00060000
#define PROVIDER_SIEMENS       0x00070000
#define PROVIDER_SONY_ERICSSON 0x00080000

#define DEVICE_WINDOWS_95      (PLATFORM_WIN32       | PROVIDER_MICROSOFT | 0x01)
#define DEVICE_WINDOWS_98      (PLATFORM_WIN32       | PROVIDER_MICROSOFT | 0x02)
#define DEVICE_WINDOWS_ME      (PLATFORM_WIN32       | PROVIDER_MICROSOFT | 0x03)
#define DEVICE_WINDOWS_NT_351  (PLATFORM_WIN32       | PROVIDER_MICROSOFT | 0x04)
#define DEVICE_WINDOWS_NT_4    (PLATFORM_WIN32       | PROVIDER_MICROSOFT | 0x05)
#define DEVICE_WINDOWS_2000    (PLATFORM_WIN32       | PROVIDER_MICROSOFT | 0x06)
#define DEVICE_WINDOWS_XP      (PLATFORM_WIN32       | PROVIDER_MICROSOFT | 0x07)
#define DEVICE_WINDOWS_2003    (PLATFORM_WIN32       | PROVIDER_MICROSOFT | 0x08)

#define DEVICE_POCKET_PC       (PLATFORM_POCKETPC    | PROVIDER_MICROSOFT | 0x01)
#define DEVICE_POCKET_PC_2000  (PLATFORM_POCKETPC    | PROVIDER_MICROSOFT | 0x02)
#define DEVICE_POCKET_PC_2003  (PLATFORM_POCKETPC    | PROVIDER_MICROSOFT | 0x03)
#define DEVICE_POCKET_PC_2005  (PLATFORM_POCKETPC    | PROVIDER_MICROSOFT | 0x04)
#define DEVICE_SMARTPHONE      (PLATFORM_SMARTPHONE  | PROVIDER_MICROSOFT | 0x05)
#define DEVICE_SMARTPHONE_2003 (PLATFORM_SMARTPHONE  | PROVIDER_MICROSOFT | 0x06)
#define DEVICE_SMARTPHONE_2005 (PLATFORM_SMARTPHONE  | PROVIDER_MICROSOFT | 0x07)

#define DEVICE_MACOSX          (PLATFORM_MACOSX      | PROVIDER_APPLE | 0x01)

#define DEVICE_NOKIA_3230      (PLATFORM_SYMBIAN_S60 | PROVIDER_NOKIA | 0x01)
#define DEVICE_NOKIA_3650      (PLATFORM_SYMBIAN_S60 | PROVIDER_NOKIA | 0x02)
#define DEVICE_NOKIA_3660      (PLATFORM_SYMBIAN_S60 | PROVIDER_NOKIA | 0x03)
#define DEVICE_NOKIA_6260      (PLATFORM_SYMBIAN_S60 | PROVIDER_NOKIA | 0x04)
#define DEVICE_NOKIA_6600      (PLATFORM_SYMBIAN_S60 | PROVIDER_NOKIA | 0x05)
#define DEVICE_NOKIA_6620      (PLATFORM_SYMBIAN_S60 | PROVIDER_NOKIA | 0x06)
#define DEVICE_NOKIA_6630      (PLATFORM_SYMBIAN_S60 | PROVIDER_NOKIA | 0x07)
#define DEVICE_NOKIA_6670      (PLATFORM_SYMBIAN_S60 | PROVIDER_NOKIA | 0x08)
#define DEVICE_NOKIA_6680      (PLATFORM_SYMBIAN_S60 | PROVIDER_NOKIA | 0x09)
#define DEVICE_NOKIA_6681      (PLATFORM_SYMBIAN_S60 | PROVIDER_NOKIA | 0x0a)
#define DEVICE_NOKIA_6682      (PLATFORM_SYMBIAN_S60 | PROVIDER_NOKIA | 0x0b)
#define DEVICE_NOKIA_7610      (PLATFORM_SYMBIAN_S60 | PROVIDER_NOKIA | 0x0c)
#define DEVICE_NOKIA_7650      (PLATFORM_SYMBIAN_S60 | PROVIDER_NOKIA | 0x0d)
#define DEVICE_NOKIA_NGAGE     (PLATFORM_SYMBIAN_S60 | PROVIDER_NOKIA | 0x0e)
#define DEVICE_NOKIA_NGAGE_QD  (PLATFORM_SYMBIAN_S60 | PROVIDER_NOKIA | 0x0f)
#define DEVICE_NOKIA_N70       (PLATFORM_SYMBIAN_S60 | PROVIDER_NOKIA | 0x10)
#define DEVICE_NOKIA_N90       (PLATFORM_SYMBIAN_S60 | PROVIDER_NOKIA | 0x11)
#define DEVICE_SENDO_X         (PLATFORM_SYMBIAN_S60 | PROVIDER_SENDO | 0x01)
#define DEVICE_SIEMENS_SX1     (PLATFORM_SYMBIAN_S60 | PROVIDER_SIEMENS | 0x01)
#define DEVICE_SAMSUNG_D730    (PLATFORM_SYMBIAN_S60 | PROVIDER_SAMSUNG | 0x01)

#define DEVICE_SE_P800         (PLATFORM_SYMBIAN_UIQ | PROVIDER_SONY_ERICSSON | 0x01)
#define DEVICE_SE_P900         (PLATFORM_SYMBIAN_UIQ | PROVIDER_SONY_ERICSSON | 0x02)
#define DEVICE_SE_P910         (PLATFORM_SYMBIAN_UIQ | PROVIDER_SONY_ERICSSON | 0x03)
#define DEVICE_MOTOROLA_A900   (PLATFORM_SYMBIAN_UIQ | PROVIDER_MOTOROLA | 0x01)

// device specific
extern uint32      _SysGetTime();
extern void        _SysGetDateTime(datetime *dt);
extern void        _SysDelay(uint32 millisec);
extern void        _SysDebugMessage(char *msg, boolean dialog);
extern void        _SysDeviceInfo(uint32 *platform, char *name);
extern void        _SysEnqueueEvent(event *e);
extern void        _SysSetGlobals(void *gbls);
extern void       *_SysGetGlobals();
extern uint16      _SysRandom(uint32 seed);

// device independent
extern void        _SysEnqueueQuitEvent();

/*************************************************************************
 * Audio
 *************************************************************************/

typedef enum { snd_8khz = 0, snd_11khz, snd_16khz, snd_22khz, snd_44khz } snd_rate;
typedef enum { snd_int8 = 0, snd_uint8, snd_int16                       } snd_format;
typedef enum { snd_mono = 0, snd_stereo                                 } snd_type;

#define INVALID_STREAM          -1

#define SND_STREAM_8KHZ         (1 << snd_8khz)
#define SND_STREAM_11KHZ        (1 << snd_11khz)
#define SND_STREAM_16KHZ        (1 << snd_16khz)
#define SND_STREAM_22KHZ        (1 << snd_22khz)
#define SND_STREAM_44KHZ        (1 << snd_44khz)

#define SND_STREAM_MONO         0x1000
#define SND_STREAM_STEREO       0x2000

typedef int32      snd_stream;
typedef boolean  (*_SndStreamBufferCallback)(snd_stream stream_id, void *user, void *buffer, uint32 frames);

// device specific
extern boolean     _SndStreamAvailable();
extern uint32      _SndStreamRatesSupported();
extern snd_stream  _SndStreamCreate(snd_rate rate, snd_format format, snd_type type, _SndStreamBufferCallback func, void *user, uint16 frames);
extern void        _SndStreamStart(snd_stream stream_id);
extern void        _SndStreamStop(snd_stream stream_id);
extern void        _SndStreamDelete(snd_stream stream_id);

// device independent
// NONE

/*************************************************************************
 * File
 *************************************************************************/

typedef enum { file_beginning = 0, file_current, file_end } file_position;
typedef struct file file;

#define FILE_STANDARD           0x00000001
#define FILE_DIRECTORY          0x00000002  // _FileGetAttributes

#define DISK_BASIC              0x00000000
#define DISK_SYSTEM             0x00000001
#define DISK_CLOBBER            0x00000010  // | CREATE (if doesn't exist)

#define FILE_READ_ONLY          0x00000001  // 'rb'
#define FILE_READ_WRITE         0x00000002  // 'wb+'
#define FILE_CLOBBER            0x00000010  // | CLOBBER (wipe file)

#define FILE_ACCESS(a)          ((a) & 0x0000000f)

#define DISK_SEPARATOR          ':'
#define DIRECTORY_SEPARATOR     '\\'

#define MAX_ENUMERATION_ENTRIES 255
#define MAX_FILENAME_LENGTH     32
#define MAX_PATH_LENGTH         256
#define MAX_DISKS               10

// device specific
extern boolean     _FileDefineDisk(char *name, char *path, uint32 mode);
extern boolean     _FileLocateDisk(char *name, uint32 *disk);

extern boolean     _DiskEnumerate(iterator *it, uint32 *disk, char *name);
extern boolean     _FileEnumerate(uint32 disk, file *dir, iterator *it, char *name, uint32 *attributes);

extern file       *_FileGetHandle(uint32 disk, file *dir, char *name);
extern void        _FileReleaseHandle(file *f);

extern boolean     _FileCreate(uint32 disk, file *dir, char *name, uint32 attributes);
extern boolean     _FileRename(uint32 disk, file *dir, char *old_name, char *new_name);
extern boolean     _FileDelete(file *f);

extern boolean     _FileOpen(file *f, uint32 mode);
extern boolean     _FileEOF(file *f);
extern void        _FileSeek(file *f, int32 ofs, file_position position);
extern uint32      _FileTell(file *f);
extern void        _FileRead(file *f, void *buf, uint32 objSize, uint32 numObject, uint32 *objRead);
extern uint32      _FileWrite(file *f, void *buf, uint32 objSize, uint32 numObject);
extern void        _FileFlush(file *f);
extern void        _FileClose(file *f);

extern uint32      _FileGetLastError();
extern void        _FileGetAttributes(file *f, uint32 *disk, file **dir, char *path, char *name, uint32 *attributes);
extern file       *_FileGetHandleFromPath(uint32 disk, char *path);
extern void        _FileReleaseHandleRecursive(file *f);

// device independent
extern boolean     _FileExists(uint32 disk, file *dir, char *name);
extern uint32      _FileGetSize(file *f);
extern boolean     _FileLoadIntoBuffer(file *f, void *buffer, uint32 size);

/*************************************************************************
 * Input
 *************************************************************************/

#define _key_unknown            0

#define _key_base               1
#define _key_padup              (_key_base+0)
#define _key_paddown            (_key_base+1)
#define _key_padleft            (_key_base+2)
#define _key_padright           (_key_base+3)
#define _key_padselect          (_key_base+4)
#define _key_actionA            (_key_base+5)
#define _key_actionB            (_key_base+6)
#define _key_actionC            (_key_base+7)
#define _key_option             (_key_base+8)

#define _key_virtual_base       10
#define _key_gfx_change         (_key_virtual_base+0)
#define _key_focuslost          (_key_virtual_base+1)
#define _key_menu               (_key_virtual_base+2)
#define _key_acknowledge        (_key_virtual_base+3)
#define _key_backspace          (_key_virtual_base+4)

#define _key_user_base          17
#define _key_user_base_end      31   // after this, its normal character values

// key values (for _KeyCurrentState)
#define KeyPadUp                (1L << (_key_padup))
#define KeyPadDown              (1L << (_key_paddown))
#define KeyPadLeft              (1L << (_key_padleft))
#define KeyPadRight             (1L << (_key_padright))
#define KeyPadSelect            (1L << (_key_padselect))
#define KeyActionA              (1L << (_key_actionA))
#define KeyActionB              (1L << (_key_actionB))
#define KeyActionC              (1L << (_key_actionC))
#define KeyOption               (1L << (_key_option))

#define INPUT_STYLUS            0x00000001   // pen events
#define INPUT_KEYS              0x00000002   // KeyX constants (dpad, et al)
#define INPUT_KEYS_ANALOG       0x00000004   // analog controls (0..255 in x,y)
#define INPUT_PHONEPAD          0x00000008   // phone keyboard (0..9,#* et al)
#define INPUT_KEYBOARD          0x00000010   // standard alpha-numeric keyboard

// device specific
extern uint32      _InputSupported();
extern uint32      _KeyCurrentState(uint8 *x, uint8 *y);
extern void        _PenState(coord *penX, coord *penY, boolean *penDown);

// device independent
// none

/*************************************************************************
 * LCD
 *************************************************************************/

typedef enum { lcd_palette = 0, lcd_direct } lcd_type;
typedef enum { lcd_dpi72 = 0, lcd_dpi96, lcd_dpi144, lcd_dpi192 } lcd_dpi;

typedef struct rectangle
{
  coord     x;
  coord     y;
  coord     width;
  coord     height;
} rectangle;

typedef struct lcd_format
{
  lcd_type type;

  uint16   depth;       // how many bits per pixel (including padding)?
  union
  {
    struct
    {
      uint16 size;
      uint8  *rgb;      // this may be required for bitmap imports
    } palette;
    struct
    {
      uint8  alpha;
      uint8  red;
      uint8  green;
      uint8  blue;

      uint32 a_mask;
      uint32 r_mask;
      uint32 g_mask;
      uint32 b_mask;    // if it isn't xRGB - this allows redefinition
    } direct;
  } data;
} lcd_format;

typedef enum lcd_orientation { lcd_rotate0 = 0, lcd_rotate90, lcd_rotate180, lcd_rotate270 } lcd_orientation;

#define DISPLAY_RESIZE_NONE     0x00000001 // no resizing support
#define DISPLAY_RESIZE_FIXED    0x00000002 // fixed sizes (app defined)
#define DISPLAY_RESIZE_ARBITARY 0x00000004 // arbitary (give it to me baby)

// device specific
extern void        _LCDSetOrientation(lcd_orientation orientation);
extern void        _LCDGetProperties(void **lcd, coord *width, coord *height, uint16 *row_bytes, lcd_format *format, lcd_orientation *orientation, lcd_dpi *dpi);
extern void        _LCDPalette(boolean mode, uint8 *palette); // only if display is index paletted
extern void        _LCDRepaintRegion(rectangle *rect);
extern void        _LCDRepaint();

// device independent
// none

/*************************************************************************
 * Memory
 *************************************************************************/

// device specific
extern void       *_MemPtrNew(uint32 size, boolean storage);
extern void        _MemSemaphore(boolean active);
extern void        _MemPtrFree(void *p);
extern uint32      _MemAvailable(boolean storage);
extern uint32      _MemPtrSize(void *p);

// device independent
extern void        _MemSet(void *p, uint8 value, uint32 count);
extern void        _MemMove(void *pDst, void *pSrc, uint32 count);
extern void        _MemCopy(void *pDst, void *pSrc, uint32 count);
extern int32       _MemCompare(void *p1, void *p2, uint32 count);
extern char       *_MemChr(void *p, char chr, uint32 count);
extern char       *_MemRChr(void *p, char chr, uint32 count);

typedef struct MemPool MemPool;

extern MemPool    *_MemPoolNew(uint32 size, uint32 count);
extern void       *_MemPoolReserve(MemPool *pool, uint32 count);
extern void        _MemPoolFree(MemPool *pool);

/*************************************************************************
 * Resource
 *************************************************************************/

typedef uint16     resource;

// device specific
extern boolean     _ResourceExists(resource resNum);
extern uint16      _ResourceSize(resource resNum);
extern void       *_ResourceLock(resource resNum);
extern void        _ResourceUnlock(void *ptr);

// device indepedent
// none

/*************************************************************************
 * Preferences
 *************************************************************************/

typedef uint16     preference;

// device specific
extern void       *_PreferencesGetReference();
extern void        _PreferencesLoad();
extern void        _PreferencesSave();
extern boolean     _PreferencesLoadBank(preference idx, void *buf, uint16 size);
extern void        _PreferencesSaveBank(preference idx, void *buf, uint16 size);
extern void        _PreferencesKillBank(preference idx);

// device independent
// NONE

/*************************************************************************
 * Network
 *************************************************************************/

// TBD

/*************************************************************************
 * Library
 *************************************************************************/

#define MAX_LIBRARIES           9
#define MAX_LIBNAME_LENGTH      32

typedef boolean   (*_LibHandleEventCallback)(event *e);

// device dependent
extern boolean     _LibRegister(uint32 id, char *name, _LibHandleEventCallback callback);
extern void        _LibSetGlobals(uint32 id, void *gbls);
extern void *      _LibGetGlobals(uint32 id);
extern void        _LibShutdown(uint32 id);

// device independent
// none

/*************************************************************************
 * String
 *************************************************************************/

// device specific
// none

// device independent
extern void        _StrSet(char *s, char chr);
extern uint32      _StrLen(char *s);
extern char       *_StrReverse(char *s);

extern void        _StrCopy(char *s, char *sX);
extern void        _StrCat(char *s, char *sX);
extern char       *_StrChr(char *s, char chr);
extern char       *_StrRChr(char *s, char chr);
extern char       *_StrStr(char *s, char *tok);
extern char       *_StrCaselessStr(char *s, char *tok);
extern int32       _StrCompare(char *s1, char *s2);
extern int32       _StrCaselessCompare(char *s1, char *s2);

extern void        _StrNCopy(char *s, char *sX, uint32 count);
extern void        _StrNCat(char *s, char *sX, uint32 count);
extern char       *_StrNChr(char *s, char chr, uint32 count);
extern char       *_StrNRChr(char *s, char chr, uint32 count);
extern char       *_StrNStr(char *s, char *tok, uint32 count);
extern char       *_StrNCaselessStr(char *s, char *tok, uint32 count);
extern int32       _StrNCompare(char *s1, char *s2, uint32 count);
extern int32       _StrNCaselessCompare(char *s1, char *s2, uint32 count);

extern int32       _StrAToI(char *s);
extern void        _StrIToA(char *s, int32 value);
extern uint32      _StrAToD(char *s);
extern void        _StrDToA(char *s, uint32 value);
extern uint32      _StrAToO(char *s);
extern void        _StrOToA(char *s, uint32 value);
extern uint32      _StrAToH(char *s);
extern void        _StrHToA(char *s, uint32 value);
extern double      _StrAToF(char *s);
extern void        _StrFToA(char *s, double value, uint32 decimal);

/*************************************************************************
 * Graphics:: Library
 *************************************************************************/

typedef enum
{
  gfxPaint = 0,      // x = y
  gfxMask,           // x = x & ~y
  gfxInvert,         // x = x ^ y
  gfxOverlay,        // x = x | y
  gfxTransparent     // special alpha-blending (for anti-aliased text)
} draw_operation;

typedef uint32     color;
typedef struct gfx_window gfx_window;

typedef struct
{
  gfx_window *win;
  rectangle   clip;
} gfx_state;

// device specific
// none

// device independent
extern gfx_window *_GfxCreateWindow(coord width, coord height, boolean storage);
extern gfx_window *_GfxCreateWindowFromBitmapStream(coord width, coord height, lcd_format *format, void *bits, boolean storage);
extern gfx_window *_GfxCreateWindowFromFile(uint32 disk, char *filename, rectangle *bounds, boolean storage);
extern void        _GfxDeleteWindow(gfx_window *win);

extern void        _GfxGetWindowInfo(gfx_window *win, void **bits, coord *width, coord *height, uint16 *rowBytes);

extern gfx_window *_GfxGetDisplayWindow();
extern void        _GfxSetDrawWindow(gfx_window *window);
extern gfx_window *_GfxGetDrawWindow();

extern void        _GfxGetPalette(uint8 *rgb);
extern void        _GfxSetPalette(uint8 *rgb);
extern color       _GfxGetPaletteIndex(uint8 r, uint8 g, uint8 b);
extern void        _GfxGetRGBFromIndex(color c, uint8 *r, uint8 *g, uint8 *b);

extern void        _GfxClearWindow();
extern void        _GfxFillRegion(rectangle *rect, color c);
extern void        _GfxFillRegionRounded(rectangle *rect, color c);
extern void        _GfxInvertRegion(rectangle *rect);
extern void        _GfxCopyRegion(gfx_window *srcWin, gfx_window *dstWin, rectangle *rect,
                                  coord x, coord y, draw_operation mode);

extern void        _GfxSetClip(rectangle *rect);
extern void        _GfxMergeClip(rectangle *rect);
extern void        _GfxGetClip(rectangle *rect);
extern void        _GfxResetClip();

extern void        _GfxSetPixel(coord x, coord y, color c);
extern color       _GfxGetPixel(coord x, coord y);
extern void        _GfxDrawLine(coord x1, coord y1, coord x2, coord y2, color c);
extern void        _GfxDrawRectangle(rectangle *rect, color c);
extern void        _GfxDrawRectangleRounded(rectangle *rect, color c);

extern void        _GfxRepaint();
extern void        _GfxRepaintRegion(rectangle *rect);
extern void        _GfxCopyToDisplay(gfx_window *win, rectangle *rect, coord scr_x, coord scr_y);

extern void        _GfxSaveState(gfx_state *state);
extern void        _GfxRestoreState(gfx_state *state);

/*************************************************************************
 * Font:: Library
 *************************************************************************/

#define MAX_FONTS               32
#define MAX_FONTNAME_LENGTH     32

#define INVALID_FONT            -1

typedef int32      font;
typedef enum font_style    { font_bitmap = 0, font_ttf  } font_style;
typedef enum font_encoding { font_raw    = 0, font_utf8 } font_encoding;

typedef struct
{
  gfx_window *win;
  rectangle   clip;
  color       c;
  font        fnt;
} fnt_state;

// device specific
// none

// device independent
extern font        _FntDefineFont(font_style style, char *name, void *fontData);
extern font        _FntLocateFont(char *name);

extern font        _FntDefineFontFromFile(font_style style, char *name, uint32 disk, file *dir, char *fileName);

extern void        _FntSetFont(font id);
extern font        _FntGetFont();
extern void        _FntReleaseFont(font id);

extern void        _FntSetColor(color c);
extern color       _FntGetColor();
extern void        _FntDrawString(char *str, uint32 len, coord x, coord y, draw_operation mode);

extern uint32      _FntGetCharBytesUsed(char *str);
extern uint32      _FntGetCharsWidth(char *str, uint32 len);
extern uint32      _FntGetCharsInWidth(char *str, uint32 len);
extern uint32      _FntGetFontHeight();
extern uint32      _FntGetWordWrap(char *str, uint32 maxPixels, boolean *force, uint32 *cntPixels);

extern char       *_FntChr(char *s, char *chr);
extern char       *_FntRChr(char *s, char *chr);
extern char       *_FntStr(char *s, char *tok);
extern char       *_FntCaselessStr(char *s, char *tok);
extern int32       _FntCompare(char *s1, char *s2);
extern int32       _FntCaselessCompare(char *s1, char *s2);

extern char       *_FntNChr(char *s, char *chr, uint32 count);
extern char       *_FntNRChr(char *s, char *chr, uint32 count);
extern char       *_FntNStr(char *s, char *tok, uint32 count);
extern char       *_FntNCaselessStr(char *s, char *tok, uint32 count);
extern int32       _FntNCompare(char *s1, char *s2, uint32 count);
extern int32       _FntNCaselessCompare(char *s1, char *s2, uint32 count);

extern void        _FntSaveState(fnt_state *state);
extern void        _FntRestoreState(fnt_state *state);

/*************************************************************************
 * Sound Effects:: Library
 *************************************************************************/

#define MAX_SAMPLES             63

typedef enum sfx_type    { sfx_pcm = 0 }                                 sfx_type;
typedef enum sfx_command { sfx_play = 0, sfx_stop, sfx_pause, sfx_seek } sfx_command;
typedef enum vol_command { vol_get  = 0, vol_set }                       vol_command;

extern boolean     _SfxDefineMusic(sfx_type type, void *data, uint32 length);
extern void        _SfxRemoveMusic();
extern boolean     _SfxDefineSample(uint32 id, void *data, uint32 length);
extern void        _SfxRemoveSample(uint32 id);

extern void        _SfxVolume(vol_command command, uint8 *music, uint8 *sfx);
extern void        _SfxToneCommand(uint16 freq, uint32 duration, uint8 volume);
extern void        _SfxSampleCommand(uint32 id, sfx_command command, void *data);
extern void        _SfxMusicCommand(sfx_command command, void *data);

/*************************************************************************
 * Application
 *************************************************************************/

typedef struct application_config
{
  char            name[32];        // the name of the application
  char            company[32];     // the name of the company (responsible)
  uint32          version;         // version number

  struct
  {
    uint32        mode;            // display resizing rules
    uint16        fps;             // frames per second (x10)
  } display;

  struct
  {
    uint16        count;           // maximum number of pointers required

    uint32        min_heap;        // minimum dynamic heap required to run
    uint32        max_heap;        // maximum dynamic heap required to run
    uint32        min_storage;     // minimum storage heap required to run
    uint32        max_storage;     // maximum storage heap required to run
  } memory;

  struct
  {
    uint16        size;            // size of preferences,
                                   //   use _PreferencesGetReference() for data
  } preferences;

  //
  // LIBRARIES::
  //

  struct
  {
    boolean       active;
    boolean       required;

    lcd_format    format;          // which graphics mode to use (RGB)
  } gfx;

  struct
  {
    boolean       active;
    boolean       required;

    font_encoding encoding;        // encoding type
    font_style    style;           // font style
  } font;

  struct
  {
    boolean       active;
    boolean       required;

    snd_rate      rate;
    snd_format    format;
    snd_type      type;
    uint16        frames;
    uint16        mix_count;
  } sfx;

} application_config;

#define VER_BASE                0x00000000
#define VER_GENRATE(maj, min)   (VER_BASE | (((maj) & 0xff) << 8) | ((min) & 0xff))
#define VER_MAJOR(ver)          (((ver) >> 8) & 0xff)
#define VER_MINOR(ver)          ((ver) & 0xff)

// device dependent
// NONE

// device independent (developer defined)
extern void         ApplicationConfig(application_config *config);
extern boolean      ApplicationInitialize();
extern boolean      ApplicationHandleEvent(event *e);
extern void         ApplicationTerminate();

#ifdef __cplusplus
}
#endif

#endif

/********************************* EOF ***********************************/

