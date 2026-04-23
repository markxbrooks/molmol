/*
************************************************************************
*
*   motif_p.h - private Motif functions for use inside I/O device
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
*   Pathname of SCCS file     : /sgiext/molmol/include/SCCS/s.motif_p.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <X11/Intrinsic.h>

extern Widget MotifGetTopW(void);

extern Widget MotifGetDrawParentW(void);

extern void MotifSetDrawW(Widget);

extern void MotifSetPopupParentW(Widget);
