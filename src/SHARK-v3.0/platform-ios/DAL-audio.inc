/*************************************************************************
 *
 * Copyright 2001+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-audio.inc
 */

/*************************************************************************
 * Configuration
 *************************************************************************/

// none

/*************************************************************************
 * Component Variables
 *************************************************************************/

#define SND_GLOBALS           g_snd
#define GLOBALS_UNINITIALIZED (SND_GLOBALS == NULL)
#define GLOBALS_DEFINITION    SndGlobals *g_snd = NULL

// SNDGlobals::   platform additions
#define DAL_SND_GLOBALS \
  AudioUnit                   snd_audio_unit;

#define g_snd_audio_unit          (SND_GLOBALS) -> snd_audio_unit

/*************************************************************************
 * Platform Independent Implementation
 *************************************************************************/

#include "../common/SHARK-audio.inc"

static OSStatus DAL_SndThread(void *inRefCon,
                              AudioUnitRenderActionFlags *ioActionFlags,
                              const AudioTimeStamp *inTimeStamp,
                              UInt32 inBusNumber, UInt32 inNumberFrames, 
                              AudioBufferList *ioData);

void  DAL_InterruptListener(void *in, UInt32 state);

/*************************************************************************
 * Implementation
 *************************************************************************/

static boolean
DAL_SndInitialize()
{
  boolean init_ok;
  UInt32  category;

  // default return value
  init_ok = true;

  // clear memory used by memory globals
  memset(SND_GLOBALS, 0, sizeof(SndGlobals));

  // initialize our audio session, register for interruptions
  AudioSessionInitialize(NULL, 
    kCFRunLoopDefaultMode, 
    (AudioSessionInterruptionListener)DAL_InterruptListener, NULL);

  // what type of audio session are we?
  category = kAudioSessionCategory_AmbientSound;
  AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, 
                          sizeof(category), &category);

  // we are now active with audio
  AudioSessionSetActive(true);

  // lets determine if there is streaming audio
  g_snd_stream_available = true;

  return init_ok;
}

static snd_stream
DAL_SndStreamCreate(uint32 frames)
{
  snd_stream                  result;
  OSStatus                    err;
  AudioComponent              comp;
  AudioComponentDescription   cDesc;
  AudioStreamBasicDescription aDesc;
  AURenderCallbackStruct      aCallback;

  // default return value
  result = STREAM_INVALID;

  // lets find an output audio unit
  memset(&cDesc, 0, sizeof(AudioComponentDescription));
  cDesc.componentType         = kAudioUnitType_Output;
  cDesc.componentSubType      = kAudioUnitSubType_RemoteIO;
  cDesc.componentManufacturer = kAudioUnitManufacturer_Apple;
  cDesc.componentFlags        = 0;
  cDesc.componentFlagsMask    = 0;

  comp = AudioComponentFindNext(NULL, &cDesc);
  if (comp == NULL) goto SND_CREATE_ABORT;

  // open the component
  err = AudioComponentInstanceNew(comp, &g_snd_audio_unit);
  if (err != noErr) goto SND_CREATE_ABORT;

  // initialize the component
  err = AudioUnitInitialize(g_snd_audio_unit);
  if (err != noErr) goto SND_CREATE_ABORT;

  // define the audio component properties
  memset(&aDesc, 0, sizeof(AudioStreamBasicDescription));
  aDesc.mFormatID         = kAudioFormatLinearPCM;
  aDesc.mFormatFlags      = kLinearPCMFormatFlagIsPacked;
  aDesc.mFramesPerPacket  = 1;
  aDesc.mChannelsPerFrame = (g_snd_type == snd_mono) ? 1 : 2;
  switch (g_snd_rate)
  {
    case snd_8khz:  aDesc.mSampleRate = 8000;  break;
    case snd_11khz: aDesc.mSampleRate = 11025; break;
    case snd_16khz: aDesc.mSampleRate = 16000; break;
    case snd_22khz: aDesc.mSampleRate = 22050; break;
    case snd_44khz: aDesc.mSampleRate = 44100; break;
    default:                                   break;
  }
  switch (g_snd_format)
  {
    case snd_int8:
    case snd_uint8: aDesc.mBitsPerChannel = 8;  break;
    case snd_int16: aDesc.mBitsPerChannel = 16; break;
    default:                                    break;
  }
  if ((g_snd_format == snd_int8) || (g_snd_format == snd_int16))
    aDesc.mFormatFlags   |= kLinearPCMFormatFlagIsSignedInteger;

  aDesc.mBytesPerFrame  = aDesc.mChannelsPerFrame * (aDesc.mBitsPerChannel / 8);
  aDesc.mBytesPerPacket = aDesc.mBytesPerFrame    * aDesc.mFramesPerPacket;

  // set the desired properties
  err = AudioUnitSetProperty(g_snd_audio_unit,
                             kAudioUnitProperty_StreamFormat,
                             kAudioUnitScope_Input, 0,
                             &aDesc, sizeof(AudioStreamBasicDescription));
  if (err != noErr) goto SND_CREATE_ABORT;

  // set the desired callback function
  memset(&aCallback, 0, sizeof(AURenderCallbackStruct));
  aCallback.inputProc = (void *)DAL_SndThread;
  err = AudioUnitSetProperty(g_snd_audio_unit,
                             kAudioUnitProperty_SetRenderCallback,
                             kAudioUnitScope_Input, 0,
                             &aCallback, sizeof(AURenderCallbackStruct));
  if (err != noErr) goto SND_CREATE_ABORT;

  // start the audio unit
  err = AudioOutputUnitStart(g_snd_audio_unit);
  if (err == noErr)
  {
    // flag the stream as valid - lets go!
    result       = 0;
  }

SND_CREATE_ABORT:;

  return result;
}

static void
DAL_SndStreamPause()
{
}

static void
DAL_SndStreamResume()
{
}

static void
DAL_SndStreamDelete(snd_stream stream_id)
{
  OSStatus               err;
  AURenderCallbackStruct aCallback;

  // stop the audio unit
  err = AudioOutputUnitStop(g_snd_audio_unit);
  if (err != noErr) goto SND_DELETE_ABORT;

  // remove the callback function
  memset(&aCallback, 0, sizeof(AURenderCallbackStruct));
  err = AudioUnitSetProperty(g_snd_audio_unit,
                             kAudioUnitProperty_SetRenderCallback,
                             kAudioUnitScope_Input, 0,
                             &aCallback, sizeof(AURenderCallbackStruct));
  if (err != noErr) goto SND_DELETE_ABORT;

  // uninitialize the audio unit
  err = AudioUnitUninitialize(g_snd_audio_unit);
  if (err != noErr) goto SND_DELETE_ABORT;

  // dispose of the component
  err = AudioComponentInstanceDispose(g_snd_audio_unit);

SND_DELETE_ABORT:;
}

static uint32
DAL_SndStreamRatesSupported()
{
  uint32 result;

  // default return value
  result = 0;

  // what rates/configurations are available
  result |= SND_STREAM_MONO  | SND_STREAM_STEREO;
  result |= SND_STREAM_8KHZ  | SND_STREAM_11KHZ |
            SND_STREAM_16KHZ | SND_STREAM_22KHZ | SND_STREAM_44KHZ;

  return result;
}

static void 
DAL_SndTerminate()
{
  // we are not longer active with audio
  AudioSessionSetActive(false);
}

static OSStatus
DAL_SndThread(void *inRefCon,
              AudioUnitRenderActionFlags *ioActionFlags,
              const AudioTimeStamp *inTimeStamp,
              UInt32 inBusNumber, UInt32 inNumberFrames, 
              AudioBufferList *ioData)
{
  void   *buffer;
  uint8   zero;
  uint32  frameCount, size;

  // lets make sure we have a valid audio buffer!
  if ((ioData != NULL) && (g_snd_active)) 
  {
    // obtain the information we need from our audio buffer
    size   = (uint32)ioData -> mBuffers[0].mDataByteSize;
    buffer = (void *)ioData -> mBuffers[0].mData;

    // call our callback engine
    if (g_snd_callback != NULL)
    {
      frameCount = size;
      if (g_snd_type   == snd_stereo) frameCount = frameCount >> 1;
      if (g_snd_format == snd_int16)  frameCount = frameCount >> 1;
  
      g_snd_callback(0, g_snd_user, buffer, frameCount);
    }
    else
    {
      zero = 0;
      if (g_snd_format == snd_uint8) zero = 0x80;
      memset(buffer, zero, size);
    }
  }

  return noErr;
}

void  
DAL_InterruptListener(void *in, UInt32 state)
{
  UInt32 category;

  switch (state)
  {
    case kAudioSessionBeginInterruption:

         // stop the audio session
         if (g_snd_stream_defined)
           AudioOutputUnitStop(g_snd_audio_unit);

         // we are not longer active with audio
         AudioSessionSetActive(false);

         break;

    case kAudioSessionEndInterruption:

         // restart the audio session
         if (g_snd_stream_defined)
           AudioOutputUnitStart(g_snd_audio_unit);

         // what type of audio session are we?
         category = kAudioSessionCategory_AmbientSound;
         AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, 
                                 sizeof(category), &category);

         // we are now active with audio
         AudioSessionSetActive(true);

         break;

     default:
         break;
  }
}

#undef GLOBALS_DEFINITION
#undef GLOBALS_UNINITIALIZED
#undef SND_GLOBALS

/********************************* EOF ***********************************/
