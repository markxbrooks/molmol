/*
************************************************************************
*
*   PrimDraw.c - draw primitives
*
*   Copyright (c) 1994-2000
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/prim/SCCS/s.PrimDraw.c
*   SCCS identification       : 1.25
*
************************************************************************
*/

#include <prim_draw.h>

#include <stdio.h>
#include <stdlib.h>
#include <values.h>
#include <math.h>

#include <princip_axis.h>
#include <sg.h>
#include <par_hand.h>
#include <par_names.h>
#include <graph_draw.h>
#include <attr_struc.h>
#include "prim_struc.h"
#include "plate_calc.h"
#include "solid_calc.h"
#include "ribbon_calc.h"
#include "sheet_calc.h"

#define MAX_POINT_NO 128
#define ARROW_SIZE 2.0f

static int CosSinNo = -1;
static float CosA[MAX_POINT_NO];
static float SinA[MAX_POINT_NO];

static void
drawCylinder(PrimObjP primP)
{
  PrimCylinder *cylinderP;
  float rad;
  Vec3 x1, x2, v;

  cylinderP = &primP->u.cylinder;
  if (cylinderP->style == PCS_INVISIBLE)
    return;

  rad = primP->attrP->radius;

  SgDrawCone(cylinderP->x1, cylinderP->x2, rad, rad,
      SG_CONE_CAP_FLAT, SG_CONE_CAP_FLAT);

  if (cylinderP->style == PCS_NORMAL)
    return;

  Vec3Copy(v, cylinderP->x2);
  Vec3Sub(v, cylinderP->x1);
  Vec3Norm(v);

  Vec3Copy(x1, cylinderP->x2);
  Vec3ScaleAdd(x1, (1.0f - ARROW_SIZE) * rad, v);

  Vec3Copy(x2, cylinderP->x2);
  Vec3ScaleAdd(x2, rad, v);

  SgDrawCone(x1, x2, ARROW_SIZE * rad, 0.2f * rad,
      SG_CONE_CAP_FLAT, SG_CONE_CAP_ROUND);
}

static void
drawEllipsoid(Vec3 *coordA, int coordNo)
{
  Vec3 cent;
  Mat4 mapM;
  float lenA[3];
  Vec3 a1, a2, a3;
  float s1, s2, s3;
  int prec, n;
  float angInc, ang;
  Vec3 *xA, *nvA, x, nv;
  float ct, st, cp, sp;
  int traceI, nextI, pointI, i;

  Vec3Zero(cent);
  for (i = 0; i < coordNo; i++)
    Vec3Add(cent, coordA[i]);
  Vec3Scale(cent, 1.0f / coordNo);
  for (i = 0; i < coordNo; i++)
    Vec3Sub(coordA[i], cent);

  PrincipAxis(coordA, coordNo, FALSE, mapM, lenA);
  for (i = 0; i < 3; i++) {
    a1[i] = mapM[i][0];
    a2[i] = mapM[i][1];
    a3[i] = mapM[i][2];
  }
  s1 = lenA[0];
  s2 = lenA[1];
  s3 = lenA[2];

  prec = ParGetIntVal(PN_DRAW_PREC);
  n = 4 * (1 << prec);
  if (n > MAX_POINT_NO)
    n = MAX_POINT_NO;

  if (n != CosSinNo) {
    angInc =  2.0f * (float) M_PI / n;
    for (i = 0; i < n; i++) {
      ang = i * angInc;
      CosA[i] = cosf(ang);
      SinA[i] = sinf(ang);
    }
    CosSinNo = n;
  }

  xA = malloc(n * n * sizeof(*xA));
  nvA = malloc(n * n * sizeof(*nvA));

  Vec3Copy(x, cent);
  Vec3ScaleAdd(x, s1, a1);
  Vec3Copy(nv, a1);
  for (i = 0; i < n; i++) {
    Vec3Copy(xA[i * n], x);
    Vec3Copy(nvA[i * n], nv);
  }

  Vec3Copy(x, cent);
  Vec3ScaleSub(x, s1, a1);
  Vec3Copy(nv, a1);
  Vec3Scale(nv, -1.0f);
  for (i = 0; i < n; i++) {
    Vec3Copy(xA[(i + 1) * n - 1], x);
    Vec3Copy(nvA[(i+ 1)  * n - 1], nv);
  }

  for (traceI = 0; traceI < n; traceI++) {
    ct = CosA[traceI];
    st = SinA[traceI];
    nextI = (traceI + 1) % n;

    for (pointI = 1; pointI < n / 2; pointI++) {
      cp = CosA[pointI];
      sp = SinA[pointI];

      Vec3Copy(x, cent);
      Vec3ScaleAdd(x, s1 * cp, a1);
      Vec3ScaleAdd(x, s2 * sp * ct, a2);
      Vec3ScaleAdd(x, s3 * sp * st, a3);

      Vec3Copy(xA[traceI * n + 2 * pointI], x);
      Vec3Copy(xA[nextI * n + 2 * pointI - 1], x);

      Vec3Zero(nv);
      Vec3ScaleAdd(nv, s2 * s3 * cp * sp, a1);
      Vec3ScaleAdd(nv, s1 * s3 * sp * sp * ct, a2);
      Vec3ScaleAdd(nv, s1 * s2 * sp * sp * st, a3);
      Vec3Norm(nv);

      Vec3Copy(nvA[traceI * n + 2 * pointI], nv);
      Vec3Copy(nvA[nextI * n + 2 * pointI - 1], nv);
    }
  }

  SgStartSurface(SG_SURF_UNIFORM);
  for (i = 0; i < n; i++)
    SgDrawTriMesh(xA + i * n, nvA + i * n, n);
  SgEndSurface();

  free(xA);
  free(nvA);
}

static void
drawMap(PrimObjP primP)
{
  PrimMap *mapP;
  PrimMapPolyline *polylineP;
  float rad;
  int i, k;

  mapP = &primP->u.map;
  if (mapP->style == PMS_INVISIBLE)
    return;

  for (i = 0; i < mapP->polylineNo; i++) {
    polylineP = mapP->polylineA + i;
    if (mapP->style == PMS_LINE) {
      SgDrawPolyline(polylineP->xA, polylineP->pointNo);
    } else if (mapP->style == PMS_CYLINDER) {
      rad = primP->attrP->radius;
      for (k = 0; k < polylineP->pointNo - 1; k++)
        SgDrawCone(polylineP->xA[k], polylineP->xA[k + 1], rad, rad,
	    SG_CONE_OPEN, SG_CONE_OPEN);
    } else {
      rad = primP->attrP->radius;
      for (k = 0; k < polylineP->pointNo - 1; k++)
        SgDrawCone(polylineP->xA[k], polylineP->xA[k + 1], rad, rad,
	    SG_CONE_CAP_ROUND, SG_CONE_CAP_ROUND);
    }
  }
}

static void
drawTrajec(PrimObjP primP, float zoomFact)
{
  PrimTrajec *trajecP;
  float rad;
  int i;

  trajecP = &primP->u.trajec;
  if (trajecP->style == PTS_INVISIBLE)
    return;

  if (trajecP->xA == NULL)
    trajecP->xA = malloc(trajecP->atomNo * sizeof(*trajecP->xA));

  for (i = 0; i < trajecP->atomNo; i++) {
    DhAtomGetCoordTransf(trajecP->atomA[i], trajecP->xA[i]);
    Vec3Scale(trajecP->xA[i], zoomFact);
  }
  
  if (trajecP->style == PTS_LINE) {
    SgDrawPolyline(trajecP->xA, trajecP->atomNo);
  } else if (trajecP->style == PTS_NEON) {
    rad = primP->attrP->radius * zoomFact;
    for (i = 0; i < trajecP->atomNo - 1; i++)
      SgDrawCone(trajecP->xA[i], trajecP->xA[i + 1], rad, rad,
	  SG_CONE_CAP_ROUND, SG_CONE_CAP_ROUND);
  } else {
    drawEllipsoid(trajecP->xA, trajecP->atomNo);
  }
}

static void
drawPlate(PrimObjP primP)
{
  PrimPlate *plateP;
  int i;

  plateP = &primP->u.plate;

  SgStartSurface(SG_SURF_UNIFORM);

  for (i = 0; i < plateP->triNo; i++)
    SgDrawTriMesh(plateP->triA[i].xA, plateP->triA[i].nvA, 3);

  SgEndSurface();

  for (i = 0; i < plateP->quadNo; i++)
    SgDrawPolygon(plateP->quadA[i].xA, 4, plateP->quadA[i].nv);
}

static void
drawSolid(PrimObjP primP)
{
  PrimSolid *solidP;
  Vec3 x1, x2;
  int i;

  solidP = &primP->u.solid;

  switch (solidP->kind) {
    case PSK_SPHERE:
      SgDrawSphere(solidP->cent, solidP->size1);
      break;
    case PSK_CYLINDER:
      Vec3Copy(x1, solidP->cent);
      Vec3ScaleAdd(x1, solidP->size1, solidP->axis1);
      Vec3Copy(x2, solidP->cent);
      Vec3ScaleSub(x2, solidP->size1, solidP->axis1);
      SgDrawCone(x1, x2, solidP->size2, solidP->size2,
          SG_CONE_CAP_FLAT, SG_CONE_CAP_FLAT);
      break;
    case PSK_BOX:
    case PSK_CONE:
    case PSK_ELLIPSOID:
      if (solidP->meshNo > 0) {
        SgStartSurface(SG_SURF_UNIFORM);
        for (i = 0; i < solidP->meshNo; i++)
          SgDrawTriMesh(solidP->meshA[i].xA, solidP->meshA[i].nvA,
              solidP->meshA[i].pointNo);
        SgEndSurface();
      }
      for (i = 0; i < solidP->polyNo; i++)
	SgDrawPolygon(solidP->polyA[i].xA, solidP->polyA[i].pointNo,
	    solidP->polyA[i].nv);
      break;
  }
}

static int
getParInd(float par, int atomNo)
{
  int ind;

  ind = (int) (par + 0.5f);

  if (ind < 0)
    return 0;
  
  if (ind >= atomNo)
    return atomNo - 1;
  
  return ind;
}

static void
setAtomColor(DhAtomP atomP)
{
  AttrP attrP;

  attrP = DhAtomGetAttr(atomP);
  SgSetColor(attrP->colR, attrP->colG, attrP->colB);
}

static void
getAtomColor(DhAtomP atomP, float c[3])
{
  AttrP attrP;

  attrP = DhAtomGetAttr(atomP);
  c[0] = attrP->colR;
  c[1] = attrP->colG;
  c[2] = attrP->colB;
}

static void
drawRibbon(PrimObjP primP)
{
  PrimRibbonPart *partP;
  AttrP attrP;
  BOOL secColor;
  int maxLen, stripI;
  RibbonStrip *stripP;
  RibbonTrace *t1P, *t2P;
  int pI1, pI2, aI1, aI2;
  float (*col)[3], c1[3], c2[3], par;
  int polyI;

  partP = &primP->u.ribbon;
  attrP = primP->attrP;

  if (partP->style == RS_INVISIBLE)
    return;

  if (partP->traceNo == 1) {
    /* draw spline as single line */
    SgDrawPolyline(partP->traceA[0].xA, partP->stripA[0].pointNo);
    return;
  }

  if (partP->paint == RP_ATOM_SMOOTH) {
    maxLen = 0;
    for (stripI = 0; stripI < partP->stripNo; stripI++)
      if (partP->stripA[stripI].pointNo > maxLen)
	maxLen = partP->stripA[stripI].pointNo;
    col = malloc(maxLen * sizeof(*col));
  }

  if (partP->paint == RP_ATOM || partP->paint == RP_ATOM_SMOOTH)
    SgStartSurface(SG_SURF_COLOR);
  else
    SgStartSurface(SG_SURF_UNIFORM);

  secColor = FALSE;

  for (stripI = 0; stripI < partP->stripNo; stripI++) {
    stripP = partP->stripA + stripI;
    t1P = partP->traceA + stripP->traceLeftI;
    t2P = partP->traceA + stripP->traceRightI;

    if (partP->paint == RP_ATOM) {
      pI1 = 0;
      aI1 = getParInd(stripP->parA[0], partP->ribbonP->atomNo);
      for (pI2 = 0; pI2 < stripP->pointNo; pI2++) {
	aI2 = getParInd(stripP->parA[pI2], partP->ribbonP->atomNo);
	if (aI2 > aI1 || pI2 == stripP->pointNo - 1) {
	  setAtomColor(partP->ribbonP->atomA[aI1]);
	  if (stripP->flat)
	    SgDrawStrip(t1P->xA + pI1, t2P->xA + pI1,
		t1P->nvA + pI1, NULL, pI2 - pI1 + 1);
	  else
	    SgDrawStrip(t1P->xA + pI1, t2P->xA + pI1,
		t1P->nvA + pI1, t2P->nvA + pI1, pI2 - pI1 + 1);
	  pI1 = pI2;
	  aI1 = aI2;
	}
      }
      continue;
    }

    if (partP->paint == RP_ATOM_SMOOTH) {
      aI2 = getParInd(stripP->parA[0] - 0.5f, partP->ribbonP->atomNo);
      for (pI1 = 0; pI1 < stripP->pointNo; pI1++) {
	aI1 = getParInd(stripP->parA[pI1] - 0.5f, partP->ribbonP->atomNo);
	aI2 = getParInd(stripP->parA[pI1] + 0.5f, partP->ribbonP->atomNo);
	getAtomColor(partP->ribbonP->atomA[aI1], c1);
	getAtomColor(partP->ribbonP->atomA[aI2], c2);
	par = stripP->parA[pI1] - aI1;
	col[pI1][0] = (1.0f - par) * c1[0] + par * c2[0];
	col[pI1][1] = (1.0f - par) * c1[1] + par * c2[1];
	col[pI1][2] = (1.0f - par) * c1[2] + par * c2[2];
      }

      if (stripP->flat)
	SgDrawColorStrip(t1P->xA, t2P->xA,
	    t1P->nvA, NULL, col, NULL, stripP->pointNo);
      else
	SgDrawColorStrip(t1P->xA, t2P->xA,
	    t1P->nvA, t2P->nvA, col, NULL, stripP->pointNo);

      continue;
    }

    if (partP->paint == RP_TWO && ! secColor && stripP->secColor) {
      SgEndSurface();
      SgSetColor(attrP->secColR, attrP->secColG, attrP->secColB);
      SgStartSurface(SG_SURF_UNIFORM);
      secColor = TRUE;
    } else if (secColor && ! stripP->secColor) {
      SgEndSurface();
      SgSetColor(attrP->colR, attrP->colG, attrP->colB);
      SgStartSurface(SG_SURF_UNIFORM);
      secColor = FALSE;
    }

    if (stripP->flat)
      SgDrawStrip(t1P->xA, t2P->xA, t1P->nvA, NULL, stripP->pointNo);
    else
      SgDrawStrip(t1P->xA, t2P->xA, t1P->nvA, t2P->nvA, stripP->pointNo);
  }

  SgEndSurface();

  if (secColor)
    SgSetColor(attrP->colR, attrP->colG, attrP->colB);

  for (polyI = 0; polyI < partP->polyNo; polyI++) {
    if (partP->paint == RP_ATOM || partP->paint == RP_ATOM_SMOOTH) {
      aI1 = getParInd(partP->polyA[polyI].par, partP->ribbonP->atomNo);
      setAtomColor(partP->ribbonP->atomA[aI1]);
    }

    SgDrawPolygon(partP->polyA[polyI].xA, partP->polyA[polyI].pointNo,
	partP->polyA[polyI].nv);
  }

  if (partP->paint == RP_ATOM ||
      (partP->paint == RP_ATOM_SMOOTH && partP->polyNo > 0))
    SgSetColor(attrP->colR, attrP->colG, attrP->colB);

  if (partP->paint == RP_ATOM_SMOOTH)
    free(col);
}

static void
drawSheet(PrimObjP primP)
{
  PrimSheet *sheetP;
  AttrP attrP;
  int tNo, iMax, i;

  sheetP = &primP->u.sheet;
  if (sheetP->xA1 == NULL)
    return;

  attrP = primP->attrP;
  tNo = sheetP->traceNo;
  
  if (sheetP->closed)
    iMax = tNo;
  else
    iMax = tNo - 1;

  SgStartSurface(SG_SURF_UNIFORM);
  for (i = 0; i < iMax; i++)
    SgDrawStrip(sheetP->xA1 + i * sheetP->traceLen,
	sheetP->xA1 + ((i + 1) % tNo) * sheetP->traceLen,
	sheetP->nvA1 + i * sheetP->traceLen,
	sheetP->nvA1 + ((i + 1) % tNo) * sheetP->traceLen,
	sheetP->traceLen);
  SgEndSurface();

  SgSetColor(attrP->secColR, attrP->secColG, attrP->secColB);

  SgStartSurface(SG_SURF_UNIFORM);
  for (i = 0; i < iMax; i++)
    SgDrawStrip(sheetP->xA2 + i * sheetP->traceLen,
	sheetP->xA2 + ((i + 1) % tNo) * sheetP->traceLen,
	sheetP->nvA2 + i * sheetP->traceLen,
	sheetP->nvA2 + ((i + 1) % tNo) * sheetP->traceLen,
	sheetP->traceLen);
  SgEndSurface();

  SgSetColor(attrP->colR, attrP->colG, attrP->colB);
}

static void
drawSurface(PrimSurface *surfaceP)
{
  Vec3 *xA;
  Vec3 *nvA;
  Vec3 *colA;
  PrimSurfaceMesh *meshP;
  int meshI, i;

  xA = malloc(surfaceP->maxSize * sizeof(*xA));
  nvA = malloc(surfaceP->maxSize * sizeof(*nvA));
  if (surfaceP->colA != NULL) {
    colA = malloc(surfaceP->maxSize * sizeof(*colA));
    SgStartSurface(SG_SURF_COLOR);
  } else {
    colA = NULL;
    SgStartSurface(SG_SURF_UNIFORM);
  }

  for (meshI = 0; meshI < surfaceP->meshNo; meshI++) {
    meshP = surfaceP->meshA + meshI;

    for (i = 0; i < meshP->size; i++) {
      Vec3Copy(xA[i], surfaceP->xA[meshP->indA[i]]);
      Vec3Copy(nvA[i], surfaceP->nvA[meshP->indA[i]]);
      if (colA != NULL)
	Vec3Copy(colA[i], surfaceP->colA[meshP->indA[i]]);
    }

    if (colA != NULL)
      SgDrawColorTriMesh(xA, nvA, colA, meshP->size);
    else
      SgDrawTriMesh(xA, nvA, meshP->size);
  }

  SgEndSurface();

  free(xA);
  free(nvA);
  if (colA != NULL)
    free(colA);
}

static void
drawDrawobj(PrimObjP primP, float zoomFact)
{
  PrimDrawobj *objP = &primP->u.drawobj;
  Vec3 x[2], xr3[5], vz;
  float xr2[5][2];
  float fact, rad;
  int i;

  if (objP->style == PDS_INVISIBLE)
    return;

  /* pointNo is always 2 at the moment */

  if (primP->molP == NULL) {
    for (i = 0; i < 2; i++) {
      x[i][0] = objP->xA[i][0] + objP->dxA[i][0];
      x[i][1] = objP->xA[i][1] + objP->dxA[i][1];
    }
    switch (objP->type) {
      case PDT_LINE:
	SgDrawLine2D(x[0], x[1]);
	break;
      case PDT_RECT:
	xr2[0][0] = x[0][0];
	xr2[0][1] = x[0][1];
	xr2[1][0] = x[1][0];
	xr2[1][1] = x[0][1];
	xr2[2][0] = x[1][0];
	xr2[2][1] = x[1][1];
	xr2[3][0] = x[0][0];
	xr2[3][1] = x[1][1];
	xr2[4][0] = x[0][0];
	xr2[4][1] = x[0][1];
	SgDrawPolyline2D(xr2, 5);
	break;
      case PDT_CIRCLE:
	SgDrawCircle2D(x[0], Vec3DiffAbs(x[0], x[1]));
	break;
    }
  } else {
    for (i = 0; i < 2; i++) {
      Vec3Copy(x[i], objP->xA[i]);
      DhTransfVec(x[i], primP->molP);
      Vec3Scale(x[i], zoomFact);
      fact = SgGetDepthFact(0.0f);
      x[i][0] += fact * objP->dxA[i][0];
      x[i][1] += fact * objP->dxA[i][1];
      x[i][2] += objP->dxA[i][2];
    }

    rad = primP->attrP->radius * zoomFact;

    switch (objP->type) {
      case PDT_LINE:
	if (objP->style == PDS_LINE)
	  SgDrawLine(x[0], x[1]);
	else if (objP->style == PDS_CYLINDER)
	  SgDrawCone(x[0], x[1], rad, rad,
	      SG_CONE_CAP_FLAT, SG_CONE_CAP_FLAT);
	else
	  SgDrawCone(x[0], x[1], rad, rad,
	      SG_CONE_CAP_ROUND, SG_CONE_CAP_ROUND);
	break;
      case PDT_RECT:
	Vec3Copy(xr3[0], x[0]);
	Vec3Copy(xr3[1], x[0]);
	xr3[1][0] = x[1][0];
	Vec3Copy(xr3[2], x[1]);
	Vec3Copy(xr3[3], x[1]);
	xr3[3][0] = x[0][0];

	if (objP->style == PDS_LINE) {
	  Vec3Copy(xr3[4], x[0]);
	  SgDrawPolyline(xr3, 5);
	} else {
	  SgDrawCone(xr3[0], xr3[1], rad, rad,
	      SG_CONE_CAP_ROUND, SG_CONE_CAP_ROUND);
	  SgDrawCone(xr3[1], xr3[2], rad, rad,
	      SG_CONE_CAP_ROUND, SG_CONE_CAP_ROUND);
	  SgDrawCone(xr3[2], xr3[3], rad, rad,
	      SG_CONE_CAP_ROUND, SG_CONE_CAP_ROUND);
	  SgDrawCone(xr3[3], xr3[0], rad, rad,
	      SG_CONE_CAP_ROUND, SG_CONE_CAP_ROUND);
	}
	break;
      case PDT_CIRCLE:
	vz[0] = 0.0f;
	vz[1] = 0.0f;
	vz[2] = 1.0f;
	SgDrawCircle(x[0], vz, Vec3DiffAbs(x[0], x[1]));
	break;
    }
  }
}

void
PrimDraw(PrimObjP primP, float zoomFact)
{
  PrimCircle *circleP;
  PrimText *textP;
  PrimDotSurface *surfaceP;
  PrimDotSurfacePatch *patchP;
  AttrP attrP;
  Vec3 x1;
  int i;

  switch (primP->type) {
    case PT_CIRCLE:
      circleP = &primP->u.circle;
      SgDrawCircle(circleP->x, circleP->v, primP->attrP->radius);
      break;
    case PT_MAP:
      drawMap(primP);
      break;
    case PT_TRAJEC:
      drawTrajec(primP, zoomFact);
      break;
    case PT_PLATE:
      PlateCalcPoints(primP);
      drawPlate(primP);
      break;
    case PT_CYLINDER:
      drawCylinder(primP);
      break;
    case PT_SOLID:
      SolidCalcPoints(primP);
      drawSolid(primP);
      break;
    case PT_TEXT:
      textP = &primP->u.text;

      if (primP->molP == NULL) {
	x1[0] = textP->x[0] + textP->dx[0];
	x1[1] = textP->x[1] + textP->dx[1];
	SgDrawText2D(x1, textP->str);
      } else {
	Vec3Copy(x1, textP->x);
	DhTransfVec(x1, primP->molP);
	Vec3Scale(x1, zoomFact);
	x1[2] += textP->dx[2];

	SgDrawText(x1, textP->dx, textP->str);
      }

      break;
    case PT_DOT_SURFACE:
      surfaceP = &primP->u.dotSurface;

      for (i = 0; i < surfaceP->size; i++) {
        patchP = surfaceP->p + i;
        if (patchP->atomP == NULL)
          attrP = primP->attrP;
        else
          attrP = DhAtomGetAttr(patchP->atomP);

	GraphSetAttr(attrP);
	SgSetLight(SG_LIGHT_OFF);

        SgDrawMarkers(patchP->p, patchP->size);
      }

      if (attrP->shadeModel != -1) {
        SgSetShadeModel(attrP->shadeModel);
        SgSetLight(ParGetIntVal(PN_LIGHT));
      }

      break;
    case PT_SURFACE:
      drawSurface(&primP->u.surface);
      break;
    case PT_RIBBON:
      RibbonCalc(primP);
      drawRibbon(primP);
      break;
    case PT_SHEET:
      SheetCalc(primP);
      drawSheet(primP);
      break;
    case PT_DRAWOBJ:
      drawDrawobj(primP, zoomFact);
      break;
  }
}
