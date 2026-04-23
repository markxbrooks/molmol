/*
************************************************************************
*
*   ExNear.c - NearPrim command
*
*   Copyright (c) 1994-97
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdattr/SCCS/s.ExNear.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <cmd_attr.h>

#include <string.h>

#include <arg.h>
#include <data_hand.h>
#include <prim_hand.h>
#include <attr_struc.h>
#include <attr_mng.h>
#include <graph_draw.h>

#define FLOAT_AS_IS -1.0f

typedef struct {
  int entNo;
  ExprP exprP;
  float val;
} NearDescr;

static void
getNear(AttrP attrP, NearDescr *nearP)
{
  if (nearP->entNo == 0) {
    nearP->val = attrP->nearClip;
  } else {
    if (nearP->val != attrP->nearClip)
      nearP->val = FLOAT_AS_IS;
  }

  nearP->entNo++;
}

static void
getNearAtom(DhAtomP atomP, void *clientData)
{
  getNear(DhAtomGetAttr(atomP), clientData);
}

static void
getNearBond(DhBondP bondP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  getNear(DhBondGetAttr(bondP), clientData);
}

static void
getNearDist(DhDistP distP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  getNear(DhDistGetAttr(distP), clientData);
}

static void
getNearPrim(PrimObjP primP, void *clientData)
{
  getNear(PrimGetAttr(primP), clientData);
}

static void
evalNear(void *entP, NearDescr *nearP)
{
  ExprRes exprRes;

  ExprEval(entP, nearP->exprP, &exprRes);
  if (exprRes.resType == ER_INT)
    nearP->val = (float) exprRes.u.intVal;
  else
    nearP->val = exprRes.u.floatVal;
}

static AttrP
changeNear(AttrP attrP, NearDescr *nearP)
{
  struct AttrS attr;

  AttrCopy(&attr, attrP);
  AttrReturn(attrP);
  if (nearP->val >= 0.0f)
    attr.nearClip = nearP->val;
  return AttrGet(&attr);
}

static void
nearAtom(DhAtomP atomP, void *clientData)
{
  evalNear(atomP, clientData);
  DhAtomSetAttr(atomP, changeNear(DhAtomGetAttr(atomP), clientData));
}

static void
nearBond(DhBondP bondP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  evalNear(bondP, clientData);
  DhBondSetAttr(bondP, changeNear(DhBondGetAttr(bondP), clientData));
}

static void
nearDist(DhDistP distP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  evalNear(distP, clientData);
  DhDistSetAttr(distP, changeNear(DhDistGetAttr(distP), clientData));
}

static void
nearPrim(PrimObjP primP, void *clientData)
{
  evalNear(primP, clientData);
  PrimSetAttr(primP, changeNear(PrimGetAttr(primP), clientData));
}

#define ARG_NO 1

ErrCode
ExNear(char *cmd)
{
  DataEntityType entType;
  ArgDescr arg[ARG_NO];
  ErrCode errCode;
  NearDescr nearDescr;
  PropRefP refP;

  if (strcmp(cmd, "NearAtom") == 0)
    entType = DE_ATOM;
  else if (strcmp(cmd, "NearBond") == 0)
    entType = DE_BOND;
  else if (strcmp(cmd, "NearDist") == 0)
    entType = DE_DIST;
  else
    entType = DE_PRIM;

  arg[0].type = AT_DOUBLE;

  ArgInit(arg, ARG_NO);

  arg[0].prompt = "Clip Dist";
  arg[0].entType = entType;

  nearDescr.entNo = 0;
  nearDescr.val = FLOAT_AS_IS;

  refP = PropGetRef(PROP_SELECTED, FALSE);

  switch (entType) {
    case DE_ATOM:
      DhApplyAtom(refP, getNearAtom, &nearDescr);
      break;
    case DE_BOND:
      DhApplyBond(refP, getNearBond, &nearDescr);
      break;
    case DE_DIST:
      DhApplyDist(refP, getNearDist, &nearDescr);
      break;
    case DE_PRIM:
      PrimApply(PT_ALL, refP, getNearPrim, &nearDescr);
      break;
  }

  arg[0].v.doubleVal = nearDescr.val;

  errCode = ArgGet(arg, ARG_NO);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NO);
    return errCode;
  }

  nearDescr.exprP = arg[0].v.exprP;

  switch (entType) {
    case DE_ATOM:
      DhApplyAtom(refP, nearAtom, &nearDescr);
      break;
    case DE_BOND:
      DhApplyBond(refP, nearBond, &nearDescr);
      break;
    case DE_DIST:
      DhApplyDist(refP, nearDist, &nearDescr);
      break;
    case DE_PRIM:
      PrimApply(PT_ALL, refP, nearPrim, &nearDescr);
      break;
  }

  ArgCleanup(arg, ARG_NO);

  GraphMolChanged(PROP_SELECTED);
  GraphRedrawNeeded();

  return EC_OK;
}
