/*************************************************************************
 *
 * Copyright (c) 2002-2003 MobileWizardry
 * All rights reservered.
 *
 *************************************************************************/

/*
 * @(#)series60/DAL-prv.h
 *
 * -- Aaron Ardiri     (mailto:aaron_ardiri@mobilewizardry.com)
 * -- Charles Kerchner (mailto:chip_kerchner@mobilewizardry.com)
 */

#ifndef _DAL_PRV_
#define _DAL_PRV_

// #define GFX_WINDOWSYNC   1  // window synchronization/locking (maybe needed?)
   #define GFX_DSARAW       1  // direct screen access (true)
// #define GFX_FULLSCREEN   1  // full screen updates (forced)

// ---------------------------------------------------------
// Sanity Checks
// ---------------------------------------------------------

#ifdef __WINS__
#ifdef GFX_DSARAW
#undef GFX_DSARAW              // emulator doesn't support this - be nice now!
#endif
#endif

/*************************************************************************
 *
 *    Include Files
 *
 *************************************************************************/

// system includes
#include <aknapp.h>
#include <aknappui.h>
#include <eikdoc.h>
#include <eikmenup.h>
#include <MdaAudioTonePlayer.h>
#include <aknsoundsystem.h> 

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "../DAL.h"
#include "application_resource.h"

/*************************************************************************
 *
 *    Application Constants
 *
 *************************************************************************/

#define PALETTE_SIZE            256
#define QUEUE_SIZE              128

#define MAX_MEM                 128

#define MAX_FONT                128
#define MAX_FONT_CHAR           256

#define MICRO_SECOND            1000000
#define MICRO_SECONDx10         10000000

#define BASEDIR                 "c:\\"
#define DEFAULT_PREF_FILENAME   "preferences.sav"

// helper macros
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

/*************************************************************************
 *
 *    Application Structures/Classes
 *
 *************************************************************************/

// ---------------------------------------------------------
// Class Definitions (base for symbian)
// ---------------------------------------------------------

// forward declarations
class Application;
class Document;
class AppUi;
class AppView;
class AppThread;
class ToneGenerator;

// ---------------------------------------------------------
// Application Class
// ---------------------------------------------------------

class Application : public CAknApplication
{
public:
  TUid AppDllUid() const;

protected:
  CApaDocument* CreateDocumentL();
};

// ---------------------------------------------------------
// Document Class
// ---------------------------------------------------------

class Document : public CEikDocument
{
public:
  static Document* NewL(CEikApplication& aApp);
  static Document* NewLC(CEikApplication& aApp);
  ~Document();

private:
  void ConstructL();
  Document(CEikApplication& aApp);
  CEikAppUi* CreateAppUiL();
};

// ---------------------------------------------------------
// AppUI Class
// ---------------------------------------------------------

class AppUi : public CAknAppUi
{
public:
  void ConstructL();
  AppUi();
  ~AppUi();

public:
  void Exit();
  void HandleCommandL(TInt aCommand);
  void HandleForegroundEventL(TBool aForeground);

private:
  AppView*            iAppView;
  CAknKeySoundSystem* iSoundSystem;
};

// ---------------------------------------------------------
// AppView Class
// ---------------------------------------------------------

class AppView : public CCoeControl
{
public:
  static AppView* NewL(const TRect& aRect);
  static AppView* NewLC(const TRect& aRect);
  ~AppView();

public:
  void Draw(const TRect& aRect) const;

protected:
  void HandlePointerEventL(const TPointerEvent& aPointerEvent);
  TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);
  TCoeInputCapabilities InputCapabilities() const;

private:
  void ConstructL(const TRect& aRect);
  AppView();

private:
  AppThread* iAppThread;
};

// ---------------------------------------------------------
// AppThread Class
// ---------------------------------------------------------

class AppThread : public CTimer, public MDirectScreenAccess
{
public:
	AppThread(RWsSession& aClient, CWsScreenDevice& aScreenDevice, RWindow& aWindow);
	~AppThread();
	void ConstructL();

  void StartInteraction();
  void StopInteraction();

  void CancelTimerRequest();

  void DelegateEvents();
  void DrawToScreen(TPoint aPoint, void *bits, coord width, coord height, TRect aRect) const;

private:
	void RunL();
	void Restart(RDirectScreenAccess::TTerminationReasons aReason);
	void AbortNow(RDirectScreenAccess::TTerminationReasons aReason);

private:
	RWsSession&          iClient;
	CWsScreenDevice&     iScreenDevice;
	RWindow&             iWindow;
	CDirectScreenAccess* iDirectScreenAccess;
	CFbsBitGc*           iGc;
};

// ---------------------------------------------------------
// ToneGenerator Class
// ---------------------------------------------------------

class ToneGenerator: public CBase, public MMdaAudioToneObserver
{
public:
  static ToneGenerator* NewL(TInt freq, TInt duration, TInt volume);
  static ToneGenerator* NewLC(TInt freq, TInt duration, TInt volume);
  ~ToneGenerator();

public:
  void MatoPrepareComplete(TInt aError);
  void MatoPlayComplete(TInt aError);

private:
  ToneGenerator();
  void ConstructL(TInt freq, TInt duration, TInt volume);

private:
  TInt                  playVolume;
  CMdaAudioToneUtility *iCMdaAudioToneUtility;
};

/*************************************************************************
 *
 *    Public Functions
 *
 *************************************************************************/

extern "C"
{
  extern uint32 SymbianGetUniqueID();  // declared by project
}

/*************************************************************************
 *
 *    Global Variables
 *
 *************************************************************************/

typedef struct
{
  void          *app_globals;           // application globals
  AppUi         *appUi;                 // appui refererence
  AppView       *appView;               // appview reference
  AppThread     *appThread;             // appthred reference
  ToneGenerator *iToneGenerator;        // tone generator reference
  TInt32         evtTimeOut;            // event handling time out controller

  struct
  {
    uint32      fps;                    // desired frames/second (x10 factor)
    uint32      seed;                   // random number seed

    uint16      q_index;
    uint16      q_max;
    event      *eventQ;                 // event queue functionality

#ifdef GFX_DSARAW
    uint16     *LCDpointer;             // LCD display pointer (0,0) in window
#endif

  } sys;

  struct
  {
    uint16      count;
    void       *info;                   // memory 'management' chunk data
  } mem;

  struct
  {
    boolean     active;                 // is the application allowed to draw to screen?
    boolean     focus;                  // does the application have focus?

    uint32      width;                  // width of display
    uint32      height;                 // height of display
    uint32      depth;                  // depth of display

    gfx_window *window;                 // primary display
    CFbsBitmap *winBitmap;

    uint8      *palette;
    uint32     *paletteDiff;
    uint16     *iPaletteLookup;

    gfx_window *activeWindow;           // active draw window
    rectangle   clip;                   // clipping area

    font        activeFont;             // active font
    void       *font_info;              // font specific information

    void       *info;                   // screen specific information

    boolean     drawn;
  } gfx;

  struct
  {
    uint32      keyMask;                // the keys not translated as events
    void       *info;                   // input/output specific information
  } io;

} DAL_GlobalsType;

// #define gDALGlobals ((DAL_GlobalsType *)(Dll::Tls()))
// register DAL_GlobalsType* gDALGlobals = (DAL_GlobalsType *)Dll::Tls();  // <- USE THIS

#endif

/********************************* EOF ***********************************/
