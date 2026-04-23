/*
************************************************************************
*
*   motif_access.h - access functions
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
*   Pathname of SCCS file     : /sgiext/molmol/src/motif/SCCS/s.motif_access.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <motif_p.h>

extern XtAppContext MotifGetAppContext(void);

extern Widget MotifGetMainW(void);

extern Widget MotifGetDrawW(void);

extern Widget MotifGetPopupParentW(void);

extern Widget MotifGetPopupW(void);

extern void MotifSetTopW(Widget);

extern void MotifSetMenubarW(Widget);

extern void MotifSetPopupW(Widget);
