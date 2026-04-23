/*
************************************************************************
*
*   GraphInput.c - graphical display, input
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/graph/SCCS/s.GraphInput.c
*   SCCS identification       : 1.18
*
************************************************************************
*/

#include <graph_input.h>

#include <stdio.h>
#include <string.h>

#include <break.h>
#include <mat_vec.h>
#include <linlist.h>
#include <track_ball.h>
#include <sg.h>
#include <pu.h>
#include <data_hand.h>
#include <data_sel.h>
#include <prim_sel.h>
#include <par_names.h>
#include <par_hand.h>
#include <graph_transf.h>
#include <graph_draw.h>

typedef struct {
  GraphInputF inputF;
  void *clientData;
} InputData;

static LINLIST InputList = NULL;

static int CurrWidth, CurrHeight;
static int CurrClickLevel = 0;
static int ClickCoord[2];

static void
exposeCB(PuWindow win, void *clientData, PuExposeCBStruc *callP)
{
  GraphRefreshNeeded();
}

static void
resizeCB(PuWindow win, void *clientData, PuResizeCBStruc *callP)
{
  CurrWidth = callP->w;
  CurrHeight = callP->h;

  ParSetIntVal(PN_CURR_WIDTH, CurrWidth);
  ParSetIntVal(PN_CURR_HEIGHT, CurrHeight);

  SgSetViewport(0.0f, 0.0f, (float) callP->w, (float) callP->h);
  SgUpdateView();
}

static void
screen2Virt(int xs, int ys, float *xtP, float *ytP)
{
  StereoMode stereoMode;

  SgConvCoord(xs, ys, xtP, ytP);

  stereoMode = ParGetIntVal(PN_STEREO_MODE);
  if (stereoMode == SM_SIDE_BY_SIDE || stereoMode == SM_CROSS_EYE) {
    if (*xtP < 0.0f)
      *xtP += 0.5f * (float) CurrWidth / (float) CurrHeight;
    else
      *xtP -= 0.5f * (float) CurrWidth / (float) CurrHeight;
  }
}

static void
screen2Track(int xs, int ys, float *xtP, float *ytP)
{
  float r;

  r = 0.7f;
  if (CurrWidth < CurrHeight)
    r *= (float) CurrWidth / (float) CurrHeight;

  SgConvCoord(xs, ys, xtP, ytP);

  *xtP /= r;
  *ytP /= r;
}

void
GraphInputGetClick(float *xP, float *yP)
{
  int oldLevel = CurrClickLevel;

  CurrClickLevel++;

  PuSetBusyCursor(FALSE);
  PuSetTextField(PU_TF_STATUS, "!Select Position");

  while (CurrClickLevel > oldLevel)
    PuProcessEvent();

  PuSetBusyCursor(TRUE);
  if (CurrClickLevel == 0)
    PuSetTextField(PU_TF_STATUS, "! ");

  screen2Virt(ClickCoord[0], ClickCoord[1], xP, yP);
}

void
GraphInputAdd(GraphInputF inputF, void *clientData)
{
  InputData data;

  if (InputList == NULL)
    InputList = ListOpen(sizeof(InputData));
  
  data.inputF = inputF;
  data.clientData = clientData;

  (void) ListInsertLast(InputList, &data);
}

static void
execPick(int xs, int ys, BOOL addSel)
{
  float xt, yt;
  void *typeP;
  void *objP;
  int closeInd;
  SelKind kind;
  char *typeStr;
  DhAngleP angleP;

  screen2Virt(xs, ys, &xt, &yt);

  SgStartPick(xt, yt);
  GraphDraw();
  SgEndPick(&typeP, &objP, &closeInd, NULL);
  if (typeP == NULL || objP == NULL)
    return;

  if (addSel)
    kind = SK_ADD;
  else
    kind = SK_REPLACE;

  typeStr = typeP;

  if (strcmp(typeStr, "Atom") == 0) {
    if (! addSel) {
      DeselAllMol();
      DeselAllRes();
    }
    SelAtom(objP, kind);
  } else if (strcmp(typeStr, "Bond") == 0) {
    if (! addSel) {
      DeselAllMol();
      DeselAllRes();
    }
    /* also select closer atom if bond is selected */
    if (closeInd == 0)
      SelAtom(DhBondGetAtom1(objP), kind);
    else
      SelAtom(DhBondGetAtom2(objP), kind);

    angleP = DhBondGetAngle(objP);
    if (angleP == NULL)
      DeselAllAngle();
    else
      SelAngle(angleP, kind);

    SelBond(objP, kind);
  } else if (strcmp(typeStr, "Angle") == 0) {
    if (! addSel) {
      DeselAllMol();
      DeselAllRes();
    }
    SelAngle(objP, kind);
  } else if (strcmp(typeStr, "Res") == 0) {
    if (! addSel)
      DeselAllMol();
    SelRes(objP, kind);
  } else if (strcmp(typeStr, "Dist") == 0) {
    if (! addSel) {
      DeselAllMol();
      DeselAllRes();
    }
    /* also select closer atom if dist is selected */
    if (closeInd == 0)
      SelAtom(DhDistGetAtom1(objP), kind);
    else
      SelAtom(DhDistGetAtom2(objP), kind);

    SelDist(objP, kind);
  } else if (strcmp(typeStr, "Prim") == 0) {
    PrimSel(objP, kind);
  }
}

static InputData *
checkAltInput(char *act, PuMouseCBStruc *callP)
{
  InputData *dataP;
  float xt, yt;

  screen2Virt(callP->x, callP->y, &xt, &yt);

  dataP = ListFirst(InputList);
  while (dataP != NULL) {
    if (dataP->inputF(act, xt, yt, callP, dataP->clientData))
      return dataP;
    dataP = ListNext(InputList, dataP);
  }

  return NULL;
}

static void
mouseCB(PuWindow win, void *clientData, PuMouseCBStruc *callP)
{
  static InputData *altInputP;
  static int xLast, yLast, xInit, yInit;
  static BOOL addSel, didDrag;
  static float zoomInit, zoomFact;
  static int drawPrec, movePrec;
  char *str = clientData;
  int xNew, yNew;
  float x0, y0, x1, y1, f;
  Mat4 m;
  Vec3 transV;

  if (CurrClickLevel > 0 && strcmp(str, "release") == 0) {
    ClickCoord[0] = callP->x;
    ClickCoord[1] = callP->y;
    CurrClickLevel--;
  } else if (strcmp(str, "press") == 0) {
    altInputP = checkAltInput(str, callP);

    if (altInputP == NULL) {
      xLast = callP->x;
      yLast = callP->y;
      addSel = callP->addSel;
      if (callP->button == PU_MB_3) {
	xInit = callP->x;
	if (xInit <= 0)
	  xInit = 1;
	yInit = callP->y;
	if (yInit <= 0)
	  yInit = 1;
	zoomInit = (float) ParGetDoubleVal(PN_ZOOM_FACT);
      }
      drawPrec = ParGetIntVal(PN_DRAW_PREC);
      movePrec = ParGetIntVal(PN_MOVE_PREC);
      didDrag = FALSE;
    }
  } else if (strcmp(str, "drag") == 0) {
    if (altInputP == NULL) {
      if (! didDrag) {
	if (movePrec < drawPrec) {
	  ParSetIntVal(PN_DRAW_PREC, movePrec);
	  SgSetPrecision(movePrec);
        }
	BreakActivate(TRUE);
      }
      xNew = callP->x;
      yNew = callP->y;
      didDrag = TRUE;
      if (callP->button == PU_MB_1) {
	screen2Track(xLast, yLast, &x0, &y0);
	screen2Track(xNew, yNew, &x1, &y1);
	TrackBallCalcMat(x0, y0, x1, y1, m);
	GraphRotate(m);
      } else if (callP->button == PU_MB_2) {
	f = 2.0f * SgGetDepthFact(0.0f) /
	    (CurrHeight * (float) ParGetDoubleVal(PN_ZOOM_FACT));
	transV[0] = f * (xNew - xLast);
	transV[1] = f * (yNew - yLast);
	transV[2] = 0.0f;
	GraphMove(transV);
      } else if (callP->button == PU_MB_3) {
	if (xNew <= 0)
	  xNew = 1;
	if (yNew <= 0)
	  yNew = 1;
	zoomFact = zoomInit * (float) (xNew + yNew) / (float) (xInit + yInit);
	ParSetDoubleVal(PN_ZOOM_FACT, zoomFact);
	GraphZoomSet(zoomFact);
      }
      GraphRedraw();
      xLast = xNew;
      yLast = yNew;
    } else {
      screen2Virt(callP->x, callP->y, &x0, &y0);
      (void) altInputP->inputF(str, x0, y0, callP, altInputP->clientData);
    }
  } else if (strcmp(str, "release") == 0) {
    if (altInputP == NULL) {
      if (didDrag) {
	BreakActivate(FALSE);
	if (movePrec < drawPrec) {
	  ParSetIntVal(PN_DRAW_PREC, drawPrec);
	  SgSetPrecision(drawPrec);
          GraphRedraw();
        }
      } else if (callP->doSel) {
	execPick(xLast, yLast, addSel);
      }
    } else {
      screen2Virt(callP->x, callP->y, &x0, &y0);
      (void) altInputP->inputF(str, x0, y0, callP, altInputP->clientData);
      altInputP = NULL;
    }
  }
}

void
GraphInputInit(void)
{
  PuAddExposeCB(exposeCB, NULL, NULL);
  PuAddResizeCB(resizeCB, NULL, NULL);

  PuAddButtonPressCB(PU_MB_1, mouseCB, "press", NULL);
  PuAddButtonPressCB(PU_MB_2, mouseCB, "press", NULL);
  PuAddButtonPressCB(PU_MB_3, mouseCB, "press", NULL);
  PuAddButtonReleaseCB(PU_MB_1, mouseCB, "release", NULL);
  PuAddButtonReleaseCB(PU_MB_2, mouseCB, "release", NULL);
  PuAddButtonReleaseCB(PU_MB_3, mouseCB, "release", NULL);
  PuAddDragCB(PU_MB_1, mouseCB, "drag", NULL);
  PuAddDragCB(PU_MB_2, mouseCB, "drag", NULL);
  PuAddDragCB(PU_MB_3, mouseCB, "drag", NULL);
}
