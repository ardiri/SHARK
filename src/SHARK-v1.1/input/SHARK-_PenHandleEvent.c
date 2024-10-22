/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_PenHandleEvent.c
 */

#include "../SHARK-prv.h"

boolean
_PenHandleEvent(event *_e)
{
  coord   penX, penY;
  boolean penDown;
  boolean result;
  GLOBALS_ACCESS;

  // default return value
  result = false;

  // access device information block
  if (_PenAvailable())
  {
    _PenState(&penX, &penY, &penDown);
    if (penDown)
    {
      _e->eType   = _penEvent;
      _e->penDown = penDown;
      _e->screenX = _e->data.pen.x = penX;
      _e->screenY = _e->data.pen.y = penY;

      // only valid if within range of drawable area :)
      result = ((_e->screenX >= 0) &&
                (_e->screenY >= 0) &&
                (_e->screenX <  (coord)g->device.display.width) &&
                (_e->screenY <  (coord)g->device.display.height));
    }
  }

  return result;
}

/********************************* EOF ***********************************/
