/*
************************************************************************
*
*   ExQuit.c - Quit command
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
*   Date of last modification : 01/05/27
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdmisc/SCCS/s.ExQuit.c
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include <cmd_misc.h>

#include <stdlib.h>

#include <io.h>
#include <arg.h>
#include <par_names.h>
#include <par_hand.h>

#define ENUM_SIZE 2

ErrCode
ExQuit(char *cmd)
{
  int state;
  ArgDescr arg;
  EnumEntryDescr enumEntry[ENUM_SIZE];
  ErrCode errCode;

  if (ParDefined(PN_SAVE_STATE))
    state = ParGetIntVal(PN_SAVE_STATE);
  else
    state = 1;

  arg.type = AT_ENUM;

  ArgInit(&arg, 1);

  enumEntry[0].str = "yes";
  enumEntry[0].onOff = (state == 1);
  enumEntry[1].str = "no";
  enumEntry[1].onOff = (state == 0);

  arg.prompt = "Save State?";
  arg.u.enumD.entryP = enumEntry;
  arg.u.enumD.n = ENUM_SIZE;
  arg.v.intVal = 1 - state;

  errCode = ArgGet(&arg, 1);
  if (errCode != EC_OK) {
    ArgCleanup(&arg, 1);
    return errCode;
  }

  state = 1 - arg.v.intVal;
  ArgCleanup(&arg, 1);

  ParSetIntVal(PN_SAVE_STATE, state);

  (void) IOCleanup();
  exit(0);

  /* not reached, avoid compiler warning */
  return EC_OK;
}
