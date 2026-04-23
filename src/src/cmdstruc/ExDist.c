/*
************************************************************************
*
*   ExDist.c - AddDist, AddUpl, AddLol and RemoveDist commands
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
*   Date of last modification : 01/05/27
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdstruc/SCCS/s.ExDist.c
*   SCCS identification       : 1.6
*
************************************************************************
*/

#include <cmd_struc.h>

#include <stdio.h>
#include <string.h>

#include <arg.h>
#include <data_hand.h>
#include <data_sel.h>
#include <attr_struc.h>
#include <attr_mng.h>
#include <graph_draw.h>

static float CurrLimit = 0.0f;

static void
setLabel(DhDistP distP)
{
  AttrP attrP;
  struct AttrS attr;

  attrP = DhDistGetAttr(distP);
  AttrCopy(&attr, attrP);
  AttrReturn(attrP);
  attr.distLabel = DL_VAL;
  DhDistSetAttr(distP, AttrGet(&attr));
}

ErrCode
ExAddDist(char *cmd)
{
  int selNo;
  DhAtomP atomPA[2];
  DhDistP distP;

  selNo = SelAtomGet(atomPA, 2);
  if (selNo != 2) {
    CipSetError("exactly 2 atoms must be selected");
    return EC_ERROR;
  }

  distP = DhDistNew(atomPA[0], atomPA[1]);
  if (strcmp(cmd, "AddDist") == 0)
    DhDistSetKind(distP, DK_ACTUAL);
  else
    DhDistSetKind(distP, DK_HBOND);

  DhDistInit(distP);
  setLabel(distP);

  GraphMolChanged(PROP_SELECTED);
  GraphRedrawNeeded();

  return EC_OK;
}

#define ARG_NUM 1

ErrCode
ExAddLimit(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  int selNo;
  DhAtomP atomPA[2];
  DhDistP distP;

  arg[0].type = AT_DOUBLE;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Limit";
  arg[0].v.doubleVal = CurrLimit;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  CurrLimit = (float) arg[0].v.doubleVal;
  ArgCleanup(arg, ARG_NUM);

  selNo = SelAtomGet(atomPA, 2);
  if (selNo != 2) {
    CipSetError("exactly 2 atoms must be selected");
    return EC_ERROR;
  }

  distP = DhDistNew(atomPA[0], atomPA[1]);
  if (strcmp(cmd, "AddUpl") == 0)
    DhDistSetKind(distP, DK_UPPER);
  else
    DhDistSetKind(distP, DK_LOWER);
  DhDistSetLimit(distP, CurrLimit);

  DhDistInit(distP);
  setLabel(distP);

  GraphMolChanged(PROP_SELECTED);
  GraphRedrawNeeded();

  return EC_OK;
}

static void
removeDist(DhDistP distP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  DhDistDestroy(distP);
}

ErrCode
ExRemoveDist(char *cmd)
{
  DhApplyDist(PropGetRef(PROP_SELECTED, FALSE), removeDist, NULL);

  GraphMolChanged(PROP_SELECTED);
  GraphRedrawNeeded();

  return EC_OK;
}
