/*
************************************************************************
*
*   GraphDobj.c - interaction with draw objects
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/graph/SCCS/s.GraphDobj.c
*   SCCS identification       : 1.5
*
************************************************************************
*/

#include <graph_dobj.h>

#include <stdio.h>
#include <string.h>

#include <sg.h>
#include <par_names.h>
#include <par_hand.h>
#include <prim_hand.h>
#include <prim_sel.h>
#include <graph_draw.h>
#include <graph_input.h>
#include <attr_struc.h>

#define HANDLE_SIZE 0.015f

/* The code in this module is partly similar to the one in GraphText.c.
   It might be nice to merge it somehow, but it's not completely
   trivial, and it's rather complex already on its own. */

typedef enum {
  OM_NONE,
  OM_MODIFY
} ObjModification;

typedef struct {
  PrimObjP primP;
  int pointInd;
  BOOL onOff;
  ObjModification mod;
  float xInit, yInit;
  float xInc, yInc;
} InputData;

static GraphDrawobjModifyF ModifyF = NULL;

static float BackColR, BackColG, BackColB;
static float ColR, ColG, ColB;
static float ZoomFact, DepthFact;
static BOOL DepthSortOn, HiddenOn, AntiAliasingOn;

static void
getPar(void)
{
  BackColR = (float) ParGetDoubleArrVal(PN_BACK_COLOR, 0);
  BackColG = (float) ParGetDoubleArrVal(PN_BACK_COLOR, 1);
  BackColB = (float) ParGetDoubleArrVal(PN_BACK_COLOR, 2);

  if (BackColR + BackColG + BackColB < 1.5f) {
    ColR = 1.0f;
    ColG = 1.0f;
    ColB = 1.0f;
  } else {
    ColR = 0.0f;
    ColG = 0.0f;
    ColB = 0.0f;
  }

  ZoomFact = (float) ParGetDoubleVal(PN_ZOOM_FACT);
  DepthFact = SgGetDepthFact(0.0f);

  DepthSortOn = ParGetIntArrVal(PN_RENDERING, SG_FEATURE_DEPTH_SORT);
  HiddenOn = ParGetIntArrVal(PN_RENDERING, SG_FEATURE_HIDDEN);
  AntiAliasingOn = ParGetIntArrVal(PN_RENDERING, SG_FEATURE_ANTI_ALIASING);
}

static void
getTransfPos(Vec3 x, PrimObjP primP, int ind, float xInc, float yInc)
{
  Vec3 dx;

  PrimGetPoint(primP, ind, x, dx);
  DhTransfVec(x, PrimGetMol(primP));
  Vec3Scale(x, ZoomFact);
  x[0] += DepthFact * (dx[0] + xInc);
  x[1] += DepthFact * (dx[1] + yInc);
  x[2] += dx[2];
}

static void
checkBox(PrimObjP primP, void *clientData)
{
  InputData *dataP = clientData;
  BOOL is2D;
  int pointNo, ind;
  Vec3 x, dx, x0;
  float boxSize, fact;

  if (dataP->primP != NULL)
    return;

  is2D = (PrimGetMol(primP) == NULL);
  pointNo = PrimGetPointNo(primP);

  for (ind = 0; ind < pointNo; ind++) {
    if (is2D) {
      PrimGetPoint(primP, ind, x, dx);
      x0[0] = x[0] + dx[0];
      x0[1] = x[1] + dx[1];
      boxSize = HANDLE_SIZE;
    } else {
      getTransfPos(x0, primP, ind, 0.0f, 0.0f);
      fact = SgGetDepthFact(x0[2]);
      x0[0] /= fact;
      x0[1] /= fact;
      boxSize = fact * HANDLE_SIZE;
    }

    if (dataP->xInit > x0[0] - boxSize &&
	dataP->xInit < x0[0] + boxSize &&
	dataP->yInit > x0[1] - boxSize &&
	dataP->yInit < x0[1] + boxSize) {
      dataP->primP = primP;
      dataP->pointInd = ind;
      break;
    }
  }
}

static void
drawBox2D(float x, float y, float boxSize)
{
  float ll[2], lr[2], ur[2], ul[2];

  ll[0] = x - boxSize;
  ll[1] = y - boxSize;

  lr[0] = x + boxSize;
  lr[1] = y - boxSize;

  ur[0] = x + boxSize;
  ur[1] = y + boxSize;

  ul[0] = x - boxSize;
  ul[1] = y + boxSize;

  SgDrawLine2D(ll, lr);
  SgDrawLine2D(lr, ur);
  SgDrawLine2D(ur, ul);
  SgDrawLine2D(ul, ll);
}

static void
drawBox3D(float x, float y, float z, float boxSize)
{
  Vec3 ll, lr, ur, ul;

  ll[0] = x - boxSize;
  ll[1] = y - boxSize;
  ll[2] = z;

  lr[0] = x + boxSize;
  lr[1] = y - boxSize;
  lr[2] = z;

  ur[0] = x + boxSize;
  ur[1] = y + boxSize;
  ur[2] = z;

  ul[0] = x - boxSize;
  ul[1] = y + boxSize;
  ul[2] = z;

  SgDrawLine(ll, lr);
  SgDrawLine(lr, ur);
  SgDrawLine(ur, ul);
  SgDrawLine(ul, ll);
}

static void
updateObjBox(void *clientData)
{
  InputData *dataP = clientData;
  PrimObjP primP = dataP->primP;
  Vec3 x, dx, x0;

  SgSetLight(SG_LIGHT_OFF);
  SgSetLineWidth(0.0f);
  SgSetLineStyle(SG_LINE_SOLID);

  if (dataP->onOff)
    SgSetColor(ColR, ColG, ColB);
  else
    SgSetColor(BackColR, BackColG, BackColB);

  if (PrimGetMol(primP) == NULL) {
    PrimGetPoint(primP, dataP->pointInd, x, dx);
    x0[0] = x[0] + dx[0];
    x0[1] = x[1] + dx[1];

    if (dataP->mod == OM_MODIFY) {
      x0[0] += dataP->xInc;
      x0[1] += dataP->yInc;
    }

    drawBox2D(x0[0], x0[1], HANDLE_SIZE);
  } else {
    if (dataP->mod == OM_MODIFY)
      getTransfPos(x0, primP, dataP->pointInd, dataP->xInc, dataP->yInc);
    else
      getTransfPos(x0, primP, dataP->pointInd, 0.0f, 0.0f);

    drawBox3D(x0[0], x0[1], x0[2], HANDLE_SIZE * SgGetDepthFact(x0[2]));
  }
}

static void
updatePrimBox(PrimObjP primP, void *clientData)
{
  InputData *dataP = clientData;
  PrimObjP origPrimP = dataP->primP;

  dataP->primP = primP;
  GraphDrawOverlay(updateObjBox, dataP);
  dataP->primP = origPrimP;
}

static void
switchAntiAliasing(BOOL onOff)
{
  if (onOff) {
    if (AntiAliasingOn)
      SgSetFeature(SG_FEATURE_ANTI_ALIASING, TRUE);
  } else {
    SgSetFeature(SG_FEATURE_ANTI_ALIASING, FALSE);
  }
}

static void
switchFront(BOOL onOff)
{
  if (onOff) {
    SgSetBuffer(SG_BUF_FRONT);
    SgSetFeature(SG_FEATURE_DEPTH_SORT, FALSE);
    SgSetFeature(SG_FEATURE_HIDDEN, FALSE);
  } else {
    SgSetBuffer(SG_BUF_BACK);
    if (DepthSortOn)
      SgSetFeature(SG_FEATURE_DEPTH_SORT, TRUE);
    if (HiddenOn)
      SgSetFeature(SG_FEATURE_HIDDEN, TRUE);
  }

  switchAntiAliasing(! onOff);
}

static BOOL
inputObj(char *act, float x, float y,
    PuMouseCBStruc *callP, void *clientData)
{
  static InputData data;
  PropRefP refP;

  if (callP->button != PU_MB_2)
    return FALSE;

  refP = PropGetRef(PROP_SELECTED, FALSE);

  if (strcmp(act, "press") == 0) {
    getPar();

    data.xInit = x;
    data.yInit = y;
    data.primP = NULL;
    PrimApply(PT_DRAWOBJ, PropGetRef(PROP_ALL, FALSE), checkBox, &data);

    if (data.primP == NULL)
      return FALSE;

    data.mod = OM_NONE;
    data.onOff = PrimGetProp(refP, data.primP);

    switchFront(TRUE);
  } else if (strcmp(act, "drag") == 0) {
    if (data.onOff) {
      data.onOff = FALSE;
      PrimApply(PT_DRAWOBJ, refP, updatePrimBox, &data);

      data.mod = OM_MODIFY;
      data.xInc = x - data.xInit;
      data.yInc = y - data.yInit;

      data.onOff = TRUE;
      PrimApply(PT_DRAWOBJ, refP, updatePrimBox, &data);
    }
  } else if (strcmp(act, "release") == 0) {
    if (data.mod == OM_NONE && callP->doSel) {
      if (data.onOff)
	PrimSel(data.primP, SK_REMOVE);
      else
	PrimSel(data.primP, SK_ADD);
    }

    switchFront(FALSE);

    if (data.mod == OM_MODIFY && ModifyF != NULL)
      ModifyF(data.pointInd, data.xInc, data.yInc);
  }

  return TRUE;
}

static void
drawObjBox(PrimObjP primP, void *clientData)
{
  int pointNo, ind;
  InputData data;

  pointNo = PrimGetPointNo(primP);

  for (ind = 0; ind < pointNo; ind++) {
    data.primP = primP;
    data.pointInd = ind;
    data.mod = OM_NONE;
    data.onOff = TRUE;
    updateObjBox(&data);
  }
}

static void
drawObjBoxes(void *clientData)
{
  getPar();

  switchAntiAliasing(FALSE);
  PrimApply(PT_DRAWOBJ, PropGetRef(PROP_SELECTED, FALSE), drawObjBox, NULL);
  switchAntiAliasing(TRUE);
}

static void
objToggleCB(PrimObjP primP, PropRefP refP, BOOL onOff, void *clientData)
{
  int pointNo, ind;
  InputData data;

  getPar();
  switchFront(TRUE);
  pointNo = PrimGetPointNo(primP);

  for (ind = 0; ind < pointNo; ind++) {
    data.primP = primP;
    data.pointInd = ind;
    data.mod = OM_NONE;
    data.onOff = onOff;
    updateObjBox(&data);

    GraphDrawOverlay(updateObjBox, &data);
  }

  switchFront(FALSE);
}

void
GraphDrawobjInit(void)
{
  GraphInputAdd(inputObj, NULL);
  GraphAddOverlay(drawObjBoxes, NULL);
  PrimAddPropCB(PT_DRAWOBJ, PropGetRef(PROP_SELECTED, FALSE),
      objToggleCB, NULL);
}

void
GraphDrawobjSetModifyFunc(GraphDrawobjModifyF modifyF)
{
  ModifyF = modifyF;
}
