/*
************************************************************************
*
*   ExLine.c - line style commands
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdattr/SCCS/s.ExLine.c
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

static int LineStyle = 0;
static float LineWidth = 0.0f;

typedef struct {
  SgLineStyle style;
  ExprP widthExprP;
  float width;
} LineDescr;

static void
evalWidth(void *entP, LineDescr *lineP)
{
  ExprRes exprRes;

  ExprEval(entP, lineP->widthExprP, &exprRes);
  if (exprRes.resType == ER_INT)
    lineP->width = (float) exprRes.u.intVal;
  else
    lineP->width = exprRes.u.floatVal;
}

static AttrP
changeLine(AttrP attrP, LineDescr *lineP)
{
  struct AttrS attr;
  
  if (lineP->width < 0.0f)
    lineP->width = 0.0f;

  LineWidth = lineP->width;

  AttrCopy(&attr, attrP);
  AttrReturn(attrP);
  attr.lineStyle = lineP->style;
  attr.lineWidth = lineP->width;
  return AttrGet(&attr);
}

static void
setAtomLine(DhAtomP atomP, void *clientData)
{
  evalWidth(atomP, clientData);
  DhAtomSetAttr(atomP, changeLine(DhAtomGetAttr(atomP), clientData));
}

static void
setBondLine(DhBondP bondP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  evalWidth(bondP, clientData);
  DhBondSetAttr(bondP, changeLine(DhBondGetAttr(bondP), clientData));
}

static void
setDistLine(DhDistP distP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  evalWidth(distP, clientData);
  DhDistSetAttr(distP, changeLine(DhDistGetAttr(distP), clientData));
}

static void
setPrimLine(PrimObjP primP, void *clientData)
{
  evalWidth(primP, clientData);
  PrimSetAttr(primP, changeLine(PrimGetAttr(primP), clientData));
}

#define ARG_NUM 2

ErrCode
ExLine(char *cmd)
{
  DataEntityType entType;
  AttrP attrP;
  struct AttrS attr;
  ArgDescr arg[ARG_NUM];
  EnumEntryDescr enumEntry[2];
  ErrCode errCode;
  LineDescr lineDescr;
  PropRefP refP;

  if (strcmp(cmd, "LineAtom") == 0) {
    entType = DE_ATOM;
  } else if (strcmp(cmd, "LineBond") == 0) {
    entType = DE_BOND;
  } else if (strcmp(cmd, "LineDist") == 0) {
    entType = DE_DIST;
  } else if (strcmp(cmd, "LinePrim") == 0) {
    entType = DE_PRIM;
  } else {
    entType = DE_NONE;
    attrP = AttrGetInit();
    AttrCopy(&attr, attrP);
    AttrReturn(attrP);
    if (attr.lineStyle == SG_LINE_SOLID)
      LineStyle = 0;
    else
      LineStyle = 1;
    LineWidth = attr.lineWidth;
  }

  arg[0].type = AT_ENUM;
  arg[1].type = AT_DOUBLE;

  ArgInit(arg, ARG_NUM);

  enumEntry[0].str = "solid";
  enumEntry[0].onOff = FALSE;
  enumEntry[1].str = "dashed";
  enumEntry[1].onOff = FALSE;

  enumEntry[LineStyle].onOff = TRUE;

  arg[0].prompt = "Line Style";
  arg[0].u.enumD.entryP = enumEntry;
  arg[0].u.enumD.n = 2;
  arg[0].v.intVal = LineStyle;

  arg[1].prompt = "Line Width";
  arg[1].entType = entType;
  arg[1].v.doubleVal = LineWidth;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  LineStyle = arg[0].v.intVal;

  if (LineStyle == 0)
    lineDescr.style = SG_LINE_SOLID;
  else
    lineDescr.style = SG_LINE_DASHED;

  lineDescr.widthExprP = arg[1].v.exprP;

  refP = PropGetRef(PROP_SELECTED, FALSE);

  switch (entType) {
    case DE_ATOM:
      DhApplyAtom(refP, setAtomLine, &lineDescr);
      break;
    case DE_BOND:
      DhApplyBond(refP, setBondLine, &lineDescr);
      break;
    case DE_DIST:
      DhApplyDist(refP, setDistLine, &lineDescr);
      break;
    case DE_PRIM:
      PrimApply(PT_ALL, refP, setPrimLine, &lineDescr);
      break;
    case DE_NONE:
      LineWidth = (float) arg[1].v.doubleVal;
      attr.lineStyle = LineStyle;
      attr.lineWidth = LineWidth;
      AttrSetInit(&attr);
      break;
  }

  ArgCleanup(arg, ARG_NUM);

  GraphMolChanged(PROP_SELECTED);
  GraphRedrawNeeded();

  return EC_OK;
}
