/*************************************************************************
 *
 * Copyright 2001+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-system.inc
 */

/*************************************************************************
 * Configuration
 *************************************************************************/

// none

/*************************************************************************
 * Component Variables
 *************************************************************************/

#define SYS_GLOBALS           g_sys
#define GLOBALS_UNINITIALIZED (SYS_GLOBALS == NULL)
#define GLOBALS_DEFINITION    SysGlobals *g_sys = NULL

// SysGlobals::   platform additions
#define DAL_SYS_GLOBALS  \
  pthread_t         sys_task_thread; \
                                     \
  pthread_mutex_t   sys_mutex;

#define g_sys_task_thread      (SYS_GLOBALS) -> sys_task_thread

#define g_sys_mutex            (SYS_GLOBALS) -> sys_mutex

/*************************************************************************
 * Platform Independent Implementation
 *************************************************************************/

#include "../common/SHARK-system.inc"

/*************************************************************************
 * Implementation
 *************************************************************************/

static void *DAL_SysTaskThread(void *);

static boolean
DAL_SysInitialize()
{
  boolean init_ok;

  // default return value
  init_ok = true;

  // clear memory used by memory globals
  memset(SYS_GLOBALS, 0, sizeof(SysGlobals));

  // initialize the system mutex
  pthread_mutex_init(&g_sys_mutex, NULL);

  return init_ok;
}

static uint32
DAL_SysConvertDateTimeToSeconds(datetime *dt)
{
  struct tm st;

  // copy the information we need into the structure
  st.tm_year = dt -> year  - 1900;
  st.tm_mon  = dt -> month - 1;
  st.tm_mday = dt -> day;
  st.tm_hour = dt -> hour;
  st.tm_min  = dt -> min;
  st.tm_sec  = dt -> sec;

  return (uint32)mktime(&st);
}

static void
DAL_SysDebugMessageDialog(char *msg)
{
  DAL_SysDialogDialog("SHARK DEBUG MESSAGE", msg);
}

#define DIALOG_WIDTH       400
#define DIALOG_HEIGHT      110
#define DIALOG_PADDING     20
#define DIALOG_FONT_HEIGHT 12
#define DIALOG_FONT_TAB    60

static void
DAL_SysDialogDialog(char *title, char *msg)
{
/**
  char cmd[1024];

  // use 'xmessage' to display x-dialog to the user
  sprintf(cmd, "xmessage -g 300 -center -buttons OKAY " \
               "\"%s\n\n%s\"", title, msg);
  system(cmd); // this waits until the command is complete
 **/

  Window                dialog;
  int                   x, y, depth;
  char                 *winTitleList[1];
  XTextProperty         winTitle;
  unsigned long         windowMask, eventMask;
  XSetWindowAttributes  windowAttr;
  XSizeHints            windowHints;
  XWMHints              windowManagerHints;
  GC                    gc;
  Atom                  wmExit;
  XEvent                e;
  KeySym                key;
  struct timeval        timeout;
  fd_set                rset;
  boolean               modal;
  char                 *p;
  int                   len;
  
  // create the dialog
  windowAttr.border_pixel      = WhitePixel(g_display, g_screen);
  windowAttr.background_pixel  = WhitePixel(g_display, g_screen);
  windowAttr.override_redirect = False;
  windowMask = CWBackPixel | CWBorderPixel;

  // we want the dialog to be in the center of the display
  x = (DisplayWidth(g_display, g_screen)  - DIALOG_WIDTH)  >> 1;
  y = (DisplayHeight(g_display, g_screen) - DIALOG_HEIGHT) >> 1;
  depth = DefaultDepth(g_display, g_screen);

  dialog = 
    XCreateWindow(g_display, RootWindow(g_display, g_screen),
                  x, y, DIALOG_WIDTH, DIALOG_HEIGHT, 1, depth,
                  InputOutput, CopyFromParent, windowMask, &windowAttr);

  // configure window and window manager hints
  windowManagerHints.initial_state = NormalState;
  windowManagerHints.flags         = StateHint;
  XSetWMHints(g_display, dialog, &windowManagerHints);

  windowHints.flags      = PPosition | PSize | PMinSize | PMaxSize;
  windowHints.x          = x;
  windowHints.y          = y;
  windowHints.width      = DIALOG_WIDTH;
  windowHints.height     = DIALOG_HEIGHT;
  windowHints.min_width  = windowHints.width;
  windowHints.min_height = windowHints.height;
  windowHints.max_width  = windowHints.width;
  windowHints.max_height = windowHints.height;
  XSetWMNormalHints(g_display, dialog, &windowHints); 

  // register for the close event
  wmExit = XInternAtom(g_display, "WM_DELETE_WINDOW", True);
  XSetWMProtocols(g_display, dialog, &wmExit, 1);
 
  // what events do we want to know about 
  eventMask = 0;
  eventMask |= ButtonPressMask;
  eventMask |= KeyReleaseMask;
  eventMask |= ExposureMask;
  XSelectInput(g_display, dialog, eventMask);

  winTitleList[0] = title;
  XStringListToTextProperty((char **)&winTitleList, 1, &winTitle);
  XSetWMName(g_display, dialog, &winTitle);

  // create a graphics context (for drawing)
  gc = XCreateGC(g_display, dialog, 0, NULL);
  XSetForeground(g_display, gc, BlackPixel(g_display, g_screen));

  // draw the window
  XMapWindow(g_display, dialog);

  modal = true;
  while (modal)
  {
    timeout.tv_sec = 0;
    timeout.tv_usec = (g_timer_ms * 500);

    // prepare our connection timeout
    FD_ZERO(&rset);
    FD_SET(ConnectionNumber(g_display), &rset);

    // lets check if there are any events pending
    while (XPending(g_display) != 0)
    {
      XNextEvent(g_display, &e);

      // we only care about our dialog events
      if ((((XAnyEvent *)&e)->window) != dialog) break;

      switch (e.type)
      {
        case Expose:
             while (XCheckTypedEvent(g_display, Expose, &e));

             // clear the display, fresh window
             XClearWindow(g_display, dialog);

             // draw our message
             y = DIALOG_PADDING;
             p = msg;
             while (strlen(p) > 0)
             {
               x = DIALOG_PADDING;

               // remove the new line break
               len = strlen(p);
               while (*p == '\n') { p++; len--; y += DIALOG_FONT_HEIGHT; }
               if (strchr(p, '\n') != NULL) len -= strlen(strchr(p, '\n'));

               // handle some tabs (only start of line)
               while (*p == '\t') { p++; len--; x += DIALOG_FONT_TAB; }

               // draw the text line
               XDrawString(g_display, dialog, gc, x, y, p, len); y += DIALOG_FONT_HEIGHT;
               p += len;
             }

             // draw our OK button
             x = DIALOG_WIDTH  - 40;
             y = DIALOG_HEIGHT - DIALOG_PADDING;
             XDrawString(g_display, dialog, gc, x, y, "OK", 2);
             break;

        case KeyRelease:
             XLookupString((XKeyEvent *)&e, NULL, 0, &key, NULL);

             // pressing enter is like pressing OK
             if ((key == XK_Return) || (key == XK_KP_Enter))
               modal = false;
             break;

        case ButtonPress:
             x = ((XButtonEvent *)&e) -> x;
             y = ((XButtonEvent *)&e) -> y;
 
             // did the user click on the OK button?
             if ((x > (DIALOG_WIDTH -40)) && (x < DIALOG_WIDTH) &&
                 (y > (DIALOG_HEIGHT-DIALOG_PADDING-14)) && (y < DIALOG_HEIGHT))
               modal = false;
             break;

        case ClientMessage:
             if ((((XClientMessageEvent *)&e)->data.l[0]) == wmExit)
               modal = false;
             break;

        default:
             break;
      }
    }

    // wait for timeout
    if (modal) 
      select(ConnectionNumber(g_display)+1, &rset, NULL, NULL, &timeout);
  }

  // release the graphics context (for drawing)
  XFreeGC(g_display, gc);

  // hide the window
  XUnmapWindow(g_display, dialog);

  // flush the display
  XFlush(g_display);

  // destroy the dialog
  XDestroyWindow(g_display, dialog);
}

#undef DIALOG_FONT_TAB
#undef DIALOG_FONT_HEIGHT
#undef DIALOG_PADDING
#undef DIALOG_HEIGHT
#undef DIALOG_WIDTH

static void
DAL_SysDelay(uint32 millisec)
{
  usleep(millisec * 1000L);
}

static void
DAL_SysDeviceInfo(uint32 *platform, char *name)
{
  uint32 platform_id;
  char   name_id[256];
  char   distro[128];
  int    cnt;
  FILE  *f;
  struct utsname u;

  strcpy(name_id, "Linux [unknown]");
  platform_id = PLATFORM_LINUX;

  // call uname to get the kernal version
  uname(&u);

  // identify the linux distribution 
  f = fopen("/etc/issue.net", "r");
  if (f != NULL) 
  {
    cnt = fread(distro, 1, 127, f);
    if (strchr(distro, '\n') != NULL) *(strchr(distro, '\n')) = 0;
    distro[cnt] = 0;
    fclose(f);
  }
  else
    strcpy(distro, "unknown");

  // build the linux version string
  sprintf(name_id, "%s %s (%s) [%s]", u.sysname, u.release, u.machine, distro);

  if (platform != NULL) *platform = platform_id;
  if (name != NULL)     strcpy(name, name_id);
}

static void
DAL_SysGetDateTime(datetime *dt)
{
  time_t     now;
  struct tm *st;

  // system system time
  time(&now);
  st = localtime(&now);

  // copy the information we need into the structure
  dt -> year  = st -> tm_year + 1900;
  dt -> month = st -> tm_mon  + 1;
  dt -> day   = st -> tm_mday;
  dt -> hour  = st -> tm_hour;
  dt -> min   = st -> tm_min;
  dt -> sec   = st -> tm_sec;
}

static uint32
DAL_SysGetTime()
{
  return (uint32)time(NULL);
}

static void
DAL_SysTask(sys_event id, _SysTaskCallback callback, void *user)
{
  // if the previous task is still active - kill it
  if (g_sys_task_active) 
  { 
    pthread_cancel(g_sys_task_thread);
    pthread_join(g_sys_task_thread, NULL);

    // it is no longer active
    g_sys_task_active = false;
  }

  // are we creating a new thread?
  if (callback != NULL)
  {
    g_sys_task_callback = callback;
    g_sys_task_user     = user;

    // create the background thread
    pthread_create(&g_sys_task_thread, NULL, DAL_SysTaskThread, (void *)id);

    // now the thread is active
    g_sys_task_active = true;
  }
}

static void *
DAL_SysTaskThread(void *data)
{
  event e;

  // we can only execute a callback that is defined 
  if (g_sys_task_callback != NULL)
  {
    // execute the user task
    g_sys_task_callback(g_sys_task_user);

    // post user event, indicating job is done
    memset(&e, 0, sizeof(event));
    e.eType = (sys_event)data;
    _SysEnqueueEvent(&e);
  }

  // now the task is no longer active
  g_sys_task_active = false;

  return NULL;
}

static void
DAL_SysMutexLock()
{
  pthread_mutex_lock(&g_sys_mutex);
}

static void
DAL_SysMutexUnlock()
{
  pthread_mutex_unlock(&g_sys_mutex);
}

static void 
DAL_SysTerminate()
{
  // stop any background task that is running
  DAL_SysTask(0, NULL, NULL);

  // destroy the system mutex
  pthread_mutex_destroy(&g_sys_mutex);
}

#undef GLOBALS_DEFINITION
#undef GLOBALS_UNINITIALIZED
#undef SYS_GLOBALS

/********************************* EOF ***********************************/
