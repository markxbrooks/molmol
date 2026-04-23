/*
************************************************************************
*
*   motgl.h - Motif/GL device
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
*   Date of last modification : 94/08/16
*   Pathname of SCCS file     : /sgiext/molmol/src/motgl/SCCS/s.motgl.h
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <io.h>

extern IORes IOMotifGLInit(char *, int *, char *[]);

extern IORes IOMotifGLCleanup(void);

extern IORes IOMotifGLSetWindow(PuWindow);

extern void IOMotifGLEndFrame(void);
