/*
************************************************************************
*
*   motxgl.h - Motif/XGL device
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
*   Date of last modification : 96/06/21
*   Pathname of SCCS file     : /sgiext/molmol/src/motxgl/SCCS/s.motxgl.h
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include <io.h>

extern IORes IOMotifXGLInit(char *, int *, char *[]);

extern IORes IOMotifXGLCleanup(void);

extern IORes IOMotifXGLSetWindow(PuWindow);

extern void IOMotifXGLEndFrame(void);

extern IORes IOMotifXGLStartDump(char *, char *, SgPlotOrient,
    int, int, int, float);

extern IORes IOMotifXGLEndDump(void);
