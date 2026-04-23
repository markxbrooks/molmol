/*
************************************************************************
*
*   ExListProp.c - ListProp* commands
*
*   Copyright (c) 1994-96
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
*   Date of last modification : 96/12/06
*   Pathname of SCCS file     : /local/home/kor/molmol/src/cmdprop/SCCS/s.ExListProp.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <cmd_prop.h>

#include <stdio.h>
#include <string.h>

#include <pu.h>
#include <prop_tab.h>
#include <data_hand.h>
#include <prim_hand.h>

static PuTextWindow TextW;

typedef struct {
  int entNo;
  unsigned *propTab;
} PropDescr;

static void
getProp(unsigned *propTab, PropDescr *descrP)
{
  if (descrP->entNo == 0)
    descrP->propTab = propTab;
  else if (descrP->propTab != propTab)
    descrP->propTab = NULL;

  descrP->entNo++;
}

static void
getPropMol(DhMolP molP, void *clientData)
{
  getProp(DhMolGetPropTab(molP), clientData);
}

static void
getPropRes(DhResP resP, void *clientData)
{
  getProp(DhResGetPropTab(resP), clientData);
}

static void
getPropAtom(DhAtomP atomP, void *clientData)
{
  getProp(DhAtomGetPropTab(atomP), clientData);
}

static void
getPropBond(DhBondP bondP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  getProp(DhBondGetPropTab(bondP), clientData);
}

static void
getPropAngle(DhAngleP angleP, void *clientData)
{
  getProp(DhAngleGetPropTab(angleP), clientData);
}

static void
getPropDist(DhDistP distP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  getProp(DhDistGetPropTab(distP), clientData);
}

static void
getPropPrim(PrimObjP primP, void *clientData)
{
  getProp(PrimGetPropTab(primP), clientData);
}

static void
listProp(char *name, void *clientData)
{
  if (name[0] == '_')
    /* do not list internally used properties */
    return;

  PuWriteStr(TextW, name);
  PuWriteStr(TextW, "\n");
}

ErrCode
ExListProp(char *cmd)
{
  PropDescr descr;
  PropRefP refP;

  descr.entNo = 0;
  descr.propTab = NULL;

  refP = PropGetRef(PROP_SELECTED, FALSE);

  if (strcmp(cmd, "ListPropMol") == 0)
    DhApplyMol(refP, getPropMol, &descr);
  else if (strcmp(cmd, "ListPropRes") == 0)
    DhApplyRes(refP, getPropRes, &descr);
  else if (strcmp(cmd, "ListPropAtom") == 0)
    DhApplyAtom(refP, getPropAtom, &descr);
  else if (strcmp(cmd, "ListPropBond") == 0)
    DhApplyBond(refP, getPropBond, &descr);
  else if (strcmp(cmd, "ListPropAngle") == 0)
    DhApplyAngle(refP, getPropAngle, &descr);
  else if (strcmp(cmd, "ListPropDist") == 0)
    DhApplyDist(refP, getPropDist, &descr);
  else
    PrimApply(PT_ALL, refP, getPropPrim, &descr);

  if (descr.entNo == 0) {
    CipSetError("no item selected");
    return EC_ERROR;
  }

  if (descr.propTab == NULL) {
    CipSetError("not all selected items have same properties");
    return EC_ERROR;
  }

  TextW = PuCreateTextWindow(cmd);

  PropList(descr.propTab, listProp, NULL);

  return EC_OK;
}
