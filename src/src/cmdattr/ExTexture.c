/*
************************************************************************
*
*   ExTexture.c - Texture* commands
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdattr/SCCS/s.ExTexture.c
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
#define TEX_AS_IS "as is"

typedef struct {
  int entNo;
  char name[SG_TEXTURE_NAME_SIZE];
  ExprP texScExprP, bumpExprP, bumpScExprP;
  float texSc, bump, bumpSc;
} TexDescr;

static void
getTex(AttrP attrP, TexDescr *texP)
{
  if (texP->entNo == 0) {
    (void) strcpy(texP->name, attrP->texture);
    texP->texSc = attrP->textureScale;
    texP->bump = attrP->bumpDepth;
    texP->bumpSc = attrP->bumpScale;
  } else {
    if (strcmp(texP->name, attrP->texture) != 0)
      (void) strcpy(texP->name, TEX_AS_IS);
    if (texP->texSc != attrP->textureScale)
      texP->texSc = FLOAT_AS_IS;
    if (texP->bump != attrP->bumpDepth)
      texP->bump = FLOAT_AS_IS;
    if (texP->bumpSc != attrP->bumpScale)
      texP->bumpSc = FLOAT_AS_IS;
  }

  texP->entNo++;
}

static void
getTexAtom(DhAtomP atomP, void *clientData)
{
  getTex(DhAtomGetAttr(atomP), clientData);
}

static void
getTexBond(DhBondP bondP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  getTex(DhBondGetAttr(bondP), clientData);
}

static void
getTexDist(DhDistP distP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  getTex(DhDistGetAttr(distP), clientData);
}

static void
getTexPrim(PrimObjP primP, void *clientData)
{
  getTex(PrimGetAttr(primP), clientData);
}

static void
evalTex(void *entP, TexDescr *texP)
{
  ExprRes exprRes;

  ExprEval(entP, texP->texScExprP, &exprRes);
  if (exprRes.resType == ER_INT)
    texP->texSc = (float) exprRes.u.intVal;
  else
    texP->texSc = exprRes.u.floatVal;

  ExprEval(entP, texP->bumpExprP, &exprRes);
  if (exprRes.resType == ER_INT)
    texP->bump = (float) exprRes.u.intVal;
  else
    texP->bump = exprRes.u.floatVal;

  ExprEval(entP, texP->bumpScExprP, &exprRes);
  if (exprRes.resType == ER_INT)
    texP->bumpSc = (float) exprRes.u.intVal;
  else
    texP->bumpSc = exprRes.u.floatVal;
}

static AttrP
changeTex(AttrP attrP, TexDescr *texP)
{
  struct AttrS attr;

  AttrCopy(&attr, attrP);
  AttrReturn(attrP);
  if (strcmp(texP->name, TEX_AS_IS) != 0)
    (void) strcpy(attr.texture, texP->name);
  if (texP->texSc > 0.0f)
    attr.textureScale = texP->texSc;
  if (texP->bump >= 0.0f)
    attr.bumpDepth = texP->bump;
  if (texP->bumpSc > 0.0f)
    attr.bumpScale = texP->bumpSc;
  return AttrGet(&attr);
}

static void
texAtom(DhAtomP atomP, void *clientData)
{
  evalTex(atomP, clientData);
  DhAtomSetAttr(atomP, changeTex(DhAtomGetAttr(atomP), clientData));
}

static void
texBond(DhBondP bondP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  evalTex(bondP, clientData);
  DhBondSetAttr(bondP, changeTex(DhBondGetAttr(bondP), clientData));
}

static void
texDist(DhDistP distP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  evalTex(distP, clientData);
  DhDistSetAttr(distP, changeTex(DhDistGetAttr(distP), clientData));
}

static void
texPrim(PrimObjP primP, void *clientData)
{
  evalTex(primP, clientData);
  PrimSetAttr(primP, changeTex(PrimGetAttr(primP), clientData));
}

#define ARG_NO 4

ErrCode
ExTexture(char *cmd)
{
  DataEntityType entType;
  ArgDescr arg[ARG_NO];
  AttrP attrP;
  struct AttrS attr;
  ErrCode errCode;
  TexDescr texDescr;
  PropRefP refP;

  if (strcmp(cmd, "TextureAtom") == 0)
    entType = DE_ATOM;
  else if (strcmp(cmd, "TextureBond") == 0)
    entType = DE_BOND;
  else if (strcmp(cmd, "TextureDist") == 0)
    entType = DE_DIST;
  else if (strcmp(cmd, "TexturePrim") == 0)
    entType = DE_PRIM;
  else
    entType = DE_NONE;

  arg[0].type = AT_STR;
  arg[1].type = AT_DOUBLE;
  arg[2].type = AT_DOUBLE;
  arg[3].type = AT_DOUBLE;

  ArgInit(arg, ARG_NO);

  arg[0].prompt = "Name";
  arg[1].prompt = "Scale";
  arg[2].prompt = "Bumpiness";
  arg[3].prompt = "Bump Scale";

  arg[1].entType = entType;
  arg[2].entType = entType;
  arg[3].entType = entType;

  texDescr.entNo = 0;
  texDescr.name[0] = '\0';
  texDescr.texSc = FLOAT_AS_IS;
  texDescr.bump = FLOAT_AS_IS;
  texDescr.bumpSc = FLOAT_AS_IS;

  refP = PropGetRef(PROP_SELECTED, FALSE);

  switch (entType) {
    case DE_ATOM:
      DhApplyAtom(refP, getTexAtom, &texDescr);
      break;
    case DE_BOND:
      DhApplyBond(refP, getTexBond, &texDescr);
      break;
    case DE_DIST:
      DhApplyDist(refP, getTexDist, &texDescr);
      break;
    case DE_PRIM:
      PrimApply(PT_ALL, refP, getTexPrim, &texDescr);
      break;
    case DE_NONE:
      attrP = AttrGetInit();
      AttrCopy(&attr, attrP);
      AttrReturn(attrP);
      getTex(&attr, &texDescr);
      break;
  }

  DStrAssignStr(arg[0].v.strVal, texDescr.name);
  arg[1].v.doubleVal = texDescr.texSc;
  arg[2].v.doubleVal = texDescr.bump;
  arg[3].v.doubleVal = texDescr.bumpSc;

  errCode = ArgGet(arg, ARG_NO);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NO);
    return errCode;
  }

  DStrPart(arg[0].v.strVal, 0, SG_TEXTURE_NAME_SIZE - 2);
  (void) strcpy(texDescr.name, DStrToStr(arg[0].v.strVal));
  texDescr.texScExprP = arg[1].v.exprP;
  texDescr.bumpExprP = arg[2].v.exprP;
  texDescr.bumpScExprP = arg[3].v.exprP;

  switch (entType) {
    case DE_ATOM:
      DhApplyAtom(refP, texAtom, &texDescr);
      break;
    case DE_BOND:
      DhApplyBond(refP, texBond, &texDescr);
      break;
    case DE_DIST:
      DhApplyDist(refP, texDist, &texDescr);
      break;
    case DE_PRIM:
      PrimApply(PT_ALL, refP, texPrim, &texDescr);
      break;
    case DE_NONE:
      (void) strcpy(attr.texture, texDescr.name);
      attr.textureScale = (float) arg[1].v.doubleVal;
      attr.bumpDepth = (float) arg[2].v.doubleVal;
      attr.bumpScale = (float) arg[3].v.doubleVal;
      break;
  }

  ArgCleanup(arg, ARG_NO);

  GraphMolChanged(PROP_SELECTED);
  GraphRedrawNeeded();

  return EC_OK;
}
