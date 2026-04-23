/*
************************************************************************
*
*   ExSizeRib.c - SizeRibbon command
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdprim/SCCS/s.ExSizeRib.c
*   SCCS identification       : 1.3
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
  float width, thick;
  float startLen, endLen, arrowWidth;
} SizeData;

static void
getRibbonSize(PrimObjP primP, void *clientData)
{
  SizeData *dataP = clientData;
  float width, thick, startLen, endLen, arrowWidth;

  width = RibbonGetWidth(primP);
  thick = PrimGetThick(primP);
  startLen = RibbonGetEndLen(primP, REC_START);
  endLen = RibbonGetEndLen(primP, REC_END);
  arrowWidth = RibbonGetArrowWidth(primP);

  if (dataP->primNo == 0) {
    dataP->width = width;
    dataP->thick = thick;
    dataP->startLen = startLen;
    dataP->endLen = endLen;
    dataP->arrowWidth = arrowWidth;
  } else {
    if (dataP->width != width)
      dataP->width = 0.0f;
    if (dataP->thick != thick)
      dataP->thick = 0.0f;
    if (dataP->startLen != startLen)
      dataP->startLen = 0.0f;
    if (dataP->endLen != endLen)
      dataP->endLen = 0.0f;
    if (dataP->arrowWidth != arrowWidth)
      dataP->arrowWidth = 0.0f;
  }

  dataP->primNo++;
}

static void
setRibbonSize(PrimObjP primP, void *clientData)
{
  SizeData *dataP = clientData;

  if  (dataP->width > 0.0f)
    RibbonSetWidth(primP, dataP->width);
  if  (dataP->thick > 0.0f)
    PrimSetThick(primP, dataP->thick);
  if  (dataP->startLen > 0.0f)
    RibbonSetEndLen(primP, REC_START, dataP->startLen);
  if  (dataP->endLen > 0.0f)
    RibbonSetEndLen(primP, REC_END, dataP->endLen);
  if  (dataP->arrowWidth > 0.0f)
    RibbonSetArrowWidth(primP, dataP->arrowWidth);
}

#define ARG_NUM 5

ErrCode
ExSizeRibbon(char *cmd)
{
  PropRefP refP;
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  SizeData data;

  refP = PropGetRef(PROP_SELECTED, FALSE);

  data.primNo = 0;
  data.width = 0.0f;
  data.thick = 0.0f;
  data.startLen = 0.0f;
  data.endLen = 0.0f;
  data.arrowWidth = 0.0f;
  PrimApply(PT_RIBBON, refP, getRibbonSize, &data);

  arg[0].type = AT_DOUBLE;
  arg[1].type = AT_DOUBLE;
  arg[2].type = AT_DOUBLE;
  arg[3].type = AT_DOUBLE;
  arg[4].type = AT_DOUBLE;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Width";
  arg[0].v.doubleVal = data.width;

  arg[1].prompt = "Thickness";
  arg[1].optional = TRUE;
  arg[1].v.doubleVal = data.thick;

  arg[2].prompt = "Start Length";
  arg[2].optional = TRUE;
  arg[2].v.doubleVal = data.startLen;

  arg[3].prompt = "End Length";
  arg[3].optional = TRUE;
  arg[3].v.doubleVal = data.endLen;

  arg[4].prompt = "Arrow Width";
  arg[4].optional = TRUE;
  arg[4].v.doubleVal = data.arrowWidth;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  data.width = (float) arg[0].v.doubleVal;
  data.thick = (float) arg[1].v.doubleVal;
  data.startLen = (float) arg[2].v.doubleVal;
  data.endLen = (float) arg[3].v.doubleVal;
  data.arrowWidth = (float) arg[4].v.doubleVal;

  ArgCleanup(arg, ARG_NUM);

  PrimApply(PT_RIBBON, refP, setRibbonSize, &data);

  GraphMolChanged(PROP_SELECTED);
  GraphRedrawNeeded();

  return EC_OK;
}
