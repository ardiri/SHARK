/*************************************************************************
 *
 * Copyright 2001+ MobileWizardry
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
 * SHARK: generics
 *************************************************************************
 *
 * GENERIC DEFINITIONS
 * -------------------
 *
 * SHARK defines the following generic data types and constants.
 *
 * While it is possible to use standard C data types - it is not 
 * recommended as their interpretation by the compiler may vary 
 * between target devices (int can be 16, 32 or 64 bits). SHARK 
 * data types are named uniquely to allow easy identification of 
 * both storage contents and structure.
 *
 * Do NOT assume endian formats when reading or writing via type 
 * casting of memory buffers into generic data types if the data 
 * has been sourced from an external resources; for example, file 
 * contents or network traffic. Use the macros defined to ensure 
 * maximum portability between devices.
 *
 *************************************************************************/

//------------------------------------------------------------------------
// constants and enumerations
//------------------------------------------------------------------------

#define false 0
#define true  (!false)
#ifndef NULL
#define NULL  (void *)0
#endif

typedef enum iterator
{ 
  iterator_start   = 0, 
  iterator_active, 
  iterator_stop
} iterator;

#define errNone        0

//------------------------------------------------------------------------
// data types and structures
//------------------------------------------------------------------------

typedef unsigned int   uint;

typedef   signed char  int8;
typedef unsigned char  uint8;
typedef   signed short int16;
typedef unsigned short uint16;
typedef   signed int   int32;
typedef unsigned int   uint32;

typedef unsigned char  boolean;
typedef signed short   error;

//------------------------------------------------------------------------
// application programmer interface
//------------------------------------------------------------------------

#define ABS(a)   ((a) < 0 ?  -(a) : (a))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define SWAP(a,b) (((a) == (b)) || (((a) ^= (b)), ((b) ^= (a)), ((a) ^= (b))))

#define READ_16_LE(p)     (((uint16)((uint8 *)(p))[1] <<  8) |             \
                            (uint16)((uint8 *)(p))[0])

#define READ_16_BE(p)     (((uint16)((uint8 *)(p))[0] <<  8) |             \
                            (uint16)((uint8 *)(p))[1])

#define READ_32_LE(p)     (((uint32)((uint8 *)(p))[3] << 24) |             \
                           ((uint32)((uint8 *)(p))[2] << 16) |             \
                           ((uint32)((uint8 *)(p))[1] <<  8) |             \
                            (uint32)((uint8 *)(p))[0])

#define READ_32_BE(p)     (((uint32)((uint8 *)(p))[0] << 24) |             \
                           ((uint32)((uint8 *)(p))[1] << 16) |             \
                           ((uint32)((uint8 *)(p))[2] <<  8) |             \
                            (uint32)((uint8 *)(p))[3])

#define SIGN(a)  ((a) < 0   ?  -1 : 1)

#define SWAP_16(n)        (((((uint16) n) << 8) & 0xFF00) |                \
                           ((((uint16) n) >> 8) & 0x00FF))

#define SWAP_32(n)        (((((uint32) n) << 24) & 0xFF000000) |           \
                           ((((uint32) n) <<  8) & 0x00FF0000) |           \
                           ((((uint32) n) >>  8) & 0x0000FF00) |           \
                           ((((uint32) n) >> 24) & 0x000000FF))

#define WRITE_16_LE(p, v) (((uint8 *)(p))[1] = (uint8)((uint16)(v) >>  8), \
                           ((uint8 *)(p))[0] = (uint8)((uint16)(v)) )

#define WRITE_16_BE(p, v) (((uint8 *)(p))[0] = (uint8)((uint16)(v) >>  8), \
                           ((uint8 *)(p))[1] = (uint8)((uint16)(v)) )

#define WRITE_32_LE(p, v) (((uint8 *)(p))[3] = (uint8)((uint32)(v) >> 24), \
                           ((uint8 *)(p))[2] = (uint8)((uint32)(v) >> 16), \
                           ((uint8 *)(p))[1] = (uint8)((uint32)(v) >>  8), \
                           ((uint8 *)(p))[0] = (uint8)((uint32)(v)) )

#define WRITE_32_BE(p, v) (((uint8 *)(p))[0] = (uint8)((uint32)(v) >> 24), \
                           ((uint8 *)(p))[1] = (uint8)((uint32)(v) >> 16), \
                           ((uint8 *)(p))[2] = (uint8)((uint32)(v) >>  8), \
                           ((uint8 *)(p))[3] = (uint8)((uint32)(v)) )

/*************************************************************************
 * SHARK: system
 *************************************************************************
 *
 * SYSTEM MANAGEMENT
 * -----------------
 *
 * SHARK implements an eco-system that represents abstracted hardware.
 * A FIFO based event queue is implemented that drives the application
 * execution. _penEvents and _keyEvents are generated based on system 
 * level user input devices where _nilEvents are generated based on a 
 * frames-per-second basis.
 *
 * SHARK reserves space for an application globals data value that is 
 * required to implement global variable support. A number of platforms, 
 * due to their design may not allow native global variable support.
 *
 * A SHARK application can query device information (name, type), access 
 * system date/time values and also attempt to perform background tasks.
 * (if threading is not possible, the task is blocking) 
 *
 * A debugging message (dialog or logfile) interface is provided if 
 * the underlying platform supports such a feature.
 *
 *************************************************************************/

//------------------------------------------------------------------------
// constants and enumerations
//------------------------------------------------------------------------

typedef enum sys_event
{
  _nilEvent = 0,
  _appStartEvent,        // globals initialization
  _appGoEvent,           // application startup
  _appStopEvent,         // application shutdown
  _penEvent,
  _keyEvent,
  _userEventBase = 0x1f
} sys_event;

#define PLATFORM_UNKNOWN        0x00000000
#define PLATFORM_WINDOWS        0x00000001
#define PLATFORM_MACOSX         0x00000002
#define PLATFORM_LINUX          0x00000003
#define PLATFORM_QNX            0x00000004

#define PLATFORM_EMBEDDED       0x10000000

#define PLATFORM_GENERIC(a)     ((a) & 0x000000ff)
#define IS_PLATFORM_EMBEDDED(a) (((a) & PLATFORM_EMBEDDED) != 0)

//------------------------------------------------------------------------
// data types and structures
//------------------------------------------------------------------------

typedef struct event
{
  sys_event eType;

  union
  {
    void    *generic;

    struct
    {
      uint16 outstanding;
    } nil;

    struct 
    {
      int16  x;
      int16  y;
    } pen;

    struct 
    {  
      uint32 chr;
    } key;

  } data;

} event;

typedef struct datetime
{
  uint16 year;
  uint   month:4;
  uint   day  :5;
  uint   hour :5;
  uint   min  :6;
  uint   sec  :6;
} datetime;

typedef void (*_SysTaskCallback)(void *user);

//------------------------------------------------------------------------
// application programmer interface
//------------------------------------------------------------------------

#define PLATFORM_TYPE(a)       ((a) & 0x000000ff)

#ifdef SHARK_INTERNAL
static boolean
_SysInitialize();
#endif

extern uint32
_SysConvertDateTimeToSeconds(datetime *dt);

extern void
_SysDebugMessage(char *msg, boolean dialog);

extern void
_SysDialog(char *title, char *msg);

#ifdef SHARK_INTERNAL
static void
_SysDelay(uint32 millisec);
#endif

extern void
_SysDeviceInfo(uint32 *platform, char *name);

extern void
_SysEnqueueEvent(event *e);

#ifdef SHARK_INTERNAL
static boolean
_SysEventAvailable();
#endif

extern void
_SysFlushEventQueue(int16 eType);

extern void
_SysGetDateTime(datetime *dt);

#ifdef SHARK_INTERNAL
static boolean
_SysGetEvent(event *e);
#endif

extern void *
_SysGetGlobals();

extern uint32
_SysGetTime();

extern void
_SysSetGlobals(void *gbls);

extern void
_SysTask(sys_event id, _SysTaskCallback callback, void *user);

#ifdef SHARK_INTERNAL
static void
DAL_SysMutexLock();
#endif

#ifdef SHARK_INTERNAL
static void
DAL_SysMutexUnlock();
#endif

#ifdef SHARK_INTERNAL
static void
_SysTerminate();
#endif

/*************************************************************************
 * SHARK: audio
 *************************************************************************
 *
 * STREAMING AUDIO MANAGER
 * -----------------------
 *
 * SHARK provides a linear PCM based streaming audio engine.
 *
 * PCM (Pulse Code Modulation) is a digital representation of an 
 * analog signal where the magnitude of the signal is sampled 
 * regularly at uniform intervals, then quantized to a series of
 * symbols in a binary code.
 *
 *************************************************************************/

//------------------------------------------------------------------------
// constants and enumerations
//------------------------------------------------------------------------

#define STREAM_INVALID    -1
 
typedef enum snd_format
{ 
  snd_int8 = 0, 
  snd_uint8, 
  snd_int16
} snd_format;
 
typedef enum snd_rate
{ 
  snd_8khz = 0, 
  snd_11khz, 
  snd_16khz, 
  snd_22khz, 
  snd_44khz 
} snd_rate;
 
typedef enum snd_type
{ 
  snd_mono = 0,
  snd_stereo
} snd_type;
 
#define SND_STREAM_8KHZ   (1 << snd_8khz)
#define SND_STREAM_11KHZ  (1 << snd_11khz)
#define SND_STREAM_16KHZ  (1 << snd_16khz)
#define SND_STREAM_22KHZ  (1 << snd_22khz)
#define SND_STREAM_44KHZ  (1 << snd_44khz)
 
#define SND_STREAM_MONO   (0x1000 << snd_mono)
#define SND_STREAM_STEREO (0x1000 << snd_stereo)

//------------------------------------------------------------------------
// data types and structures
//------------------------------------------------------------------------

typedef int32 snd_stream;
 
typedef boolean (*_SndStreamBufferCallback)
                   (snd_stream id, void *user, void *buf, uint32 frames);

//------------------------------------------------------------------------
// application programmer interface
//------------------------------------------------------------------------

#ifdef SHARK_INTERNAL
static boolean
_SndInitialize();
#endif

extern boolean     
_SndStreamAvailable();

extern snd_stream  
_SndStreamCreate(snd_rate rate, snd_format format, snd_type type, 
                 _SndStreamBufferCallback func, void *user, uint16 frames);

extern void        
_SndStreamDelete(snd_stream stream_id);

extern uint32
_SndStreamRatesSupported();

extern void
_SndStreamStart(snd_stream stream_id);

extern void
_SndStreamStop(snd_stream stream_id);

#ifdef SHARK_INTERNAL
static void
_SndTerminate();
#endif

/*************************************************************************
 * SHARK: file
 *************************************************************************/

//------------------------------------------------------------------------
// constants and enumerations
//------------------------------------------------------------------------

#define MAX_DISKS               10
#define MAX_ENUMERATION_ENTRIES 255
#define MAX_FILENAME_LENGTH     32
#define MAX_PATH_LENGTH         256

typedef enum file_position
{ 
  file_beginning = 0, 
  file_current,
  file_end 
} file_position;

#define DISK_INVALID            (uint32)-1

#define DISK_BASIC              0x00000001
#define DISK_SYSTEM             0x00000002
#define DISK_CLOBBER            0x00000010  // | CLOBBER (create)

#define FILE_STANDARD           0x00000001
#define FILE_DIRECTORY          0x00000002

#define FILE_READ_ONLY          0x00000001  // 'rb'
#define FILE_READ_WRITE         0x00000002  // 'wb+'
#define FILE_CLOBBER            0x00000010  // | CLOBBER (create if exist)

#define PATH_PROGRAM            0  // same directory as executable
#define PATH_COMMON             1  // common area
#define PATH_EXTERNAL_MEDIA     2  // root of external media

#ifdef SHARK_INTERNAL
#define PATH_RESOURCE           3  // where resources are stored 
#define PATH_PREFERENCE         4  // where preferences are stored
#endif

//------------------------------------------------------------------------
// data types and structures
//------------------------------------------------------------------------

typedef struct file file;

//------------------------------------------------------------------------
// application programmer interface
//------------------------------------------------------------------------

#define DISK_TYPE(a)            ((a) & 0x0000000f)
#define FILE_TYPE(a)            ((a) & 0x0000000f)

#define FILE_ACCESS(a)          ((a) & 0x0000000f)

#ifdef SHARK_INTERNAL
static boolean
_FileInitialize();
#endif

extern boolean
_DiskEnumerate(iterator *it, uint32 *disk, char *name);

extern boolean
_FileDefineDisk(char *id, uint32 area, char *name, uint32 mode);

extern uint32
_FileLocateDisk(char *id);

#ifdef SHARK_INTERNAL
static void
_FileBuildLocalPath(file *dir, char *name, char *path);
#endif

extern void
_FileClose(file *f);

extern boolean
_FileCreate(uint32 disk, file *dir, char *name, uint32 attributes);

extern boolean
_FileDelete(file *f);

extern boolean
_FileEnumerate(uint32 disk, file *dir, 
               iterator *it, char *name, uint32 *attributes);

extern boolean
_FileEOF(file *f);

extern void
_FileGetAttributes(file *f, uint32 *disk, 
                   file **dir, char *name, uint32 *attributes);

extern file *
_FileGetHandle(uint32 disk, file *dir, char *name);

extern boolean
_FileOpen(file *f, uint32 mode);

extern error
_FileRead(file *f, void *buf, uint32 size, uint32 *cnt); 

extern void
_FileReleaseHandle(file *f);

extern boolean
_FileRename(uint32 disk, file *dir, char *old_name, char *new_name);

extern void
_FileSeek(file *f, int32 ofs, file_position position);

extern uint32 
_FileTell(file *f);

extern error
_FileWrite(file *f, void *buf, uint32 size, uint32 *cnt);

#ifdef SHARK_INTERNAL
static void
_FileVolumeChange(uint32 disk);
#endif

#ifdef SHARK_INTERNAL
static void
_FileVolumeRestore();
#endif

#ifdef SHARK_INTERNAL
static void
_FileTerminate();
#endif

/*************************************************************************
 * SHARK: LCD
 *************************************************************************
 * 
 * DISPLAY (LCD) DRIVER
 * ---------------------
 *
 * SHARK provides a virtual frame buffer to the device LCD interface. 
 *
 * The display format is defined as the most optimal format for 
 * the device - which can vary greatly between devices. The display 
 * can either be grayscale or direct (ARGB or BGRA) where colors are 
 * defined as combinations of red, green and blue components.
 *
 * Screen orientation is provided with an optimized software solution.
 *
 * DPI (dots-per-inch) is defined on based on the resolution of the 
 * host display and is fixed. It should be used as a guide when 
 * preparing graphics and text for presentation. 72 dpi typically 
 * represents a resolution of 800x600 on a 14 inch monitor.
 *
 *************************************************************************/

//------------------------------------------------------------------------
// constants and enumerations
//------------------------------------------------------------------------

typedef enum lcd_type
{
  lcd_grayscale = 0,
  lcd_direct
} lcd_type;

typedef enum lcd_dpi
{ 
  lcd_dpi72 = 0, 
  lcd_dpi96, 
  lcd_dpi144, 
  lcd_dpi192 
} lcd_dpi;
 
typedef enum lcd_orientation 
{ 
  lcd_rotate0 = 0, 
  lcd_rotate90, 
  lcd_rotate180, 
  lcd_rotate270 
} lcd_orientation;

//------------------------------------------------------------------------
// data types and structures
//------------------------------------------------------------------------

typedef struct lcd_format
{
  lcd_type type;

  uint16   depth;       // how many bits per pixel?
  uint16   depth_bus;   // what is the bus width of each pixel?

  union
  {
    struct
    {
      boolean inverted; // 0 = white or black
    } grayscale;

    struct
    {
      uint8   alpha;
      uint8   red;
      uint8   green;
      uint8   blue;     // color channels, sum() = depth

      uint32  a_mask;
      uint32  r_mask;
      uint32  g_mask;
      uint32  b_mask;   // allows definition of ARGB and BGRA
    } direct;
  } data;

} lcd_format;

typedef struct lcd_region
{
  uint16 x;
  uint16 y;
  uint16 width;
  uint16 height;
} lcd_region;

//------------------------------------------------------------------------
// application programmer interface
//------------------------------------------------------------------------

#ifdef SHARK_INTERNAL
static boolean
_LCDInitialize(uint16 width, uint16 height);
#endif

#ifdef SHARK_INTERNAL
static void
_LCDInitialize_RESIZE(uint16 width, uint16 height);
#endif

#ifdef SHARK_INTERNAL
static void
_LCDTerminate_RESIZE();
#endif

extern void
_LCDGetProperties(void **lcd, 
                  uint16 *width, uint16 *height, uint16 *row_bytes, 
                  lcd_format *format, 
                  lcd_orientation *orientation, lcd_dpi *dpi);

extern void
_LCDLock();

extern void
_LCDRepaintRegion(lcd_region *region);

#ifdef SHARK_INTERNAL
static void
_LCDRepaintRegion_1(lcd_region *region);
#endif

#ifdef SHARK_INTERNAL
static void
_LCDRepaintRegion_2(lcd_region *region);
#endif

#ifdef SHARK_INTERNAL
static void
_LCDRepaintRegion_4(lcd_region *region);
#endif

#ifdef SHARK_INTERNAL
static void
_LCDRepaintRegion_8(lcd_region *region);
#endif

#ifdef SHARK_INTERNAL
static void
_LCDRepaintRegion_12(lcd_region *region);
#endif

#ifdef SHARK_INTERNAL
static void
_LCDRepaintRegion_16(lcd_region *region);
#endif

#ifdef SHARK_INTERNAL
static void
_LCDRepaintRegion_24(lcd_region *region);
#endif

#ifdef SHARK_INTERNAL
static void
_LCDRepaintRegion_32(lcd_region *region);
#endif

extern void
_LCDRepaint();

#ifdef SHARK_INTERNAL
static void
_LCDResume();
#endif

extern void
_LCDSetOrientation(lcd_orientation orientation);

#ifdef SHARK_INTERNAL
static void
_LCDSuspend();
#endif

extern void
_LCDUnlock();

#ifdef SHARK_INTERNAL
static void
_LCDTerminate();
#endif

/*************************************************************************
 * SHARK: library
 *************************************************************************
 *
 * LIBRARY MANAGEMENT
 * ------------------
 *
 * SHARK enables the registration and use of additional libraries that
 * can be integrated at a system level rather than at application level.
 * This allows cleaner code and ensures the libraries receive events.
 *
 * NOTE:
 * each SHARK library should have a unique id associated with it.
 *
 *************************************************************************/

//------------------------------------------------------------------------
// constants and enumerations
//------------------------------------------------------------------------

#define MAX_LIBRARIES      10
#define MAX_LIBNAME_LENGTH 32

//------------------------------------------------------------------------
// data types and structures
//------------------------------------------------------------------------

typedef uint32 library;

typedef boolean (*_LibHandleEventCallback)(event *e);

//------------------------------------------------------------------------
// application programmer interface
//------------------------------------------------------------------------

#ifdef SHARK_INTERNAL
static boolean
_LibInitialize();
#endif

#ifdef SHARK_INTERNAL
static boolean
_LibHandleEvent(event *e);
#endif

extern void *
_LibGetGlobals(library id);

extern boolean 
_LibRegister(library id, char *name, _LibHandleEventCallback callback);

extern void
_LibSetGlobals(library id, void *gbls);

extern void
_LibShutdown(library id);

#ifdef SHARK_INTERNAL
static void
_LibTerminate();
#endif

/*************************************************************************
 * SHARK: input: key/pen/analog
 *************************************************************************
 *
 * INPUT MANAGEMENT 
 * -----------------
 *
 * The SHARK input manager handles two basic level of input events.
 *
 * Key Events:
 * key events are sent as unicode keycode values - the control range has
 * been modified to support additional keys for navigational purposes.
 *
 * Pen Events:
 * pen events are handled uniquely such that they are only created as 
 * events when the stylus/mouse/tablet is pressed - meaning an action
 * should be performed.
 *
 * There are quite a number of input alternatives available for use in
 * applications and it is impossible to abstract them harmogenously. In
 * addition to the basic event level is the logic input object - which 
 * allows the programmer to query specific device types and handle them
 * accordingly during a nilEvent.
 *
 *************************************************************************/

//------------------------------------------------------------------------
// constants and enumerations
//------------------------------------------------------------------------

typedef enum key
{
  // non-printable virtual key codes
  _key_null        = 0x0000,
  _key_up          = 0x0001,
  _key_down        = 0x0002,
  _key_left        = 0x0003,
  _key_right       = 0x0004,
  _key_select      = 0x0005,  
//_key_            = 0x0006,
//_key_            = 0x0007,
  _key_backspace   = 0x0008,
  _key_tab         = 0x0009,
//_key_            = 0x000a,
//_key_            = 0x000b,
//_key_            = 0x000c,
  _key_enter       = 0x000d,
  _key_home        = 0x000e,
  _key_end         = 0x000f,
  _key_pageup      = 0x0010,
  _key_pagedown    = 0x0011,
  _key_scrollup    = 0x0012,
  _key_scrolldown  = 0x0013,
  _key_help        = 0x0014,
  _key_menu        = 0x0015,
  _key_acknowledge = 0x0016,
  _key_cancel      = 0x0017,  
//_key_            = 0x0018,
//_key_            = 0x0019,
//_key_            = 0x001a,
  _key_escape      = 0x001b,
//_key_            = 0x001c,
  _key_focus_gain  = 0x001d,
  _key_focus_lost  = 0x001e,
  _key_lcd_resize  = 0x001f,

  // printable unicode key codes
  _key_space       = 0x0020,
  _key_delete      = 0x007f,

  _key_user_base   = 0xe000,
  _key_user_max    = 0xf8ff

} key;

typedef enum input_device
{
  input_stylus,           // (x,y) = lcd co-ord, value = unused
  input_pointer,          // (x,y) = lcd co-ord, value = button state
  input_tablet,           // (x,y) = lcd co-ord, value = pressure

  input_dpad,             // (x,y) = direction,  value = button state
  input_joystick,         // (x,y) = direction,  value = unused
  input_analog,           // (x,y) = direction,  value = unused

  input_gyroscope,        // (x,y) = unused,     value = angle
  input_accelerometer     // (x,y) = unused,     value = speed

} input_device;

#define INPUT_VALUE_EXTREME  0x7fff

//------------------------------------------------------------------------
// data types and structures
//------------------------------------------------------------------------

typedef struct input
{
  input_device type;

  int16        x;
  int16        y;
  int16        value;
} input;

//------------------------------------------------------------------------
// application programmer interface
//------------------------------------------------------------------------

#ifdef SHARK_INTERNAL
static boolean
_InputInitialize();
#endif

extern boolean
_InputAvailable(input_device type);

extern void
_InputState(input *device);

extern void
_InputKeyboardSetVisible(boolean visible);

#ifdef SHARK_INTERNAL
static void
_KeyHandleEvent(uint16 keycode);
#endif

extern uint32
_KeyState(uint16 *keyX, uint16 *keyY);

#ifdef SHARK_INTERNAL
static void
_PenHandleEvent(int16 penX, int16 penY);
#endif

extern void
_PenState(uint16 *penX, uint16 *penY, boolean *penDown);
 
#ifdef SHARK_INTERNAL
static void
_InputTerminate();
#endif

/*************************************************************************
 * SHARK: memory
 *************************************************************************
 *
 * MEMORY MANAGEMENT 
 * -----------------
 *
 * Memory Management is handled different on every platform.
 *
 * SHARK provides an internally managed memory environment, that 
 * allows for two types of memory allocations, read-only and 
 * read-write, which effectively map to what is known as storage 
 * and dynamic heaps respectively. Not all devices have a storage 
 * heap and it is simulated in such cases.
 *
 * Memory allocation and deallocation is managed internally, which 
 * makes it impossible for a SHARK application to have internal 
 * memory leaks. In the event the developer forgets to release 
 * memory the memory management unit cleans up all allocations on 
 * the programs termination.
 *
 * NOTE
 * When logging is enabled all memory allocations, deallocations 
 * and memory leaks are reported to assist with debugging purposes.
 *
 *************************************************************************/

//------------------------------------------------------------------------
// constants and enumerations
//------------------------------------------------------------------------

// none

//------------------------------------------------------------------------
// data types and structures
//------------------------------------------------------------------------

// none

//------------------------------------------------------------------------
// application programmer interface
//------------------------------------------------------------------------

#ifdef SHARK_INTERNAL
static boolean
_MemInitialize();
#endif

extern uint32
_MemAvailable(boolean storage);

extern void *
_MemCopy(void *pDst, void *pSrc, uint32 count);

extern void *
_MemMove(void *pDst, void *pSrc, uint32 count);

extern void
_MemPtrFree(void *p);

extern void *
_MemPtrNew(uint32 size, boolean storage);

extern uint32 
_MemPtrSize(void *p);

extern void 
_MemSemaphore(boolean active);

extern void *
_MemSet(void *p, uint8 value, uint32 count);

#ifdef SHARK_INTERNAL
static void
_MemTerminate();
#endif

/*************************************************************************
 * SHARK: preferences
 *************************************************************************
 *
 * PREFERENCE MANAGEMENT
 * ---------------------
 *
 * SHARK provides the ability to save 'preference' information on the
 * target platform - namely for storing user preferences or save state
 * information to be used for the next user session.
 *
 *************************************************************************/

//------------------------------------------------------------------------
// constants and enumerations
//------------------------------------------------------------------------

// none

//------------------------------------------------------------------------
// data types and structures
//------------------------------------------------------------------------

// none

//------------------------------------------------------------------------
// application programmer interface
//------------------------------------------------------------------------

#ifdef SHARK_INTERNAL
static boolean
_PreferencesInitialize();
#endif

extern void *
_PreferencesGetReference();

extern void
_PreferencesLoad();

extern void
_PreferencesSave();

#ifdef SHARK_INTERNAL
static void
_PreferencesTerminate();
#endif

/*************************************************************************
 * SHARK: resource
 *************************************************************************
 *
 * RESOURCE MANAGEMENT
 * -------------------
 *
 * SHARK provides the ability to "bundle" resources with the application
 * binary to assist in delivery of the product - such as allowing the 
 * user to download and use the product as a single binary. 
 *
 * NOTE
 * PalmOS has a 64K limit on HotSync - see: NutShell for installation
 *
 *************************************************************************/

//------------------------------------------------------------------------
// constants and enumerations
//------------------------------------------------------------------------

// none

//------------------------------------------------------------------------
// data types and structures
//------------------------------------------------------------------------

typedef uint16 resource;

//------------------------------------------------------------------------
// application programmer interface
//------------------------------------------------------------------------

#ifdef SHARK_INTERNAL
static boolean
_ResourceInitialize();
#endif

extern void *
_ResourceLock(resource id);

extern void
_ResourceUnlock(void *p);

extern uint32
_ResourceSize(resource id);

#ifdef SHARK_INTERNAL
static void
_ResourceTerminate();
#endif

/*************************************************************************
 * SHARK: socket
 *************************************************************************
 *
 * NETWORKING SOCKETS
 * ------------------
 *
 * SHARK defines a TCP and UDP basic networking sockets interface.
 *
 * A socket is defined using a hostname (name or ip address), port and 
 * the protocol (TCP - stream, UDP - datagram) which should be used.
 * When communicating using sockets it is important to treat the data
 * stream in a platform independent manner.
 *
 * A proxy-server handler using HTTP tunnelling with authentication is 
 * implemented for TCP based sockets - required when connecting from 
 * within corporate intranets to the Internet.
 *
 *************************************************************************/

//------------------------------------------------------------------------
// constants and enumerations
//------------------------------------------------------------------------

#define MAX_HOSTNAME_LENGTH 128

#define SOCKET_TCP          1 // stream
#define SOCKET_UDP          2 // datagram

//------------------------------------------------------------------------
// data types and structures
//------------------------------------------------------------------------

typedef struct sock sock;

//------------------------------------------------------------------------
// application programmer interface
//------------------------------------------------------------------------

#ifdef SHARK_INTERNAL
static boolean
_SocketInitialize();
#endif

extern void
_SocketClose(sock *s);

extern sock *
_SocketGetHandle(char *host, uint16 port, uint32 mode);

extern boolean
_SocketOpen(sock *s);

extern error
_SocketRead(sock *s, void *buf, uint32 len, uint32 *cnt);

extern error
_SocketReadLine(sock *s, void *buf, uint32 len, uint32 *cnt);

extern error
_SocketReadWaitAll(sock *s, void *buf, uint32 len, uint32 *cnt);

extern void
_SocketReleaseHandle(sock *s);

extern void
_SocketSetProxy(sock *s, char *host, uint16 port, char *user, char *pass);

extern error
_SocketWrite(sock *s, void *buf, uint32 len, uint32 *cnt);

#ifdef SHARK_INTERNAL
static void
_SocketTerminate();
#endif

/*************************************************************************
 * SHARK: architecture: internal 
 *************************************************************************
 * 
 * SHARK INTERNALS 
 * ---------------
 *
 * SHARK provides an abstract eco-system to simulate a virtual device.
 * A number of *internal* functionality requirements exists to ensure
 * the application correctly receives and handles system events.
 *
 *************************************************************************/

//------------------------------------------------------------------------
// data types and structures
//------------------------------------------------------------------------

// none

//------------------------------------------------------------------------
// application programmer interface
//------------------------------------------------------------------------

#ifdef SHARK_INTERNAL
static boolean
_SHARKInitialize(uint16 width, uint16 height);
#endif

#ifdef SHARK_INTERNAL
static void
_SHARKDispatchEvents();
#endif

#ifdef SHARK_INTERNAL
static boolean
_SHARKHandleEvent(event *e);
#endif

#ifdef SHARK_INTERNAL
#if (LOG_DISABLE == 0)
static void
_SHARKLogCreateEntry(char *str);
#endif
#endif

#ifdef SHARK_INTERNAL
static void
_SHARKTerminate();
#endif

/*=======================================================================*
 * Application Programmer Interface
 *=======================================================================*/

//------------------------------------------------------------------------
// constants and enumerations
//------------------------------------------------------------------------

// none

//------------------------------------------------------------------------
// data types and structures
//------------------------------------------------------------------------

typedef struct application_config
{
  char            name[32];        // application name
  uint32          version;         // version number
 
  char            company[32];     // company name
  char            domain[128];     // company internet domain 
 
  struct
  {
    uint16        fps;             // frames per second (x10)
  } event;
 
  struct
  {
    uint16        count;           // max pointers required
 
    uint32        min_heap;        // min dynamic heap
    uint32        max_heap;        // max dynamic heap
    uint32        min_storage;     // min storage heap
    uint32        max_storage;     // max storage heap
  } memory;
 
  struct
  {
    uint16        size;            // size of preference structure
  } preferences;
 
  struct
  { 
    boolean       active;

    uint8         r;               // number of red   bits
    uint8         g;               // number of green bits
    uint8         b;               // number of blue  bits (xRGB)
  } framebuffer;

} application_config;

//------------------------------------------------------------------------
// application programmer interface
//------------------------------------------------------------------------

#define VER_GENERATE(maj, min)  ((((maj) & 0xff) << 8) | ((min) & 0xff))
#define VER_MAJOR(ver)          (((ver) >> 8) & 0xff)
#define VER_MINOR(ver)          ((ver) & 0xff)

extern void
ApplicationConfig(application_config *config);

extern boolean
ApplicationInitialize();

extern boolean
ApplicationHandleEvent(event *e);

extern void
ApplicationTerminate();

#endif

/********************************* EOF ***********************************/
