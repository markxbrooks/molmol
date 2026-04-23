/*
************************************************************************
*
*   ExHelpAprop.c - HelpApropos command
*
*   Copyright (c) 1995
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
*   Date of last modification : 95/04/11
*   Pathname of SCCS file     : /sgiext/molmol/src/cmdmisc/SCCS/s.ExHelpAprop.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <cmd_misc.h>

#include <arg.h>
#include <cip.h>

#define ARG_NUM 1

ErrCode
ExHelpApropos(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;

  arg[0].type = AT_STR;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Search String";

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  CipHelpApropos(DStrToStr(arg[0].v.strVal));

  ArgCleanup(arg, ARG_NUM);

  return EC_OK;
}
