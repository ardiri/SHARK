/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)DAL-file.h
 */

#ifndef DAL_FILE_H
#define DAL_FILE_H

#include "../DAL.h"

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
 *
 * Platform Constants/Structures
 *
 *************************************************************************/

// none

/*************************************************************************
 *
 * Initialization / Termination
 *
 *************************************************************************/

extern boolean     DAL_FileInitialize();
extern void        DAL_FileTerminate();

/*************************************************************************
 *
 * Device Specific
 *
 *************************************************************************/

extern file       *DAL_FindFile(uint8 *fileName);
extern file       *DAL_FileCreate(uint8 *fileName);

extern boolean     DAL_FileOpen(file *f);
extern void        DAL_FileSeek(file *f, uint32 ofs, file_position position);
extern uint32      DAL_FileTell(file *f);
extern boolean     DAL_FileEOF(file *f);
extern void        DAL_FileRead(file *f, void *buf, uint16 objSize, uint16 numObject, uint16 *objRead);
extern uint16      DAL_FileWrite(file *f, void *buf, uint16 objSize, uint16 numObject, int16 *err);
extern void        DAL_FileClose(file *f);
extern void        DAL_FileDelete(file *f);

/*************************************************************************
 *
 * SHARK Internal Use API's
 *
 *************************************************************************/

// none

#ifdef __cplusplus
}
#endif

#endif

/********************************* EOF ***********************************/
