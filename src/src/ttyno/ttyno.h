/*
************************************************************************
*
*   ttyno.h - TTY/NO device
*
*   Copyright (c) 1994-95
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
*   Date of last modification : 95/06/15
*   Pathname of SCCS file     : /sgiext/molmol/src/ttyno/SCCS/s.ttyno.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <io.h>

extern IORes IOTTYNOInit(char *, int *, char *[]);

extern IORes IOTTYNOCleanup(void);

extern IORes IOTTYNOSetWindow(PuWindow);

extern void IOTTYNOEndFrame(void);
