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
  PALM_PlatformInfo *platform;
  boolean            processed;
  uint32             keyState;
  GLOBALS_ACCESS;

  // default return value
  processed = false;
  keyState  = KeyCurrentState();

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
           _e->data.key.chr = '0' + (key - '0');

           ApplicationHandleEvent(_e); // special case, sent TWO events
           break;
  
      case chrBackspace:
           _e->eType        = _keyEvent;
           _e->data.key.chr = _key_backspace;

           processed = true;
           break;

      default:
           break;
    }
  }

  // access device/platform information block
  platform = (PALM_PlatformInfo *)g->platform;
  if (platform != NULL)
  {
    // tapwave zodiac - lets zap the normal keys (interference)
    if (platform->input.tapwave)
      keyState = keyState & 0xffffff00;   
  
#ifdef INP_SUPPORT_POWER
    // filter the power button from processing
    if (platform->input.power)
      keyState = keyState & (~keyBitHard4);
#endif

    // which key are we processing?
    switch (key)
    {
      case vchrMenu:
           _e->eType        = _keyEvent;
           _e->data.key.chr = _key_menu;
    
           processed = true;
           break;
  
      case vchrHard1:
           // special - both keys?
           if ((keyState & keyBitHard2) &&
               ((g->input.keyMask & KeyOption1) == 0))
           {
             _e->eType        = _keyEvent;
             _e->data.key.chr = _key_option1;
  
             processed = true;
           }
           else
  
           // normal processing
           if ((g->input.keyMask & KeyAction) == 0)
           {
             _e->eType        = _keyEvent;
             _e->data.key.chr = _key_action;
  
             processed = true;
           }
           break;
  
      case vchrHard2:
           // special - both keys?
           if ((keyState & keyBitHard1) &&
               ((g->input.keyMask & KeyOption1) == 0))
           {
             _e->eType        = _keyEvent;
             _e->data.key.chr = _key_option1;
  
             processed = true;
           }
           else
  
           // normal processing
           if (platform->input.dpad)
           {
             if ((g->input.keyMask & KeyAction) == 0)
             {
               _e->eType        = _keyEvent;
               _e->data.key.chr = _key_action;
    
               processed = true;
             }
           }
           else
           {
             if ((g->input.keyMask & KeyPadLeft) == 0)
             {
               _e->eType        = _keyEvent;
               _e->data.key.chr = _key_padleft;
    
               processed = true;
             }
           }
           break;
  
      case vchrHard3:
           // special - both keys?
           if ((keyState & keyBitHard4) &&
               ((g->input.keyMask & KeyOption2) == 0))
           {
             _e->eType        = _keyEvent;
             _e->data.key.chr = _key_option2;
  
             processed = true;
           }
           else
  
           // normal processing
           if (platform->input.dpad)
           {
             if ((g->input.keyMask & KeyAction) == 0)
             {
               _e->eType        = _keyEvent;
               _e->data.key.chr = _key_action;
    
               processed = true;
             }
           }
           else
           {
             if ((g->input.keyMask & KeyPadLeft) == 0)
             {
               _e->eType        = _keyEvent;
               _e->data.key.chr = _key_padright;
    
               processed = true;
             }
           }
           break;
  
      case vchrHard4:

#ifdef INP_SUPPORT_POWER
           // filter the power button from processing
           if (platform->input.power) goto IGNORE_KEY;
#endif

           // special - both keys?
           if ((keyState & keyBitHard3) &&
               ((g->input.keyMask & KeyOption2) == 0))
           {
             _e->eType        = _keyEvent;
             _e->data.key.chr = _key_option2;
  
             processed = true;
           }
           else
  
           // normal processing
           if (platform->input.dpad)
           {
             if ((g->input.keyMask & KeyAction) == 0)
             {
               _e->eType        = _keyEvent;
               _e->data.key.chr = _key_action;
    
               processed = true;
             }
           }
           else
           {
             if ((g->input.keyMask & KeyPadSelect) == 0)
             {
               _e->eType        = _keyEvent;
               _e->data.key.chr = _key_padselect;
    
               processed = true;
             }
           }
           break;
  
      case vchrPageUp:
      case vchrRockerUp:
      case vchrJogUp:
      case vchrTrgJogUp:
      case vchrPrevField:
      case vchrThumbWheelUp:
      case vchrJogPageUp:
           if ((g->input.keyMask & KeyPadUp) == 0)
           {
             _e->eType        = _keyEvent;
             _e->data.key.chr = _key_padup;
      
             processed = true;
           }
           break;
  
      case vchrPageDown:
      case vchrRockerDown:
      case vchrJogDown:
      case vchrNextField:
      case vchrTrgJogDown:
      case vchrThumbWheelDown:
      case vchrJogPageDown:
           if ((g->input.keyMask & KeyPadDown) == 0)
           {
             _e->eType        = _keyEvent;
             _e->data.key.chr = _key_paddown;
      
             processed = true;
           }
           break;

      case vchrRockerLeft:
           if ((g->input.keyMask & KeyPadLeft) == 0)
           {
             _e->eType        = _keyEvent;
             _e->data.key.chr = _key_padleft;
    
             processed = true;
           }
           break;

      case vchrRockerRight:
           if ((g->input.keyMask & KeyPadRight) == 0)
           {
             _e->eType        = _keyEvent;
             _e->data.key.chr = _key_padright;
    
             processed = true;
           }
           break;  

      case vchrJogPress:
      case vchrTrgJogPush:
      case vchrThumbWheelPush:
      case vchrRockerCenter:
           if ((g->input.keyMask & KeyPadSelect) == 0)
           {
             _e->eType        = _keyEvent;
             _e->data.key.chr = _key_padselect;
           }
           processed = true;
           break;

      case vchrTriggerA:
      case vchrTriggerB:
      case vchrActionA:
      case vchrActionB:
      case vchrActionC:
      case vchrActionD:
           if ((g->input.keyMask & KeyAction) == 0)
           {
             _e->eType        = _keyEvent;
             _e->data.key.chr = _key_action;
           }
           processed = true;
           break;

      case vchrFunction:
           if ((g->input.keyMask & KeyPause) == 0)
           {
             _e->eType        = _keyEvent;
             _e->data.key.chr = _key_pause;
           }
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
  }

#ifdef INP_SUPPORT_POWER
IGNORE_KEY:
#endif

  return processed;
}

/********************************* EOF ***********************************/
