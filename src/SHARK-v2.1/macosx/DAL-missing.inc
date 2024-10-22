/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-missing.inc
 */

/*************************************************************************
 *
 * Configuration
 *
 *************************************************************************/

/*************************************************************************
 *
 * Global Variables
 *
 *************************************************************************/

/*************************************************************************
 *
 * Implementation
 *
 *************************************************************************/

static char *
strrev(char *str)
{
  int32 cnt, len;
  char *tmp, *p;

  if (str != NULL)
  {
    len = strlen(str);
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

  return str;
}

#define MKDIR_MODE (S_IFDIR | S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)
#define mkdir(s) mkdir(s, MKDIR_MODE)

/*************************************************************************
 *
 * Implementation - device independent
 *
 *************************************************************************/

/********************************* EOF ***********************************/
