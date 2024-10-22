/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-_UIMenuSetMenuTriggerRegion.c
 */

#include "../SHARK-prv.h"

void
_UIMenuSetMenuTriggerRegion(rectangle *rect)
{
  GLOBALS_ACCESS;

  // no font defined? lets get outa here
  if (g->ui.menu.font_id == font_undefined) return;

  // only valid under these conditions
  if (_PenAvailable())
  {
    if (rect == NULL)
    {
      g->ui.menu.button[0].rect.x      = -1;
      g->ui.menu.button[0].rect.y      = -1;
      g->ui.menu.button[0].rect.width  = 0;
      g->ui.menu.button[0].rect.height = 0;   // there is no trigger region available
    }
    else
    {
      g->ui.menu.button[0].rect.x      = rect->x;
      g->ui.menu.button[0].rect.y      = rect->y;
      g->ui.menu.button[0].rect.width  = rect->width;
      g->ui.menu.button[0].rect.height = rect->height;
    }
  }

  // DEVELOPER ERROR: warn them
  else
    _SysDebugMessage("_UIMenuSetMenuTriggerRegion() can only be used when stylus exists", true);
}

/********************************* EOF ***********************************/
