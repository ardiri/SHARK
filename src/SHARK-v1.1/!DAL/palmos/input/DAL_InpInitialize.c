/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL_InpInitialize.c
 */

#include "../DAL-prv.h"

boolean
DAL_InpInitialize()
{
  PALM_PlatformInfo *platform;
  UInt32             version;
  UInt32             companyID, deviceID;
  GLOBALS_ACCESS;

  // setup default values
  g->input.keyMask          = 0;
  g->input.report_numeric   = false;

  // access device/platform information block
  platform = (PALM_PlatformInfo *)g->platform;
  if (platform != NULL)
  {
    platform->input.dpad    = false;
    platform->input.tapwave = false;
#ifdef INP_SUPPORT_POWER
    platform->input.power   = false;
#endif
  
    // DETECT #1: navigator feature
    platform->input.dpad |= (FtrGet(0x66776179, 0, &version) != ftrErrNoSuchFeature); // 'fway' = 0x66776179
    // DETECT #2: hardware flags feature
    FtrGet(sysFtrCreator, sysFtrNumUIHardwareFlags, &version);
    platform->input.dpad |= (version & sysFtrNumUIHardwareHas5Way);
    // DETECT #3: Treo 600 manual detection (bug in GM version 1)
    FtrGet(sysFtrCreator, sysFtrNumOEMCompanyID, &companyID);
    FtrGet(sysFtrCreator, sysFtrNumOEMDeviceID,  &deviceID);
    platform->input.dpad |=  ((companyID == 0x68737072) && (deviceID == 0x48313031)); // 'hspr' = 0x68737072, 'H101' = 0x48313031

#ifdef INP_SUPPORT_POWER
    // DETECT #1: Treo 600/ACE special power-button as hard4
    platform->input.power |= (((companyID == 0x68737072) && (deviceID == 0x48313031)) || // 'hspr' = 0x68737072, 'H101' = 0x48313031
                              ((companyID == 0x68737072) && (deviceID == 0x48313032)));  // 'hspr' = 0x68737072, 'H102' = 0x48313032
#endif

    // DETECT #1: navigator feature
    platform->input.tapwave |= (FtrGet(0x54707776, 0, &version) != ftrErrNoSuchFeature); // 'Tpwv' == 0x54707776
  }

  return true;
}

/********************************* EOF ***********************************/
