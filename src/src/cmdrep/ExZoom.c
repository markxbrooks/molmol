/*
************************************************************************
*
*   ExZoom.c - ZoomIn and ZoomOut commands
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
*   Date of last modification : 01/05/27
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdrep/SCCS/s.ExZoom.c
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include <cmd_rep.h>

#include <string.h>

#include <arg.h>
#include <par_names.h>
#include <par_hand.h>
#include <graph_draw.h>

#define ARG_NUM 1

ErrCode
ExZoom(char *cmd)
{
  BOOL isAbs = (strcmp(cmd, "ZoomAbs") == 0);
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  float fact;

  arg[0].type = AT_DOUBLE;

  ArgInit(arg, ARG_NUM);

  fact = (float) ParGetDoubleVal(PN_ZOOM_FACT);

  arg[0].prompt = "Factor";
  if (isAbs)
    arg[0].v.doubleVal = fact;
  else
    arg[0].v.doubleVal = 1.0;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  if (isAbs)
    fact = (float) arg[0].v.doubleVal;
  else
    fact *= (float) arg[0].v.doubleVal;

  ArgCleanup(arg, ARG_NUM);

  ParSetDoubleVal(PN_ZOOM_FACT, fact);
  GraphZoomSet(fact);
  GraphRedrawNeeded();

  return EC_OK;
}
