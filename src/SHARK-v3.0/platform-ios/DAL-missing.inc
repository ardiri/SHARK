/*************************************************************************
 *
 * Copyright 2001+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-missing.inc
 */

/*************************************************************************
 * Configuration
 *************************************************************************/

// none

/*************************************************************************
 * Component Variables
 *************************************************************************/

// none

/*************************************************************************
 * Platform Independent Implementation
 *************************************************************************/

// none

/*************************************************************************
 * Implementation
 *************************************************************************/

// NOTE:
// - the following functions are used through the base SHARK code and
//   must be mapped to the appropriate device level for compiling
//
// #define malloc  malloc
// #define free    free
// #define memset  memset
// #define sprintf sprintf
// #define strlen  strlen
// #define strcat  strcat
// #define strcpy  strcpy
// #define strrev  strrev  * not available on all platforms, see below:

static char *
strrev(char *str)
{
  int32 cnt, len;
  char *tmp, *p;

  if (str != NULL)
  {
    len = strlen(str);
    if (len != 0)
    {
      tmp = (char *)malloc(len+1);

      // copy the string in the reverse order
      cnt = len;
      p = tmp; str += len;
      do
      {
        *p++ = *--str;
      } while (--cnt);
      *p = 0;

      // copy the new string over the old string
      strcpy(str, tmp);
      free(tmp);
    }
  }

  return str;
}

/********************************* EOF ***********************************/
