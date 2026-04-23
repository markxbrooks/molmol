/*
************************************************************************
*
*   motif_text_w.h - Motif text windows
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
*   Date of last modification : 94/06/02
*   Pathname of SCCS file     : /sgiext/molmol/src/motif/SCCS/s.motif_text_w.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <pu_types.h>

extern PuTextWindow PuMotifCreateTextWindow(char *);

extern void PuMotifWriteStr(PuTextWindow, char *);
