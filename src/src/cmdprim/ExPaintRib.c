/*
************************************************************************
*
*   ExPaintRib.c - PaintRibbon command
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
*   Date of last modification : 95/12/10
*   Pathname of SCCS file     : /sgiext/molmol/src/cmdprim/SCCS/s.ExPaintRib.c
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include <cmd_prim.h>

#include <stdio.h>

#include <arg.h>
#include <prim_hand.h>
#include <graph_draw.h>

#define PAINT_NUM 5

typedef struct {
  int primNo;
  int paint;
} PaintData;

static void
getRibbonPaint(PrimObjP primP, void *clientData)
{
  PaintData *dataP = clientData;
  RibbonPaint paint;

  paint = RibbonGetPaint(primP);

  if (dataP->primNo == 0) {
    dataP->paint = paint;
  } else {
    if (dataP->paint != paint)
      dataP->paint = PAINT_NUM - 1;
  }

  dataP->primNo++;
}

static void
setRibbonPaint(PrimObjP primP, void *clientData)
{
  PaintData *dataP = clientData;

  if  (dataP->paint != PAINT_NUM - 1)
    RibbonSetPaint(primP, dataP->paint);
}

#define ARG_NUM 1

ErrCode
ExPaintRibbon(char *cmd)
{
  PropRefP refP;
  ArgDescr arg[ARG_NUM];
  EnumEntryDescr enumEntry[PAINT_NUM];
  ErrCode errCode;
  PaintData data;

  refP = PropGetRef(PROP_SELECTED, FALSE);

  data.primNo = 0;
  data.paint = PAINT_NUM - 1;
  PrimApply(PT_RIBBON, refP, getRibbonPaint, &data);

  arg[0].type = AT_ENUM;

  ArgInit(arg, ARG_NUM);

  enumEntry[0].str = "one_color";
  enumEntry[0].onOff = FALSE;
  enumEntry[1].str = "two_color";
  enumEntry[1].onOff = FALSE;
  enumEntry[2].str = "atom";
  enumEntry[2].onOff = FALSE;
  enumEntry[3].str = "atom_smooth";
  enumEntry[3].onOff = FALSE;
  enumEntry[4].str = "as_is";
  enumEntry[4].onOff = FALSE;

  enumEntry[data.paint].onOff = TRUE;

  arg[0].prompt = "Paint";
  arg[0].u.enumD.entryP = enumEntry;
  arg[0].u.enumD.n = PAINT_NUM;
  arg[0].u.enumD.lineNo = 2;
  arg[0].v.intVal = data.paint;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  data.paint = arg[0].v.intVal;

  ArgCleanup(arg, ARG_NUM);

  PrimApply(PT_RIBBON, refP, setRibbonPaint, &data);

  GraphMolChanged(PROP_SELECTED);
  GraphRedrawNeeded();

  return EC_OK;
}
