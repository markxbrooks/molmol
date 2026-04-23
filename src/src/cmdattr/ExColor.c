/*
************************************************************************
*
*   ExColor.c - color setting commands
*
*   Copyright (c) 1994
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdattr/SCCS/s.ExColor.c
*   SCCS identification       : 1.5
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
  ExprP rExprP, gExprP, bExprP;
  float r, g, b;
} ColorDescr;

static float
normCol(float f)
{
  if (f < 0.0f)
    return 0.0f;
  if (f > 1.0f)
    return 1.0f;
  return f;
}

static void
evalColor(void *entP, ColorDescr *colorP)
{
  ExprRes exprRes;

  ExprEval(entP, colorP->rExprP, &exprRes);
  if (exprRes.resType == ER_INT)
    colorP->r = normCol((float) exprRes.u.intVal);
  else
    colorP->r = normCol(exprRes.u.floatVal);

  ExprEval(entP, colorP->gExprP, &exprRes);
  if (exprRes.resType == ER_INT)
    colorP->g = normCol((float) exprRes.u.intVal);
  else
    colorP->g = normCol(exprRes.u.floatVal);

  ExprEval(entP, colorP->bExprP, &exprRes);
  if (exprRes.resType == ER_INT)
    colorP->b = normCol((float) exprRes.u.intVal);
  else
    colorP->b = normCol(exprRes.u.floatVal);
}

static AttrP
changeColor(AttrP attrP, ColorDescr *colorP)
{
  struct AttrS attr;

  AttrCopy(&attr, attrP);
  AttrReturn(attrP);
  attr.colR = colorP->r;
  attr.colG = colorP->g;
  attr.colB = colorP->b;
  return AttrGet(&attr);
}

static void
colorAtom(DhAtomP atomP, void *clientData)
{
  evalColor(atomP, clientData);
  DhAtomSetAttr(atomP, changeColor(DhAtomGetAttr(atomP), clientData));
}

static void
colorBond(DhBondP bondP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  evalColor(bondP, clientData);
  DhBondSetAttr(bondP, changeColor(DhBondGetAttr(bondP), clientData));
}

static void
colorDist(DhDistP distP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  evalColor(distP, clientData);
  DhDistSetAttr(distP, changeColor(DhDistGetAttr(distP), clientData));
}

static void
colorPrim(PrimObjP primP, void *clientData)
{
  evalColor(primP, clientData);
  PrimSetAttr(primP, changeColor(PrimGetAttr(primP), clientData));
}

static AttrP
changeTint(AttrP attrP, ColorDescr *colorP)
{
  struct AttrS attr;

  AttrCopy(&attr, attrP);
  AttrReturn(attrP);
  attr.secColR = colorP->r;
  attr.secColG = colorP->g;
  attr.secColB = colorP->b;
  return AttrGet(&attr);
}

static void
tintDist(DhDistP distP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  evalColor(distP, clientData);
  DhDistSetAttr(distP, changeTint(DhDistGetAttr(distP), clientData));
}

static void
tintPrim(PrimObjP primP, void *clientData)
{
  evalColor(primP, clientData);
  PrimSetAttr(primP, changeTint(PrimGetAttr(primP), clientData));
}

ErrCode
ExColor(char *cmd)
{
  DataEntityType entType;
  AttrP attrP;
  struct AttrS attr;
  float col[3];
  ArgDescr arg[3];
  ErrCode errCode;
  ColorDescr colDescr;
  PropRefP refP;
  int i;

  for (i = 0; i < 3; i++)
    col[i] = 0.0f;

  if (strcmp(cmd, "ColorAtom") == 0) {
    entType = DE_ATOM;
  } else if (strcmp(cmd, "ColorBond") == 0) {
    entType = DE_BOND;
  } else if (strcmp(cmd, "ColorDist") == 0 || strcmp(cmd, "TintDist") == 0) {
    entType = DE_DIST;
  } else if (strcmp(cmd, "ColorPrim") == 0 || strcmp(cmd, "TintPrim") == 0) {
    entType = DE_PRIM;
  } else {
    entType = DE_NONE;
    attrP = AttrGetInit();
    AttrCopy(&attr, attrP);
    AttrReturn(attrP);
    if (cmd[0] == 'T') {
      col[0] = attr.secColR;
      col[1] = attr.secColG;
      col[2] = attr.secColB;
    } else {
      col[0] = attr.colR;
      col[1] = attr.colG;
      col[2] = attr.colB;
    }
  }

  arg[0].type = AT_DOUBLE;
  arg[1].type = AT_DOUBLE;
  arg[2].type = AT_DOUBLE;

  ArgInit(arg, 3);

  arg[0].prompt = "Red";
  arg[1].prompt = "Green";
  arg[2].prompt = "Blue";

  for (i = 0; i < 3; i++) {
    arg[i].entType = entType;
    arg[i].v.doubleVal = (float) col[i];
  }

  errCode = ArgGet(arg, 3);
  if (errCode != EC_OK) {
    ArgCleanup(arg, 3);
    return errCode;
  }

  colDescr.rExprP = arg[0].v.exprP;
  colDescr.gExprP = arg[1].v.exprP;
  colDescr.bExprP = arg[2].v.exprP;

  refP = PropGetRef(PROP_SELECTED, FALSE);

  switch (entType) {
    case DE_ATOM:
      DhApplyAtom(refP, colorAtom, &colDescr);
      break;
    case DE_BOND:
      DhApplyBond(refP, colorBond, &colDescr);
      break;
    case DE_DIST:
      if (cmd[0] == 'T')
	DhApplyDist(refP, tintDist, &colDescr);
      else
	DhApplyDist(refP, colorDist, &colDescr);
      break;
    case DE_PRIM:
      if (cmd[0] == 'T')
	PrimApply(PT_ALL, refP, tintPrim, &colDescr);
      else
	PrimApply(PT_ALL, refP, colorPrim, &colDescr);
      break;
    case DE_NONE:
      if (cmd[0] == 'T') {
	attr.secColR = (float) arg[0].v.doubleVal;
	attr.secColG = (float) arg[1].v.doubleVal;
	attr.secColB = (float) arg[2].v.doubleVal;
      } else {
	attr.colR = (float) arg[0].v.doubleVal;
	attr.colG = (float) arg[1].v.doubleVal;
	attr.colB = (float) arg[2].v.doubleVal;
      }
      AttrSetInit(&attr);
      break;
  }

  ArgCleanup(arg, 3);

  GraphMolChanged(PROP_SELECTED);
  GraphRedrawNeeded();

  return EC_OK;
}
