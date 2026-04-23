/*
************************************************************************
*
*   ExUserInterf.c - setup of user interface
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
*   Date of last modification : 95/03/07
*   Pathname of SCCS file     : /sgiext/molmol/src/cmddial/SCCS/s.ExUserInterf.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <cmd_dial.h>

#include <par_names.h>
#include <par_hand.h>
#include <arg.h>
#include <ui_switch.h>

#define SWITCH_NO 6

static char *ParNames[] = {
  PN_SWITCH_VALUATOR,
  PN_SWITCH_LOG_WIN,
  PN_SWITCH_BUTTONS,
  PN_SWITCH_CMD_LINE,
  PN_SWITCH_STATUS,
  PN_SWITCH_MENU_BAR
};

ErrCode
ExUserInterface(char *cmd)
{
  ArgDescr arg;
  EnumEntryDescr enumEntry[SWITCH_NO];
  ErrCode errCode;
  int i;

  arg.type = AT_MULT_ENUM;

  ArgInit(&arg, 1);

  arg.prompt = "User Interface";
  arg.u.enumD.entryP = enumEntry;
  arg.u.enumD.n = SWITCH_NO;

  enumEntry[0].str = "Valuator Box";
  enumEntry[1].str = "Log Window";
  enumEntry[2].str = "Buttons";
  enumEntry[3].str = "Command Line";
  enumEntry[4].str = "Status Line";
  enumEntry[5].str = "Menubar";

  for (i = 0; i < SWITCH_NO; i++)
    enumEntry[i].onOff = ParGetIntVal(ParNames[i]);

  errCode = ArgGet(&arg, 1);
  if (errCode != EC_OK) {
    ArgCleanup(&arg, 1);
    return errCode;
  }

  for (i = 0; i < SWITCH_NO; i++)
    ParSetIntVal(ParNames[i], enumEntry[i].onOff);

  ArgCleanup(&arg, 1);

  UiSwitch();

  return EC_OK;
}
