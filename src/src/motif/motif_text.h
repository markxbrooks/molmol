/*
************************************************************************
*
*   motif_text.h - Motif text utility functions
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
*   Date of last modification : 95/01/23
*   Pathname of SCCS file     : /sgiext/molmol/src/motif/SCCS/s.motif_text.h
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <Xm/Text.h>

#include <pu_types.h>

extern void TextValueChangedCB(Widget, XtPointer, XtPointer);

extern BOOL TextStrucMotifToPu(
    XmTextVerifyCallbackStruct *, PuTextCBStruc *);

extern void TextStrucPuToMotif(
    PuTextCBStruc *, XmTextVerifyCallbackStruct *);
