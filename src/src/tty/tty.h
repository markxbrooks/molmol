/*
************************************************************************
*
*   tty.h - TTY functions
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
*   Date of last modification : 97/02/17
*   Pathname of SCCS file     : /local/home/kor/molmol/src/tty/SCCS/s.tty.h
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <pu_types.h>

extern PuRes PuTTYInit(char *, int *, char *[]);

extern PuRes PuTTYCleanup(void);

extern void PuTTYEventLoop(void);

extern void PuTTYProcessEvent(void);

extern void PuTTYAddTimeOut(int, PuTimeOut, void *);

extern void PuTTYAddExtInput(int, PuExtInput, void *, PuFreeCB);

extern void PuTTYRemoveExtInput(int, PuExtInput, void *);

extern void PuTTYSwitchFullscreen(BOOL);

extern void PuTTYSetDrawSize(int, int);

extern void PuTTYSetTextField(PuTextFieldChoice, char *);

extern void PuTTYSwitchTextField(PuTextFieldChoice, BOOL);

extern PuMenubar PuTTYCreateMenubar(void);

extern PuMenu PuTTYCreatePulldown(PuMenubar, char *, char);

extern PuMenu PuTTYCreatePopup(char *);

extern PuMenu PuTTYCreatePullright(PuMenu, char *, char);

extern PuMenuEntry PuTTYCreateMenuEntry(PuMenu, char *,
    char, char *, char *,
    PuMenuCB, void *, PuFreeCB);

extern void PuTTYSwitchMenubar(PuMenubar, BOOL);

extern void PuTTYSwitchPopup(PuMenu, BOOL);

extern PuCmdField PuTTYCreateCmdField(char *,
    PuCmdFieldCB, void *, PuFreeCB);

extern void PuTTYSwitchCmdFields(BOOL);

extern PuValuatorBox PuTTYCreateValuatorBox(char *);

extern PuValuator PuTTYCreateValuator(PuValuatorBox, char *,
    float, float, int, float,
    PuValuatorCB, void *, PuFreeCB);

extern void PuTTYSwitchValuatorBox(PuValuatorBox, BOOL);

extern void PuTTYSetBusyCursor(BOOL);

extern void PuTTYShowMessage(PuMessageType, char *, char *);

extern void PuTTYShowHelp(PuGizmo, char *, char *);

extern void PuTTYBeep(void);

extern PuTextWindow PuTTYCreateTextWindow(char *);

extern void PuTTYWriteStr(PuTextWindow, char *);

extern PuGizmo PuTTYCreateFileSelectionBox(char *,
    char *, char *, PuFileAccess);

extern PuGizmo PuTTYCreateDialog(char *, int, int);

extern PuGizmo PuTTYCreateLabel(PuGizmo, char *);

extern PuGizmo PuTTYCreateButton(PuGizmo, char *);

extern PuGizmo PuTTYCreateColorField(PuGizmo, char *);

extern PuGizmo PuTTYCreateSlider(PuGizmo, char *,
    float, float, int, float);

extern PuGizmo PuTTYCreateTextField(PuGizmo, char *, char *);

extern PuGizmo PuTTYCreateText(PuGizmo, char *, char *);

extern PuGizmo PuTTYCreateFileViewer(PuGizmo, char *, BOOL);

extern PuGizmo PuTTYCreateList(PuGizmo, char *);

extern void PuTTYAddListEntry(PuGizmo, char *, BOOL);

extern void PuTTYRemoveListEntries(PuGizmo, int, int);

extern PuGizmo PuTTYCreateRadioBox(PuGizmo, char *);

extern PuGizmo PuTTYCreateCheckBox(PuGizmo, char *);

extern void PuTTYAddToggle(PuGizmo, char *, BOOL);

extern void PuTTYSetBool(PuGizmo, PuBoolChoice, BOOL);

extern void PuTTYSetInt(PuGizmo, PuIntChoice, int);

extern void PuTTYSetStr(PuGizmo, PuStrChoice, char *);

extern void PuTTYSetColor(PuGizmo, float, float, float);

extern void PuTTYSetConstraints(PuGizmo, PuConstraints);

extern void PuTTYSwitchGizmo(PuGizmo, BOOL);

extern void PuTTYDestroyGizmo(PuGizmo);

extern void PuTTYAddGizmoCB(PuGizmo, PuGizmoCBType,
    PuGizmoCB, void *, PuFreeCB);
