/*
************************************************************************
*
*   ExRadius.c - radius setting commands
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdattr/SCCS/s.ExRadius.c
*   SCCS identification       : 1.7
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

typedef struct {
  ExprP radExprP;
  float rad;
  BOOL changeShading;
} RadDescr;

static float CurrRad = 0.2f;

static void
evalRad(void *entP, RadDescr *radP)
{
  ExprRes exprRes;

  ExprEval(entP, radP->radExprP, &exprRes);
  if (exprRes.resType == ER_INT)
    radP->rad = (float) exprRes.u.intVal;
  else
    radP->rad = exprRes.u.floatVal;
}

static AttrP
changeRad(AttrP attrP, RadDescr *radP)
{
  struct AttrS attr;

  if (radP->rad < 0.0f)
    radP->rad = 0.0f;

  CurrRad = radP->rad;

  AttrCopy(&attr, attrP);
  AttrReturn(attrP);
  if (radP->changeShading) {
    if (radP->rad == 0.0f)
      attr.shadeModel = -1;
    else if (attr.radius == 0.0f)
      attr.shadeModel = SHADE_DEFAULT;
  }
  attr.radius = radP->rad;
  return AttrGet(&attr);
}

static void
atomRad(DhAtomP atomP, void *clientData)
{
  evalRad(atomP, clientData);
  DhAtomSetAttr(atomP, changeRad(DhAtomGetAttr(atomP), clientData));
}

static void
bondRad(DhBondP bondP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  evalRad(bondP, clientData);
  DhBondSetAttr(bondP, changeRad(DhBondGetAttr(bondP), clientData));
}

static void
distRad(DhDistP distP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  evalRad(distP, clientData);
  DhDistSetAttr(distP, changeRad(DhDistGetAttr(distP), clientData));
}

static void
primRad(PrimObjP primP, void *clientData)
{
  evalRad(primP, clientData);
  PrimSetAttr(primP, changeRad(PrimGetAttr(primP), clientData));
}

#define ARG_NUM 1

ErrCode
ExRadius(char *cmd)
{
  DataEntityType entType;
  AttrP attrP;
  struct AttrS attr;
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  RadDescr radDescr;
  PropRefP refP;

  if (strcmp(cmd, "RadiusAtom") == 0) {
    entType = DE_ATOM;
  } else if (strcmp(cmd, "RadiusBond") == 0) {
    entType = DE_BOND;
  } else if (strcmp(cmd, "RadiusDist") == 0) {
    entType = DE_DIST;
  } else if (strcmp(cmd, "RadiusPrim") == 0) {
    entType = DE_PRIM;
  } else {
    entType = DE_NONE;
    attrP = AttrGetInit();
    AttrCopy(&attr, attrP);
    AttrReturn(attrP);
    CurrRad = attr.radius;
  }

  arg[0].type = AT_DOUBLE;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Radius";
  arg[0].entType = entType;
  arg[0].v.doubleVal = CurrRad;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  radDescr.radExprP = arg[0].v.exprP;
  radDescr.changeShading = FALSE;

  refP = PropGetRef(PROP_SELECTED, FALSE);

  switch (entType) {
    case DE_ATOM:
      DhApplyAtom(refP, atomRad, &radDescr);
      break;
    case DE_BOND:
      DhApplyBond(refP, bondRad, &radDescr);
      break;
    case DE_DIST:
      DhApplyDist(refP, distRad, &radDescr);
      break;
    case DE_PRIM:
      radDescr.changeShading = TRUE;
      PrimApply(PT_ALL, refP, primRad, &radDescr);
      break;
    case DE_NONE:
      CurrRad = (float) arg[0].v.doubleVal;
      attr.radius = CurrRad;
      AttrSetInit(&attr);
      break;
  }

  ArgCleanup(arg, ARG_NUM);

  GraphMolChanged(PROP_SELECTED);
  GraphRedrawNeeded();

  return EC_OK;
}
