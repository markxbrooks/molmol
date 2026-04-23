/*
************************************************************************
*
*   motif_dial.h - Motif functions for building dialogs
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
*   Date of last modification : 97/02/17
*   Pathname of SCCS file     : /local/home/kor/molmol/src/motif/SCCS/s.motif_dial.h
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include <pu_types.h>

extern PuGizmo PuMotifCreateFileSelectionBox(char *,
    char *, char *, PuFileAccess);

extern PuGizmo PuMotifCreateDialog(char *, int, int);

extern PuGizmo PuMotifCreateLabel(PuGizmo, char *);

extern PuGizmo PuMotifCreateButton(PuGizmo, char *);

extern PuGizmo PuMotifCreateColorField(PuGizmo, char *);

extern PuGizmo PuMotifCreateSlider(PuGizmo, char *,
    float, float, int, float);

extern PuGizmo PuMotifCreateTextField(PuGizmo, char *, char *);

extern PuGizmo PuMotifCreateText(PuGizmo, char *, char *);

extern PuGizmo PuMotifCreateFileViewer(PuGizmo, char *, BOOL);

extern PuGizmo PuMotifCreateList(PuGizmo, char *);

extern void PuMotifAddListEntry(PuGizmo, char *, BOOL);

extern void PuMotifRemoveListEntries(PuGizmo, int, int);

extern PuGizmo PuMotifCreateRadioBox(PuGizmo, char *);

extern PuGizmo PuMotifCreateCheckBox(PuGizmo, char *);

extern void PuMotifAddToggle(PuGizmo, char *, BOOL);

extern void PuMotifSetBool(PuGizmo, PuBoolChoice, BOOL);

extern void PuMotifSetInt(PuGizmo, PuIntChoice, int);

extern void PuMotifSetStr(PuGizmo, PuStrChoice, char *);

extern void PuMotifSetColor(PuGizmo, float, float, float);

extern void PuMotifSetConstraints(PuGizmo, PuConstraints);

extern void PuMotifSwitchGizmo(PuGizmo, BOOL);

extern void PuMotifDestroyGizmo(PuGizmo);

extern void PuMotifAddGizmoCB(PuGizmo, PuGizmoCBType,
    PuGizmoCB, void *, PuFreeCB);
