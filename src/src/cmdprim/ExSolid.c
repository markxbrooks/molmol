/*
************************************************************************
*
*   ExSolid.c - AddSolid command
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdprim/SCCS/s.ExSolid.c
*   SCCS identification       : 1.6
*
************************************************************************
*/

#include <cmd_prim.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arg.h>
#include <data_hand.h>
#include <prim_hand.h>
#include <graph_draw.h>

static PrimSolidKind CurrKind = PSK_SPHERE;
static float CurrCover = 1.0f;

static void
addSolid(DhMolP molP, void *clientData)
{
  PrimObjP primP;

  primP = PrimNew(PT_SOLID, molP);
  if (primP == NULL)
    return;

  PrimSetSolidKind(primP, CurrKind);
  PrimSetCover(primP, CurrCover);

  PrimSetProp(PropGetRef("solid", TRUE), primP, TRUE);
}

#define ARG_NUM 2
#define KIND_NUM 5

ErrCode
ExAddSolid(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  EnumEntryDescr enumEntry[KIND_NUM];
  ErrCode errCode;

  arg[0].type = AT_ENUM;
  arg[1].type = AT_DOUBLE;

  ArgInit(arg, ARG_NUM);

  enumEntry[0].str = "sphere";
  enumEntry[0].onOff = FALSE;
  enumEntry[1].str = "ellipsoid";
  enumEntry[1].onOff = FALSE;
  enumEntry[2].str = "cylinder";
  enumEntry[2].onOff = FALSE;
  enumEntry[3].str = "cone";
  enumEntry[3].onOff = FALSE;
  enumEntry[4].str = "box";
  enumEntry[4].onOff = FALSE;

  enumEntry[CurrKind].onOff = TRUE;

  arg[0].prompt = "Shape";
  arg[0].u.enumD.entryP = enumEntry;
  arg[0].u.enumD.n = KIND_NUM;
  arg[0].u.enumD.lineNo = 2;
  arg[0].v.intVal = CurrKind;

  arg[1].prompt = "Coverage";
  arg[1].v.doubleVal = CurrCover;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  CurrKind = arg[0].v.intVal;
  CurrCover = (float) arg[1].v.doubleVal;

  ArgCleanup(arg, ARG_NUM);

  DhApplyMol(PropGetRef(PROP_SELECTED, FALSE), addSolid, NULL);

  GraphMolChanged(PROP_SELECTED);
  GraphRedrawNeeded();

  return EC_OK;
}
