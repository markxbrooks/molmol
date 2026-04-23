/*
************************************************************************
*
*   motogl.h - Motif/OpenGL device
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
*   Pathname of SCCS file     : /sgiext/molmol/src/motogl/SCCS/s.motogl.h
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include <io.h>

extern IORes IOMotifOGLInit(char *, int *, char *[]);

extern IORes IOMotifOGLCleanup(void);

extern IORes IOMotifOGLSetWindow(PuWindow);

extern void IOMotifOGLEndFrame(void);

extern IORes IOMotifOGLStartDump(char *, char *,
    SgPlotOrient, int, int, int, float);

extern IORes IOMotifOGLEndDump(void);
