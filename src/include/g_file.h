/*
************************************************************************
*
*   g_file.h - general file handling
*
*   Copyright (c) 1994
*
*   ETH Zuerich
*   Institut fuer Molekularbiologie und Biophysik
*   ETH-Hoenggerberg
*   CH-8093 Zuerich
*
*   SPECTROSPIN AG
*   Industriestr. 26
*   CH-8117 Faellanden
*
*   All Rights Reserved
*
*   Date of last modification : 95/06/24
*   Pathname of SCCS file     : /sgiext/molmol/include/SCCS/s.g_file.h
*   SCCS identification       : 1.3
*
************************************************************************
*/

#ifndef _G_FILE_H_
#define _G_FILE_H_

#include <mach_types.h>
#include <bool.h>

typedef struct GFileStruc *GFile;

typedef enum {
  GF_FORMAT_ASCII,
  GF_FORMAT_BINARY,
  GF_FORMAT_MEMORY
} GFileFormat;

typedef enum {
  GF_RES_OK,
  GF_RES_ERR
} GFileRes;

typedef void (*GFileErrorHandler) (char *, char *);

extern void GFileSetErrorHandler(GFileErrorHandler);

extern GFile GFileOpenNew(char *, GFileFormat);

extern GFile GFileOpenRewrite(char *);

extern GFile GFileOpenAppend(char *);

extern GFile GFileOpenRead(char *);

extern void GFileClose(GFile);

extern BOOL GFileEOF(GFile);

extern void GFileFlush(GFile);

extern GFileRes GFileWriteChar(GFile, char);

extern GFileRes GFileWriteINT16(GFile, INT16);

extern GFileRes GFileWriteINT32(GFile, INT32);

extern GFileRes GFileWriteFLOAT32(GFile, FLOAT32);

extern GFileRes GFileWriteFLOAT64(GFile, FLOAT64);

extern GFileRes GFileWriteStr(GFile, char *);

extern GFileRes GFileWriteQuotedStr(GFile, char *);

extern GFileRes GFileWriteLine(GFile, char *);

extern GFileRes GFileWriteNL(GFile);

extern GFileRes GFileReadChar(GFile, char *);

extern GFileRes GFileReadINT16(GFile, INT16 *);

extern GFileRes GFileReadINT32(GFile, INT32 *);

extern GFileRes GFileReadFLOAT32(GFile, FLOAT32 *);

extern GFileRes GFileReadFLOAT64(GFile, FLOAT64 *);

extern GFileRes GFileReadStr(GFile, char *, int);

extern GFileRes GFileReadQuotedStr(GFile, char *, int);

extern GFileRes GFileReadLine(GFile, char *, int);

#endif  /* _G_FILE_H_ */
