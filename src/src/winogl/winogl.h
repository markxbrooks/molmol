/*
************************************************************************
*
*   winogl.h - Windows/OpenGL device
*
*   Copyright (c) 1996
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
*   Pathname of SCCS file     : /local/home/kor/molmol/src/winogl/SCCS/s.winogl.h
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <io.h>

extern IORes IOWinOGLInit(char *, int *, char *[]);

extern IORes IOWinOGLCleanup(void);

extern IORes IOWinOGLSetWindow(PuWindow);

extern void IOWinOGLEndFrame(void);

extern IORes IOWinOGLStartDump(char *, char *,
    SgPlotOrient, int, int, int, float);

extern IORes IOWinOGLEndDump(void);
