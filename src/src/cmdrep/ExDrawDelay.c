/*
************************************************************************
*
*   ExDrawDelay.c - DrawDelay command
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
*   Pathname of SCCS file     : /sgiext/molmol/src/cmdrep/SCCS/s.ExDrawDelay.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <cmd_rep.h>

#include <arg.h>
#include <par_names.h>
#include <par_hand.h>
#include <graph_draw.h>

#define ARG_NUM 1

ErrCode
ExDrawDelay(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  int delay;

  arg[0].type = AT_INT;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Drawing Delay";
  arg[0].v.intVal = ParGetIntVal(PN_DRAW_DELAY);

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  delay = arg[0].v.intVal;
  ArgCleanup(arg, ARG_NUM);

  ParSetIntVal(PN_DRAW_DELAY, delay);
  GraphDelaySet(delay);

  return EC_OK;
}
