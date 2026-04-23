/*
************************************************************************
*
*   ExClipPlanes.c - Clip* commands
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdrep/SCCS/s.ExClipPlanes.c
*   SCCS identification       : 1.5
*
************************************************************************
*/

#include <cmd_rep.h>

#include <string.h>

#include <sg.h>
#include <arg.h>
#include <par_names.h>
#include <par_hand.h>
#include <graph_draw.h>

#define ARG_NUM 1

ErrCode
ExClip(char *cmd)
{
  BOOL isPos = (strcmp(cmd, "ClipPos") == 0);
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  float nearPlane, farPlane, pos, size;

  arg[0].type = AT_DOUBLE;

  ArgInit(arg, ARG_NUM);

  nearPlane = (float) ParGetDoubleVal(PN_NEAR_PLANE);
  farPlane = (float) ParGetDoubleVal(PN_FAR_PLANE);
  pos = 0.5f * (nearPlane + farPlane);
  size = farPlane - nearPlane;

  if (isPos) {
    arg[0].prompt = "Clip Position";
    arg[0].v.doubleVal = pos;
  } else {
    arg[0].prompt = "Clip Size";
    arg[0].v.doubleVal = size;
  }

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  if (isPos) {
    pos = (float) arg[0].v.doubleVal;
    if (pos < 0.1f)
      pos = 0.1f;
  } else {
    size = (float) arg[0].v.doubleVal;
    if (size < 0.1f)
      size = 0.1f;
  }

  ArgCleanup(arg, ARG_NUM);

  nearPlane = pos - 0.5f * size;
  farPlane = pos + 0.5f * size;

  ParSetDoubleVal(PN_NEAR_PLANE, nearPlane);
  ParSetDoubleVal(PN_FAR_PLANE, farPlane);
  SgSetNearPlane(nearPlane, FALSE);
  SgSetFarPlane(farPlane, FALSE);

  SgUpdateView();
  GraphRedrawNeeded();

  return EC_OK;
}
