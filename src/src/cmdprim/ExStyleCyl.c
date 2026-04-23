/*
************************************************************************
*
*   ExStyleCyl.c - StyleCylinder command
*
*   Copyright (c) 1997
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
*   Date of last modification : 97/07/22
*   Pathname of SCCS file     : /local/home/kor/molmol/src/cmdprim/SCCS/s.ExStyleCyl.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <cmd_prim.h>

#include <stdio.h>

#include <arg.h>
#include <prim_hand.h>
#include <graph_draw.h>

#define STYLE_NUM 3

static int CurrStyle = 1;
static PropRefP VisibleRefP;

static void
setStyle(PrimObjP primP, void *clientData)
{
  PrimSetCylinderStyle(primP, CurrStyle);
  PrimSetProp(VisibleRefP, primP, CurrStyle != PCS_INVISIBLE);
}

#define ARG_NUM 1

ErrCode
ExStyleCylinder(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  EnumEntryDescr enumEntry[STYLE_NUM];
  ErrCode errCode;

  arg[0].type = AT_ENUM;

  ArgInit(arg, ARG_NUM);

  enumEntry[0].str = "invisible";
  enumEntry[0].onOff = FALSE;
  enumEntry[1].str = "normal";
  enumEntry[1].onOff = FALSE;
  enumEntry[2].str = "arrow";
  enumEntry[2].onOff = FALSE;

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
  PrimApply(PT_CYLINDER, PropGetRef(PROP_SELECTED, FALSE), setStyle, NULL);

  GraphRedrawNeeded();

  return EC_OK;
}
