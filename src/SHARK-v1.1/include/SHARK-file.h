/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-file.h
 */

#ifndef SHARK_FILE_H
#define SHARK_FILE_H

#include "../SHARK.h"

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************
 *
 * Constants / Structures
 *
 *************************************************************************/

typedef enum { file_beginning = 0, file_current, file_end } file_position;

typedef struct file file;

/*************************************************************************
 *
 * Application Programmers Interface
 *
 *************************************************************************/

// device specific
extern file       *_FindFile(uint8 *fileName);
extern file       *_FileCreate(uint8 *fileName);

extern boolean     _FileOpen(file *f);
extern void        _FileSeek(file *f, uint32 ofs, file_position position);
extern uint32      _FileTell(file *f);
extern boolean     _FileEOF(file *f);
extern void        _FileRead(file *f, void *buf, uint16 objSize, uint16 numObject, uint16 *objRead);
extern uint16      _FileWrite(file *f, void *buf, uint16 objSize, uint16 numObject, int16 *err);
extern void        _FileClose(file *f);
extern void        _FileDelete(file *f);

// device independent
// NONE

#ifdef __cplusplus
}
#endif

#endif

/********************************* EOF ***********************************/
