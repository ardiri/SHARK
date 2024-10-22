/*************************************************************************
 *
 * Copyright 2001+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-preferences.inc
 */

/*************************************************************************
 * Configuration
 *************************************************************************/

// none

/*************************************************************************
 * Component Variables
 *************************************************************************/

#define PREF_GLOBALS          g_pref
#define GLOBALS_UNINITIALIZED (PREF_GLOBALS == NULL)
#define GLOBALS_DEFINITION    PrefGlobals *g_pref = NULL

/*************************************************************************
 * Platform Independent Implementation
 *************************************************************************/

#include "../common/SHARK-preferences.inc"

/*************************************************************************
 * Implementation
 *************************************************************************/

static boolean
DAL_PreferencesInitialize()
{
  boolean init_ok;

  // default return value
  init_ok = true;

  // clear memory used by memory globals
  memset(PREF_GLOBALS, 0, sizeof(PrefGlobals));

  return init_ok;
}

void
DAL_PreferencesLoad()
{
  NSString *nBlock;
  char     *cBlock;
  char      table[256], val;
  int       i, len, cnt;
  uint8     o[3], v[4];
  uint8    *pS, *pD;

  // do we have any preferences?
  if (g_config.preferences.size > 0)
  {
    // clear the existing preferences
    memset(g_pref_data, 0, g_config.preferences.size);

    // load the block from preferences
    nBlock = (NSString *)
      CFPreferencesCopyAppValue((CFStringRef)@"prefs",
                                kCFPreferencesCurrentApplication);
    if (nBlock)
    {
      // get the UTF8 encoding
      cBlock = (char *)[ nBlock cStringUsingEncoding:NSASCIIStringEncoding ];

      // initialize our decoding table
      val = 0;
      for (i=0; i<256; i++)    table[i] = 0x80;
      for (i='A'; i<='Z'; i++) table[i] = val++;
      for (i='a'; i<='z'; i++) table[i] = val++;
      for (i='0'; i<='9'; i++) table[i] = val++;
      table['+'] = val++; table['/'] = val++; table['='] = 0;

      // convert cBlock -> g_pref_data
      pS = (uint8 *)cBlock; 
      pD = (uint8 *)g_pref_data;

      len = strlen(cBlock);
      while (len > 0)
      {
        for (i=0; i<4; i++) v[i] = table[pS[i]];

        // decode 4 bytes into 3 (remember, padding = '=')
        o[0] = (v[0] << 2) | (v[1] >> 4);
        o[1] = (v[1] << 4) | (v[2] >> 2);
        o[2] = (v[2] << 6) |  v[3];
   
        // how many of these bytes are actually real data?
        cnt = (pS[2] == '=') ? 1 : (pS[3] == '=') ? 2 : 3;
        for (i=0;i<cnt;i++) pD[i] = o[i];

        pS += 4; len -= 4; pD += cnt;
      }
 
      // release the memory block
      [ nBlock release ];
    }
  }
}

void
DAL_PreferencesSave()
{
  NSString *nBlock;
  char     *cBlock;
  uint8    *pS, *pD;
  int       size, cnt;

  char table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

  // do we have any preferences?
  if (g_config.preferences.size > 0)
  {
    size = (((g_config.preferences.size + 2) / 3) * 4) + 1; // allow for \0

    // allocate memory for cString
    cBlock = malloc(size);
    if (cBlock)
    {
      // convert g_pref_data -> cBlock
      pS = (uint8 *)g_pref_data; 
      pD = (uint8 *)cBlock; 

      cnt = g_config.preferences.size;
      while (cnt > 0)
      {
        switch (MIN(3, cnt))
        {
          case 3:
               *pD++ = table[  pS[0]         >> 2 ];
               *pD++ = table[((pS[0] & 0x03) << 4) | (pS[1] >> 4)];
               *pD++ = table[((pS[1] & 0x0F) << 2) | (pS[2] >> 6)];
               *pD++ = table[  pS[2] & 0x3F ];

               cnt -= 3; pS += 3;
               break;

          case 2:
               *pD++ = table[  pS[0]         >> 2 ];
               *pD++ = table[((pS[0] & 0x03) << 4) | (pS[1] >> 4)];
               *pD++ = table[((pS[1] & 0x0F) << 2)];
               *pD++ = '=';

               cnt -= 2; pS += 2;
               break;

          case 1:
               *pD++ = table[  pS[0]         >> 2 ];
               *pD++ = table[((pS[0] & 0x03) << 4)];
               *pD++ = '=';
               *pD++ = '=';

               cnt--; pS++;
               break;

          default:
               break;
        }
      }
      *pD++ = 0;

      // create the memory block for writing preferences
      nBlock = [ [ NSString alloc ] initWithCString:cBlock 
                                    encoding:NSASCIIStringEncoding ];
      if (nBlock)
      {
        // save the block chunk
        CFPreferencesSetAppValue((CFStringRef)@"prefs", nBlock, 
                                 kCFPreferencesCurrentApplication);
    
        // commit the preferences 
        CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
  
        // free the memory block
        [ nBlock release ];
      }

      // release memory used for cStrign
      free(cBlock);
    }
  }
}

void 
DAL_PreferencesTerminate()
{
}

#undef GLOBALS_DEFINITION
#undef GLOBALS_UNINITIALIZED
#undef PREF_GLOBALS

/********************************* EOF ***********************************/
