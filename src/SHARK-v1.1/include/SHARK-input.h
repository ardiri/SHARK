/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-input.h
 */

#ifndef SHARK_INPUT_H
#define SHARK_INPUT_H

#include "../SHARK.h"

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
 *
 * Constants / Structures
 *
 *************************************************************************/

// unknown key
#define _key_unknown            0

// physical keys
#define _key_base               1
#define _key_padup              (_key_base+0)
#define _key_paddown            (_key_base+1)
#define _key_padleft            (_key_base+2)
#define _key_padright           (_key_base+3)
#define _key_padselect          (_key_base+4)
#define _key_action             (_key_base+5)
#define _key_option1            (_key_base+6)
#define _key_option2            (_key_base+7)
#define _key_pause              (_key_base+8)

// virtual keys
#define _key_virtual_base       10
#define _key_scrollup           (_key_virtual_base+0) 
#define _key_scrolldown         (_key_virtual_base+1) 
#define _key_focuslost          (_key_virtual_base+2) 
#define _key_gfx_change         (_key_virtual_base+3) 
#define _key_menu               (_key_virtual_base+4)
#define _key_acknowledge        (_key_virtual_base+5)
#define _key_backspace          (_key_virtual_base+6)

#define _key_user_base          17

// key values (for _KeyCurrentState)
#define KeyPadUp                (1L << (_key_padup))
#define KeyPadDown              (1L << (_key_paddown))
#define KeyPadLeft              (1L << (_key_padleft))
#define KeyPadRight             (1L << (_key_padright))
#define KeyPadSelect            (1L << (_key_padselect))
#define KeyAction               (1L << (_key_action))
#define KeyOption1              (1L << (_key_option1))
#define KeyOption2              (1L << (_key_option2))
#define KeyPause                (1L << (_key_pause))

#define KeyAllMask              0xFFFFFFFF

/*************************************************************************
 *
 * Application Programmers Interface
 *
 *************************************************************************/

// device specific
extern uint32      _KeyCurrentState();
extern void        _KeyCurrentStateAnalog(uint8 *x, uint8 *y);
extern void        _PenState(coord *penX, coord *penY, boolean *penDown);

// device independent
extern void        _KeySetMask(uint32 mask);
extern uint32      _KeyGetMask();
extern void        _KeySetNumericReporting(boolean report);
extern boolean     _KeyAnalogAvailable();
extern boolean     _PenAvailable();
extern boolean     _PenDisplayToWindow(coord *penX, coord *penY);

#ifdef __cplusplus
}
#endif

#endif

/********************************* EOF ***********************************/
