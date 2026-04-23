/*
************************************************************************
*
*   io.h - I/O device (merges graphics device and user inteface device)
*
*   Copyright (c) 1994-96
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
*   Date of last modification : 96/06/21
*   Pathname of SCCS file     : /sgiext/molmol/include/SCCS/s.io.h
*   SCCS identification       : 1.5
*
************************************************************************
*/

#ifndef _IO_H_
#define _IO_H_

#include <sg_types.h>
#include <pu.h>

typedef enum {
  IO_RES_OK,
  IO_RES_ERR
} IORes;

typedef void (*IOErrorHandler) (char *);

extern IORes IOInit(char *, int *, char *argv[]);

extern IORes IOCleanup(void);

extern IORes IOSetDevice(char *);

extern void IOSetErrorHandler(IOErrorHandler);

extern IORes IOSetWindow(PuWindow);

extern void IOEndFrame(void);

/* dumping of image file is not in Sg because some graphics
   systems (OpenGL) require involvment of window system for
   off-screen rendering */

extern IORes IOStartDump(char *, char *,
    SgPlotOrient, int, int, int, float);

extern IORes IOEndDump(void);

#endif  /* _IO_H_ */
