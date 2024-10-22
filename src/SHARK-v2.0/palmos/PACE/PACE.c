#include "../DAL-prv.h"

//------------------------------------------------------------------------
// PACE Interface
//------------------------------------------------------------------------

#define kPceNativeWantA0     (0x10000000)
#define kPceNativeTrapNoMask (0x00000FFF)

#define PceNativeTrapNo(sysTrapNo) (sysTrapNo & kPceNativeTrapNoMask)

typedef struct EmulStateType 
{
  UInt32 instr;
  UInt32 regD[8];
  UInt32 regA[8];
  UInt32 regPC;
} EmulStateType;

#define PACE_PARAMS_INIT()            \
  UInt8 params[] = {
  
#define PACE_PARAMS_ADD8(param)       \
  (  (UInt8)(param)        & 0xff), 0,

#define PACE_PARAMS_ADD16(param)      \
  (((UInt16)(param) >> 8)  & 0xff),   \
  ( (UInt16)(param)        & 0xff),

#define PACE_PARAMS_ADD32(param)      \
  (((UInt32)(param) >> 24) & 0xff),   \
  (((UInt32)(param) >> 16) & 0xff),   \
  (((UInt32)(param) >> 8)  & 0xff),   \
  ( (UInt32)(param)        & 0xff),

#define PACE_PARAMS_DONE()            \
  };

#define PACE_EXEC(trap, flag)         \
  ((Call68KFuncType *)g -> PACE._68k)((void *)g -> PACE._emu, \
                      PceNativeTrapNo(trap), &params, sizeof(params) | (flag));

#define PACE_EXEC_SELECTOR(selector)  \
  ((EmulStateType *)g -> PACE._emu) -> regD[2] = selector; 

//------------------------------------------------------------------------
// SndStream* API                      (pre 5.1 - Palm T|T, Sony NX,NZ,TG)
//------------------------------------------------------------------------

#define sysTrapSndStreamCreate              0xA45B
#define sysTrapSndStreamDelete              0xA45C
#define sysTrapSndStreamStart               0xA45D
#define sysTrapSndStreamStop                0xA45F
#define sysTrapSndStreamSetVolume           0xA460

Err
SndStreamCreate_PACE(SndStreamRef *channel, 
                     SndStreamMode mode, UInt32 samplerate, 
                     SndSampleType type, SndStreamWidth width, 
                     SndStreamBufferCallback func, 
                     void *userdata, UInt32 buffsize, Boolean armNative)
{
  Err          result;
  SndStreamRef x;

  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(channel)
  PACE_PARAMS_ADD8(mode)
  PACE_PARAMS_ADD32(samplerate)
  PACE_PARAMS_ADD16(type)
  PACE_PARAMS_ADD8(width)
  PACE_PARAMS_ADD32(func)
  PACE_PARAMS_ADD32(userdata)
  PACE_PARAMS_ADD32(buffsize)
  PACE_PARAMS_ADD8(armNative)
  PACE_PARAMS_DONE()
  result = (Err)PACE_EXEC(sysTrapSndStreamCreate, 0)

  // fixup return values (in pointer)
  if (channel) { x = __byte_swap32(*channel); *channel = x; }

  return result;
}

Err
SndStreamDelete_PACE(SndStreamRef channel)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(channel)
  PACE_PARAMS_DONE()
  return (Err)PACE_EXEC(sysTrapSndStreamDelete, 0)
}

Err
SndStreamStart_PACE(SndStreamRef channel)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(channel)
  PACE_PARAMS_DONE()
  return (Err)PACE_EXEC(sysTrapSndStreamStart, 0)
}

Err
SndStreamStop_PACE(SndStreamRef channel)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(channel)
  PACE_PARAMS_DONE()
  return (Err)PACE_EXEC(sysTrapSndStreamStop, 0)
}

Err
SndStreamSetVolume_PACE(SndStreamRef channel, Int32 volume)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(channel)
  PACE_PARAMS_ADD32(volume)
  PACE_PARAMS_DONE()
  return (Err)PACE_EXEC(sysTrapSndStreamSetVolume, 0)
}

//------------------------------------------------------------------------
// PIN* API
//------------------------------------------------------------------------

#define sysTrapSysPinSelector               0xA470

#define pinPINSetInputTriggerState          2
#define pinWinSetConstraintsSize            13
#define pinFrmSetDIAPolicyAttr              14

Err
PINSetInputTriggerState_PACE(UInt16 state)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD16(state)
  PACE_PARAMS_DONE()
  PACE_EXEC_SELECTOR(pinPINSetInputTriggerState)
  return (Err)PACE_EXEC(sysTrapSysPinSelector, 0)
}

Err
WinSetConstraintsSize_PACE(WinHandle winHandle, 
                           Coord minH, Coord prefH, Coord maxH,
                           Coord minW, Coord prefW, Coord maxW)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(winHandle)
  PACE_PARAMS_ADD16(minH)
  PACE_PARAMS_ADD16(prefH)
  PACE_PARAMS_ADD16(maxH)
  PACE_PARAMS_ADD16(minW)
  PACE_PARAMS_ADD16(prefW)
  PACE_PARAMS_ADD16(maxW)
  PACE_PARAMS_DONE()
  PACE_EXEC_SELECTOR(pinWinSetConstraintsSize)
  return (Err)PACE_EXEC(sysTrapSysPinSelector, 0)
}

Err
FrmSetDIAPolicyAttr_PACE(FormType *formP, UInt16 diaPolicy)
{
  PACE_PARAMS_INIT()
  PACE_PARAMS_ADD32(formP)
  PACE_PARAMS_ADD16(diaPolicy)
  PACE_PARAMS_DONE()
  PACE_EXEC_SELECTOR(pinFrmSetDIAPolicyAttr)
  return (Err)PACE_EXEC(sysTrapSysPinSelector, 0)
}

/********************************* EOF ***********************************/
