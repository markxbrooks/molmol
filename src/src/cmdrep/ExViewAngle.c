/*
************************************************************************
*
*   ExViewAngle.c - ViewAngle command
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdrep/SCCS/s.ExViewAngle.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <cmd_rep.h>

#include <sg.h>
#include <arg.h>
#include <par_names.h>
#include <par_hand.h>
#include <graph_draw.h>

#define ARG_NUM 1

ErrCode
ExViewAngle(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  float ang;

  arg[0].type = AT_DOUBLE;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "View Angle";
  arg[0].v.doubleVal = ParGetDoubleVal(PN_VIEW_ANGLE);

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  ang = (float) arg[0].v.doubleVal;
  ArgCleanup(arg, ARG_NUM);

  ParSetDoubleVal(PN_VIEW_ANGLE, ang);
  SgSetViewAngle(ang);

  SgUpdateView();
  GraphRedrawNeeded();

  return EC_OK;
}
