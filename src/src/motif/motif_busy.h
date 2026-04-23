/*
************************************************************************
*
*   motif_busy.h - management of busy cursor
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
*   Pathname of SCCS file     : /sgiext/molmol/src/motif/SCCS/s.motif_busy.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <bool.h>
#include <X11/Intrinsic.h>

extern void PuMotifSetBusyCursor(BOOL);

extern void MotifAddDialog(Widget);

extern void MotifUpdateCursor(Widget);
