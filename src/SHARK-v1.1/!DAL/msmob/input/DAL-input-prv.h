/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-input-prv.h
 */

#include "../DAL-prv.h"

#ifndef DAL_INPUT_PRV_H
#define DAL_INPUT_PRV_H

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
 *
 *    Application Constants/Structures
 *
 *************************************************************************/

/**
 ** Microsoft Mobile KEY SETUP extentions
 **
    sofkey1     softkey2    VK_TSOFT1, VK_TSOFT2
            ^               VK_TUP
        <   +   >           VK_TLEFT, VK_TACTION, VK_TRIGHT
            v               VK_TDOWN
    home        back        VK_THOME, VK_TBACK
    talk        end         VK_TTALK, VK_TEND
    1       2       3       VK_T0..VK_T9
    4       5       6       ...
    7       8       9       ...
    *       0       #       VK_TSTAR, VK_TPOUND
    ----
    other buttons include
    VK_TRECORD
    VK_TPOWER, VK_TVOLUMEUP, VK_TVOLUMEDOWN
    VK_TFLIP
 **/

#define VK_TSOFT1       VK_F1
#define VK_TSOFT2       VK_F2

#define VK_TUP          VK_UP
#define VK_TDOWN        VK_DOWN
#define VK_TLEFT        VK_LEFT
#define VK_TRIGHT       VK_RIGHT
#define VK_TACTION      VK_RETURN

#define VK_TTALK        VK_F3
#define VK_TEND         VK_F4
#define VK_THOME        VK_LWIN
#define VK_TBACK        VK_ESCAPE

#define VK_T0           L'0'
#define VK_T1           L'1'
#define VK_T2           L'2'
#define VK_T3           L'3'
#define VK_T4           L'4'
#define VK_T5           L'5'
#define VK_T6           L'6'
#define VK_T7           L'7'
#define VK_T8           L'8'
#define VK_T9           L'9'
#define VK_TSTAR        VK_F8
#define VK_TPOUND       VK_F9

#define VK_TRECORD      VK_F10
#define VK_TPOWER       VK_F18
#define VK_TVOLUMEUP    VK_F6
#define VK_TVOLUMEDOWN  VK_F7
#define VK_TFLIP        VK_F17

#define VK_APP1         0xC1
#define VK_APP2         0xC2 
#define VK_APP3         0xC3
#define VK_APP4         0xC4
#define VK_APP5         0xC5
#define VK_APP6         0xC6

// used to handle 'manual' key pressing
#define KEY_STATE_APP1  0x00000001
#define KEY_STATE_APP2  0x00000002
#define KEY_STATE_APP3  0x00000004
#define KEY_STATE_APP4  0x00000008
#define KEY_STATE_APP5  0x00000010
#define KEY_STATE_APP6  0x00000020
#define KEY_STATE_TTALK 0x00000040
#define KEY_STATE_TEND  0x00000080

/*************************************************************************
 *
 * DAL Internal Use API's
 *
 *************************************************************************/

extern void       DAL_InpDisableHardwareKeys(HWND hWnd);
extern void       DAL_InpEnableHardwareKeys(HWND hWnd);

#ifdef __cplusplus
}
#endif

#endif

/********************************* EOF ***********************************/
