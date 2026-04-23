/*
************************************************************************
*
*   ExSizePlate.c - SizePlate command
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdprim/SCCS/s.ExSizePlate.c
*   SCCS identification       : 1.2
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
  float thick;
} SizeData;

static void
getPlateSize(PrimObjP primP, void *clientData)
{
  SizeData *dataP = clientData;
  float thick;

  thick = PrimGetThick(primP);

  if (dataP->primNo == 0) {
    dataP->thick = thick;
  } else {
    if (dataP->thick != thick)
      dataP->thick = 0.0f;
  }

  dataP->primNo++;
}

static void
setPlateSize(PrimObjP primP, void *clientData)
{
  SizeData *dataP = clientData;

  if  (dataP->thick > 0.0f)
    PrimSetThick(primP, dataP->thick);
}

#define ARG_NUM 1

ErrCode
ExSizePlate(char *cmd)
{
  PropRefP refP;
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  SizeData data;

  refP = PropGetRef(PROP_SELECTED, FALSE);

  data.primNo = 0;
  data.thick = 0.0f;
  PrimApply(PT_PLATE, refP, getPlateSize, &data);

  arg[0].type = AT_DOUBLE;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Thickness";
  arg[0].v.doubleVal = data.thick;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  data.thick = (float) arg[0].v.doubleVal;

  ArgCleanup(arg, ARG_NUM);

  PrimApply(PT_PLATE, refP, setPlateSize, &data);

  GraphMolChanged(PROP_SELECTED);
  GraphRedrawNeeded();

  return EC_OK;
}
