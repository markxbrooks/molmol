/*
************************************************************************
*
*   TTYDev.c - TTY device
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
*   Date of last modification : 95/12/14
*   Pathname of SCCS file     : /sgiext/molmol/src/tty/SCCS/s.TTYDev.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <pu_dev.h>

#include "tty.h"

static PuDev PuTTYDev = {
  PuTTYInit,
  PuTTYCleanup,
  PuTTYEventLoop,
  PuTTYProcessEvent,
  PuTTYAddTimeOut,
  PuTTYAddExtInput,
  PuTTYRemoveExtInput,
  PuTTYSwitchFullscreen,
  PuTTYSetDrawSize,
  PuTTYSetTextField,
  PuTTYSwitchTextField,
  PuTTYCreateMenubar,
  PuTTYCreatePulldown,
  PuTTYCreatePopup,
  PuTTYCreatePullright,
  PuTTYCreateMenuEntry,
  PuTTYSwitchMenubar,
  PuTTYSwitchPopup,
  PuTTYCreateCmdField,
  PuTTYSwitchCmdFields,
  PuTTYCreateValuatorBox,
  PuTTYCreateValuator,
  PuTTYSwitchValuatorBox,
  PuTTYSetBusyCursor,
  PuTTYShowMessage,
  PuTTYShowHelp,
  PuTTYBeep,
  PuTTYCreateTextWindow,
  PuTTYWriteStr,
  PuTTYCreateFileSelectionBox,
  PuTTYCreateDialog,
  PuTTYCreateLabel,
  PuTTYCreateButton,
  PuTTYCreateColorField,
  PuTTYCreateSlider,
  PuTTYCreateTextField,
  PuTTYCreateText,
  PuTTYCreateFileViewer,
  PuTTYCreateList,
  PuTTYAddListEntry,
  PuTTYRemoveListEntries,
  PuTTYCreateRadioBox,
  PuTTYCreateCheckBox,
  PuTTYAddToggle,
  PuTTYSetBool,
  PuTTYSetInt,
  PuTTYSetStr,
  PuTTYSetColor,
  PuTTYSetConstraints,
  PuTTYSwitchGizmo,
  PuTTYDestroyGizmo,
  PuTTYAddGizmoCB
};

void
PuTTYSetDev(void)
{
  CurrPuDevP = &PuTTYDev;
}
