/*
************************************************************************
*
*   ExMaterial.c - Material* commands
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdattr/SCCS/s.ExMaterial.c
*   SCCS identification       : 1.4
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
  ExprP ambExprP, diffExprP, specExprP, shinExprP;
  ExprP reflExprP, alphaExprP, refrExprP;
  float amb, diff, spec, shin, refl, alpha, refr;
} MatDescr;

static void
getMat(AttrP attrP, MatDescr *matP)
{
  if (matP->entNo == 0) {
    matP->amb = attrP->ambCoeff;
    matP->diff = attrP->diffCoeff;
    matP->spec = attrP->specCoeff;
    matP->shin = attrP->shininess;
    matP->refl = attrP->reflect;
    matP->alpha = attrP->alpha;
    matP->refr = attrP->refract;
  } else {
    if (matP->amb != attrP->ambCoeff)
      matP->amb = FLOAT_AS_IS;
    if (matP->diff != attrP->diffCoeff)
      matP->diff = FLOAT_AS_IS;
    if (matP->spec != attrP->specCoeff)
      matP->spec = FLOAT_AS_IS;
    if (matP->shin != attrP->shininess)
      matP->shin = FLOAT_AS_IS;
    if (matP->refl != attrP->reflect)
      matP->refl = FLOAT_AS_IS;
    if (matP->alpha != attrP->alpha)
      matP->alpha = FLOAT_AS_IS;
    if (matP->refr != attrP->refract)
      matP->refr = FLOAT_AS_IS;
  }

  matP->entNo++;
}

static void
getMatAtom(DhAtomP atomP, void *clientData)
{
  getMat(DhAtomGetAttr(atomP), clientData);
}

static void
getMatBond(DhBondP bondP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  getMat(DhBondGetAttr(bondP), clientData);
}

static void
getMatDist(DhDistP distP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  getMat(DhDistGetAttr(distP), clientData);
}

static void
getMatPrim(PrimObjP primP, void *clientData)
{
  getMat(PrimGetAttr(primP), clientData);
}

static void
evalMat(void *entP, MatDescr *matP)
{
  ExprRes exprRes;

  ExprEval(entP, matP->ambExprP, &exprRes);
  if (exprRes.resType == ER_INT)
    matP->amb = (float) exprRes.u.intVal;
  else
    matP->amb = exprRes.u.floatVal;

  ExprEval(entP, matP->diffExprP, &exprRes);
  if (exprRes.resType == ER_INT)
    matP->diff = (float) exprRes.u.intVal;
  else
    matP->diff = exprRes.u.floatVal;

  ExprEval(entP, matP->specExprP, &exprRes);
  if (exprRes.resType == ER_INT)
    matP->spec = (float) exprRes.u.intVal;
  else
    matP->spec = exprRes.u.floatVal;

  ExprEval(entP, matP->shinExprP, &exprRes);
  if (exprRes.resType == ER_INT)
    matP->shin = (float) exprRes.u.intVal;
  else
    matP->shin = exprRes.u.floatVal;

  ExprEval(entP, matP->reflExprP, &exprRes);
  if (exprRes.resType == ER_INT)
    matP->refl = (float) exprRes.u.intVal;
  else
    matP->refl = exprRes.u.floatVal;

  ExprEval(entP, matP->alphaExprP, &exprRes);
  if (exprRes.resType == ER_INT)
    matP->alpha = (float) exprRes.u.intVal;
  else
    matP->alpha = exprRes.u.floatVal;

  ExprEval(entP, matP->refrExprP, &exprRes);
  if (exprRes.resType == ER_INT)
    matP->refr = (float) exprRes.u.intVal;
  else
    matP->refr = exprRes.u.floatVal;
}

static AttrP
changeMat(AttrP attrP, MatDescr *matP)
{
  struct AttrS attr;

  AttrCopy(&attr, attrP);
  AttrReturn(attrP);
  if (matP->amb >= 0.0f)
    attr.ambCoeff = matP->amb;
  if (matP->diff >= 0.0f)
    attr.diffCoeff = matP->diff;
  if (matP->spec >= 0.0f)
    attr.specCoeff = matP->spec;
  if (matP->shin > 0.0f)
    attr.shininess = matP->shin;
  if (matP->refl >= 0.0f)
    attr.reflect = matP->refl;
  if (matP->alpha >= 0.0f)
    attr.alpha = matP->alpha;
  if (matP->refr > 0.0f)
    attr.refract = matP->refr;
  return AttrGet(&attr);
}

static void
matAtom(DhAtomP atomP, void *clientData)
{
  evalMat(atomP, clientData);
  DhAtomSetAttr(atomP, changeMat(DhAtomGetAttr(atomP), clientData));
}

static void
matBond(DhBondP bondP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  evalMat(bondP, clientData);
  DhBondSetAttr(bondP, changeMat(DhBondGetAttr(bondP), clientData));
}

static void
matDist(DhDistP distP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  evalMat(distP, clientData);
  DhDistSetAttr(distP, changeMat(DhDistGetAttr(distP), clientData));
}

static void
matPrim(PrimObjP primP, void *clientData)
{
  evalMat(primP, clientData);
  PrimSetAttr(primP, changeMat(PrimGetAttr(primP), clientData));
}

#define MAT_PROP_NO 7

ErrCode
ExMaterial(char *cmd)
{
  DataEntityType entType;
  ArgDescr arg[MAT_PROP_NO];
  AttrP attrP;
  struct AttrS attr;
  ErrCode errCode;
  MatDescr matDescr;
  PropRefP refP;
  int i;

  if (strcmp(cmd, "MaterialAtom") == 0)
    entType = DE_ATOM;
  else if (strcmp(cmd, "MaterialBond") == 0)
    entType = DE_BOND;
  else if (strcmp(cmd, "MaterialDist") == 0)
    entType = DE_DIST;
  else if (strcmp(cmd, "MaterialPrim") == 0)
    entType = DE_PRIM;
  else
    entType = DE_NONE;

  for (i = 0; i < MAT_PROP_NO; i++)
    arg[i].type = AT_DOUBLE;

  ArgInit(arg, MAT_PROP_NO);

  arg[0].prompt = "Ambient Coeff";
  arg[1].prompt = "Diffuse Coeff";
  arg[2].prompt = "Specular Coeff";
  arg[3].prompt = "Shininess";
  arg[4].prompt = "Reflection";
  arg[5].prompt = "Opacity";
  arg[6].prompt = "Refraction";

  for (i = 0; i < MAT_PROP_NO; i++)
    arg[i].entType = entType;

  matDescr.entNo = 0;
  matDescr.amb = FLOAT_AS_IS;
  matDescr.diff = FLOAT_AS_IS;
  matDescr.spec = FLOAT_AS_IS;
  matDescr.shin = FLOAT_AS_IS;
  matDescr.refl = FLOAT_AS_IS;
  matDescr.alpha = FLOAT_AS_IS;
  matDescr.refr = FLOAT_AS_IS;

  refP = PropGetRef(PROP_SELECTED, FALSE);

  switch (entType) {
    case DE_ATOM:
      DhApplyAtom(refP, getMatAtom, &matDescr);
      break;
    case DE_BOND:
      DhApplyBond(refP, getMatBond, &matDescr);
      break;
    case DE_DIST:
      DhApplyDist(refP, getMatDist, &matDescr);
      break;
    case DE_PRIM:
      PrimApply(PT_ALL, refP, getMatPrim, &matDescr);
      break;
    case DE_NONE:
      attrP = AttrGetInit();
      AttrCopy(&attr, attrP);
      AttrReturn(attrP);
      getMat(&attr, &matDescr);
      break;
  }

  arg[0].v.doubleVal = matDescr.amb;
  arg[1].v.doubleVal = matDescr.diff;
  arg[2].v.doubleVal = matDescr.spec;
  arg[3].v.doubleVal = matDescr.shin;
  arg[4].v.doubleVal = matDescr.refl;
  arg[5].v.doubleVal = matDescr.alpha;
  arg[6].v.doubleVal = matDescr.refr;

  errCode = ArgGet(arg, MAT_PROP_NO);
  if (errCode != EC_OK) {
    ArgCleanup(arg, MAT_PROP_NO);
    return errCode;
  }

  matDescr.ambExprP = arg[0].v.exprP;
  matDescr.diffExprP = arg[1].v.exprP;
  matDescr.specExprP = arg[2].v.exprP;
  matDescr.shinExprP = arg[3].v.exprP;
  matDescr.reflExprP = arg[4].v.exprP;
  matDescr.alphaExprP = arg[5].v.exprP;
  matDescr.refrExprP = arg[6].v.exprP;

  switch (entType) {
    case DE_ATOM:
      DhApplyAtom(refP, matAtom, &matDescr);
      break;
    case DE_BOND:
      DhApplyBond(refP, matBond, &matDescr);
      break;
    case DE_DIST:
      DhApplyDist(refP, matDist, &matDescr);
      break;
    case DE_PRIM:
      PrimApply(PT_ALL, refP, matPrim, &matDescr);
      break;
    case DE_NONE:
      attr.ambCoeff = (float) arg[0].v.doubleVal;
      attr.diffCoeff = (float) arg[1].v.doubleVal;
      attr.specCoeff = (float) arg[2].v.doubleVal;
      attr.shininess = (float) arg[3].v.doubleVal;
      attr.reflect = (float) arg[4].v.doubleVal;
      attr.alpha = (float) arg[5].v.doubleVal;
      attr.refract = (float) arg[6].v.doubleVal;
      AttrSetInit(&attr);
      break;
  }

  ArgCleanup(arg, MAT_PROP_NO);

  GraphMolChanged(PROP_SELECTED);
  GraphRedrawNeeded();

  return EC_OK;
}
