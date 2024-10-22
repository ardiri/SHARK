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
      case VK_T1: case VK_T2: case VK_T3:
      case VK_T4: case VK_T5: case VK_T6:
      case VK_T7: case VK_T8: case VK_T9:
      case VK_T0: 
           _e->eType        = _keyEvent;
           _e->data.key.chr = '0' + (key - VK_T0);
           ApplicationHandleEvent(_e); // special case, sent TWO events
           break;
  
      case VK_TBACK:
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
    case VK_TBACK:

         // if we are reporting numerics, back doesn't exit
         if (!g->input.report_numeric)
         {
           _e->eType        = _keyEvent;
           _e->data.key.chr = _key_unknown;
           _SysEnqueueQuitEvent();
           processed = true;
         }
         break;

#ifdef INP_BUTTONS_AS_ACTIONS
    case VK_APP1:
    case VK_APP2:
    case VK_APP3:
#endif
    case VK_APP4:
    case VK_APP5:
    case VK_APP6:
    case VK_TTALK:
    case VK_TEND:
    case VK_T7:
    case VK_T9:
    case VK_T0:
    case VK_TPOUND:
         if ((g->input.keyMask & KeyAction) == 0)
         {
           _e->eType        = _keyEvent;
           _e->data.key.chr = _key_action;
           processed = true;
         }
         break;

    case VK_TUP:
    case VK_T2:
         if ((g->input.keyMask & KeyPadUp) == 0)
         {
           _e->eType        = _keyEvent;
           _e->data.key.chr = _key_padup;
           processed = true;
         }
         break;

    case VK_TDOWN:
    case VK_T8:
         if ((g->input.keyMask & KeyPadDown) == 0)
         {
           _e->eType        = _keyEvent;
           _e->data.key.chr = _key_paddown;
           processed = true;
         }
         break;

    case VK_TLEFT:
    case VK_T4:
         if ((g->input.keyMask & KeyPadLeft) == 0)
         {
           _e->eType        = _keyEvent;
           _e->data.key.chr = _key_padleft;
           processed = true;
         }
         break;

    case VK_RIGHT:
    case VK_T6:
         if ((g->input.keyMask & KeyPadRight) == 0)
         {
           _e->eType        = _keyEvent;
           _e->data.key.chr = _key_padright;
           processed = true;
         }
         break;

    case VK_RETURN:
//  case 0x86:       // is always sent, secondary
    case VK_T5:
         if ((g->input.keyMask & KeyPadSelect) == 0)
         {
           _e->eType        = _keyEvent;
           _e->data.key.chr = _key_padselect;
           processed = true;
         }
         break;

#ifndef INP_BUTTONS_AS_ACTIONS
    case VK_APP1:
#endif
    case VK_T1:
         if ((g->input.keyMask & KeyOption1) == 0)
         {
           _e->eType        = _keyEvent;
           _e->data.key.chr = _key_option1;
           processed = true;
         }
         break;

#ifndef INP_BUTTONS_AS_ACTIONS
    case VK_APP2:
#endif
    case VK_T3:
         if ((g->input.keyMask & KeyOption2) == 0)
         {
           _e->eType        = _keyEvent;
           _e->data.key.chr = _key_option2;
           processed = true;
         }
         break;

#ifndef INP_BUTTONS_AS_ACTIONS
    case VK_APP3:
#endif
    case VK_TSTAR:
         if ((g->input.keyMask & KeyPause) == 0)
         {
           _e->eType        = _keyEvent;
           _e->data.key.chr = _key_pause;
           processed = true;
         }
         break;

    case VK_TSOFT1:
         _e->eType        = _keyEvent;
         _e->data.key.chr = _key_menu;
         processed = true;
         break;

    case VK_TSOFT2:
         _e->eType        = _keyEvent;
         _e->data.key.chr = _key_acknowledge;
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
