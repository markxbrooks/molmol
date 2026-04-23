/*
************************************************************************
*
*   ExMovePrim.c - MovePrim command
*
*   Copyright (c) 1994-95
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdprim/SCCS/s.ExMovePrim.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <cmd_prim.h>

#include <arg.h>
#include <prim_hand.h>
#include <graph_draw.h>

#define ARG_NUM 3

static void
moveCircCylind(PrimObjP primP, void *clientData)
{
  float *v = clientData;
  Vec3 x;

  PrimGetPos(primP, x);
  Vec3Add(x, v);
  PrimSetPos(primP, x);
}

static void
moveText(PrimObjP primP, void *clientData)
{
  float *v = clientData;
  Vec3 dx;

  PrimGetVec(primP, dx);
  Vec3Add(dx, v);
  PrimSetVec(primP, dx);
}

ErrCode
ExMovePrim(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  Vec3 moveV;
  PropRefP refP;

  arg[0].type = AT_DOUBLE;
  arg[1].type = AT_DOUBLE;
  arg[2].type = AT_DOUBLE;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Move x";
  arg[0].v.doubleVal = 0.0;

  arg[1].prompt = "Move y";
  arg[1].v.doubleVal = 0.0;

  arg[2].prompt = "Move z";
  arg[2].v.doubleVal = 0.0;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  moveV[0] = (float) arg[0].v.doubleVal;
  moveV[1] = (float) arg[1].v.doubleVal;
  moveV[2] = (float) arg[2].v.doubleVal;

  ArgCleanup(arg, ARG_NUM);

  refP = PropGetRef(PROP_SELECTED, FALSE);
  PrimApply(PT_CIRCLE, refP, moveCircCylind, moveV);
  PrimApply(PT_CYLINDER, refP, moveCircCylind, moveV);
  PrimApply(PT_TEXT, refP, moveText, moveV);

  GraphMolChanged(PROP_SELECTED);
  GraphRedrawNeeded();

  return EC_OK;
}
