/*
************************************************************************
*
*   GraphDraw.c - graphical display, drawing
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
*   Date of last modification : 01/06/02
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/graph/SCCS/s.GraphDraw.c
*   SCCS identification       : 1.58
*
************************************************************************
*/

#include <graph_draw.h>

#include <stdio.h>
#include <string.h>
#include <math.h>

#ifdef PRINT_TIME
#include <time.h>
#endif

#include <break.h>
#include <mat_vec.h>
#include <linlist.h>
#include <sg.h>
#include <sg_get.h>
#include <pu.h>
#include <io.h>
#include <os_sleep.h>
#include <par_names.h>
#include <par_hand.h>
#include <data_hand.h>
#include <prim_hand.h>
#include <prim_draw.h>
#include <attr_struc.h>
#include <attr_mng.h>

#define DRAW_MARK "_draw_mark"

typedef struct {
  GraphDrawF drawF;
  void *clientData;
} OverlayData;

static float ViewP[3];
static float ZoomFact;
static float NearPlanePar, NearPlaneLimit, NearPlaneCurr;
static int DrawDelay;
static AttrP LastAttrP;
static SgLightState LightState;

static Mat4 RotMat;
static Vec3 RotPoint, TransVect;


static BOOL RemoveCBAdded = FALSE;

static int RedrawBlockLevel = 0;
static BOOL RefreshPending = FALSE;
static BOOL RedrawPending = FALSE;

static int BreakPoly, SpherePoly, CylinderPoly, NeonPoly, PolyNo;

static LINLIST OverlayList = NULL;

static GraphDrawF AltDrawF = NULL;
static void *AltClientData;

static void
doNothing(Mat4 m)
{
}

static void
doRot(Mat4 m)
{
  SgMultMatrix(m);
}

void
GraphDrawInit(void)
{
  float backCol[3];
  int i;

  /* this one doesn't really fit here, but it's the best place */
  if (ParDefined(PN_BOND_MODE))
    DhSetPseudoMode(ParGetIntVal(PN_BOND_MODE) == 1);
  else
    DhSetPseudoMode(FALSE);

  if (! ParDefined(PN_STEREO_MODE))
    ParSetIntVal(PN_STEREO_MODE, SM_OFF);

  if (ParDefined(PN_DRAW_DELAY)) {
    DrawDelay = ParGetIntVal(PN_DRAW_DELAY);
  } else {
    DrawDelay = 0;
    ParSetIntVal(PN_DRAW_DELAY, DrawDelay);
  }

  if (ParDefined(PN_DRAW_PREC)) {
    SgSetPrecision(ParGetIntVal(PN_DRAW_PREC));
  } else {
    ParSetIntVal(PN_DRAW_PREC, 3);
    SgSetPrecision(3);
  }

  if (! ParDefined(PN_MOVE_PREC))
    ParSetIntVal(PN_MOVE_PREC, 2);

  if (ParDefined(PN_PROJECTION)) {
    SgSetProjection(ParGetIntVal(PN_PROJECTION));
  } else {
    ParSetIntVal(PN_PROJECTION, SG_PROJ_ORTHO);
    SgSetProjection(SG_PROJ_ORTHO);
  }

  if (ParDefined(PN_NEAR_PLANE)) {
    SgSetNearPlane((float) ParGetDoubleVal(PN_NEAR_PLANE), FALSE);
  } else {
    ParSetDoubleVal(PN_NEAR_PLANE, 1.0);
    SgSetNearPlane(1.0f, FALSE);
  }

  if (ParDefined(PN_FAR_PLANE)) {
    SgSetFarPlane((float) ParGetDoubleVal(PN_FAR_PLANE), FALSE);
  } else {
    ParSetDoubleVal(PN_FAR_PLANE, 20.0);
    SgSetFarPlane(20.0f, FALSE);
  }

  if (ParDefined(PN_VIEW_ANGLE)) {
    SgSetViewAngle((float) ParGetDoubleVal(PN_VIEW_ANGLE));
  } else {
    ParSetDoubleVal(PN_VIEW_ANGLE, 0.7);
    SgSetViewAngle(0.7f);
  }

  if (ParDefined(PN_BACK_COLOR)) {
    for (i = 0; i < 3; i++)
      backCol[i] = (float) ParGetDoubleArrVal(PN_BACK_COLOR, i);
    SgSetBackgroundColor(backCol[0], backCol[1], backCol[2]);
  } else {
    for (i = 0; i < 3; i++)
      ParSetDoubleArrVal(PN_BACK_COLOR, i, 1.0);
    SgSetBackgroundColor(1.0f, 1.0f, 1.0f);
  }

  if (ParDefined(PN_FOG)) {
    SgSetFogMode(ParGetIntVal(PN_FOG));
  } else {
    ParSetIntVal(PN_FOG, SG_FOG_MODE_OFF);
    SgSetFogMode(SG_FOG_MODE_OFF);
  }

  if (ParDefined(PN_FOG_DENSITY)) {
    SgSetFogPar(SG_FOG_DENSITY, (float) ParGetDoubleVal(PN_FOG_DENSITY));
  } else {
    ParSetDoubleVal(PN_FOG_DENSITY, 0.1);
    SgSetFogPar(SG_FOG_DENSITY, 0.1f);
  }

  if (ParDefined(PN_FOG_START)) {
    SgSetFogPar(SG_FOG_START, (float) ParGetDoubleVal(PN_FOG_START));
  } else {
    ParSetDoubleVal(PN_FOG_START, 7.0);
    SgSetFogPar(SG_FOG_START, 7.0f);
  }

  if (ParDefined(PN_FOG_END)) {
    SgSetFogPar(SG_FOG_END, (float) ParGetDoubleVal(PN_FOG_END));
  } else {
    ParSetDoubleVal(PN_FOG_END, 13.0);
    SgSetFogPar(SG_FOG_END, 13.0f);
  }

  if (! ParDefined(PN_LIGHT_POS)) {
    ParSetDoubleArrVal(PN_LIGHT_POS, 0, -5.0);
    ParSetDoubleArrVal(PN_LIGHT_POS, 1, 5.0);
    ParSetDoubleArrVal(PN_LIGHT_POS, 2, 10.0);
    /* LightPos is set in drawInit() */
  }

  if (! ParDefined(PN_LIGHT))
    ParSetIntVal(PN_LIGHT, SG_LIGHT_INFINITE);
    /* Light is set in drawInit() */

  if (ParDefined(PN_ZOOM_FACT)) {
    ZoomFact = (float) ParGetDoubleVal(PN_ZOOM_FACT);
  } else {
    ZoomFact = 0.2f;
    ParSetDoubleVal(PN_ZOOM_FACT, ZoomFact);
  }

  ViewP[0] = 0.0f;
  ViewP[1] = 0.0f;
  ViewP[2] = 10.0f;
  SgSetViewPoint(ViewP);

  DhRotSetFunc(doNothing);
}

void
GraphDelaySet(int delay)
{
  DrawDelay = delay;
}

void
GraphZoomSet(float zoomFact)
{
  ZoomFact = zoomFact;
}

static void
clearProp(DhAtomP atomP, void *clientData)
{
  PropRefP refP = clientData;

  DhAtomSetProp(refP, atomP, FALSE);
}

static void
setTransf(void)
{
  SgScale(ZoomFact, ZoomFact, ZoomFact);
  SgTranslate(TransVect);
  SgMultMatrix(RotMat);
  SgTranslate(RotPoint);
}

void
GraphSetAttr(AttrP attrP)
{
  if (attrP == LastAttrP)
    return;

  if (attrP->shadeModel == -1) {
    SgSetLight(SG_LIGHT_OFF);
  } else {
    SgSetShadeModel(attrP->shadeModel);
    SgSetLight(LightState);
  }

  /* alpha must be set before color because some graphics devices
     prefer to update it together with the color */
  SgSetMatProp(SG_MAT_ALPHA, attrP->alpha);
  SgSetColor(attrP->colR, attrP->colG, attrP->colB);
  SgSetMatProp(SG_MAT_AMBIENT_FACTOR, attrP->ambCoeff);
  SgSetMatProp(SG_MAT_DIFFUSE_FACTOR, attrP->diffCoeff);
  SgSetMatProp(SG_MAT_SPECULAR_FACTOR, attrP->specCoeff);
  SgSetMatProp(SG_MAT_SHININESS, attrP->shininess);
  SgSetMatProp(SG_MAT_REFLECTION, attrP->reflect);
  SgSetMatProp(SG_MAT_REFRACTION_INDEX, attrP->refract);
  SgSetTexture(attrP->texture);
  SgSetMatProp(SG_MAT_TEXTURE_SCALE, attrP->textureScale);
  SgSetMatProp(SG_MAT_BUMP_DEPTH, attrP->bumpDepth);
  SgSetMatProp(SG_MAT_BUMP_SCALE, attrP->bumpScale);
  SgSetLineWidth(attrP->lineWidth);
  SgSetLineStyle(attrP->lineStyle);
  SgSetFontSize(attrP->fontSize);

  if (attrP->nearClip > NearPlaneLimit) {
    SgSetNearPlane(attrP->nearClip, TRUE);
    SgUpdateView();
    setTransf();
    NearPlaneCurr = attrP->nearClip;
  } else if (NearPlaneCurr > NearPlaneLimit) {
    SgSetNearPlane(NearPlanePar, TRUE);
    SgUpdateView();
    setTransf();
    NearPlaneCurr = NearPlanePar;
  }

  LastAttrP = attrP;
}

static void
drawAtom(DhAtomP atomP, void *clientData)
{
  AttrP attrP;
  Vec3 x;
  Vec3 x1, x2, x3, x4;
  char *labelFormat;
  DSTR labelStr;
  char numStr[7];

  attrP = DhAtomGetAttr(atomP);
  if (attrP->atomStyle == AS_INVISIBLE && attrP->labelFormat[0] == '\0')
    return;

  if (BreakInterrupted())
    return;

  GraphSetAttr(attrP);
  SgSetPickObj("Atom", atomP);

  DhAtomGetCoord(atomP, x);

  if (attrP->atomStyle == AS_SPHERE) {
    SgDrawSphere(x, attrP->radius);
    PolyNo += SpherePoly;
    if (PolyNo > BreakPoly)
      (void) BreakCheck(1);
  } else if (attrP->atomStyle == AS_TETRAHEDRON) {
    x1[0] = x[0] - 1.000000f * attrP->radius;
    x1[1] = x[1] - 0.577350f * attrP->radius;
    x1[2] = x[2] - 0.408248f * attrP->radius;

    x2[0] = x[0] + 0.500000f * attrP->radius;
    x2[1] = x[1] - 0.577350f * attrP->radius;
    x2[2] = x[2] - 0.408248f * attrP->radius;

    x3[0] = x[0] + 0.000000f * attrP->radius;
    x3[1] = x[1] + 1.154700f * attrP->radius;
    x3[2] = x[2] - 0.408248f * attrP->radius;

    x4[0] = x[0] + 0.000000f * attrP->radius;
    x4[1] = x[1] + 0.000000f * attrP->radius;
    x4[2] = x[2] + 1.224745f * attrP->radius;

    SgDrawLine(x1, x2);
    SgDrawLine(x1, x3);
    SgDrawLine(x1, x4);
    SgDrawLine(x2, x3);
    SgDrawLine(x2, x4);
    SgDrawLine(x3, x4);
  }

  if (attrP->labelFormat[0] != '\0') {
    labelFormat = attrP->labelFormat;
    labelStr = DStrNew();
    while (labelFormat[0] != '\0') {
      if (labelFormat[0] == 'A') {
	DStrAppDStr(labelStr, DhAtomGetName(atomP));
      } else if (labelFormat[0] == 'R') {
	DStrAppDStr(labelStr, DhResGetName(DhAtomGetRes(atomP)));
      } else if (labelFormat[0] == 'N') {
	(void) sprintf(numStr, "%d", DhResGetNumber(DhAtomGetRes(atomP)));
	DStrAppStr(labelStr, numStr);
      } else {
	DStrAppChar(labelStr, labelFormat[0]);
      }
      labelFormat++;
    }
    if (attrP->shadeModel == -1) {
      SgDrawAnnot(x, DStrToStr(labelStr));
    } else {
      SgSetLight(SG_LIGHT_OFF);
      SgDrawAnnot(x, DStrToStr(labelStr));
      SgSetLight(LightState);
    }
    DStrFree(labelStr);
  }
}

static SgConeEnd
getNeonEnd(DhAtomP atomP, PropRefP markP)
{
  if (DhAtomGetProp(markP, atomP))
    return SG_CONE_OPEN;

  DhAtomSetProp(markP, atomP, TRUE);
  return SG_CONE_CAP_ROUND;
}

static void
drawDashedCone(Vec3 x0, Vec3 x1, float len, float rad0, float rad1,
    SgConeEnd end0, SgConeEnd end1)
{
  Vec3 v, xi0, xi1;
  float ri0, ri1, rInc;
  int n, i;

  Vec3Copy(v, x1);
  Vec3Sub(v, x0);

  n = (int) (2.0f * len);
  Vec3Scale(v, 1.0f / (2 * n + 1));
  rInc = (rad1 - rad0) / (2 * n + 1);

  Vec3Copy(xi0, x0);
  ri0 = rad0;

  for (i = 0; i < n; i++) {
    Vec3Copy(xi1, xi0);
    Vec3Add(xi1, v);
    ri1 = ri0 + rInc;

    SgDrawCone(xi0, xi1, ri0, ri1, end0, SG_CONE_CAP_FLAT);
    end0 = SG_CONE_CAP_FLAT;

    Vec3Copy(xi0, xi1);
    Vec3Add(xi0, v);
    ri0 = ri1;
    ri0 += rInc;
  }

  SgDrawCone(xi0, x1, ri0, rad1, SG_CONE_CAP_FLAT, end1);
}

static void
drawBond(DhBondP bondP, DhAtomP atom0P, DhAtomP atom1P, void *clientData)
{
  PropRefP markP = clientData;
  AttrP attrP, attr0P, attr1P;
  Vec3 x0, x1, v, xi;
  float ri;

  attrP = DhBondGetAttr(bondP);
  if (attrP->bondStyle == BS_INVISIBLE)
    return;

  if (BreakInterrupted())
    return;

  GraphSetAttr(attrP);
  SgSetPickObj("Bond", bondP);

  DhAtomGetCoord(atom0P, x0);
  DhAtomGetCoord(atom1P, x1);

  if (attrP->bondStyle == BS_LINE) {
    SgDrawLine(x0, x1);
  } else if (attrP->bondStyle == BS_CYLINDER) {
    if (attrP->lineStyle == SG_LINE_SOLID)
      SgDrawCone(x0, x1, attrP->radius, attrP->radius,
	  SG_CONE_OPEN, SG_CONE_OPEN);
    else
      drawDashedCone(x0, x1, Vec3DiffAbs(x0, x1),
	  attrP->radius, attrP->radius, SG_CONE_OPEN, SG_CONE_OPEN);
    PolyNo += CylinderPoly;
    if (PolyNo > BreakPoly)
      (void) BreakCheck(1);
  } else if (attrP->bondStyle == BS_NEON) {
    if (attrP->lineStyle == SG_LINE_SOLID)
      SgDrawCone(x0, x1, attrP->radius, attrP->radius,
	  getNeonEnd(atom0P, markP), getNeonEnd(atom1P, markP));
    else
      drawDashedCone(x0, x1, Vec3DiffAbs(x0, x1),
	  attrP->radius, attrP->radius,
	  getNeonEnd(atom0P, markP), getNeonEnd(atom1P, markP));
    PolyNo += NeonPoly;
    if (PolyNo > BreakPoly)
      (void) BreakCheck(1);
  } else if (attrP->bondStyle == BS_CONE) {
    attr0P = DhAtomGetAttr(atom0P);
    attr1P = DhAtomGetAttr(atom1P);
    if (attrP->lineStyle == SG_LINE_SOLID)
      SgDrawCone(x0, x1, attr0P->radius, attr1P->radius,
	  getNeonEnd(atom0P, markP), getNeonEnd(atom1P, markP));
    else
      drawDashedCone(x0, x1, Vec3DiffAbs(x0, x1),
	  attr0P->radius, attr1P->radius,
	  getNeonEnd(atom0P, markP), getNeonEnd(atom1P, markP));
    PolyNo += NeonPoly;
    if (PolyNo > BreakPoly)
      (void) BreakCheck(1);
  } else if (attrP->bondStyle == BS_HALF_LINE ||
      attrP->bondStyle == BS_HALF_CYLINDER ||
      attrP->bondStyle == BS_HALF_NEON ||
      attrP->bondStyle == BS_HALF_CONE) {
    attr0P = DhAtomGetAttr(atom0P);
    attr1P = DhAtomGetAttr(atom1P);
    if (attr0P != attrP)
      SgSetColor(attr0P->colR, attr0P->colG, attr0P->colB);

    Vec3Copy(v, x1);
    Vec3Sub(v, x0);
    Vec3Copy(xi, x0);
    Vec3ScaleAdd(xi, 0.5f, v);

    if (attrP->bondStyle == BS_HALF_LINE) {
      SgDrawLine(x0, xi);
    } else if (attrP->bondStyle == BS_HALF_CYLINDER) {
      SgDrawCone(x0, xi, attrP->radius, attrP->radius,
	  SG_CONE_OPEN, SG_CONE_OPEN);
    } else if (attrP->bondStyle == BS_HALF_NEON) {
      SgDrawCone(x0, xi, attrP->radius, attrP->radius,
	  getNeonEnd(atom0P, markP), SG_CONE_OPEN);
    } else {
      ri = 0.5f * (attr0P->radius + attr1P->radius);
      /* precision is determined by first radius, start from
	 the middle so that precision is the same for both halfs */
      SgDrawCone(xi, x0, ri, attr0P->radius,
	  SG_CONE_OPEN, getNeonEnd(atom0P, markP));
    }

    if (attr1P != attr0P)
      SgSetColor(attr1P->colR, attr1P->colG, attr1P->colB);

    if (attrP->bondStyle == BS_HALF_LINE) {
      SgDrawLine(xi, x1);
    } else if (attrP->bondStyle == BS_HALF_CYLINDER) {
      SgDrawCone(xi, x1, attrP->radius, attrP->radius,
	  SG_CONE_OPEN, SG_CONE_OPEN);
      PolyNo += 2 * CylinderPoly;
      if (PolyNo > BreakPoly)
	(void) BreakCheck(1);
    } else if (attrP->bondStyle == BS_HALF_NEON) {
      SgDrawCone(xi, x1, attrP->radius, attrP->radius,
	  SG_CONE_OPEN, getNeonEnd(atom1P, markP));
      PolyNo += CylinderPoly + NeonPoly;
      if (PolyNo > BreakPoly)
	(void) BreakCheck(1);
    } else {
      SgDrawCone(xi, x1, ri, attr1P->radius,
	  SG_CONE_OPEN, getNeonEnd(atom1P, markP));
      PolyNo += CylinderPoly + NeonPoly;
      if (PolyNo > BreakPoly)
	(void) BreakCheck(1);
    }

    if (attrP != attr1P)
      SgSetColor(attrP->colR, attrP->colG, attrP->colB);
  }
}

static void
drawDist(DhDistP distP, Vec3 x0, Vec3 x1, float val, float viol,
    AttrP attrP, DhAtomP atom0P, DhAtomP atom1P, PropRefP markP, float zoomF)
{
  Vec3 v, x0i, x1i;
  float labelVal;
  float rad, rad0, rad1;
  char numStr[20];

  if (BreakInterrupted())
    return;

  GraphSetAttr(attrP);
  SgSetPickObj("Dist", distP);

  if (attrP->distStyle == DS_LINE) {
    SgDrawLine(x0, x1);
  } else if (attrP->distStyle == DS_VIOL) {
    Vec3Copy(v, x1);
    Vec3Sub(v, x0);
    Vec3Copy(x0i, x0);
    Vec3ScaleAdd(x0i, 0.5f * viol / val, v);
    Vec3Copy(x1i, x1);
    Vec3ScaleSub(x1i, 0.5f * viol / val, v);
    SgDrawLine(x0i, x1i);
    SgSetColor(attrP->secColR, attrP->secColG, attrP->secColB);
    SgDrawLine(x0, x0i);
    SgDrawLine(x1, x1i);
    SgSetColor(attrP->colR, attrP->colG, attrP->colB);
  } else if (attrP->distStyle == DS_CYLINDER) {
    rad = zoomF * attrP->radius;
    if (attrP->lineStyle == SG_LINE_SOLID)
      SgDrawCone(x0, x1, rad, rad, SG_CONE_OPEN, SG_CONE_OPEN);
    else
      drawDashedCone(x0, x1, val, rad, rad, FALSE, FALSE);
    PolyNo += CylinderPoly;
    if (PolyNo > BreakPoly)
      (void) BreakCheck(1);
  } else if (attrP->distStyle == DS_NEON) {
    rad = zoomF * attrP->radius;
    if (attrP->lineStyle == SG_LINE_SOLID)
      SgDrawCone(x0, x1, rad, rad,
	  getNeonEnd(atom0P, markP), getNeonEnd(atom1P, markP));
    else
      drawDashedCone(x0, x1, val, rad, rad,
	  getNeonEnd(atom0P, markP), getNeonEnd(atom1P, markP));
    PolyNo += NeonPoly;
    if (PolyNo > BreakPoly)
      (void) BreakCheck(1);
  } else if (attrP->distStyle == DS_CONE) {
    rad0 = zoomF * DhAtomGetAttr(atom0P)->radius;
    rad1 = zoomF * DhAtomGetAttr(atom1P)->radius;
    if (attrP->lineStyle == SG_LINE_SOLID)
      SgDrawCone(x0, x1, rad0, rad1,
	  getNeonEnd(atom0P, markP), getNeonEnd(atom1P, markP));
    else
      drawDashedCone(x0, x1, val, rad0, rad1,
	  getNeonEnd(atom0P, markP), getNeonEnd(atom1P, markP));
    PolyNo += NeonPoly;
    if (PolyNo > BreakPoly)
      (void) BreakCheck(1);
  }

  if (attrP->distLabel != DL_INVISIBLE) {
    if (attrP->distLabel == DL_VAL)
      labelVal = val;
    else if (attrP->distLabel == DL_LIMIT)
      labelVal = DhDistGetLimit(distP);
    else
      labelVal = viol;

    if (attrP->shadeModel != -1)
      SgSetLight(SG_LIGHT_OFF);

    Vec3Sub(x1, x0);
    Vec3ScaleAdd(x0, 0.5f, x1);
    (void) sprintf(numStr, "%.2f", labelVal);
    SgDrawAnnot(x0, numStr);

    if (attrP->shadeModel != -1)
      SgSetLight(LightState);
  }
}

static void
drawIntraDist(DhDistP distP, DhAtomP atom0P, DhAtomP atom1P, void *clientData)
{
  PropRefP markP = clientData;
  AttrP attrP;
  Vec3 x0, x1;

  attrP = DhDistGetAttr(distP);
  if (attrP->distStyle == DS_INVISIBLE)
    return;

  if (attrP->distStyle == DS_VIOL && DhDistGetViol(distP) < 0.0f)
    return;

  DhAtomGetCoord(atom0P, x0);
  DhAtomGetCoord(atom1P, x1);

  drawDist(distP, x0, x1, DhDistGetVal(distP), DhDistGetViol(distP),
      attrP, atom0P, atom1P, markP, 1.0f);
}

static void
drawInterDist(DhDistP distP, DhAtomP atom0P, DhAtomP atom1P, void *clientData)
{
  PropRefP markP = clientData;
  AttrP attrP;
  Vec3 x0, x1;
  float val, viol;
  DhDistKind kind;

  attrP = DhDistGetAttr(distP);
  if (attrP->distStyle == DS_INVISIBLE)
    return;

  DhAtomGetCoordTransf(atom0P, x0);
  DhAtomGetCoordTransf(atom1P, x1);

  val = Vec3DiffAbs(x0, x1);
  kind = DhDistGetKind(distP);
  if (kind == DK_UPPER) {
    viol = val - DhDistGetLimit(distP);
    if (attrP->distStyle == DS_VIOL && viol < 0.0f)
      return;
  } else if (kind == DK_LOWER) {
    viol = DhDistGetLimit(distP) - val;
    if (attrP->distStyle == DS_VIOL && viol < 0.0f)
      return;
  } else {
    viol = 0.0f;
  }

  Vec3Scale(x0, ZoomFact);
  Vec3Scale(x1, ZoomFact);

  drawDist(distP, x0, x1, val, viol,
      attrP, atom0P, atom1P, markP, ZoomFact);
}

static void
drawPrim(PrimObjP primP, void *clientData)
{
  if (PrimGetType(primP) == PT_TRAJEC ||
      PrimGetType(primP) == PT_TEXT ||
      PrimGetType(primP) == PT_DRAWOBJ)
    /* handled seperately in drawNoTrans */
    return;

  if (BreakCheck(1))
    return;

  GraphSetAttr(PrimGetAttr(primP));
  SgSetPickObj("Prim", primP);

  PrimDraw(primP, ZoomFact);
}

static void
drawNoTrans(PrimObjP primP, void *clientData)
{
  GraphSetAttr(PrimGetAttr(primP));
  SgSetPickObj("Prim", primP);

  PrimDraw(primP, ZoomFact);
}

static void
drawMolObj(void *clientData)
{
  DhMolP molP = clientData;
  PropRefP refP = PropGetRef(PROP_DISPLAYED, FALSE);
  PropRefP markP = PropGetRef(DRAW_MARK, TRUE);

  DhApplyAtom(markP, clearProp, markP);

  if (DhRotMode()) {
    DhRotSetFunc(doRot);

    SgPushMatrix();
    DhSetAltCoord(TRUE);
    DhMolApplyBond(refP, molP, drawBond, markP);
    DhSetAltCoord(FALSE);
    SgPopMatrix();
    SgPushMatrix();
    DhMolApplyAtom(refP, molP, drawAtom, NULL);
    SgPopMatrix();

    DhRotSetFunc(doNothing);
  } else {
    DhSetAltCoord(TRUE);
    DhMolApplyBond(refP, molP, drawBond, markP);
    DhSetAltCoord(FALSE);
    DhMolApplyAtom(refP, molP, drawAtom, NULL);
    DhMolApplyDist(refP, molP, drawIntraDist, markP);
  }

  PrimMolApply(PT_ALL, refP, molP, drawPrim, NULL);
}

static void
drawMol(DhMolP molP, void *clientData)
{
  MolAttrP attrP;

  if (BreakCheck(1))
    return;

  attrP = DhMolGetAttr(molP);

  if (! attrP->objDefined)
    return;

  DhMolGetRotMat(molP, RotMat);
  DhMolGetRotPoint(molP, RotPoint);
  DhMolGetTransVect(molP, TransVect);
  Vec3Scale(RotPoint, -1.0f);

  SgPushMatrix();
  setTransf();

  if (DhRotMode())
    drawMolObj(molP);
  else
    SgDrawObj(attrP->objId);

  SgPopMatrix();
}

static void
drawScene(void *clientData)
{
  PropRefP refP = PropGetRef(PROP_DISPLAYED, FALSE);
  PropRefP markP = PropGetRef(DRAW_MARK, TRUE);

  NearPlaneCurr = NearPlanePar;

  DhApplyMol(refP, drawMol, NULL);
  if (DhRotMode())
    DhApplyDist(refP, drawInterDist, markP);
  else
    DhApplyInterDist(refP, drawInterDist, markP);

  PrimApply(PT_TRAJEC, refP, drawNoTrans, NULL);
  PrimApply(PT_TEXT, refP, drawNoTrans, NULL);
  PrimApply(PT_DRAWOBJ, refP, drawNoTrans, NULL);

  if (NearPlaneCurr > NearPlaneLimit) {
    SgSetNearPlane(NearPlanePar, TRUE);
    SgUpdateView();
  }
}

static void
drawOverlay(void *clientData)
{
  OverlayData *dataP;

  dataP = ListFirst(OverlayList);
  while (dataP != NULL) {
    dataP->drawF(dataP->clientData);
    dataP = ListNext(OverlayList, dataP);
  }
}

static void
drawInit(void)
{
  float lightPos[3];
  int i;

  NearPlanePar = (float) ParGetDoubleVal(PN_NEAR_PLANE);
  if (NearPlanePar < 0.0f)
    NearPlaneLimit = 0.0f;
  else
    NearPlaneLimit = NearPlanePar;

  LastAttrP = NULL;

  /* setup lighting before matrix is changed */
  SgSetLight(SG_LIGHT_OFF);

  for (i = 0; i < 3; i++)
    lightPos[i] = (float) ParGetDoubleArrVal(PN_LIGHT_POS, i);
  SgSetLightPosition(lightPos);

  SgSetShadeModel(SHADE_DEFAULT);
  LightState = ParGetIntVal(PN_LIGHT);
  SgSetLight(LightState);
}

static void
molRemove(DhMolP molP, void *clientData)
{
  MolAttrP attrP;

  attrP = DhMolGetAttr(molP);
  if (attrP->objDefined)
    SgDestroyObj(attrP->objId);
}

void
GraphMolAdd(DhMolP molP)
{
  MolAttrP molAttrP;

  molAttrP = DhMolGetAttr(molP);
  if (molAttrP->objDefined)
    return;

  if (! RemoveCBAdded) {
    DhAddMolInvalidCB(molRemove, NULL);
    RemoveCBAdded = TRUE;
  }

  drawInit();
  molAttrP->objId = SgCreateObj(drawMolObj, molP);
  molAttrP->objDefined = TRUE;
  DhMolSetAttr(molP, molAttrP);
}

static void
molChanged(DhMolP molP, void *clientData)
{
  MolAttrP attrP;

  attrP = DhMolGetAttr(molP);
  if (! attrP->objDefined)
    return;

  drawInit();
  SgReplaceObj(attrP->objId, drawMolObj, molP);
  DhMolSetAttr(molP, attrP);
}

void
GraphMolChanged(char *prop)
{
  DhApplyMol(PropGetRef(prop, FALSE), molChanged, NULL);
}

static void
graphRefresh(void)
{
  if (! SgRefresh())
    GraphRedraw();
}

void
GraphRedrawEnable(BOOL onOff)
{
  if (onOff) {
    RedrawBlockLevel--;
    if (RedrawBlockLevel == 0) {
      if (RedrawPending) {
	GraphRedraw();
	RefreshPending = FALSE;
	RedrawPending = FALSE;
      } else if (RefreshPending) {
	graphRefresh();
	RefreshPending = FALSE;
      }
    }
  } else {
    RedrawBlockLevel++;
  }
}

void
GraphRedrawNeeded(void)
{
  if (RedrawBlockLevel > 0)
    RedrawPending = TRUE;
  else
    GraphRedraw();
}

void
GraphRefreshNeeded(void)
{
  if (RedrawBlockLevel > 0)
    RefreshPending = TRUE;
  else
    graphRefresh();
}

void
drawFrame(GraphDrawF drawF, void *clientData)
{
#ifdef PRINT_TIME
  clock_t t0 = clock();
#endif
  float d;
  float x, y, w, h;

  d = ViewP[2] * 0.05f;
  SgSetEye(SG_EYE_LEFT);

  switch (ParGetIntVal(PN_STEREO_MODE)) {
    case SM_OFF:
      ViewP[0] = 0.0f;
      SgSetViewPoint(ViewP);
      SgUpdateView();
      drawF(clientData);
      break;
    case SM_LEFT:
      ViewP[0] = - d;
      SgSetViewPoint(ViewP);
      SgUpdateView();
      drawF(clientData);
      break;
    case SM_RIGHT:
      ViewP[0] = d;
      SgSetViewPoint(ViewP);
      SgUpdateView();
      drawF(clientData);
      break;
    case SM_SIDE_BY_SIDE:
      SgGetViewport(&x, &y, &w, &h);

      ViewP[0] = - d;
      SgSetViewPoint(ViewP);
      SgSetViewport(x, y, 0.5f * w, h);
      SgUpdateView();
      drawF(clientData);

      SgSetEye(SG_EYE_RIGHT);
      ViewP[0] = d;
      SgSetViewPoint(ViewP);
      SgSetViewport(x + 0.5f * w, y, 0.5f * w, h);
      SgUpdateView();
      drawF(clientData);

      SgSetViewport(x, y, w, h);

      break;
    case SM_CROSS_EYE:
      SgGetViewport(&x, &y, &w, &h);

      ViewP[0] = d;
      SgSetViewPoint(ViewP);
      SgSetViewport(x, y, 0.5f * w, h);
      SgUpdateView();
      drawF(clientData);

      SgSetEye(SG_EYE_RIGHT);
      ViewP[0] = - d;
      SgSetViewPoint(ViewP);
      SgSetViewport(x + 0.5f * w, y, 0.5f * w, h);
      SgUpdateView();
      drawF(clientData);

      SgSetViewport(x, y, w, h);

      break;
    case SM_HARDWARE:
      if (SgGetStereo()) {
	ViewP[0] = -d;
	SgSetViewPoint(ViewP);
	SgUpdateView();
	drawF(clientData);

	SgSetEye(SG_EYE_RIGHT);
	ViewP[0] = d;
	SgSetViewPoint(ViewP);
	SgUpdateView();
	drawF(clientData);
      } else {
	/* hardware stereo not supported */
	ViewP[0] = 0.0f;
	SgSetViewPoint(ViewP);
	SgUpdateView();
	drawF(clientData);
      }
      break;
  }

#ifdef PRINT_TIME
  (void) printf("draw time: %d\n", clock() - t0);
#endif
}

static void
initBreak(void)
{
  char *dev;
  int slowness, prec;

  dev = SgGetDeviceName();
  if (strcmp(dev, "GL") == 0 ||
      strcmp(dev, "OpenGL") == 0 ||
      strcmp(dev, "XGL") == 0 ||
      strcmp(dev, "Pick") == 0)
    slowness = 1;
  else if (SgGetFeature(SG_FEATURE_HIDDEN))
    slowness = 10;
  else if (strcmp(dev, "X11") == 0)
    slowness = 2;
  else  /* plot device */
    slowness = 10;

  BreakPoly = 10000 / slowness;

  prec = 1 << ParGetIntVal(PN_DRAW_PREC);
  SpherePoly = 8 * prec * prec;
  CylinderPoly = 8 * prec;
  NeonPoly = CylinderPoly + SpherePoly / 2;

  PolyNo = 0;

  BreakActivate(TRUE);
}

static void
cleanupBreak(void)
{
  BreakActivate(FALSE);
}

void
GraphDraw(void)
/* Draw scene without calling IOEndFrame(), used for plotting
   and picking because they don't have a full IO device, only
   the Sg device. */
{
  drawInit();
  SgClear();
  initBreak();

  if (AltDrawF != NULL)
    AltDrawF(AltClientData);
  else
    drawFrame(drawScene, NULL);

  cleanupBreak();
  SgEndFrame();
}

void
GraphRedraw(void)
/* Used for screen redraw, calls IOEndFrame. */
{
  drawInit();
  SgClear();
  initBreak();

  if (AltDrawF != NULL) {
    AltDrawF(AltClientData);
  } else {
    drawFrame(drawScene, NULL);
    drawFrame(drawOverlay, NULL);
  }

  cleanupBreak();
  SgEndFrame();
  IOEndFrame();

  if (DrawDelay > 0)
    OsSleep(DrawDelay);
}

void
GraphDrawOverlay(GraphDrawF drawF, void *clientData)
{
  drawFrame(drawF, clientData);
  SgFlushFrame();
}

void
GraphAddOverlay(GraphDrawF drawF, void *clientData)
{
  OverlayData data;

  if (OverlayList == NULL)
    OverlayList = ListOpen(sizeof(OverlayData));

  data.drawF = drawF;
  data.clientData = clientData;

  (void) ListInsertLast(OverlayList, &data);
}

void
GraphShowAlt(GraphDrawF drawF, void *clientData)
{
  AltDrawF = drawF;
  AltClientData = clientData;

  GraphRedrawNeeded();
}
