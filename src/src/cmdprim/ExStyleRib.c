/*
************************************************************************
*
*   ExStyleRib.c - StyleRibbon command
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
*   Date of last modification : 95/11/24
*   Pathname of SCCS file     : /sgiext/molmol/src/cmdprim/SCCS/s.ExStyleRib.c
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include <cmd_prim.h>

#include <stdio.h>

#include <arg.h>
#include <prim_hand.h>
#include <graph_draw.h>

#define STYLE_NUM 7
#define START_NUM 4
#define END_NUM 5
#define RAD_NUM 3

typedef struct {
  int primNo;
  int style, startStyle, endStyle, rad;
  PropRefP visRefP;
} StyleData;

static void
getRibbonStyle(PrimObjP primP, void *clientData)
{
  StyleData *dataP = clientData;
  RibbonStyle style;
  RibbonEnd startStyle, endStyle;
  RibbonRadius rad;

  style = RibbonGetStyle(primP);
  startStyle = RibbonGetEndStyle(primP, REC_START);
  endStyle = RibbonGetEndStyle(primP, REC_END);
  rad = RibbonGetRadius(primP);

  if (dataP->primNo == 0) {
    dataP->style = style;
    dataP->startStyle = startStyle;
    dataP->endStyle = endStyle;
    dataP->rad = rad;
  } else {
    if (dataP->style != style)
      dataP->style = STYLE_NUM - 1;
    if (dataP->startStyle != startStyle)
      dataP->startStyle = START_NUM - 1;
    if (dataP->endStyle != endStyle)
      dataP->endStyle = END_NUM - 1;
    if (dataP->rad != rad)
      dataP->rad = RAD_NUM - 1;
  }

  dataP->primNo++;
}

static void
setRibbonStyle(PrimObjP primP, void *clientData)
{
  StyleData *dataP = clientData;

  if  (dataP->style != STYLE_NUM - 1) {
    RibbonSetStyle(primP, dataP->style);
    PrimSetProp(dataP->visRefP, primP, dataP->style != RS_INVISIBLE);
  }

  if  (dataP->startStyle != START_NUM - 1)
    RibbonSetEndStyle(primP, REC_START, dataP->startStyle);
  if  (dataP->endStyle != END_NUM - 1)
    RibbonSetEndStyle(primP, REC_END, dataP->endStyle);
  if  (dataP->rad != RAD_NUM - 1)
    RibbonSetRadius(primP, dataP->rad);
}

#define ARG_NUM 4

ErrCode
ExStyleRibbon(char *cmd)
{
  PropRefP refP;
  ArgDescr arg[ARG_NUM];
  EnumEntryDescr enum1Entry[STYLE_NUM];
  EnumEntryDescr enum2Entry[START_NUM];
  EnumEntryDescr enum3Entry[END_NUM];
  EnumEntryDescr enum4Entry[RAD_NUM];
  ErrCode errCode;
  StyleData data;

  refP = PropGetRef(PROP_SELECTED, FALSE);

  data.primNo = 0;
  data.style = STYLE_NUM - 1;
  data.startStyle = START_NUM - 1;
  data.endStyle = END_NUM - 1;
  data.rad = RAD_NUM - 1;
  PrimApply(PT_RIBBON, refP, getRibbonStyle, &data);

  arg[0].type = AT_ENUM;
  arg[1].type = AT_ENUM;
  arg[2].type = AT_ENUM;
  arg[3].type = AT_ENUM;

  ArgInit(arg, ARG_NUM);

  enum1Entry[0].str = "invisible";
  enum1Entry[0].onOff = FALSE;
  enum1Entry[1].str = "rect";
  enum1Entry[1].onOff = FALSE;
  enum1Entry[2].str = "round";
  enum1Entry[2].onOff = FALSE;
  enum1Entry[3].str = "rounded";
  enum1Entry[3].onOff = FALSE;
  enum1Entry[4].str = "ellip";
  enum1Entry[4].onOff = FALSE;
  enum1Entry[5].str = "lense";
  enum1Entry[5].onOff = FALSE;
  enum1Entry[6].str = "as_is";
  enum1Entry[6].onOff = FALSE;

  enum1Entry[data.style].onOff = TRUE;

  arg[0].prompt = "Style";
  arg[0].u.enumD.entryP = enum1Entry;
  arg[0].u.enumD.n = STYLE_NUM;
  arg[0].u.enumD.lineNo = 2;
  arg[0].v.intVal = data.style;

  enum2Entry[0].str = "open";
  enum2Entry[0].onOff = FALSE;
  enum2Entry[1].str = "sharp";
  enum2Entry[1].onOff = FALSE;
  enum2Entry[2].str = "soft";
  enum2Entry[2].onOff = FALSE;
  enum2Entry[3].str = "as_is";
  enum2Entry[3].onOff = FALSE;

  enum2Entry[data.startStyle].onOff = TRUE;

  arg[1].prompt = "Start Style";
  arg[1].optional = TRUE;
  arg[1].u.enumD.entryP = enum2Entry;
  arg[1].u.enumD.n = START_NUM;
  arg[1].v.intVal = data.startStyle;

  enum3Entry[0].str = "open";
  enum3Entry[0].onOff = FALSE;
  enum3Entry[1].str = "sharp";
  enum3Entry[1].onOff = FALSE;
  enum3Entry[2].str = "soft";
  enum3Entry[2].onOff = FALSE;
  enum3Entry[3].str = "arrow";
  enum3Entry[3].onOff = FALSE;
  enum3Entry[4].str = "as_is";
  enum3Entry[4].onOff = FALSE;

  enum3Entry[data.endStyle].onOff = TRUE;

  arg[2].prompt = "End Style";
  arg[2].optional = TRUE;
  arg[2].u.enumD.entryP = enum3Entry;
  arg[2].u.enumD.n = END_NUM;
  arg[2].v.intVal = data.endStyle;

  enum4Entry[0].str = "const";
  enum4Entry[0].onOff = FALSE;
  enum4Entry[1].str = "atom";
  enum4Entry[1].onOff = FALSE;
  enum4Entry[2].str = "as_is";
  enum4Entry[2].onOff = FALSE;

  enum4Entry[data.rad].onOff = TRUE;

  arg[3].prompt = "Radius";
  arg[3].optional = TRUE;
  arg[3].u.enumD.entryP = enum4Entry;
  arg[3].u.enumD.n = RAD_NUM;
  arg[3].v.intVal = data.rad;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  data.style = arg[0].v.intVal;
  data.startStyle = arg[1].v.intVal;
  data.endStyle = arg[2].v.intVal;
  data.rad = arg[3].v.intVal;

  ArgCleanup(arg, ARG_NUM);

  data.visRefP = PropGetRef(PROP_VISIBLE, FALSE);
  PrimApply(PT_RIBBON, refP, setRibbonStyle, &data);

  GraphMolChanged(PROP_SELECTED);
  GraphRedrawNeeded();

  return EC_OK;
}
