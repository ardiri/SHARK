/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_PenState.c
 */

#include "../DAL-prv.h"

void
DAL_PenState(coord *penX, coord *penY, boolean *penDown)
{
  MSmob_PlatformInfo *platform;
  DWORD               pos;
  POINT               pt;
  MSG                 msg;
  GLOBALS_ACCESS;

  // default values
  if (penX != NULL) *penX = 0;
  if (penY != NULL) *penY = 0;

  // access device information block
  platform = (MSmob_PlatformInfo *)g->platform;
  if ((platform != NULL) && (penDown != NULL))
  {
    *penDown = (boolean)(GetAsyncKeyState(VK_LBUTTON));
    if (*penDown)
    {
      pos = GetMessagePos();
      pt.x = LOWORD(pos);
      pt.y = HIWORD(pos);
      ScreenToClient(platform->windows.LCDWnd, &pt);

      // lets ensure the pen is within the screen boundaries 
      if (((pt.x > 0) &&
           (pt.y > 0) &&
           (pt.x < (coord)g->device.display.width) &&
           (pt.y < (coord)g->device.display.height)))
      {  
        if (penX != NULL) *penX = (coord)pt.x;
        if (penY != NULL) *penY = (coord)pt.y;
      }
      else
        *penDown = false;
  
      // flush the pen queue
      PeekMessage(&msg, platform->windows.LCDWnd, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE);
    }
  }
}

/********************************* EOF ***********************************/
