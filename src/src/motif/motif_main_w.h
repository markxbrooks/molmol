/*
************************************************************************
*
*   motif_main_w.h - Motif main window
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
*   Date of last modification : 95/12/14
*   Pathname of SCCS file     : /sgiext/molmol/src/motif/SCCS/s.motif_main_w.h
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <pu_types.h>

extern void PuMotifSwitchFullscreen(BOOL);

extern void PuMotifSetDrawSize(int, int);

extern void PuMotifSetTextField(PuTextFieldChoice, char *);

extern void PuMotifSwitchTextField(PuTextFieldChoice, BOOL);

extern PuCmdField PuMotifCreateCmdField(char *,
    PuCmdFieldCB, void *, PuFreeCB);

extern void PuMotifSwitchCmdFields(BOOL);

extern void MotifSetFocus(void);
