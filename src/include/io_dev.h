/*
************************************************************************
*
*   io_dev.h - structure for I/O device
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
*   Pathname of SCCS file     : /sgiext/molmol/include/SCCS/s.io_dev.h
*   SCCS identification       : 1.5
*
************************************************************************
*/

#include <io.h>

typedef IORes (*IOInitF) (char *, int *, char *[]);
typedef IORes (*IOCleanupF) (void);
typedef IORes (*IOSetWindowF) (PuWindow);
typedef void (*IOEndFrameF) (void);
typedef IORes (*IOStartDumpF) (char *, char *,
    SgPlotOrient, int, int, int, float);
typedef IORes (*IOEndDumpF) (void);

typedef struct {
  IOInitF init;
  IOCleanupF cleanup;
  IOSetWindowF setWindow;
  IOEndFrameF endFrame;
  IOStartDumpF startDump;
  IOEndDumpF endDump;
  char *puDevName;
  char *sgDevName;
} IODev;

extern IODev *CurrIODevP;
