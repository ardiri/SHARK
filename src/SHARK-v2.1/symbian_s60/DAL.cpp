/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL.cpp
 */

// system includes
#include <aknapp.h>
#include <aknappui.h>
#include <aknsoundsystem.h>
#include <aknnotewrappers.h>
#include <eikdoc.h>
#include <hal.h>
#include <mdaaudiooutputstream.h>
#include <mda/common/audio.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <sys/dirent.h>

// SHARK includes
#include "../SHARK.h"
#include "../SHARK-prv.h"

extern "C"
{
  extern uint32 SymbianGetUniqueID();
}

// ---------------------------------------------------------
// Class Definitions (base for symbian)
// ---------------------------------------------------------

// forward declarations
class Application;
class Document;
class AppUi;
class AppView;
class AppThread;
class AppAudioStream;

// ---------------------------------------------------------
// Application Class
// ---------------------------------------------------------

class Application : public CAknApplication
{
// functions
public:
  CApaDocument* CreateDocumentL();
  TUid AppDllUid() const;

// variables
};

// ---------------------------------------------------------
// Document Class
// ---------------------------------------------------------

class Document : public CEikDocument
{
public:
  static Document* NewL(CEikApplication& aApp);
  static Document* NewLC(CEikApplication& aApp);
  Document(CEikApplication& aApp);
  ~Document();
  void ConstructL();
  void DestructL();

// functions
  CEikAppUi *CreateAppUiL();

// variables
  TFileName  iPath;
};

// ---------------------------------------------------------
// AppUI Class
// ---------------------------------------------------------

class AppUi : public CAknAppUi
{
public:
  AppUi(TFileName path);
  ~AppUi();
  void ConstructL();
  void DestructL();

// functions
  void Exit();
  void HandleCommandL(TInt aCommand);
  void HandleForegroundEventL(TBool aForeground);

// variables
};

// ---------------------------------------------------------
// AppView Class
// ---------------------------------------------------------

class AppView : public CCoeControl
{
public:
  static AppView* NewL(const TRect& aRect);
  static AppView* NewLC(const TRect& aRect);
  AppView();
  ~AppView();
  void ConstructL(const TRect& aRect);
  void DestructL();

// functions
  void Draw(const TRect& aRect) const;

  void HandlePointerEventL(const TPointerEvent& aPointerEvent);
  TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);
  TCoeInputCapabilities InputCapabilities() const;

// variables
};

// ---------------------------------------------------------
// AppThread Class
// ---------------------------------------------------------

class AppThread : public CTimer, MDirectScreenAccess
{
public:
  AppThread(RWsSession& aClient, CWsScreenDevice& aDevice, RWindow& aWindow);
  ~AppThread();
  void ConstructL();
  void DestructL();

// functions
  void StartInteraction();
  void StopInteraction();
  void CancelTimerRequest();
  void RunL();
  void Restart(RDirectScreenAccess::TTerminationReasons aReason);
  void AbortNow(RDirectScreenAccess::TTerminationReasons aReason);

// variables
  boolean              iThreadActive;
  RWsSession&          iClient;
  CWsScreenDevice&     iScreenDevice;
  RWindow&             iWindow;
  CDirectScreenAccess* iDirectScreenAccess;
};

// ---------------------------------------------------------
// AppAudioStream Class
// ---------------------------------------------------------

class AppAudioStream : public MMdaAudioOutputStreamCallback
{
public:
  AppAudioStream();
  virtual ~AppAudioStream();
  void ConstructL(TInt frames);
  void DestructL();

// functions
  void MaoscOpenComplete(TInt aError);
  void MaoscBufferCopied(TInt aError, const TDesC8& aBuffer);
  void MaoscPlayComplete(TInt aError);

  boolean Start(snd_rate rate, snd_format format, snd_type type, 
                void *user, _SndStreamBufferCallback callback);
  void    Stop();
  void    UpdateBuffer();

// variables
  CMdaAudioOutputStream    *iStream;
  TMdaAudioDataSettings     iSettings;
  TInt                      iRate;
  TInt                      iChannels;
  TInt                      iFormat;

  _SndStreamBufferCallback  iCallBack;
  void                     *iUserData;

  TInt                      iFrameSize;
  int16                    *iBuf16;                
  int8                     *iBuf8;                
  TPtr8                    *iSoundBuf;
};

/*************************************************************************
 *
 * Global Variables
 *
 *************************************************************************/

typedef struct
{
  AppUi              *appUi;
  AppView            *appView;
  AppThread          *appThread;
  AppAudioStream     *appAudioStream;

  CAknKeySoundSystem *soundSystem;

  FILE               *logfile;
  uint32              initialized;

  uint8               lcd_a;
  uint8               lcd_r;
  uint8               lcd_g;
  uint8               lcd_b;
  uint32              lcd_r_mask;
  uint32              lcd_g_mask;
  uint32              lcd_b_mask;

#define LCD_DEPTH (uint8)(((g_lcd_a+g_lcd_r+g_lcd_g+g_lcd_b) + 15) & ~0xf)

  boolean             focus;
  boolean             active;

  coord               width;
  coord               width_rb;
  coord               height;
  coord               height_rb;

  char                path[256];
  application_config  config;

  void                *mem;
  void                *lcd;
  void                *sys;
  void                *lib;
  void                *inp;
  void                *prf;
  void                *fil;
  void                *snd;
  void                *res;

} GlobalsType;

#define GLOBALS_DEFINE GlobalsType *g;
#define GLOBALS_ACCESS GlobalsType *g = (GlobalsType *)Dll::Tls();

#define g_appUi          g->appUi
#define g_appView        g->appView
#define g_appThread      g->appThread
#define g_appAudioStream g->appAudioStream
#define g_soundSystem    g->soundSystem
#define g_logfile        g->logfile
#define g_initialized    g->initialized
#define g_lcd_a          g->lcd_a
#define g_lcd_r          g->lcd_r
#define g_lcd_g          g->lcd_g
#define g_lcd_b          g->lcd_b
#define g_lcd_r_mask     g->lcd_r_mask
#define g_lcd_g_mask     g->lcd_g_mask
#define g_lcd_b_mask     g->lcd_b_mask
#define g_focus          g->focus
#define g_active         g->active
#define g_width          g->width
#define g_width_rb       g->width_rb
#define g_height         g->height
#define g_height_rb      g->height_rb
#define g_path           g->path
#define g_config         g->config

/*************************************************************************
 *
 * Local Dependencies
 *
 *************************************************************************/

extern "C"
{

#if (LOG_DISABLE == 0)
static void _LOGInitialize();
static void _LOGCreateEntry(char *str);
static void _LOGTerminate();
#endif

static boolean DAL_HandleEvent(event *e);
static void    DAL_DispatchEvents();

// any routines that are missing from the base libraries defined here
#include "DAL-missing.inc"

#include "DAL-memory.inc"
#include "DAL-display.inc"
#include "DAL-system.inc"
#include "DAL-library.inc"
#include "DAL-input.inc"
#include "DAL-preferences.inc"
#include "DAL-file.inc"
#include "DAL-audio.inc"
#include "DAL-resource.inc"

}

/*************************************************************************
 *
 * Platform Depending Application Framework
 *
 *************************************************************************/

extern "C"
{

#if (LOG_DISABLE == 0)
// initialize the logging process
static void
_LOGInitialize()
{
  uint32 platform;
  char   platform_name[256];
  char   fileName[256];
  uint32 input;
  GLOBALS_ACCESS;

  // application logfile file name
  sprintf(fileName, "%s%capplication-runtime.txt",
          g_path, DIRECTORY_SEPARATOR);

  g_logfile = fopen(fileName, "w");
  if (g_logfile != NULL)
  {
    fprintf(g_logfile, "APPLICATION_LOG_FILE:\n");
    fprintf(g_logfile, "---------------------\n");
    fprintf(g_logfile, "\n");

    _SysDeviceInfo(&platform, platform_name);
    fprintf(g_logfile, "PLATFORM:\n");
    fprintf(g_logfile, "platform id\t:: %s\n", platform_name);
    fprintf(g_logfile, "\n");

    input = _InputSupported();
    fprintf(g_logfile, "INPUT SUPPORTED:\n");
    fprintf(g_logfile, "stylus / mouse\t:: ");
    fprintf(g_logfile, (input & INPUT_STYLUS) ? "yes\n" : "no\n");
    fprintf(g_logfile, "digital keys\t:: ");
    fprintf(g_logfile, (input & INPUT_KEY) ? "yes\n" : "no\n");
    fprintf(g_logfile, "analog control\t:: ");
    fprintf(g_logfile, (input & INPUT_KEY_ANALOG) ? "yes\n" : "no\n");
    fprintf(g_logfile, "\n");

    fprintf(g_logfile, "SOFTWARE:\n");
    fprintf(g_logfile, "company    \t:: %s\n", g_config.company);
    fprintf(g_logfile, "application\t:: %s\n", g_config.name);
    fprintf(g_logfile, "\n");

    fprintf(g_logfile, "DISPLAY:\n");
    fprintf(g_logfile, "display depth\t:: %d\n", LCD_DEPTH);
    fprintf(g_logfile, "display RGB\t:: %d%d%d\n", g_lcd_r, g_lcd_g, g_lcd_b);
    fprintf(g_logfile, "display R mask\t:: 0x%08lx\n", g_lcd_r_mask);
    fprintf(g_logfile, "display G mask\t:: 0x%08lx\n", g_lcd_g_mask);
    fprintf(g_logfile, "display B mask\t:: 0x%08lx\n", g_lcd_b_mask);
    fprintf(g_logfile, "\n");

    fprintf(g_logfile, "AUDIO STREAMING:\n");
    if (_SndStreamAvailable())
    {
      uint32 rates;
      rates = _SndStreamRatesSupported();

      fprintf(g_logfile, "available  \t:: yes\n");
      fprintf(g_logfile, "rates      \t:: ");
 
      if (rates & SND_STREAM_8KHZ)   fprintf(g_logfile, "8Khz ");
      if (rates & SND_STREAM_11KHZ)  fprintf(g_logfile, "11Khz ");
      if (rates & SND_STREAM_16KHZ)  fprintf(g_logfile, "16Khz ");
      if (rates & SND_STREAM_22KHZ)  fprintf(g_logfile, "22Khz ");
      if (rates & SND_STREAM_44KHZ)  fprintf(g_logfile, "44Khz ");
      if (rates & SND_STREAM_MONO)   fprintf(g_logfile, "[mono] ");
      if (rates & SND_STREAM_STEREO) fprintf(g_logfile, "[stereo] ");
      if (rates == 0) fprintf(g_logfile, "- none -");
      fprintf(g_logfile, "\n");
    }
    else
      fprintf(g_logfile, "available  \t:: no\n");
  }
}

// create an entry in the log file
static void
_LOGCreateEntry(char *str)
{
  GLOBALS_ACCESS;

  if (g_logfile != NULL)
  {
    fprintf(g_logfile, "%s\n", str);
    fflush(g_logfile);
  }
}

// terminate the logging process; close log file
static void
_LOGTerminate()
{
  GLOBALS_ACCESS;

  if (g_logfile != NULL)
  {
    fprintf(g_logfile, "\n");
    fprintf(g_logfile, "- END\n");
    fclose(g_logfile);
  }
}
#endif

static boolean
DAL_HandleEvent(event *e)
{
  boolean processed;

  processed = _LibHandleEvent(e);
  if (!processed)
    processed = ApplicationHandleEvent(e);

  return processed;
}

static void
DAL_DispatchEvents()
{
  event  _e;
  uint32 eType;
  GLOBALS_ACCESS;

  // we have to ensure we have focus
  if (!g_focus) return;

  // platform independent 'processing' of events
  while (_SysEventAvailable())
  {
    _SysGetEvent(&_e);
    eType = _e.eType;
    DAL_HandleEvent(&_e);

    // application termination condition
    if (eType == _appStopEvent)
    {
      g_appThread->StopInteraction();
      g_appUi->Exit();
    }
  }
}

static boolean
GlobalsInitialize()
{
  boolean result;
  GLOBALS_ACCESS;

  // initialize all sub-components
  result = true;
  result &= _MemInitialize();  // REQ: must be done first
  result &= _LCDInitialize();  // REQ: must be done second
  result &= _SysInitialize();
  result &= _LibInitialize();
  result &= _InputInitialize();
  result &= _PreferencesInitialize();
  result &= _FileInitialize();
  result &= _SndInitialize();
  result &= _ResourceInitialize();

  // initialize the default orientation state based on window resolution
  _LCDSetOrientation(lcd_rotate0);

  // load any additional libraries
  if (g_config.gfx.active)
    result &= _GfxInitialize(&g_config.gfx.format);
  if ((g_config.font.active) && (g_config.gfx.active))
    result &= _FntInitialize(&g_config.gfx.format,
                              g_config.font.encoding,
                              g_config.font.style);
  if (g_config.sfx.active)
    result &= _SfxInitialize(g_config.sfx.rate,
                             g_config.sfx.format,
                             g_config.sfx.type,
                             g_config.sfx.frames,
                             g_config.sfx.mix_count);
  if ((g_config.font.active) && (g_config.gfx.active) && (g_config.gui.active))
    result &= _GUIInitialize(g_config.gui.face);

  return result;
} 

static void
GlobalsTerminate()
{
  GLOBALS_ACCESS;

  // shutdown any additional libraries
  if ((g_config.font.active) &&
      (g_config.gfx.active) &&
      (g_config.gui.active)) _GUITerminate();
  if (g_config.sfx.active)   _SfxTerminate();
  if ((g_config.font.active) &&
      (g_config.gfx.active)) _FntTerminate();
  if  (g_config.gfx.active)  _GfxTerminate();

  _ResourceTerminate();
  _SndTerminate();
  _FileTerminate();
  _PreferencesTerminate();
  _InputTerminate();
  _LibTerminate();
  _SysTerminate();
  _LCDTerminate();             // REQ: must be done second last
  _MemTerminate();             // REQ: must be done last
}

}

// ---------------------------------------------------------
// EPOC/Symbian Interface
// ---------------------------------------------------------

GLDEF_C TInt 
E32Dll(TDllReason)
{
  return KErrNone;
}

EXPORT_C CApaApplication *
NewApplication()
{
  return (new Application);
}

// ---------------------------------------------------------
// Application Class
// ---------------------------------------------------------

CApaDocument *
Application::CreateDocumentL()
{
  CApaDocument* document = Document::NewL(*this);
  return document;
}

TUid 
Application::AppDllUid() const
{
  TUid id = { SymbianGetUniqueID() };
  return id;
}

// ---------------------------------------------------------
// Document Class
// ---------------------------------------------------------

Document *
Document::NewL(CEikApplication& aApp)
{
  Document* self = NewLC(aApp);
  CleanupStack::Pop();
  return self;
}

Document *
Document::NewLC(CEikApplication& aApp)
{
  Document* self = new (ELeave)Document(aApp);
  CleanupStack::PushL(self);
  self->ConstructL();
  return self;
}

Document::Document(CEikApplication& aApp) 
  : CEikDocument(aApp)
{
  iPath = aApp.AppFullName();
}

Document::~Document()
{
  DestructL();
}

void 
Document::ConstructL()
{
}

void
Document::DestructL()
{
}

CEikAppUi *
Document::CreateAppUiL()
{
  CEikAppUi* appUi = new (ELeave)AppUi(iPath);
  return appUi;
}

// ---------------------------------------------------------
// AppUI Class
// ---------------------------------------------------------

AppUi::AppUi(TFileName path)
{
  GLOBALS_DEFINE;

  // globals:: allocate appropriate memory
  g = (GlobalsType *)malloc(sizeof(GlobalsType));
  memset(g, 0, sizeof(GlobalsType));
  Dll::SetTls(g);

  g_appUi = this;

  // obtain the application configuration parameters
  ApplicationConfig(&g_config);

  // lets get the path to the application sorted out
  TBuf8<256> buf8;
  buf8.Zero();
  buf8.Copy(path);
  strcpy(g_path, (char *)buf8.Ptr());
  *((char *)(strrchr(g_path, DIRECTORY_SEPARATOR))) = 0;  
}

AppUi::~AppUi()
{
  GLOBALS_ACCESS;

  DestructL();

  g_appUi = NULL;

  // globals:: clean up memory
  Dll::SetTls(NULL);
  free(g); g = NULL;
}

void 
AppUi::ConstructL()
{
  TRect rect;
  TInt  mRGB;
  GLOBALS_ACCESS;

#ifdef _S60_OLD_
#define EStandardAppFlag    0x00
#define ELayoutAwareAppFlag 0x08
#endif
  BaseConstructL(EStandardAppFlag | ELayoutAwareAppFlag);

  // hide the status pane (we dont want this - we want full screen!)
  ((CAknAppUi*)iEikonEnv->EikAppUi())->StatusPane()->MakeVisible(EFalse);

// LCD SETTINGS
// symbian s60 - can use 444, 565 (16bit) or 888 display modes

  // this is our "setting" for our LCD engine (hard coded)
  CWsScreenDevice *device = (CWsScreenDevice *)iEikonEnv->ScreenDevice();
  switch (device->DisplayMode())
  {
    case EColor4K:
         g_lcd_a = 4;
         g_lcd_r = 4;
         g_lcd_g = 4;
         g_lcd_b = 4;   // 12 bits (using 16 bits)
         break;

    case EColor64K:
         g_lcd_a = 0;
         g_lcd_r = 5;
         g_lcd_g = 6;
         g_lcd_b = 5;   // 16 bits (using 16 bits)
         break;

#ifdef _S60_OLD_
    case (EColor4K+1): // EColor16MU
    case (EColor4K+2): // EColor16MA
#else
    case EColor16MU:
    case EColor16MA:
#endif
         g_lcd_a = 0;
         g_lcd_r = 8;
         g_lcd_g = 8;
         g_lcd_b = 8;  // 24 bits (using 32 bits)
         break;

    default:
         // unsupported graphics modes
         break;
  }

  mRGB = 1;
#ifdef _S60_OLD_
  HAL::Get((HALData::TAttribute)(HALData::ESystemDrive+10), mRGB);
#else
  HAL::Get(HALData::EDisplayIsPixelOrderRGB, mRGB);
#endif
  if (mRGB)
  {
    // ARGB (standard)
    g_lcd_r_mask = ((0xff >> (8 - g_lcd_r)) << (g_lcd_g + g_lcd_b));
    g_lcd_g_mask = ((0xff >> (8 - g_lcd_g)) << (g_lcd_b));
    g_lcd_b_mask =  (0xff >> (8 - g_lcd_b));
  }
  else
  {
    // ABGR (windows)
    g_lcd_b_mask = ((0xff >> (8 - g_lcd_b)) << (g_lcd_g + g_lcd_r));
    g_lcd_g_mask = ((0xff >> (8 - g_lcd_g)) << (g_lcd_r));
    g_lcd_r_mask =  (0xff >> (8 - g_lcd_r));
  }

  // create the audio stream
  g_appAudioStream = new AppAudioStream();
  g_appAudioStream->ConstructL(2048);

#if (LOG_DISABLE == 0)
  // initialize the log file function
  _LOGInitialize();
#endif

  // standard default window size
  rect = ApplicationRect();
  g_width     = rect.Width();
  g_width_rb  = (uint16)(((g_width  + 3) & ~3) * (LCD_DEPTH >> 3));
  g_height    = rect.Height();
  g_height_rb = (uint16)(((g_height + 3) & ~3) * (LCD_DEPTH >> 3));

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _LOGCreateEntry("\n:: - hardware abstraction initialize -\n");
#endif

  g_appView = AppView::NewL(rect);
  AddToStackL(g_appView);

  g_soundSystem = KeySounds();
  g_soundSystem->PushContextL(0x8cc0116); // R_AVKON_SILENT_SKEY_LIST
  CAknAppUi::SetKeyBlockMode(ENoKeyBlock);

  // initialize the globals
  g_initialized = 0;
  if (GlobalsInitialize())
  {
    g_initialized++;

#if (LOG_DISABLE == 0)
    // LOGGING EVENT
    _LOGCreateEntry("\n:: - application initialize -");
#endif

    if (ApplicationInitialize())
    {
      g_initialized++;

      // load the application preferences
      _PreferencesLoad();
    }
  }

  // did something go wrong?
  if (g_initialized < 2) Exit();
}

void
AppUi::DestructL()
{
  GLOBALS_ACCESS;

  // shutdown process
  if (g_initialized > 0)
  {
    if (g_initialized > 1)
    {
      // save the application preferences
      _PreferencesSave();
      ApplicationTerminate();
    }

#if (LOG_DISABLE == 0)
    // LOGGING EVENT
    _LOGCreateEntry(":: - application terminate -\n");
#endif

    // terminate our globals
    GlobalsTerminate();
  }

#if (LOG_DISABLE == 0)
  // LOGGING EVENT
  _LOGCreateEntry("\n:: - hardware abstraction terminate -");

  // shutdown the log file function
  _LOGTerminate();
#endif

  if (g_appAudioStream)
  {
    delete g_appAudioStream;
    g_appAudioStream = NULL;
  }

  g_soundSystem->PopContext();
  CAknAppUi::SetKeyBlockMode(EDefaultBlockMode);

  if (g_appView)
  {
    RemoveFromStack(g_appView);

    delete g_appView;
    g_appView = NULL;
  }
}

void
AppUi::Exit()
{
  GLOBALS_ACCESS;

  g_appThread->CancelTimerRequest();
  CAknAppUi::Exit();
}

void 
AppUi::HandleCommandL(TInt aCommand)
{
//GLOBALS_ACCESS;

  switch(aCommand)
  {
    case EEikCmdExit:
    case EAknSoftkeyExit:
         _SysEnqueueQuitEvent();
         break;

    default:
         break;
  }
}

void 
AppUi::HandleForegroundEventL(TBool aForeground)
{
//GLOBALS_ACCESS;
  
  if (aForeground) 
  {
    _LCDResume();
    _SndStreamStart(0);
  }
  else
  {
    _LCDSuspend();
    _InputKeyStateSet(0);
    _SndStreamStop(0);
  }

  CAknAppUi::HandleForegroundEventL(aForeground);
}

// ---------------------------------------------------------
// AppView Class
// ---------------------------------------------------------

AppView *
AppView::NewL(const TRect& aRect)
{
  AppView* self;
  self = AppView::NewLC(aRect);
  CleanupStack::Pop();
  return self;
}

AppView *
AppView::NewLC(const TRect& aRect)
{
  AppView* self = new (ELeave)AppView();
  CleanupStack::PushL(self);
  self->ConstructL(aRect);
  return self;
}

AppView::AppView()
{
  GLOBALS_ACCESS;

  g_appView = this;
}

AppView::~AppView()
{
  GLOBALS_ACCESS;

  DestructL();

  g_appView = NULL;
}

void 
AppView::ConstructL(const TRect& aRect)
{
  GLOBALS_ACCESS;

  CreateWindowL();
  SetRect(aRect);

  g_appThread = 
    new AppThread(iEikonEnv->WsSession(), 
                  *(CCoeEnv::Static()->ScreenDevice()), Window());
  g_appThread->ConstructL();

  // default interactivity values
  g_focus  = true;
  g_active = false;

  ActivateL();
  EnableDragEvents();
}

void
AppView::DestructL()
{
  GLOBALS_ACCESS;

  if (g_appThread != NULL)
  {
    delete g_appThread;
    g_appThread = NULL;
  }
}

void 
AppView::Draw(const TRect& aRect) const
{
  GLOBALS_ACCESS;

  // coming back into focus? must redraw whole screen :)
  if ((g_focus != IsFocused()) && (IsFocused()))
  {
    // this is the first point that we know the program is alive! start!
    if (!g_active)
    {
      event _e;

      memset(&_e, 0, sizeof(event));
      _e.eType = _appStartEvent;
      DAL_HandleEvent(&_e);

      memset(&_e, 0, sizeof(event));
      _e.eType = _appGoEvent;
      DAL_HandleEvent(&_e);
    }
    g_active = true;

    _LCDRepaint();
  }

  g_focus = IsFocused();
  g_appThread->StartInteraction();
}

void 
AppView::HandlePointerEventL(const TPointerEvent& aPointerEvent)
{
//GLOBALS_ACCESS;
}

TKeyResponse 
AppView::OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType)
{
  TKeyResponse result;
  boolean      processed;
  event        e;
//GLOBALS_ACCESS;

  result = EKeyWasNotConsumed;
  processed = _KeyHandleEvent(aKeyEvent.iScanCode, aType, &e);
  if (processed)
    if (DAL_HandleEvent(&e)) result = EKeyWasConsumed;

  return result;
}

TCoeInputCapabilities 
AppView::InputCapabilities() const
{
//GLOBALS_ACCESS;

  // this class does not implement any 'unusual' input capabilities
  return TCoeInputCapabilities::ENone;
}

// ---------------------------------------------------------
// AppThread Class
// ---------------------------------------------------------

AppThread::AppThread(RWsSession& aClient, 
                     CWsScreenDevice& aDevice, RWindow& aWindow)
  : CTimer(CActive::EPriorityHigh),
    iClient(aClient),
    iScreenDevice(aDevice),
    iWindow(aWindow)
{
  GLOBALS_ACCESS;

  g_appThread = this;
}

AppThread::~AppThread()
{
  GLOBALS_ACCESS;

  DestructL();

  g_appThread = NULL;
}

void 
AppThread::ConstructL()
{
//GLOBALS_ACCESS;

  CTimer::ConstructL();

  // create the direct screen access object
  iDirectScreenAccess = 
    CDirectScreenAccess::NewL(iClient, iScreenDevice, iWindow, *this);

  CActiveScheduler::Add(this);
}

void
AppThread::DestructL()
{
//GLOBALS_ACCESS;

  CancelTimerRequest();

  // stop and free the direct access server
  if (iDirectScreenAccess)
  {
    iDirectScreenAccess->Cancel();
    delete iDirectScreenAccess; iDirectScreenAccess = NULL;
  }
}

void 
AppThread::StartInteraction()
{
//GLOBALS_ACCESS;

  iThreadActive = true;

  // start the DSA server and draw away
  iDirectScreenAccess->StartL();

  // start the animations
  CancelTimerRequest();
  After(TTimeIntervalMicroSeconds32(1));  
}

void 
AppThread::StopInteraction()
{
  GLOBALS_ACCESS;

  // terminate animations
  CancelTimerRequest();

  // stop the direct screen access server
  iDirectScreenAccess->Cancel();
  g_focus = false;

  iThreadActive = false;
}

void 
AppThread::CancelTimerRequest()
{
//GLOBALS_ACCESS;

  if (IsActive()) CTimer::Cancel();
}

void 
AppThread::RunL()
{
  event  e;
  TTime  timeA, timeB;
  TInt32 timerDiff;
  GLOBALS_ACCESS;

  // timer request has completed, lets post a nilEvent!
  if ((iStatus == KErrNone) && (iThreadActive))
  {
    timeA.HomeTime();

    if (g_focus)
    {
      memset(&e, 0, sizeof(event));
      e.eType = _nilEvent;
      _SysEnqueueEvent(&e);

      DAL_DispatchEvents();
    }

    timeB.HomeTime();
    timerDiff = (10000000 / g_config.display.fps) -
                timeB.MicroSecondsFrom(timeA).Int64().Low();
    timerDiff -= 3333;
    if (timerDiff < 1) timerDiff = 1;  // must be positive

    // put a request for the next event
    After(TTimeIntervalMicroSeconds32(timerDiff));  
  }
}

void 
AppThread::Restart(RDirectScreenAccess::TTerminationReasons aReason)
{
//GLOBALS_ACCESS;

  // we do nothing
}

void 
AppThread::AbortNow(RDirectScreenAccess::TTerminationReasons aReason)
{
//GLOBALS_ACCESS;

  // terminate the thread
  StopInteraction();
}

// ---------------------------------------------------------
// AppAudioStream Class
// ---------------------------------------------------------

AppAudioStream::AppAudioStream()
{
  GLOBALS_ACCESS;

  g_appAudioStream = this;
}

AppAudioStream::~AppAudioStream()
{
  GLOBALS_ACCESS;

  DestructL();

  g_appAudioStream = NULL;
}

void
AppAudioStream::ConstructL(TInt frames)
{
//GLOBALS_ACCESS;
  iFrameSize = frames;

  // allocate 8bit and 16bit buffers (we may need both)
  frames = frames * 2;  
  iBuf16 = (int16 *)malloc(frames * sizeof(int16));
  iBuf8  =  (int8 *)malloc(frames * sizeof(int8));
  iSoundBuf = new TPtr8((TUint8 *)iBuf16, frames*2, frames*2);

  // default configuration
  iCallBack = (_SndStreamBufferCallback)NULL;
  iUserData = NULL;

  // create the stream and open it
  iStream = CMdaAudioOutputStream::NewL(*this);
}

void
AppAudioStream::DestructL()
{
//GLOBALS_ACCESS;

  if (iStream)
  {
    delete iStream;
    iStream = NULL;
  }

  if (iSoundBuf)
  {
    delete iSoundBuf;
    iSoundBuf = NULL;
  }

  if (iBuf8)  free(iBuf8);  iBuf8 = NULL;
  if (iBuf16) free(iBuf16); iBuf16 = NULL;
}

boolean
AppAudioStream::Start(snd_rate rate, snd_format format, snd_type type,
                      void *user, _SndStreamBufferCallback callback)
{
//GLOBALS_ACCESS;

  switch (rate)
  {
    case snd_8khz:   iRate = TMdaAudioDataSettings::ESampleRate8000Hz;  break; 
    case snd_11khz:  iRate = TMdaAudioDataSettings::ESampleRate11025Hz; break; 
    case snd_16khz:  iRate = TMdaAudioDataSettings::ESampleRate16000Hz; break; 
    case snd_22khz:  iRate = TMdaAudioDataSettings::ESampleRate22050Hz; break; 
    case snd_44khz:  iRate = TMdaAudioDataSettings::ESampleRate44100Hz; break; 
    default:   
         return false; 
  }

  switch (type)
  {
    case snd_mono:   iChannels = TMdaAudioDataSettings::EChannelsMono;  break;
    case snd_stereo: iChannels = TMdaAudioDataSettings::EChannelsStereo; break;
    default:   
         return false; 
  }

  // we need these
  iFormat    = format;
  iUserData  = user;
  iCallBack  = callback;

  // open the stream
  memset(&iSettings, 0, sizeof(TMdaAudioDataSettings));
  iStream->Open(&iSettings);

  return true;
}

void
AppAudioStream::Stop()
{
//GLOBALS_ACCESS;

  if (iStream) iStream->Stop();
}

void
AppAudioStream::MaoscOpenComplete(TInt aError)
{
//GLOBALS_ACCESS;

  if ((aError == KErrNone) && (iStream))
  {
    iStream->SetAudioPropertiesL(iRate, iChannels);
    iStream->SetVolume(iStream->MaxVolume());
    iStream->SetPriority(EPriorityMuchMore, EMdaPriorityPreferenceNone);

    UpdateBuffer();
  }
}

void
AppAudioStream::MaoscBufferCopied(TInt aError, const TDesC8& aBuffer)
{
//GLOBALS_ACCESS;

  if ((aError == KErrNone) && (iStream))
  {
    UpdateBuffer();
  }
}

void
AppAudioStream::MaoscPlayComplete(TInt aError)
{
//GLOBALS_ACCESS;

  if ((aError == KErrUnderflow) && (iStream))
  {
    // underflow resets the sample rate - we want to override this
    iStream->SetAudioPropertiesL(iRate, iChannels);
    UpdateBuffer();
  }
}

void 
AppAudioStream::UpdateBuffer()
{
  TInt   cnt;
  int16 *p16;
  int8  *p8;
//GLOBALS_ACCESS;

  cnt = iFrameSize;
  if (iChannels == TMdaAudioDataSettings::EChannelsStereo) cnt = cnt << 1;

  switch (iFormat)
  {
    case snd_int8:
         if (iCallBack)
         {
           iCallBack(0, iUserData, iBuf8, iFrameSize);

           // convert from 8 -> 16bit 
           p16 = iBuf16;
           p8  = iBuf8;
           while (cnt--)
             *p16++ = ((*p8++) << 8); 
         }
         else
           memset(iBuf16, 0, cnt); 
         break;

    case snd_uint8:
         if (iCallBack)
         {
           iCallBack(0, iUserData, iBuf8, iFrameSize);

           // convert from 8 -> 16bit 
           p16 = iBuf16;
           p8  = iBuf8;
           while (cnt--)
             *p16++ = ((*p8++ ^ 0x80) << 8); 
         }
         else
           memset(iBuf16, 0, cnt); 
         break;

    case snd_int16:

         if (iCallBack)
           iCallBack(0, iUserData, iBuf16, iFrameSize);
         else
         {
           cnt = cnt << 1;
           memset(iBuf16, 0, cnt); 
         }
         break;

    default:
         cnt = cnt << 1;
         memset(iBuf16, 0, cnt); 
         break;
  } 
 
  iStream->WriteL(*iSoundBuf);
}

/********************************* EOF ***********************************/
