/*
************************************************************************
*
*   WinDev.c - Windows device
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
*   Pathname of SCCS file     : /local/home/kor/molmol/src/win/SCCS/s.WinDev.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <pu_dev.h>

#include "win_init.h"
#include "win_event.h"
#include "win_ext_in.h"
#include "win_main_w.h"
#include "win_menu.h"
#include "win_msg.h"
#include "win_help.h"
#include "win_text_w.h"
#include "win_dial.h"

static PuDev PuWinDev = {
  PuWinInit,
  PuWinCleanup,
  PuWinEventLoop,
  PuWinProcessEvent,
  PuWinAddTimeOut,
  PuWinAddExtInput,
  PuWinRemoveExtInput,
  PuWinSwitchFullscreen,
  PuWinSetDrawSize,
  PuWinSetTextField,
  PuWinSwitchTextField,
  PuWinCreateMenubar,
  PuWinCreatePulldown,
  PuWinCreatePopup,
  PuWinCreatePullright,
  PuWinCreateMenuEntry,
  PuWinSwitchMenubar,
  PuWinSwitchPopup,
  PuWinCreateCmdField,
  PuWinSwitchCmdFields,
  PuWinCreateValuatorBox,
  PuWinCreateValuator,
  PuWinSwitchValuatorBox,
  PuWinSetBusyCursor,
  PuWinShowMessage,
  PuWinShowHelp,
  PuWinBeep,
  PuWinCreateTextWindow,
  PuWinWriteStr,
  PuWinCreateFileSelectionBox,
  PuWinCreateDialog,
  PuWinCreateLabel,
  PuWinCreateButton,
  PuWinCreateColorField,
  PuWinCreateSlider,
  PuWinCreateTextField,
  PuWinCreateText,
  PuWinCreateFileViewer,
  PuWinCreateList,
  PuWinAddListEntry,
  PuWinRemoveListEntries,
  PuWinCreateRadioBox,
  PuWinCreateCheckBox,
  PuWinAddToggle,
  PuWinSetBool,
  PuWinSetInt,
  PuWinSetStr,
  PuWinSetColor,
  PuWinSetConstraints,
  PuWinSwitchGizmo,
  PuWinDestroyGizmo,
  PuWinAddGizmoCB,
};

void
PuWinSetDev(void)
{
  CurrPuDevP = &PuWinDev;
}
