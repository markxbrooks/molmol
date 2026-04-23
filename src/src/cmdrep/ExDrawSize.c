/*
************************************************************************
*
*   ExDrawSize.c - DrawSize command
*
*   Copyright (c) 1994-96
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
*   Date of last modification : 96/12/13
*   Pathname of SCCS file     : /local/home/kor/molmol/src/cmdrep/SCCS/s.ExDrawSize.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <cmd_rep.h>

#include <pu.h>
#include <arg.h>
#include <par_names.h>
#include <par_hand.h>

#define ARG_NUM 3

ErrCode
ExDrawSize(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  EnumEntryDescr enumEntry[2];
  ErrCode errCode;
  int width, height;

  arg[0].type = AT_ENUM;
  arg[1].type = AT_INT;
  arg[2].type = AT_INT;

  ArgInit(arg, ARG_NUM);

  enumEntry[0].str = "set";
  enumEntry[0].onOff = TRUE;
  enumEntry[1].str = "restore";
  enumEntry[1].onOff = FALSE;

  arg[0].prompt = "Operation";
  arg[0].u.enumD.entryP = enumEntry;
  arg[0].u.enumD.n = 2;
  arg[0].v.intVal = 0;

  arg[1].prompt = "Width";
  arg[1].optional = TRUE;
  if (ParDefined(PN_CURR_WIDTH))
    arg[1].v.intVal = ParGetIntVal(PN_CURR_WIDTH);
  else
    arg[1].v.intVal = 1000;

  arg[2].prompt = "Height";
  arg[2].optional = TRUE;
  if (ParDefined(PN_CURR_HEIGHT))
    arg[2].v.intVal = ParGetIntVal(PN_CURR_HEIGHT);
  else
    arg[2].v.intVal = 750;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  if (arg[0].v.intVal == 0) {
    width = arg[1].v.intVal;
    height = arg[2].v.intVal;
    PuSetDrawSize(width, height);
    ParSetIntVal(PN_CURR_WIDTH, width);
    ParSetIntVal(PN_CURR_HEIGHT, height);
    ParSetIntVal(PN_SAVE_WIDTH, width);
    ParSetIntVal(PN_SAVE_HEIGHT, height);
  } else if (ParDefined(PN_SAVE_WIDTH) && ParDefined(PN_SAVE_HEIGHT)) {
    width = ParGetIntVal(PN_SAVE_WIDTH);
    height = ParGetIntVal(PN_SAVE_HEIGHT);
    PuSetDrawSize(width, height);
  }

  ArgCleanup(arg, ARG_NUM);

  return EC_OK;
}
