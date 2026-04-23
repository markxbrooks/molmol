/*
************************************************************************
*
*   ExLenPrim.c - LengthPrim command
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdprim/SCCS/s.ExLenPrim.c
*   SCCS identification       : 1.5
*
************************************************************************
*/

#include <cmd_prim.h>

#include <stdio.h>

#include <arg.h>
#include <prim_hand.h>
#include <graph_draw.h>

typedef struct {
  int primNo;
  float startPar, endPar;
} LengthData;

static void
getPrimLength(PrimObjP primP, void *clientData)
{
  LengthData *dataP = clientData;
  float startPar, endPar;

  if (PrimGetType(primP) == PT_CYLINDER) {
    startPar = 0.0f;
    endPar = 1.0f;
  } else {
    /* internally the values start with 0.0, for the user it
       is more convenient to start at 1.0 */
    startPar = RibbonGetEndPar(primP, REC_START) + 1.0f;
    endPar = RibbonGetEndPar(primP, REC_END) + 1.0f;
  }

  if (dataP->primNo == 0) {
    dataP->startPar = startPar;
    dataP->endPar = endPar;
  } else {
    if (dataP->startPar != startPar)
      dataP->startPar = 0.0f;
    if (dataP->endPar != endPar)
      dataP->endPar = 0.0f;
  }

  dataP->primNo++;
}

static void
setRibbonLength(PrimObjP primP, void *clientData)
{
  LengthData *dataP = clientData;

  if (dataP->startPar != 0.0f)
    RibbonSetEndPar(primP, REC_START, dataP->startPar - 1.0f);
  if (dataP->endPar != 0.0f)
    RibbonSetEndPar(primP, REC_END, dataP->endPar - 1.0f);
}

static void
setCylinderLength(PrimObjP primP, void *clientData)
{
  LengthData *dataP = clientData;
  Vec3 x, v;

  PrimGetPos(primP, x);
  PrimGetVec(primP, v);

  if (dataP->startPar != 0.0f && dataP->endPar != 0.0f) {
    Vec3ScaleAdd(x, dataP->startPar, v);
    Vec3Scale(v, dataP->endPar - dataP->startPar);
  } else if (dataP->startPar != 0.0f) {
    Vec3ScaleAdd(x, dataP->startPar, v);
    Vec3Scale(v, 1.0f - dataP->startPar);
  } else if (dataP->endPar != 0.0f) {
    Vec3Scale(v, dataP->endPar);
  }

  PrimSetPos(primP, x);
  PrimSetVec(primP, v);
}

#define ARG_NUM 2

ErrCode
ExLengthPrim(char *cmd)
{
  PropRefP refP;
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  LengthData data;

  refP = PropGetRef(PROP_SELECTED, FALSE);

  data.primNo = 0;
  data.startPar = 0.0f;
  data.endPar = 0.0f;
  PrimApply(PT_CYLINDER, refP, getPrimLength, &data);
  PrimApply(PT_RIBBON, refP, getPrimLength, &data);

  arg[0].type = AT_DOUBLE;
  arg[1].type = AT_DOUBLE;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Start Par.";
  arg[0].v.doubleVal = data.startPar;

  arg[1].prompt = "End Par.";
  arg[1].v.doubleVal = data.endPar;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  data.startPar = (float) arg[0].v.doubleVal;
  data.endPar = (float) arg[1].v.doubleVal;

  ArgCleanup(arg, ARG_NUM);

  PrimApply(PT_CYLINDER, refP, setCylinderLength, &data);
  PrimApply(PT_RIBBON, refP, setRibbonLength, &data);

  GraphMolChanged(PROP_SELECTED);
  GraphRedrawNeeded();

  return EC_OK;
}
