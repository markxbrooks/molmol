/*
************************************************************************
*
*   ExTypeRib.c - TypeRibbon command
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
*   Date of last modification : 96/03/07
*   Pathname of SCCS file     : /sgiext/molmol/src/cmdprim/SCCS/s.ExTypeRib.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <cmd_prim.h>

#include <stdio.h>

#include <arg.h>
#include <prim_hand.h>
#include <graph_draw.h>

#define SHAPE_NUM 3
#define ORIENT_NUM 5

typedef struct {
  int primNo;
  int shape, orient;
} TypeData;

static void
getRibbonType(PrimObjP primP, void *clientData)
{
  TypeData *dataP = clientData;
  RibbonShape shape;
  RibbonOrient orient;

  shape = RibbonGetShape(primP);
  orient = RibbonGetOrient(primP);

  if (dataP->primNo == 0) {
    dataP->shape = shape;
    dataP->orient = orient;
  } else {
    if (dataP->shape != shape)
      dataP->shape = SHAPE_NUM - 1;
    if (dataP->orient != orient)
      dataP->orient = ORIENT_NUM - 1;
  }

  dataP->primNo++;
}

static void
setRibbonType(PrimObjP primP, void *clientData)
{
  TypeData *dataP = clientData;

  if  (dataP->shape != SHAPE_NUM - 1)
    RibbonSetShape(primP, dataP->shape);
  if  (dataP->orient != ORIENT_NUM - 1)
    RibbonSetOrient(primP, dataP->orient);
}

#define ARG_NUM 2

ErrCode
ExTypeRibbon(char *cmd)
{
  PropRefP refP;
  ArgDescr arg[ARG_NUM];
  EnumEntryDescr enum1Entry[SHAPE_NUM];
  EnumEntryDescr enum2Entry[ORIENT_NUM];
  ErrCode errCode;
  TypeData data;

  refP = PropGetRef(PROP_SELECTED, FALSE);

  data.primNo = 0;
  data.shape = SHAPE_NUM - 1;
  data.orient = ORIENT_NUM - 1;
  PrimApply(PT_RIBBON, refP, getRibbonType, &data);

  arg[0].type = AT_ENUM;
  arg[1].type = AT_ENUM;

  ArgInit(arg, ARG_NUM);

  enum1Entry[0].str = "interpol";
  enum1Entry[0].onOff = FALSE;
  enum1Entry[1].str = "smooth";
  enum1Entry[1].onOff = FALSE;
  enum1Entry[2].str = "as_is";
  enum1Entry[2].onOff = FALSE;

  enum1Entry[data.shape].onOff = TRUE;

  arg[0].prompt = "Shape";
  arg[0].u.enumD.entryP = enum1Entry;
  arg[0].u.enumD.n = SHAPE_NUM;
  arg[0].v.intVal = data.shape;

  enum2Entry[0].str = "free";
  enum2Entry[0].onOff = FALSE;
  enum2Entry[1].str = "cylinder";
  enum2Entry[1].onOff = FALSE;
  enum2Entry[2].str = "spacing";
  enum2Entry[2].onOff = FALSE;
  enum2Entry[3].str = "neighbour";
  enum2Entry[3].onOff = FALSE;
  enum2Entry[4].str = "as_is";
  enum2Entry[4].onOff = FALSE;

  enum2Entry[data.orient].onOff = TRUE;

  arg[1].prompt = "Orient";
  arg[1].u.enumD.entryP = enum2Entry;
  arg[1].u.enumD.n = ORIENT_NUM;
  arg[1].v.intVal = data.orient;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  data.shape = arg[0].v.intVal;
  data.orient = arg[1].v.intVal;

  ArgCleanup(arg, ARG_NUM);

  PrimApply(PT_RIBBON, refP, setRibbonType, &data);

  GraphMolChanged(PROP_SELECTED);
  GraphRedrawNeeded();

  return EC_OK;
}
