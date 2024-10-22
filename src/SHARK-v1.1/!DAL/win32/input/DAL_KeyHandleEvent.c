/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_KeyHandleEvent.c
 */

#include "../DAL-prv.h"

boolean
DAL_KeyHandleEvent(uint16 key, event *_e)
{
  boolean processed;
  GLOBALS_ACCESS;

  // default return value
  processed = false;

  // are we reporting 'numberic' key pressing?
  if (g->input.report_numeric)
  {
    switch (key)
    {
      case '1': case '2': case '3':
      case '4': case '5': case '6':
      case '7': case '8': case '9':
      case '0': 
           _e->eType        = _keyEvent;
           _e->data.key.chr = (uint8)key;
           ApplicationHandleEvent(_e); // special case, sent TWO events
           break;
  
      case VK_ESCAPE:
           _e->eType        = _keyEvent;
           _e->data.key.chr = _key_backspace;
           processed = true;
           break;

      default:
           break;
    }
  }

  // which key are we processing?
  switch (key)
  {
    case VK_ESCAPE:

         // if we are reporting numerics, back doesn't exit
         if (!g->input.report_numeric)
         {
           _e->eType        = _keyEvent;
           _e->data.key.chr = _key_unknown;
           _SysEnqueueQuitEvent();
           processed = true;
         }
         break;

    case VK_F1:
    case '7':
    case '9':
    case '0':
    case VK_CONTROL:
         if ((g->input.keyMask & KeyAction) == 0)
         {
           _e->eType        = _keyEvent;
           _e->data.key.chr = _key_action;
           processed = true;
         }
         break;

    case VK_F2:
    case '1':
         if ((g->input.keyMask & KeyOption1) == 0)
         {
           _e->eType        = _keyEvent;
           _e->data.key.chr = _key_option1;
           processed = true;
         }
         break;

    case VK_F3:
    case '3':
         if ((g->input.keyMask & KeyOption2) == 0)
         {
           _e->eType        = _keyEvent;
           _e->data.key.chr = _key_option2;
           processed = true;
         }
         break;

    case VK_F4:
    case '*':
         if ((g->input.keyMask & KeyPause) == 0)
         {
           _e->eType        = _keyEvent;
           _e->data.key.chr = _key_pause;
           processed = true;
         }
         break;

    case VK_UP:
    case '2':
         if ((g->input.keyMask & KeyPadUp) == 0)
         {
           _e->eType        = _keyEvent;
           _e->data.key.chr = _key_padup;
           processed = true;
         }
         break;

    case VK_DOWN:
    case '8':
         if ((g->input.keyMask & KeyPadDown) == 0)
         {
           _e->eType        = _keyEvent;
           _e->data.key.chr = _key_paddown;
           processed = true;
         }
         break;

    case VK_LEFT:
    case '4':
         if ((g->input.keyMask & KeyPadLeft) == 0)
         {
           _e->eType        = _keyEvent;
           _e->data.key.chr = _key_padleft;
           processed = true;
         }
         break;

    case VK_RIGHT:
    case '6':
         if ((g->input.keyMask & KeyPadRight) == 0)
         {
           _e->eType        = _keyEvent;
           _e->data.key.chr = _key_padright;
           processed = true;
         }
         break;

    case VK_RETURN:
    case '5':
         if ((g->input.keyMask & KeyPadSelect) == 0)
         {
           _e->eType        = _keyEvent;
           _e->data.key.chr = _key_padselect;
           processed = true;
         }
         break;

    case VK_PRIOR:
         _e->eType        = _keyEvent;
         _e->data.key.chr = _key_scrollup;
         processed = true;
         break;

    case VK_NEXT:
         _e->eType        = _keyEvent;
         _e->data.key.chr = _key_scrolldown;
         processed = true;
         break;

    case VK_F8:
         _e->eType        = _keyEvent;
         _e->data.key.chr = _key_menu;
         processed = true;
         break;

    case VK_F9:
         _e->eType        = _keyEvent;
         _e->data.key.chr = _key_acknowledge;
         processed = true;
         break;

    case VK_F11:
         DAL_ApplicationResizeWindow(false);  // make the window smaller
         processed = true;
         break;

    case VK_F12:
         DAL_ApplicationResizeWindow(true);  // make the window larger
         processed = true;
         break;

    default:
         if ((key >= 0x20) && (key <= 0x80))
         {
           _e->eType        = _keyEvent;
           _e->data.key.chr = (uint8)key;
           processed = true;
         }
         break;
  }

  return processed;
}

/********************************* EOF ***********************************/
