/*
************************************************************************
*
*   ExDrawobj.c - AddDrawobj and MoveDrawobj commands
*
*   Copyright (c) 1995-97
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdprim/SCCS/s.ExDrawobj.c
*   SCCS identification       : 1.6
*
************************************************************************
*/

#include <cmd_prim.h>

#include <stdio.h>
#include <stdlib.h>

#include <arg.h>
#include <data_hand.h>
#include <data_sel.h>
#include <prim_hand.h>
#include <graph_draw.h>

#define ARG_NUM 2
#define TYPE_NUM 3

typedef struct {
  int ind;
  Vec3 v;
} MoveData;

static int CurrType = 0, CurrDim = 1;

static void
countAtoms(DhAtomP atomP, void *clientData)
{
  *(int *) clientData += 1;
}

ErrCode
ExAddDrawobj(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  EnumEntryDescr enum1Entry[TYPE_NUM];
  EnumEntryDescr enum2Entry[2];
  ErrCode errCode;
  int molNo, molI, atomNo;
  DhMolP *molPA;
  PropRefP refP;
  PrimObjP primP;
  Vec3 x, dx;

  arg[0].type = AT_ENUM;
  arg[1].type = AT_ENUM;

  ArgInit(arg, ARG_NUM);

  enum1Entry[0].str = "line";
  enum1Entry[0].onOff = FALSE;
  enum1Entry[1].str = "rect";
  enum1Entry[1].onOff = FALSE;
  enum1Entry[2].str = "circle";
  enum1Entry[2].onOff = FALSE;

  enum1Entry[CurrType].onOff = TRUE;

  arg[0].prompt = "Type";
  arg[0].u.enumD.entryP = enum1Entry;
  arg[0].u.enumD.n = TYPE_NUM;
  arg[0].v.intVal = CurrType;

  enum2Entry[0].str = "2D";
  enum2Entry[0].onOff = FALSE;
  enum2Entry[1].str = "3D";
  enum2Entry[1].onOff = FALSE;

  enum2Entry[CurrDim].onOff = TRUE;

  arg[1].prompt = "Dim";
  arg[1].u.enumD.entryP = enum2Entry;
  arg[1].u.enumD.n = 2;
  arg[1].v.intVal = CurrDim;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  CurrType = arg[0].v.intVal;
  CurrDim = arg[1].v.intVal;

  ArgCleanup(arg, ARG_NUM);

  if (CurrDim == 0) {
    primP = PrimNew(PT_DRAWOBJ, NULL);
    PrimSetDrawobjType(primP, CurrType);

    Vec3Zero(x);
    dx[0] = -0.2f;
    dx[1] = -0.9f;
    dx[2] = 0.0f;
    PrimSetPoint(primP, 0, x, dx);
    dx[0] = 0.2f;
    dx[1] = -0.7f;
    dx[2] = 0.0f;
    PrimSetPoint(primP, 1, x, dx);

    PrimSetProp(PropGetRef("drawobj", TRUE), primP, TRUE);
  } else {
    molNo = SelMolGet(NULL, 0);
    if (molNo == 0) {
      CipSetError("no molecule selected");
      return EC_WARNING;
    }

    molPA = malloc(molNo * sizeof(*molPA));
    (void) SelMolGet(molPA, molNo);

    refP = PropGetRef(PROP_SELECTED, FALSE);

    for (molI = 0; molI < molNo; molI++) {
      atomNo = 0;
      DhMolApplyAtom(refP, molPA[molI], countAtoms, &atomNo);
      if (atomNo == 0)
	continue;

      primP = PrimNew(PT_DRAWOBJ, molPA[molI]);
      PrimSetDrawobjType(primP, CurrType);

      DhCalcCenter(x, molPA[molI]);
      Vec3Zero(dx);
      PrimSetPoint(primP, 0, x, dx);
      dx[0] = 0.4f;
      dx[1] = 0.2f;
      PrimSetPoint(primP, 1, x, dx);

      PrimSetProp(PropGetRef("drawobj", TRUE), primP, TRUE);
    }

    free(molPA);
  }
  
  GraphRedrawNeeded();

  return EC_OK;
}

static void
moveObj(PrimObjP primP, void *clientData)
{
  MoveData *dataP = clientData;
  Vec3 x, dx;

  PrimGetPoint(primP, dataP->ind, x, dx);
  Vec3Add(dx, dataP->v);
  PrimSetPoint(primP, dataP->ind, x, dx);
}

#undef ARG_NUM
#define ARG_NUM 4

ErrCode
ExMoveDrawobj(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  MoveData data;

  arg[0].type = AT_INT;
  arg[1].type = AT_DOUBLE;
  arg[2].type = AT_DOUBLE;
  arg[3].type = AT_DOUBLE;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Point Index";
  arg[0].v.intVal = 0;

  arg[1].prompt = "Move x";
  arg[1].v.doubleVal = 0.0;

  arg[2].prompt = "Move y";
  arg[2].v.doubleVal = 0.0;

  arg[3].prompt = "Move z";
  arg[3].v.doubleVal = 0.0;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  data.ind = arg[0].v.intVal;
  data.v[0] = (float) arg[1].v.doubleVal;
  data.v[1] = (float) arg[2].v.doubleVal;
  data.v[2] = (float) arg[3].v.doubleVal;

  ArgCleanup(arg, ARG_NUM);

  PrimApply(PT_DRAWOBJ, PropGetRef(PROP_SELECTED, FALSE), moveObj, &data);

  GraphRedrawNeeded();

  return EC_OK;
}
