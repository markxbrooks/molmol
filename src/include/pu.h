/*
************************************************************************
*
*   pu.h - portable user interface library
*
*   Copyright (c) 1994-97
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
*   Date of last modification : 97/07/28
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/include/SCCS/s.pu.h
*   SCCS identification       : 1.8
*
************************************************************************
*/

#ifndef _PU_H_
#define _PU_H_

#include <pu_types.h>


/* administration */

extern PuRes PuInit(char *, int *, char *argv[]);

extern PuRes PuCleanup(void);

extern PuRes PuSetDevice(char *);


/* event handling */

extern void PuEventLoop(void);

extern void PuProcessEvent(void);


/* callback */

extern void PuAddExposeCB(PuExposeCB, void *, PuFreeCB);

extern void PuRemoveExposeCB(PuExposeCB, void *);

extern void PuAddResizeCB(PuResizeCB, void *, PuFreeCB);

extern void PuRemoveResizeCB(PuResizeCB, void *);

extern void PuAddKeyCB(PuKeyCB, void *, PuFreeCB);

extern void PuRemoveKeyCB(PuKeyCB, void *);

extern void PuAddButtonPressCB(PuMouseButton,
    PuMouseCB, void *, PuFreeCB);

extern void PuRemoveButtonPressCB(PuMouseButton, PuMouseCB, void *);

extern void PuAddButtonReleaseCB(PuMouseButton,
    PuMouseCB, void *, PuFreeCB);

extern void PuRemoveButtonReleaseCB(PuMouseButton, PuMouseCB, void *);

extern void PuAddDragCB(PuMouseButton, PuMouseCB, void *, PuFreeCB);

extern void PuRemoveDragCB(PuMouseButton, PuMouseCB, void *);

extern void PuAddMoveCB(PuMouseCB, void *, PuFreeCB);

extern void PuRemoveMoveCB(PuMouseCB, void *);

extern void PuAddCmdCB(PuCmdCB, void *, PuFreeCB);

extern void PuRemoveCmdCB(PuCmdCB, void *);

extern void PuAddDropCB(PuDropCB, void *, PuFreeCB);

extern void PuRemoveDropCB(PuDropCB, void *);

extern void PuAddQuitCB(PuQuitCB, void *, PuFreeCB);

extern void PuRemoveQuitCB(PuQuitCB, void *);


/* time out */

extern void PuAddTimeOut(int, PuTimeOut, void *);


/* external input */

extern void PuAddExtInput(int, PuExtInput, void *, PuFreeCB);

extern void PuRemoveExtInput(int, PuExtInput, void *);


/* main window */

extern void PuSwitchFullscreen(BOOL);

extern void PuSetDrawSize(int, int);

extern void PuSetTextField(PuTextFieldChoice, char *);

extern void PuSwitchTextField(PuTextFieldChoice, BOOL);


/* menus */

extern PuMenubar PuCreateMenubar(void);

extern PuMenu PuCreatePulldown(PuMenubar, char *, char);

extern PuMenu PuCreatePopup(char *);

extern PuMenu PuCreatePullright(PuMenu, char *, char);

extern PuMenuEntry PuCreateMenuEntry(PuMenu, char *,
    char, char *, char *,
    PuMenuCB, void *, PuFreeCB);

extern void PuSwitchMenubar(PuMenubar, BOOL);

extern void PuSwitchPopup(PuMenu, BOOL);


/* buttons */

extern PuCmdField PuCreateCmdField(char *,
    PuCmdFieldCB, void *, PuFreeCB);

extern void PuSwitchCmdFields(BOOL);


/* valuators */

extern PuValuatorBox PuCreateValuatorBox(char *);

extern PuValuator PuCreateValuator(PuValuatorBox, char *,
    float, float, int, float,
    PuValuatorCB, void *, PuFreeCB);

extern void PuSwitchValuatorBox(PuValuatorBox, BOOL);


/* busy cursor */

extern void PuSetBusyCursor(BOOL);


/* messages */

extern void PuShowMessage(PuMessageType, char *, char *);

extern void PuShowHelp(PuGizmo, char *, char *);

extern void PuBeep(void);


/* text windows */

extern PuTextWindow PuCreateTextWindow(char *);

extern void PuWriteStr(PuTextWindow, char *);


/* dialogs */

extern PuGizmo PuCreateFileSelectionBox(char *,
    char *, char *, PuFileAccess);

extern PuGizmo PuCreateDialog(char *, int, int);

extern PuGizmo PuCreateLabel(PuGizmo, char *);

extern PuGizmo PuCreateButton(PuGizmo, char *);

extern PuGizmo PuCreateColorField(PuGizmo, char *);

extern PuGizmo PuCreateSlider(PuGizmo, char *, float, float, int, float);

extern PuGizmo PuCreateTextField(PuGizmo, char *, char *);

extern PuGizmo PuCreateText(PuGizmo, char *, char *);

extern PuGizmo PuCreateFileViewer(PuGizmo, char *, BOOL);

extern PuGizmo PuCreateList(PuGizmo, char *);

extern void PuAddListEntry(PuGizmo, char *, BOOL);

extern void PuRemoveListEntries(PuGizmo, int, int);

extern PuGizmo PuCreateRadioBox(PuGizmo, char *);

extern PuGizmo PuCreateCheckBox(PuGizmo, char *);

extern void PuAddToggle(PuGizmo, char *, BOOL);

extern void PuSetBool(PuGizmo, PuBoolChoice, BOOL);

extern void PuSetInt(PuGizmo, PuIntChoice, int);

extern void PuSetStr(PuGizmo, PuStrChoice, char *);

extern void PuSetColor(PuGizmo, float, float, float);

extern void PuSetConstraints(PuGizmo, PuConstraints);

extern void PuSwitchGizmo(PuGizmo, BOOL);

extern void PuDestroyGizmo(PuGizmo);

extern void PuAddGizmoCB(PuGizmo, PuGizmoCBType, PuGizmoCB, void *, PuFreeCB);

#endif  /* _PU_H_ */
