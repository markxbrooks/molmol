/*
************************************************************************
*
*   UiSwitch.c - switch main window parts and log window
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
*   Date of last modification : 94/08/17
*   Pathname of SCCS file     : /sgiext/molmol/src/ui/SCCS/s.UiSwitch.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <ui_switch.h>

#include <stdio.h>

#include <pu.h>
#include <par_names.h>
#include <par_hand.h>
#include <ui_menu.h>
#include <ui_valuator.h>
#include <history.h>

void
UiSwitch(void)
{
  BOOL onOff, cmdLine;

  if (ParDefined(PN_SWITCH_VALUATOR)) {
    onOff = ParGetIntVal(PN_SWITCH_VALUATOR);
  } else {
    onOff = TRUE;
    ParSetIntVal(PN_SWITCH_VALUATOR, onOff);
  }
  UiValuatorSwitch(onOff);

  if (ParDefined(PN_SWITCH_LOG_WIN)) {
    onOff = ParGetIntVal(PN_SWITCH_LOG_WIN);
  } else {
    onOff = TRUE;
    ParSetIntVal(PN_SWITCH_LOG_WIN, onOff);
  }
  HistoryLogWin(onOff);

  if (ParDefined(PN_SWITCH_BUTTONS)) {
    onOff = ParGetIntVal(PN_SWITCH_BUTTONS);
  } else {
    onOff = TRUE;
    ParSetIntVal(PN_SWITCH_BUTTONS, onOff);
  }
  PuSwitchCmdFields(onOff);

  if (ParDefined(PN_SWITCH_CMD_LINE)) {
    onOff = ParGetIntVal(PN_SWITCH_CMD_LINE);
  } else {
    onOff = TRUE;
    ParSetIntVal(PN_SWITCH_CMD_LINE, onOff);
  }
  PuSwitchTextField(PU_TF_CMD, onOff);
  cmdLine = onOff;

  if (ParDefined(PN_SWITCH_STATUS)) {
    onOff = ParGetIntVal(PN_SWITCH_STATUS);
  } else {
    onOff = TRUE;
    ParSetIntVal(PN_SWITCH_STATUS, onOff);
  }
  PuSwitchTextField(PU_TF_STATUS, onOff);

  /* only allow switching off menubar if command line is switched on,
     otherwise there's no possibility to enter commands anymore! */
  if (ParDefined(PN_SWITCH_MENU_BAR) && cmdLine) {
    onOff = ParGetIntVal(PN_SWITCH_MENU_BAR);
  } else {
    onOff = TRUE;
    ParSetIntVal(PN_SWITCH_MENU_BAR, onOff);
  }
  UiMenuSwitchBar(onOff);

  if (ParDefined(PN_SWITCH_POPUP)) {
    onOff = ParGetIntVal(PN_SWITCH_POPUP);
  } else {
    onOff = TRUE;
    ParSetIntVal(PN_SWITCH_POPUP, onOff);
  }
  UiMenuSwitchPopup(onOff);
}
