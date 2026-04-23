/*
************************************************************************
*
*   motx11.h - Motif/X11 device
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
*   Pathname of SCCS file     : /sgiext/molmol/src/motx11/SCCS/s.motx11.h
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <io.h>

extern IORes IOMotifX11Init(char *, int *, char *[]);

extern IORes IOMotifX11Cleanup(void);

extern IORes IOMotifX11SetWindow(PuWindow);

extern void IOMotifX11EndFrame(void);
