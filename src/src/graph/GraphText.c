/*
************************************************************************
*
*   GraphText.c - interaction with texts
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/graph/SCCS/s.GraphText.c
*   SCCS identification       : 1.13
*
************************************************************************
*/

#include <graph_text.h>

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

#define BORDER_FRACT 0.25f

typedef enum {
  TM_NONE,
  TM_MOVE,
  TM_RESIZE
} TestModification;

typedef struct {
  PrimObjP primP;
  float xText, yText;
  float wText, hText;
  BOOL onOff;
  TestModification mod;
  float xInit, yInit;
  float xInc, yInc, hInc;
  int xHand, yHand;
} InputData;

static GraphTextMoveF MoveF = NULL;
static GraphTextResizeF ResizeF = NULL;

static float BackColR, BackColG, BackColB;
static float ColR, ColG, ColB;
static float ZoomFact;
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

  DepthSortOn = ParGetIntArrVal(PN_RENDERING, SG_FEATURE_DEPTH_SORT);
  HiddenOn = ParGetIntArrVal(PN_RENDERING, SG_FEATURE_HIDDEN);
  AntiAliasingOn = ParGetIntArrVal(PN_RENDERING, SG_FEATURE_ANTI_ALIASING);
}

static void
getTransfPos(Vec3 x, PrimObjP primP)
{
  Vec3 dx;

  PrimGetPos(primP, x);
  PrimGetVec(primP, dx);

  DhTransfVec(x, PrimGetMol(primP));
  Vec3Scale(x, ZoomFact);
  x[2] += dx[2];
}

static float
getBorder(float w, float h)
{
  if (h < w)
    return BORDER_FRACT * h;
  else
    return BORDER_FRACT * w;
}

static void
checkBox(PrimObjP primP, void *clientData)
{
  InputData *dataP = clientData;
  char *str;
  float w, h, b;
  Vec3 x, dx;
  float ll2[2], ur2[2];
  Vec3 ll3, ur3;

  if (dataP->primP != NULL)
    return;

  str = PrimGetText(primP);
  h = PrimGetAttr(primP)->fontSize;
  PrimGetVec(primP, dx);

  if (PrimGetMol(primP) == NULL) {
    w = SgGetTextWidth(str, h);
    PrimGetPos(primP, x);

    ll2[0] = x[0] + dx[0];
    ll2[1] = x[1] + dx[1];
    ur2[0] = ll2[0] + w;
    ur2[1] = ll2[1] + h;
  } else {
    getTransfPos(x, primP);

    SgGetTextBox(ll3, ur3, ll2, ur2, x, dx, str, h);
  }

  if (dataP->xInit >= ll2[0] && dataP->xInit <= ur2[0] &&
      dataP->yInit >= ll2[1] && dataP->yInit <= ur2[1]) {
    dataP->primP = primP;
    dataP->xText = ll2[0];
    dataP->yText = ll2[1];
    dataP->wText = ur2[0] - ll2[0];
    dataP->hText = ur2[1] - ll2[1];

    b = getBorder(dataP->wText, dataP->hText);

    if (dataP->xInit < ll2[0] + b)
      dataP->xHand = -1;
    else if (dataP->xInit > ur2[0] - b)
      dataP->xHand = 1;
    else
      dataP->xHand = 0;

    if (dataP->yInit < ll2[1] + b)
      dataP->yHand = -1;
    else if (dataP->yInit > ur2[1] - b)
      dataP->yHand = 1;
    else
      dataP->yHand = 0;
  }
}

static void
drawBox2D(float x, float y, float w, float h)
{
  float ll[2], lr[2], ur[2], ul[2];
  float b;
  float x1[2], x2[2];

  ll[0] = x;
  ll[1] = y;

  lr[0] = x + w;
  lr[1] = y;

  ur[0] = x + w;
  ur[1] = y + h;

  ul[0] = x;
  ul[1] = y + h;

  SgSetLineStyle(SG_LINE_SOLID);

  SgDrawLine2D(ll, lr);
  SgDrawLine2D(lr, ur);
  SgDrawLine2D(ur, ul);
  SgDrawLine2D(ul, ll);

  b = getBorder(w, h);

  SgSetLineStyle(SG_LINE_DASHED);

  x1[0] = x;
  x1[1] = y + b;
  x2[0] = x + w;
  x2[1] = y + b;
  SgDrawLine2D(x1, x2);
  x1[1] = y + h - b;
  x2[1] = y + h - b;
  SgDrawLine2D(x1, x2);

  x1[0] = x + b;
  x1[1] = y;
  x2[0] = x + b;
  x2[1] = y + h;
  SgDrawLine2D(x1, x2);
  x1[0] = x + w - b;
  x2[0] = x + w - b;
  SgDrawLine2D(x1, x2);
}

static void
drawBox3D(float x, float y, float z, float w, float h)
{
  Vec3 ll, lr, ur, ul;
  float d;
  Vec3 x1, x2;

  ll[0] = x;
  ll[1] = y;
  ll[2] = z;

  lr[0] = x + w;
  lr[1] = y;
  lr[2] = z;

  ur[0] = x + w;
  ur[1] = y + h;
  ur[2] = z;

  ul[0] = x;
  ul[1] = y + h;
  ul[2] = z;

  SgSetLineStyle(SG_LINE_SOLID);

  SgDrawLine(ll, lr);
  SgDrawLine(lr, ur);
  SgDrawLine(ur, ul);
  SgDrawLine(ul, ll);

  d = getBorder(w, h);

  SgSetLineStyle(SG_LINE_DASHED);

  x1[2] = z;
  x2[2] = z;

  x1[0] = x;
  x1[1] = y + d;
  x2[0] = x + w;
  x2[1] = y + d;
  SgDrawLine(x1, x2);
  x1[1] = y + h - d;
  x2[1] = y + h - d;
  SgDrawLine(x1, x2);

  x1[0] = x + d;
  x1[1] = y;
  x2[0] = x + d;
  x2[1] = y + h;
  SgDrawLine(x1, x2);
  x1[0] = x + w - d;
  x2[0] = x + w - d;
  SgDrawLine(x1, x2);
}

static void
updateTextBox(void *clientData)
{
  InputData *dataP = clientData;
  PrimObjP primP = dataP->primP;
  char *str;
  float x, y, w, h;
  Vec3 x3, dx;
  float ll2[2], ur2[2];
  Vec3 ll3, ur3;

  SgSetLight(SG_LIGHT_OFF);
  SgSetLineWidth(0.0f);

  if (dataP->onOff)
    SgSetColor(ColR, ColG, ColB);
  else
    SgSetColor(BackColR, BackColG, BackColB);

  str = PrimGetText(primP);
  h = PrimGetAttr(primP)->fontSize;
  PrimGetVec(primP, dx);

  if (PrimGetMol(primP) == NULL) {
    PrimGetPos(primP, x3);

    x = x3[0] + dx[0];
    y = x3[1] + dx[1];

    if (dataP->mod == TM_MOVE) {
      x += dataP->xInc;
      y += dataP->yInc;
    } else if (dataP->mod == TM_RESIZE) {
      x += dataP->xInc;
      y += dataP->yInc;
      h += dataP->hInc;
    }

    w = SgGetTextWidth(str, h);

    drawBox2D(x, y, w, h);
  } else {
    getTransfPos(x3, primP);

    if (dataP->mod == TM_MOVE) {
      dx[0] += dataP->xInc;
      dx[1] += dataP->yInc;

      SgGetTextBox(ll3, ur3, ll2, ur2, x3, dx, str, h);
    } else if (dataP->mod == TM_RESIZE) {
      dx[0] += dataP->xInc;
      dx[1] += dataP->yInc;
      h += dataP->hInc;

      SgGetTextBox(ll3, ur3, ll2, ur2, x3, dx, str, h);
    } else {
      SgGetTextBox(ll3, ur3, ll2, ur2, x3, dx, str, h);
    }

    drawBox3D(ll3[0], ll3[1], ll3[2], ur3[0] - ll3[0], ur3[1] - ll3[1]);
  }
}

static void
updatePrimBox(PrimObjP primP, void *clientData)
{
  InputData *dataP = clientData;
  PrimObjP origPrimP = dataP->primP;

  dataP->primP = primP;
  GraphDrawOverlay(updateTextBox, dataP);
  dataP->primP = origPrimP;
}

static void
calcResize(InputData *dataP, float x, float y)
{
  float wInc, hInc;

  if (dataP->xHand == -1)
    wInc = dataP->xText - x;
  else if (dataP->xHand == 0)
    wInc = - dataP->wText;
  else
    wInc = x - dataP->xText - dataP->wText;

  if (dataP->yHand == -1)
    hInc = dataP->yText - y;
  else if (dataP->yHand == 0)
    hInc = - dataP->hText;
  else
    hInc = y - dataP->yText - dataP->hText;

  if (wInc * dataP->hText > hInc * dataP->wText) {
    if (wInc < - 0.9f * dataP->wText)
      wInc = - 0.9f * dataP->wText;
    hInc = dataP->hText * wInc / dataP->wText;
  } else {
    if (hInc < - 0.9f * dataP->hText)
      hInc = - 0.9f * dataP->hText;
    wInc = dataP->wText * hInc / dataP->hText;
  }

  dataP->hInc = hInc;

  if (dataP->xHand == -1)
    dataP->xInc = - wInc;
  else if (dataP->xHand == 0)
    dataP->xInc = - 0.5f * wInc;
  else
    dataP->xInc = 0.0f;

  if (dataP->yHand == -1)
    dataP->yInc = - hInc;
  else if (dataP->yHand == 0)
    dataP->yInc = - 0.5f * hInc;
  else
    dataP->yInc = 0.0f;
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
inputText(char *act, float x, float y,
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
    PrimApply(PT_TEXT, PropGetRef(PROP_ALL, FALSE), checkBox, &data);

    if (data.primP == NULL)
      return FALSE;

    data.mod = TM_NONE;
    data.onOff = PrimGetProp(refP, data.primP);

    switchFront(TRUE);
  } else if (strcmp(act, "drag") == 0) {
    if (data.onOff) {
      data.onOff = FALSE;
      PrimApply(PT_TEXT, refP, updatePrimBox, &data);

      if (data.xHand == 0 && data.yHand == 0) {
	data.mod = TM_MOVE;
	data.xInc = x - data.xInit;
	data.yInc = y - data.yInit;
      } else {
	data.mod = TM_RESIZE;
	calcResize(&data, x, y);
      }

      data.onOff = TRUE;
      PrimApply(PT_TEXT, refP, updatePrimBox, &data);
    }
  } else if (strcmp(act, "release") == 0) {
    if (data.mod == TM_NONE && callP->doSel) {
      if (data.onOff)
	PrimSel(data.primP, SK_REMOVE);
      else
	PrimSel(data.primP, SK_ADD);
    }

    switchFront(FALSE);

    if (data.mod == TM_MOVE) {
      if (MoveF != NULL)
	MoveF(data.xInc, data.yInc);
    } else if (data.mod == TM_RESIZE) {
      GraphRedrawEnable(FALSE);  /* avoid double redraw */

      if (MoveF != NULL && (data.xInc != 0.0f || data.yInc != 0.0f))
	MoveF(data.xInc, data.yInc);
      if (ResizeF != NULL)
	ResizeF(data.hInc);

      GraphRedrawEnable(TRUE);
    }
  }

  return TRUE;
}

static void
drawTextBox(PrimObjP primP, void *clientData)
{
  InputData data;

  data.primP = primP;
  data.mod = TM_NONE;
  data.onOff = TRUE;

  updateTextBox(&data);
}

static void
drawTextBoxes(void *clientData)
{
  getPar();

  switchAntiAliasing(FALSE);
  PrimApply(PT_TEXT, PropGetRef(PROP_SELECTED, FALSE), drawTextBox, NULL);
  switchAntiAliasing(TRUE);
}

static void
textToggleCB(PrimObjP primP, PropRefP refP, BOOL onOff, void *clientData)
{
  InputData data;

  getPar();
  switchFront(TRUE);

  data.primP = primP;
  data.mod = TM_NONE;
  data.onOff = onOff;

  GraphDrawOverlay(updateTextBox, &data);

  switchFront(FALSE);
}

void
GraphTextInit(void)
{
  GraphInputAdd(inputText, NULL);
  GraphAddOverlay(drawTextBoxes, NULL);
  PrimAddPropCB(PT_TEXT, PropGetRef(PROP_SELECTED, FALSE),
      textToggleCB, NULL);
}

void
GraphTextSetMoveFunc(GraphTextMoveF moveF)
{
  MoveF = moveF;
}

void
GraphTextSetResizeFunc(GraphTextResizeF resizeF)
{
  ResizeF = resizeF;
}
