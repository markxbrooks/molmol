/*
************************************************************************
*
*   ExBuild.c - NewMol, AddRes and ChangeRes commands
*
*   Copyright (c) 1994-98
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
*   Date of last modification : 98/07/21
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homea/rkoradi/molmol-master/src/cmdstruc/SCCS/s.ExBuild.c
*   SCCS identification       : 1.6
*
************************************************************************
*/

#include <cmd_struc.h>

#include <stdio.h>

#include <arg.h>
#include <data_hand.h>
#include <data_sel.h>
#include <graph_draw.h>

#define ARG_NUM 1

static int CurrPos = 1;
static int CurrKind = 0;

ErrCode
ExNewMol(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  DhMolP molP;

  arg[0].type = AT_STR;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Molecule Name";

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  DeselAllMol();

  molP = DhMolNew();
  DhMolSetName(molP, arg[0].v.strVal);

  ArgCleanup(arg, ARG_NUM);

  DhMolInit(molP);
  GraphMolAdd(molP);

  return EC_OK;
}

static void
resAdd(DhMolP molP, void *clientData)
{
  DhResDefP resDefP = clientData;
  DhResP resP;

  if (CurrPos == 0) {
    resP = DhResNew(molP, resDefP, SP_FIRST);
    DhResStandGeom(resP);
    DhResDockNext(resP);
  } else {
    resP = DhResNew(molP, resDefP, SP_LAST);
    DhResStandGeom(resP);
    DhResDockPrev(resP);
  }

  DhResInit(resP);
}

#undef ARG_NUM
#define ARG_NUM 2
#define POS_NUM 2

ErrCode
ExAddRes(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  EnumEntryDescr enumEntry[POS_NUM];
  ErrCode errCode;
  DhResDefP resDefP;

  arg[0].type = AT_ENUM;
  arg[1].type = AT_STR;

  ArgInit(arg, ARG_NUM);

  enumEntry[0].str = "first";
  enumEntry[0].onOff = FALSE;
  enumEntry[1].str = "last";
  enumEntry[1].onOff = FALSE;

  enumEntry[CurrPos].onOff = TRUE;

  arg[0].prompt = "Position";
  arg[0].u.enumD.entryP = enumEntry;
  arg[0].u.enumD.n = POS_NUM;
  arg[0].v.intVal = CurrPos;

  arg[1].prompt = "Residue Type";

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  CurrPos = arg[0].v.intVal;
  resDefP = DhResDefGet(arg[1].v.strVal);

  ArgCleanup(arg, ARG_NUM);

  if (resDefP == NULL) {
    CipSetError("unknown residue type");
    return EC_ERROR;
  }

  DhApplyMol(PropGetRef(PROP_SELECTED, FALSE), resAdd, resDefP);

  GraphMolChanged(PROP_SELECTED);
  GraphRedrawNeeded();

  return EC_OK;
}

static void
resMutate(DhResP resP, void *clientData)
{
  DhResDefP resDefP = clientData;

  DhResMutate(resP, resDefP, CurrKind);
  DhResInit(resP);
}

#undef ARG_NUM
#define ARG_NUM 2
#define KIND_NUM 2

ErrCode
ExChangeRes(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  EnumEntryDescr enumEntry[KIND_NUM];
  ErrCode errCode;
  DhResDefP resDefP;

  arg[0].type = AT_ENUM;
  arg[1].type = AT_STR;

  ArgInit(arg, ARG_NUM);

  enumEntry[0].str = "angles";
  enumEntry[0].onOff = FALSE;
  enumEntry[1].str = "atoms";
  enumEntry[1].onOff = FALSE;

  enumEntry[CurrKind].onOff = TRUE;

  arg[0].prompt = "Keep";
  arg[0].u.enumD.entryP = enumEntry;
  arg[0].u.enumD.n = KIND_NUM;
  arg[0].v.intVal = CurrKind;

  arg[1].prompt = "Residue Type";

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  CurrKind = arg[0].v.intVal;
  resDefP = DhResDefGet(arg[1].v.strVal);

  ArgCleanup(arg, ARG_NUM);

  if (resDefP == NULL) {
    CipSetError("unknown residue type");
    return EC_ERROR;
  }

  DhApplyRes(PropGetRef(PROP_SELECTED, FALSE), resMutate, resDefP);

  GraphMolChanged(PROP_SELECTED);
  GraphRedrawNeeded();

  return EC_OK;
}
