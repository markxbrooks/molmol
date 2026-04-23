/*
************************************************************************
*
*   MotifDev.c - Motif device
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
*   Pathname of SCCS file     : /sgiext/molmol/src/motif/SCCS/s.MotifDev.c
*   SCCS identification       : 1.6
*
************************************************************************
*/

#include <pu_dev.h>

#include "motif_init.h"
#include "motif_event.h"
#include "motif_ext_in.h"
#include "motif_main_w.h"
#include "motif_menu.h"
#include "motif_val.h"
#include "motif_busy.h"
#include "motif_msg.h"
#include "motif_help.h"
#include "motif_text_w.h"
#include "motif_dial.h"

static PuDev PuMotifDev = {
  PuMotifInit,
  PuMotifCleanup,
  PuMotifEventLoop,
  PuMotifProcessEvent,
  PuMotifAddTimeOut,
  PuMotifAddExtInput,
  PuMotifRemoveExtInput,
  PuMotifSwitchFullscreen,
  PuMotifSetDrawSize,
  PuMotifSetTextField,
  PuMotifSwitchTextField,
  PuMotifCreateMenubar,
  PuMotifCreatePulldown,
  PuMotifCreatePopup,
  PuMotifCreatePullright,
  PuMotifCreateMenuEntry,
  PuMotifSwitchMenubar,
  PuMotifSwitchPopup,
  PuMotifCreateCmdField,
  PuMotifSwitchCmdFields,
  PuMotifCreateValuatorBox,
  PuMotifCreateValuator,
  PuMotifSwitchValuatorBox,
  PuMotifSetBusyCursor,
  PuMotifShowMessage,
  PuMotifShowHelp,
  PuMotifBeep,
  PuMotifCreateTextWindow,
  PuMotifWriteStr,
  PuMotifCreateFileSelectionBox,
  PuMotifCreateDialog,
  PuMotifCreateLabel,
  PuMotifCreateButton,
  PuMotifCreateColorField,
  PuMotifCreateSlider,
  PuMotifCreateTextField,
  PuMotifCreateText,
  PuMotifCreateFileViewer,
  PuMotifCreateList,
  PuMotifAddListEntry,
  PuMotifRemoveListEntries,
  PuMotifCreateRadioBox,
  PuMotifCreateCheckBox,
  PuMotifAddToggle,
  PuMotifSetBool,
  PuMotifSetInt,
  PuMotifSetStr,
  PuMotifSetColor,
  PuMotifSetConstraints,
  PuMotifSwitchGizmo,
  PuMotifDestroyGizmo,
  PuMotifAddGizmoCB,
};

void
PuMotifSetDev(void)
{
  CurrPuDevP = &PuMotifDev;
}
