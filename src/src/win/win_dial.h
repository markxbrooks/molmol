/*
************************************************************************
*
*   win_dial.h - Windows functions for building dialogs
*
*   Copyright (c) 1996-97
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
*   Date of last modification : 97/02/24
*   Pathname of SCCS file     : /local/home/kor/molmol/src/win/SCCS/s.win_dial.h
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <pu_types.h>

extern PuGizmo PuWinCreateFileSelectionBox(char *,
    char *, char *, PuFileAccess);

extern PuGizmo PuWinCreateDialog(char *, int, int);

extern PuGizmo PuWinCreateLabel(PuGizmo, char *);

extern PuGizmo PuWinCreateButton(PuGizmo, char *);

extern PuGizmo PuWinCreateColorField(PuGizmo, char *);

extern PuGizmo PuWinCreateSlider(PuGizmo, char *,
    float, float, int, float);

extern PuGizmo PuWinCreateTextField(PuGizmo, char *, char *);

extern PuGizmo PuWinCreateText(PuGizmo, char *, char *);

extern PuGizmo PuWinCreateFileViewer(PuGizmo, char *, BOOL);

extern PuGizmo PuWinCreateList(PuGizmo, char *);

extern void PuWinAddListEntry(PuGizmo, char *, BOOL);

extern void PuWinRemoveListEntries(PuGizmo, int, int);

extern PuGizmo PuWinCreateRadioBox(PuGizmo, char *);

extern PuGizmo PuWinCreateCheckBox(PuGizmo, char *);

extern void PuWinAddToggle(PuGizmo, char *, BOOL);

extern void PuWinSetBool(PuGizmo, PuBoolChoice, BOOL);

extern void PuWinSetInt(PuGizmo, PuIntChoice, int);

extern void PuWinSetStr(PuGizmo, PuStrChoice, char *);

extern void PuWinSetColor(PuGizmo, float, float, float);

extern void PuWinSetConstraints(PuGizmo, PuConstraints);

extern void PuWinSwitchGizmo(PuGizmo, BOOL);

extern void PuWinDestroyGizmo(PuGizmo);

extern void PuWinAddGizmoCB(PuGizmo, PuGizmoCBType,
    PuGizmoCB, void *, PuFreeCB);

extern PuValuatorBox PuWinCreateValuatorBox(char *);

extern PuValuator PuWinCreateValuator(PuValuatorBox, char *,
    float, float, int, float,
    PuValuatorCB, void *, PuFreeCB);

extern void PuWinSwitchValuatorBox(PuValuatorBox, BOOL);
