/*
************************************************************************
*
*   pu_dev.h - structure for Pu device
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
*   Pathname of SCCS file     : /local/home/kor/molmol/include/SCCS/s.pu_dev.h
*   SCCS identification       : 1.7
*
************************************************************************
*/

#include <pu.h>

typedef PuRes (*PuInitF) (char *, int *, char *argv[]);
typedef PuRes (*PuCleanupF) (void);

typedef void (*PuEventLoopF) (void);
typedef void (*PuProcessEventF) (void);

typedef void (*PuAddTimeOutF) (int, PuTimeOut, void *);

typedef void (*PuAddExtInputF) (int, PuExtInput, void *, PuFreeCB);
typedef void (*PuRemoveExtInputF) (int, PuExtInput, void *);

typedef void (*PuSwitchFullscreenF) (BOOL);
typedef void (*PuSetDrawSizeF) (int, int);
typedef void (*PuSetTextFieldF) (PuTextFieldChoice, char *);
typedef void (*PuSwitchTextFieldF) (PuTextFieldChoice, BOOL);

typedef PuMenubar (*PuCreateMenubarF) (void);
typedef PuMenu (*PuCreatePulldownF) (PuMenubar, char *, char);
typedef PuMenu (*PuCreatePopupF) (char *);
typedef PuMenu (*PuCreatePullrightF) (PuMenu, char *, char);
typedef PuMenuEntry (*PuCreateMenuEntryF) (PuMenu, char *,
    char, char *, char *,
    PuMenuCB, void *, PuFreeCB);
typedef void (*PuSwitchMenubarF) (PuMenubar, BOOL);
typedef void (*PuSwitchPopupF) (PuMenu, BOOL);

typedef PuCmdField (*PuCreateCmdFieldF) (char *,
    PuCmdFieldCB, void *, PuFreeCB);
typedef void (*PuSwitchCmdFieldsF) (BOOL);

typedef PuValuatorBox (*PuCreateValuatorBoxF) (char *);
typedef PuValuator (*PuCreateValuatorF) (PuValuatorBox, char *,
    float, float, int, float,
    PuValuatorCB, void *, PuFreeCB);
typedef void (*PuSwitchValuatorBoxF) (PuValuatorBox, BOOL);

typedef void (*PuSetBusyCursorF) (BOOL);

typedef void (*PuShowMessageF) (PuMessageType, char *, char *);
typedef void (*PuShowHelpF) (PuGizmo, char *, char *);
typedef void (*PuBeepF) (void);

typedef PuTextWindow (*PuCreateTextWindowF) (char *);
typedef void (*PuWriteStrF) (PuTextWindow, char *);

typedef PuGizmo (*PuCreateFileSelectionBoxF) (char *,
    char *, char *, PuFileAccess);
typedef PuGizmo (*PuCreateDialogF) (char *, int, int);
typedef PuGizmo (*PuCreateLabelF) (PuGizmo, char *);
typedef PuGizmo (*PuCreateButtonF) (PuGizmo, char *);
typedef PuGizmo (*PuCreateColorFieldF) (PuGizmo, char *);
typedef PuGizmo (*PuCreateSliderF) (PuGizmo, char *,
    float, float, int, float);
typedef PuGizmo (*PuCreateTextFieldF) (PuGizmo, char *, char *);
typedef PuGizmo (*PuCreateTextF) (PuGizmo, char *, char *);
typedef PuGizmo (*PuCreateFileViewerF) (PuGizmo, char *, BOOL);
typedef PuGizmo (*PuCreateListF) (PuGizmo, char *);
typedef void (*PuAddListEntryF) (PuGizmo, char *, BOOL);
typedef void (*PuRemoveListEntriesF) (PuGizmo, int, int);
typedef PuGizmo (*PuCreateRadioBoxF) (PuGizmo, char *);
typedef PuGizmo (*PuCreateCheckBoxF) (PuGizmo, char *);
typedef void (*PuAddToggleF) (PuGizmo, char *, BOOL);
typedef void (*PuSetBoolF) (PuGizmo, PuBoolChoice, BOOL);
typedef void (*PuSetIntF) (PuGizmo, PuIntChoice, int);
typedef void (*PuSetStrF) (PuGizmo, PuStrChoice, char *);
typedef void (*PuSetColorF) (PuGizmo, float, float, float);
typedef void (*PuSetConstraintsF) (PuGizmo, PuConstraints);
typedef void (*PuSwitchGizmoF) (PuGizmo, BOOL);
typedef void (*PuDestroyGizmoF) (PuGizmo);
typedef void (*PuAddGizmoCBF) (PuGizmo, PuGizmoCBType,
    PuGizmoCB, void *, PuFreeCB);

typedef struct {
  PuInitF init;
  PuCleanupF cleanup;

  PuEventLoopF eventLoop;
  PuProcessEventF processEvent;

  PuAddTimeOutF addTimeOut;

  PuAddExtInputF addExtInput;
  PuRemoveExtInputF removeExtInput;

  PuSwitchFullscreenF switchFullscreen;
  PuSetDrawSizeF setDrawSize;
  PuSetTextFieldF setTextField;
  PuSwitchTextFieldF switchTextField;

  PuCreateMenubarF createMenubar;
  PuCreatePulldownF createPulldown;
  PuCreatePopupF createPopup;
  PuCreatePullrightF createPullright;
  PuCreateMenuEntryF createMenuEntry;
  PuSwitchMenubarF switchMenubar;
  PuSwitchPopupF switchPopup;

  PuCreateCmdFieldF createCmdField;
  PuSwitchCmdFieldsF switchCmdFields;

  PuCreateValuatorBoxF createValuatorBox;
  PuCreateValuatorF createValuator;
  PuSwitchValuatorBoxF switchValuatorBox;

  PuSetBusyCursorF setBusyCursor;

  PuShowMessageF showMessage;
  PuShowHelpF showHelp;
  PuBeepF beep;

  PuCreateTextWindowF createTextWindow;
  PuWriteStrF writeStr;

  PuCreateFileSelectionBoxF createFileSelectionBox;
  PuCreateDialogF createDialog;
  PuCreateLabelF createLabel;
  PuCreateButtonF createButton;
  PuCreateColorFieldF createColorField;
  PuCreateSliderF createSlider;
  PuCreateTextFieldF createTextField;
  PuCreateTextF createText;
  PuCreateFileViewerF createFileViewer;
  PuCreateListF createList;
  PuAddListEntryF addListEntry;
  PuRemoveListEntriesF removeListEntries;
  PuCreateRadioBoxF createRadioBox;
  PuCreateCheckBoxF createCheckBox;
  PuAddToggleF addToggle;
  PuSetBoolF setBool;
  PuSetIntF setInt;
  PuSetStrF setStr;
  PuSetColorF setColor;
  PuSetConstraintsF setConstraints;
  PuSwitchGizmoF switchGizmo;
  PuDestroyGizmoF destroyGizmo;
  PuAddGizmoCBF addGizmoCB;
} PuDev;

extern PuDev *CurrPuDevP;
