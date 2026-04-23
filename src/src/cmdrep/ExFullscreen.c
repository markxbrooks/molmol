/*
************************************************************************
*
*   ExFullscreen.c - Fullscreen command
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
*   Date of last modification : 94/06/02
*   Pathname of SCCS file     : /sgiext/molmol/src/cmdrep/SCCS/s.ExFullscreen.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <cmd_rep.h>

#include <pu.h>
#include <arg.h>

ErrCode
ExFullscreen(char *cmd)
{
  ArgDescr arg;
  EnumEntryDescr enumEntry[2];
  ErrCode errCode;

  arg.type = AT_ENUM;

  ArgInit(&arg, 1);

  arg.prompt = "Fullscreen Mode";
  arg.u.enumD.entryP = enumEntry;
  arg.u.enumD.n = 2;

  enumEntry[0].str = "off";
  enumEntry[1].str = "on";

  enumEntry[0].onOff = FALSE;
  enumEntry[1].onOff = TRUE;
  arg.v.intVal = 1;

  errCode = ArgGet(&arg, 1);
  if (errCode != EC_OK) {
    ArgCleanup(&arg, 1);
    return errCode;
  }

  PuSwitchFullscreen(arg.v.intVal == 1);

  ArgCleanup(&arg, 1);

  return EC_OK;
}
