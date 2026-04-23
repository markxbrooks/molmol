/*
************************************************************************
*
*   motif_event.h - Motif functions for handling events
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
*   Pathname of SCCS file     : /sgiext/molmol/src/motif/SCCS/s.motif_event.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <pu_types.h>

extern void PuMotifEventLoop(void);

extern void PuMotifProcessEvent(void);

extern void PuMotifAddTimeOut(int, PuTimeOut, void *);
