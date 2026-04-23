/*
************************************************************************
*
*   ExSystem.c - System command
*
*   Copyright (c) 1996
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
*   Date of last modification : 96/04/18
*   Pathname of SCCS file     : /sgiext/molmol/src/cmdmisc/SCCS/s.ExSystem.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <cmd_misc.h>

#include <os_system.h>
#include <arg.h>

#define ARG_NUM 1

ErrCode
ExSystem(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  BOOL ok;

  arg[0].type = AT_STR;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "System Command";

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  ok = OsSystem(DStrToStr(arg[0].v.strVal));

  ArgCleanup(arg, ARG_NUM);

  if (ok) {
    return EC_OK;
  } else {
    CipSetError("error in system command");
    return EC_ERROR;
  }
}
