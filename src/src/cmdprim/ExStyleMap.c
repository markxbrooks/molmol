/*
************************************************************************
*
*   ExStyleMap.c - StyleMap command
*
*   Copyright (c) 1994-99
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
*   Date of last modification : 99/10/23
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/src/cmdprim/SCCS/s.ExStyleMap.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <cmd_prim.h>

#include <stdio.h>

#include <arg.h>
#include <prim_hand.h>
#include <attr_struc.h>
#include <attr_mng.h>
#include <graph_draw.h>

#define STYLE_NUM 4

static int CurrStyle = 1;
static PropRefP VisibleRefP;

static void
setStyle(PrimObjP primP, void *clientData)
{
  AttrP attrP;
  struct AttrS attr;

  PrimSetMapStyle(primP, CurrStyle);

  attrP = PrimGetAttr(primP);
  AttrCopy(&attr, attrP);
  AttrReturn(attrP);
  if (CurrStyle == PMS_CYLINDER || CurrStyle == PMS_NEON)
    attr.shadeModel = SHADE_DEFAULT;
  else
    attr.shadeModel = -1;
  PrimSetAttr(primP, AttrGet(&attr));

  PrimSetProp(VisibleRefP, primP, CurrStyle != PMS_INVISIBLE);
}

#define ARG_NUM 1

ErrCode
ExStyleMap(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  EnumEntryDescr enumEntry[STYLE_NUM];
  ErrCode errCode;

  arg[0].type = AT_ENUM;

  ArgInit(arg, ARG_NUM);

  enumEntry[0].str = "invisible";
  enumEntry[0].onOff = FALSE;
  enumEntry[1].str = "line";
  enumEntry[1].onOff = FALSE;
  enumEntry[2].str = "cylinder";
  enumEntry[2].onOff = FALSE;
  enumEntry[3].str = "neon";
  enumEntry[3].onOff = FALSE;

  enumEntry[CurrStyle].onOff = TRUE;

  arg[0].prompt = "Style";
  arg[0].u.enumD.entryP = enumEntry;
  arg[0].u.enumD.n = STYLE_NUM;
  arg[0].v.intVal = CurrStyle;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  CurrStyle = arg[0].v.intVal;
  ArgCleanup(arg, ARG_NUM);

  VisibleRefP = PropGetRef(PROP_VISIBLE, FALSE);
  PrimApply(PT_MAP, PropGetRef(PROP_SELECTED, FALSE), setStyle, NULL);

  GraphRedrawNeeded();

  return EC_OK;
}
