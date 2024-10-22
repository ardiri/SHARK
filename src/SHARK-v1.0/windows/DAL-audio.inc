/*************************************************************************
 *
 * Copyright (c) 2002-2003 MobileWizardry
 * All rights reservered.
 *
 *************************************************************************/

/*
 * @(#)windows/DAL-audio.inc
 *
 * -- Aaron Ardiri     (mailto:aaron_ardiri@mobilewizardry.com)
 * -- Charles Kerchner (mailto:chip_kerchner@mobilewizardry.com)
 */

#include "DAL-prv.h"

/*************************************************************************
 *
 * Platform Implementation Structures
 *
 *************************************************************************/

typedef struct _Audio_DeviceInfo
{
  HANDLE thread;
  int    active;

  HANDLE request;
  uint32 freq;
  uint16 duration;
  uint16 volume;

} _Audio_DeviceInfo;

// volume look-up tables
static int max_v[] =
{
  0x80, 0x7f, 0x7e, 0x7d, 0x7b, 0x7a, 0x79, 0x78,
  0x76, 0x75, 0x74, 0x72, 0x71, 0x70, 0x6f, 0x6d,
  0x6c, 0x6b, 0x69, 0x68, 0x67, 0x66, 0x64, 0x63,
  0x62, 0x60, 0x5f, 0x5e, 0x5d, 0x5b, 0x5a, 0x59,
  0x58, 0x56, 0x55, 0x54, 0x52, 0x51, 0x50, 0x4f,
  0x4d, 0x4c, 0x4b, 0x49, 0x48, 0x47, 0x46, 0x44,
  0x43, 0x42, 0x40, 0x3f, 0x3e, 0x3d, 0x3b, 0x3a,
  0x39, 0x38, 0x36, 0x35, 0x34, 0x32, 0x31, 0x30,
  0x2f, 0x2d, 0x2c, 0x2b, 0x29, 0x28, 0x27, 0x26,
  0x24, 0x23, 0x22, 0x20, 0x1f, 0x1e, 0x1d, 0x1b,
  0x1a, 0x19, 0x18, 0x16, 0x15, 0x14, 0x12, 0x11,
  0x10, 0x0f, 0x0d, 0x0c, 0x0b, 0x09, 0x08, 0x07,
  0x06, 0x04, 0x03, 0x02, 0x00
};

static int min_v[] =
{
  0x80, 0x81, 0x82, 0x83, 0x85, 0x86, 0x87, 0x88, 
  0x8a, 0x8b, 0x8c, 0x8d, 0x8f, 0x90, 0x91, 0x93, 
  0x94, 0x95, 0x96, 0x98, 0x99, 0x9a, 0x9b, 0x9d, 
  0x9e, 0x9f, 0xa1, 0xa2, 0xa3, 0xa4, 0xa6, 0xa7, 
  0xa8, 0xa9, 0xab, 0xac, 0xad, 0xae, 0xb0, 0xb1,
  0xb2, 0xb4, 0xb5, 0xb6, 0xb7, 0xb9, 0xba, 0xbb, 
  0xbc, 0xbe, 0xbf, 0xc0, 0xc2, 0xc3, 0xc4, 0xc5, 
  0xc7, 0xc8, 0xc9, 0xca, 0xcc, 0xcd, 0xce, 0xd0, 
  0xd1, 0xd2, 0xd3, 0xd5, 0xd6, 0xd7, 0xd8, 0xda, 
  0xdb, 0xdc, 0xdd, 0xdf, 0xe0, 0xe1, 0xe3, 0xe4,
  0xe5, 0xe6, 0xe8, 0xe9, 0xea, 0xeb, 0xed, 0xee, 
  0xef, 0xf1, 0xf2, 0xf3, 0xf4, 0xf6, 0xf7, 0xf8, 
  0xf9, 0xfb, 0xfc, 0xfd, 0xff,
};

/*************************************************************************
 *
 * Initialization / Termination
 *
 *************************************************************************/

DWORD WINAPI _AudioThread(void *);

void
_AudioInitialize()
{
  _Audio_DeviceInfo *deviceP;

  gDALGlobals.audio.info = (void *)malloc(sizeof(_Audio_DeviceInfo));
  memset(gDALGlobals.audio.info, 0, sizeof(_Audio_DeviceInfo));
  deviceP = (_Audio_DeviceInfo *)gDALGlobals.audio.info;

  deviceP->request = CreateEvent(NULL, TRUE, FALSE, TEXT("AudioEvent"));
  deviceP->thread  = (HANDLE)CreateThread(NULL, 0, _AudioThread, (void *)deviceP, 0, NULL);
}

DWORD WINAPI 
_AudioThread(void *lpParam)
{
  _Audio_DeviceInfo *deviceP;

  HWAVEOUT        hwo  = NULL;
  WAVEFORMATEX    wfmt;
  WAVEHDR         whdr;
  void           *waveform_buffer;

  uint8          *bufP;
  uint8           min, max;
  uint32          min4, max4, mod4;
  int32           audio_len;
  register int32  i, cnt = 0;
  register int32  WAV_SQRCYCLEx;

  // initialize thread
  deviceP = (_Audio_DeviceInfo *)lpParam;
  SetThreadPriority(deviceP->thread, THREAD_PRIORITY_IDLE); 

  waveform_buffer = (void *)malloc(22000);  // max of 1 second @ 22Hz
  memset(waveform_buffer, 0, 22000);

  // prepare streaming audio
  wfmt.wFormatTag      = WAVE_FORMAT_PCM;
  wfmt.nChannels       = 1;
  wfmt.nSamplesPerSec  = 22050;
  wfmt.wBitsPerSample  = 8;
  wfmt.nBlockAlign     = wfmt.nChannels * wfmt.wBitsPerSample / 8;
  wfmt.nAvgBytesPerSec = wfmt.nSamplesPerSec * wfmt.nBlockAlign;
  wfmt.cbSize          = 0;
    
  // find and open audio device
#ifdef _M_IX86
  // choose the device wave mapper device
  waveOutOpen(&hwo, WAVE_MAPPER, &wfmt, 0, 0, CALLBACK_NULL);
#else
  // loop through all possible 'waveOut' devices (could be more than one)
  {
    MMRESULT mmres;
    UINT devId;

    devId = 0;
    do
    {
      mmres = waveOutOpen(&hwo, devId++, &wfmt, 0, 0, CALLBACK_NULL);
    } while ((mmres != MMSYSERR_NOERROR) && (devId < waveOutGetNumDevs()));
  }
//waveOutOpen(&hwo, 0, &wfmt, 0, 0, CALLBACK_NULL);  // <-- this works on every PPC so far
#endif

  memset(&whdr, 0, sizeof(WAVEHDR));
  whdr.lpData          = waveform_buffer;
  whdr.dwBufferLength  = 0;
  whdr.dwUser          = 0;
  whdr.dwFlags         = 0;
  whdr.dwLoops         = 0;
  whdr.dwBytesRecorded = 0;
  whdr.lpNext          = 0;
  whdr.reserved        = 0;
  waveOutPrepareHeader(hwo, &whdr, sizeof(WAVEHDR));  

  deviceP->active = true;
  while (deviceP->active)
  {
    WaitForSingleObject(deviceP->request, INFINITE);

    // do we have valid playback information?
    if ((deviceP->freq != 0)     && (deviceP->freq     < 11025) &&  // 22Hz, best we can do is half :)
        (deviceP->duration != 0) && (deviceP->duration < 1000))     // max 1 seconds (memory buffer)
    {
      // initialize variables for freq+duration configuration
      if (deviceP->volume > 100) deviceP->volume = 100;
      min           = (uint8)(*(min_v + deviceP->volume));  // ((-128 * volume)     / 100) + 128;
      max           = (uint8)(*(max_v + deviceP->volume));  // (( 127 * volume)     / 100) + 128;
      audio_len     = deviceP->duration * 22;                // ((22050L * duration) / 1000);
      WAV_SQRCYCLEx = (int32)(22050L / (deviceP->freq * 2));
  
      // wav data chunk
      bufP = (int8 *)waveform_buffer;
      i    = audio_len;
  
      min4 = ((uint32)(min) << 8) | min;
      max4 = ((uint32)(max) << 8) | max;
      min4 = (min4 << 16) | min4;
      max4 = (max4 << 16) | max4;
      while (i)
      {
        cnt = WAV_SQRCYCLEx; if (i < cnt) cnt = i; i -= cnt;
        if (cnt)
        {
          mod4 = (4 - ((uint)(bufP))) & 3;
          if (mod4)
          {
            if ((uint32)(cnt) < mod4) mod4 = cnt;
            do
            {
              *bufP++ = max;
            }
            while (--mod4);
          }
          mod4 = (cnt & 3);
          if (cnt >>= 2)
          {
            do
            {
              *((uint32 *)(bufP))++ = max4;
            }
            while (--cnt);
          }
          if (mod4)
          {
            do
            {
              *bufP++ = max;
            }
            while (--mod4);
          }
        }
        cnt = WAV_SQRCYCLEx; if (i < cnt) cnt = i; i -= cnt;
        if (cnt)                                  // square wave formation
        {
          mod4 = (4 - ((uint)(bufP))) & 3;
          if (mod4)
          {
            if ((uint32)(cnt) < mod4) mod4 = cnt;
            do
            {
              *bufP++ = min;
            }
            while (--mod4);
          }
          mod4 = (cnt & 3);
          if (cnt >>= 2)
          {
            do
            {
              *((uint32 *)(bufP))++ = min4;
            }
            while (--cnt);
          }
          if (mod4)
          {
            do
            {
              *bufP++ = min;
            }
            while (--mod4);
          }
        }
      }

      // terminate current playing sound (if it is playing)
      waveOutReset(hwo);  
  
      // prepare buffer for playback
      waveOutUnprepareHeader(hwo, &whdr, sizeof(WAVEHDR));  
      memset(&whdr, 0, sizeof(WAVEHDR));
      whdr.lpData          = waveform_buffer;
      whdr.dwBufferLength  = audio_len;  // need to re-prepare header due to this
      whdr.dwUser          = 0;
      whdr.dwFlags         = 0;
      whdr.dwLoops         = 0;
      whdr.dwBytesRecorded = 0;
      whdr.lpNext          = 0;
      whdr.reserved        = 0;
      waveOutPrepareHeader(hwo, &whdr, sizeof(WAVEHDR));  

      // play buffer
      waveOutWrite(hwo, &whdr, sizeof(WAVEHDR));

      SetThreadPriority(deviceP->thread, THREAD_PRIORITY_IDLE); 
      ResetEvent(deviceP->request);
    }
  }

  // unprepare header/shutdown
  waveOutUnprepareHeader(hwo, &whdr, sizeof(WAVEHDR));  
  waveOutClose(hwo);

  free(waveform_buffer);
  waveform_buffer = NULL;

  // terminate thread
  ExitThread(0);
  return 0;
}

void
_AudioTerminate()
{
  _Audio_DeviceInfo *deviceP;

  deviceP = (_Audio_DeviceInfo *)gDALGlobals.audio.info;

  deviceP->active = false;
  CloseHandle(deviceP->thread);

  free(gDALGlobals.audio.info);
}

/*************************************************************************
 *
 * Platform Implementation
 *
 *************************************************************************/

void
_PlaySound(uint32 freq, uint16 duration, uint16 volume)
{
  _Audio_DeviceInfo *deviceP;

  deviceP = (_Audio_DeviceInfo *)gDALGlobals.audio.info;

  deviceP->freq     = freq;
  deviceP->duration = duration;
  deviceP->volume   = volume;

  SetThreadPriority(deviceP->thread, THREAD_PRIORITY_NORMAL); 
  PulseEvent(deviceP->request);
}

/********************************* EOF ***********************************/
