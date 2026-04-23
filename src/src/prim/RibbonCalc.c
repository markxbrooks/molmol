/*
************************************************************************
*
*   RibbonCalc.c - calculate ribbon
*
*   Copyright (c) 1994-97
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/prim/SCCS/s.RibbonCalc.c
*   SCCS identification       : 1.20
*
************************************************************************
*/

#include "ribbon_calc.h"

#include <stdio.h>
#include <stdlib.h>
#include <values.h>
#include <math.h>

#include <break.h>
#include <cubic_spline.h>
#include <fit_cylinder.h>
#include <par_hand.h>
#include <par_names.h>
#include <attr_struc.h>
#include "prim_struc.h"

#define MAX_POINT_NO 128
#define PAR_UNKNOWN -999.9f
#define MIN_LEN 0.01f
#define MIN_RAD 0.02f  /* 0.0 leads to problems (normals) */
#define MAX_BULGE 3.0f

typedef struct {
  float c3, c2, c1, c0;
} RadCoeff;

typedef struct {
  Vec3 nv;
  float d;
  Ribbon *ribbonP;
} IntersData;

static float CosTab[MAX_POINT_NO] = {0.0f};
static float SinTab[MAX_POINT_NO];

static void
calcCosSinTab(void)
{
  float angInc, ang;
  int i;

  if (CosTab[0] != 0.0f)
    return;

  angInc = 2.0f * (float) M_PI / MAX_POINT_NO;

  for (i = 0; i < MAX_POINT_NO; i++) {
    ang = i * angInc;
    CosTab[i] = cosf(ang);
    SinTab[i] = sinf(ang);
  }
}

static BOOL
isRibbonPart(PrimObjP primP, Ribbon *ribbonP)
{
  if (primP == NULL)
    return FALSE;
  
  if (primP->type != PT_RIBBON)
    return FALSE;
  
  if (primP->u.ribbon.ribbonP != ribbonP)
    return FALSE;
  
  return TRUE;
}

static void
getResAtoms(Ribbon *ribbonP, DhResP resP, int *startIP, int *endIP)
{
  int atomI;

  *startIP = -1;
  *endIP = -1;

  if (resP == NULL)
    return;

  for (atomI = 0; atomI < ribbonP->atomNo; atomI++)
    if (DhAtomGetRes(ribbonP->atomA[atomI]) == resP) {
      if (*startIP == -1)
	*startIP = atomI;
      *endIP = atomI;
    } else if (*startIP >=0) {
      break;
    }
}

static void
calcSpline(Ribbon *ribbonP, PrimObjP primP)
{
  Vec3 *xA, x;
  LINLIST primL;
  PrimObjP firstP;
  int atomI;

  xA = malloc(ribbonP->atomNo * sizeof(*xA));

  primL = PrimListGet();

  firstP = primP;
  for (;;) {
    primP = ListPrev(primL, firstP);
    if (! isRibbonPart(primP, ribbonP))
      break;
    firstP = primP;
  }

  primP = firstP;

  for (atomI = 0; atomI < ribbonP->atomNo; atomI++) {
    DhAtomGetCoord(ribbonP->atomA[atomI], xA[atomI]);

    if (primP != NULL && atomI > primP->u.ribbon.endPar) {
      primP = ListNext(primL, primP);
      if (! isRibbonPart(primP, ribbonP))
	primP = NULL;
    }

    if (primP != NULL && primP->u.ribbon.shape == RS_SMOOTH &&
	atomI > primP->u.ribbon.startPar &&
	atomI < primP->u.ribbon.endPar &&
	atomI > 0 && atomI < ribbonP->atomNo - 1) {
      /* use previous and next atom for smoothing */
      Vec3Scale(xA[atomI], 0.5f);
      DhAtomGetCoord(ribbonP->atomA[atomI - 1], x);
      Vec3ScaleAdd(xA[atomI], 0.25f, x);
      DhAtomGetCoord(ribbonP->atomA[atomI + 1], x);
      Vec3ScaleAdd(xA[atomI], 0.25f, x);
    }
  }

  if (ribbonP->coeffA == NULL)
    ribbonP->coeffA = malloc((ribbonP->atomNo - 1) *
	sizeof(*ribbonP->coeffA));

  CubicSplineCalc(xA, ribbonP->atomNo, ribbonP->coeffA);

  free(xA);

  primP = firstP;
  while (isRibbonPart(primP, ribbonP)) {
    primP->u.ribbon.pointValid = FALSE;
    primP = ListNext(primL, primP);
  }
}

static void
getSplinePoint(Ribbon *ribbonP, float par, Vec3 x, Vec3 dz)
{
  CubicSplineCoeff *coeffP;
  int parI, i;

  parI = (int) par;

  if (parI < 0)
    parI = 0;
  else if (parI > ribbonP->atomNo - 2)
    parI = ribbonP->atomNo - 2;

  coeffP = ribbonP->coeffA + parI;
  par -= parI;

  for (i = 0; i < 3; i++) {
    x[i] = ((coeffP->c3[i] * par + coeffP->c2[i]) * par +
	coeffP->c1[i]) * par + coeffP->c0[i];
    if (dz != NULL)
      dz[i] = (3.0f * coeffP->c3[i] * par +
	  2.0f * coeffP->c2[i]) * par + coeffP->c1[i];
  }
}

static void
getSplinePoints(PrimRibbonPart *partP, Ribbon *ribbonP,
    float sPar, float ePar, int pointNo,
    float *parA, Vec3 *xA, Vec3 *dzA)
{
  float parInc, par;
  int pointI;

  if (pointNo < 2)
    return;

  parInc = (ePar - sPar) / (pointNo - 1);

  par = sPar;
  for (pointI = 0; pointI < pointNo; pointI++) {
    if (parA != NULL)
      parA[pointI] = par;

    if (dzA == NULL)
      getSplinePoint(ribbonP, par, xA[pointI], NULL);
    else
      getSplinePoint(ribbonP, par, xA[pointI], dzA[pointI]);

    par += parInc;
  }
}

static BOOL
hasVarRad(PrimObjP primP)
{
  return primP->u.ribbon.style == RS_ROUND &&
      primP->u.ribbon.radius == RR_ATOM &&
      primP->u.ribbon.endStyle != RE_ARROW;
}

static int
getParInd(float par, int atomNo)
{
  int ind;

  ind = (int) (par + 0.5f);
  if (ind < 0)
    ind = 0;
  else if (ind >= atomNo)
    ind = atomNo - 1;

  return ind;
}

static RadCoeff *
calcRadSpline(float *radA, int radNo, float startPar, float endPar)
{
  int startI, endI, intNo;
  RadCoeff *coeffA;
  int intI, i1, i2;
  float v0, v1, v2, v3, d1, d2;

  startI = getParInd(startPar, radNo);
  endI = getParInd(endPar, radNo);

  intNo = endI - startI;
  if (intNo == 0)
    intNo = 1;
  
  coeffA = malloc(intNo * sizeof(*coeffA));

  for (intI = 0; intI < intNo; intI++) {
    i1 = startI + intI;
    i2 = i1 + 1;
    if (i2 > endI)
      i2 = endI;
    
    v1 = radA[i1];
    v2 = radA[i2];

    if (i1 == startI) {
      d1 = 0.0f;
    } else {
      v0 = radA[i1 - 1];
      if (v1 < v0 && v1 < v2)
	/* set deviations to 0 at "minima" to avoid radii < 0
	   in interpolation */
	d1 = 0.0f;
      else
	d1 = 0.5f * (v2 - v0);
    }

    if (i2 == endI) {
      d2 = 0.0f;
    } else {
      v3 = radA[i2 + 1];
      if (v2 < v1 && v2 < v3)
	d2 = 0.0f;
      else
	d2 = 0.5f * (v3 - v1);
    }

    coeffA[intI].c0 = v1;
    coeffA[intI].c1 = d1;
    coeffA[intI].c2 = 3.0f * (v2 - v1) - 2.0f * d1 - d2;
    coeffA[intI].c3 = 2.0f * (v1 - v2) + d1 + d2;
  }

  return coeffA;
}

static void
getRadSplinePoint(int radNo, RadCoeff *coeffA,
    float startPar, float endPar, float par,
    float *vP, float *dP)
{
  int startI, endI;
  int intNo, intI;

  startI = getParInd(startPar, radNo);
  endI = getParInd(endPar, radNo);

  intNo = endI - startI;
  if (intNo == 0)
    intNo = 1;
  
  intI = (int) par - startI;
  if (intI < 0)
    intI = 0;
  else if (intI >= intNo)
    intI = intNo - 1;
  
  par -= intI + startI;
  if (par < 0.0f)
    par = 0.0f;
  else if (par > 1.0f)
    par = 1.0f;
  
  *vP = ((coeffA[intI].c3 * par + coeffA[intI].c2) * par +
      coeffA[intI].c1) * par + coeffA[intI].c0;
  if (dP != NULL)
    *dP = (3.0f * coeffA[intI].c3 * par +
	2.0f * coeffA[intI].c2) * par + coeffA[intI].c1;
}

static void
getAxisDirV(PrimRibbonPart *partP, Ribbon *ribbonP, Vec3 axisDirV)
{
  int pointNo, startI, endI, i;
  Vec3 *xA, x1, x2;
  float r;

  if (partP->orient == RO_CYLIND) {
    pointNo = (int) (2.0f * (partP->endPar - partP->startPar));
    if (pointNo < 5)
      pointNo = 5;

    xA = malloc(pointNo * sizeof(*xA));

    getSplinePoints(partP, ribbonP, partP->startPar, partP->endPar, pointNo,
	NULL, xA, NULL);
    FitCylinderLeastSq(xA, pointNo, x1, x2, &r);
  } else {
    startI = (int) (partP->startPar + 1.0f);
    if (startI < 0)
      startI = 0;
    if (startI >= ribbonP->atomNo)
      startI = ribbonP->atomNo - 1;

    endI = (int) partP->endPar;
    if (endI < 0)
      endI = 0;
    if (endI >= ribbonP->atomNo)
      endI = ribbonP->atomNo - 1;

    pointNo = endI - startI + 1;
    if (pointNo < 4) {
      xA = malloc(4 * sizeof(*xA));
      getSplinePoints(partP, ribbonP, partP->startPar, partP->endPar, 4,
	  NULL, xA, NULL);
      FitCylinderSpacing(xA, 4, x1, x2, &r);
    } else {
      xA = malloc(pointNo * sizeof(*xA));
      for (i = 0; i < pointNo; i++)
	DhAtomGetCoord(ribbonP->atomA[startI + i], xA[i]);
      FitCylinderSpacing(xA, pointNo, x1, x2, &r);
    }
  }

  free(xA);

  Vec3Copy(axisDirV, x2);
  Vec3Sub(axisDirV, x1);
  Vec3Norm(axisDirV);
}

static int
getCircPointNo(PrimRibbonPart *partP)
{
  int n;

  n = 4 * (1 << partP->prec);
  if (n > MAX_POINT_NO)
    n = MAX_POINT_NO;

  return n;
}

static int
getProf(PrimRibbonPart *partP,
    float **xProfP, float **yProfP, float **xNormP, float **yNormP,
    BOOL **insideP)
{
  float *xProf, *yProf, *xNorm, *yNorm;
  BOOL *inside;
  float rad, width, thick, a, angInc, ang, x, y;
  int n, tabInc, tNo, wNo, i;

  n = getCircPointNo(partP);
  calcCosSinTab();
  tabInc = MAX_POINT_NO / n;

  xProf = malloc(n * sizeof(*xProf));
  yProf = malloc(n * sizeof(*yProf));
  xNorm = malloc(n * sizeof(*xNorm));
  yNorm = malloc(n * sizeof(*yNorm));
  inside = malloc(n * sizeof(*inside));

  width = 0.5f * partP->width;
  thick = 0.5f * partP->thick;
  if (thick > width)
    thick = width;

  switch (partP->style) {
    case RS_RECT:
      tNo = (int) (thick / (width + thick) * (n / 2) + 0.5f);
      if (tNo == 0)
	tNo = 1;
      wNo = n / 2 - tNo;

      for (i = 0; i < tNo / 2; i++) {
	xProf[i] = width;
	yProf[i] = i * partP->thick / tNo;
	xNorm[i] = 1.0f;
	yNorm[i] = 0.0f;
	inside[i] = FALSE;
      }

      for (i = 0; i < wNo; i++) {
	xProf[tNo / 2 + i] = width - i * partP->width / wNo;
	yProf[tNo / 2 + i] = thick;
	xNorm[tNo / 2 + i] = 0.0f;
	yNorm[tNo / 2 + i] = 1.0f;
	inside[tNo / 2 + i] = FALSE;
      }

      for (i = 0; i < tNo; i++) {
	xProf[tNo / 2 + wNo + i] = - width;
	yProf[tNo / 2 + wNo + i] = thick - i * partP->thick / tNo;
	xNorm[tNo / 2 + wNo + i] = -1.0f;
	yNorm[tNo / 2 + wNo + i] = 0.0f;
	inside[tNo / 2 + wNo + i] = FALSE;
      }

      for (i = 0; i < wNo; i++) {
	xProf[tNo / 2 + wNo + tNo + i] = - width + i * partP->width / wNo;
	yProf[tNo / 2 + wNo + tNo + i] = - thick;
	xNorm[tNo / 2 + wNo + tNo + i] = 0.0f;
	yNorm[tNo / 2 + wNo + tNo + i] = -1.0f;
	inside[tNo / 2 + wNo + tNo + i] = TRUE;
      }

      for (i = tNo / 2; i < tNo; i++) {
	xProf[2 * wNo + tNo + i] = width;
	yProf[2 * wNo + tNo + i] = - thick +
	    (i - tNo / 2) * partP->thick / tNo;
	xNorm[2 * wNo + tNo + i] = 1.0f;
	yNorm[2 * wNo + tNo + i] = 0.0f;
	inside[2 * wNo + tNo + i] = FALSE;
      }

      break;
    case RS_ROUND:
      rad = partP->midRad;
      for (i = 0; i < n; i++) {
	xProf[i] = rad * CosTab[i * tabInc];
	yProf[i] = rad * SinTab[i * tabInc];
	xNorm[i] = CosTab[i * tabInc];
	yNorm[i] = SinTab[i * tabInc];
	inside[i] = FALSE;
      }
      break;
    case RS_ROUNDED:
      for (i = 0; i < n / 4; i++)
	xProf[i] = width + thick * (CosTab[i * tabInc] - 1.0f);
      for (i = n / 4; i < 3 * n / 4; i++)
	xProf[i] = - width + thick * (CosTab[i * tabInc] + 1.0f);
      for (i = 3 * n / 4; i < n; i++)
	xProf[i] = width + thick * (CosTab[i * tabInc] - 1.0f);
      for (i = 0; i < n; i++) {
	yProf[i] = thick * SinTab[i * tabInc];
	xNorm[i] = CosTab[i * tabInc];
	yNorm[i] = SinTab[i * tabInc];
	inside[i] = FALSE;
      }
      inside[3 * n / 4 - 1] = TRUE;
      break;
    case RS_ELLIP:
      for (i = 0; i < n; i++) {
	xProf[i] = width * CosTab[i * tabInc];
	yProf[i] = thick * SinTab[i * tabInc];
	xNorm[i] = thick * CosTab[i * tabInc];
	yNorm[i] = width * SinTab[i * tabInc];
	a = sqrtf(xNorm[i] * xNorm[i] + yNorm[i] * yNorm[i]);
	xNorm[i] /= a;
	yNorm[i] /= a;
	inside[i] = (i >= n / 2);
      }
      break;
    case RS_LENSE:
      rad = 0.5f * (thick + width * width / thick);
      ang = atan2f(width / rad, 1.0f - thick / rad);
      angInc = 2.0f * ang / (n / 2 - 1);
      for (i = 0; i < n / 4; i++) {
	x = sinf(ang);
	y = cosf(ang);
	xNorm[i] = x;
	yNorm[i] = y;
	xNorm[n / 2 - 1 - i] = - x;
	yNorm[n / 2 - 1 - i] = y;
	xNorm[n / 2 + i] = - x;
	yNorm[n / 2 + i] = - y;
	xNorm[n - 1 - i] = x;
	yNorm[n - 1 - i] = - y;

	x = rad * x;
	y = rad * (y - 1.0f) + thick;
	xProf[i] = x;
	yProf[i] = y;
	xProf[n / 2 - 1 - i] = - x;
	yProf[n / 2 - 1 - i] = y;
	xProf[n / 2 + i] = - x;
	yProf[n / 2 + i] = - y;
	xProf[n - 1 - i] = x;
	yProf[n - 1 - i] = - y;
	ang -= angInc;

	inside[i] = FALSE;
	inside[n / 2 - 1 - i] = FALSE;
	inside[n / 2 + i] = TRUE;
	inside[n - 1 - i] = TRUE;
      }
      break;
  }

  *xProfP = xProf;
  *yProfP = yProf;
  *xNormP = xNorm;
  *yNormP = yNorm;
  *insideP = inside;

  return n;
}

static void
getStartV(PrimRibbonPart *partP, Vec3 dz, PrimObjP prevP,
    BOOL startSet, Vec3 startV)
{
  Vec3 dx, dy;
  float ang;
  int n, tabInc, angI;

  if (prevP == NULL || ! prevP->u.ribbon.pointValid) {
    if (! startSet) {
      startV[0] = dz[1];
      startV[1] = - dz[0];
      startV[2] = 0.0f;
      Vec3Norm(startV);
    }
    return;
  }

  if (! startSet) {
    Vec3Copy(startV, prevP->u.ribbon.endRefV);
    return;
  }

  Vec3Copy(dx, prevP->u.ribbon.endRefV);
  Vec3ScaleSub(dx, Vec3Scalar(dx, dz), dz);
  Vec3Copy(dy, dz);
  Vec3Cross(dy, dx);

  n = getCircPointNo(partP);
  calcCosSinTab();
  tabInc = MAX_POINT_NO / n;

  ang = atan2f(Vec3Scalar(startV, dy), Vec3Scalar(startV, dx));
  if (ang < 0.0f)
    ang += 2.0f * (float) M_PI;

  angI = (int) ((ang / (2.0f * (float) M_PI)) * n + 0.5f);
  if (angI == n)
    angI = 0;

  Vec3Zero(startV);
  Vec3ScaleAdd(startV, CosTab[angI * tabInc], dx);
  Vec3ScaleAdd(startV, SinTab[angI * tabInc], dy);
}

static BOOL
isUnknown(float par)
{
  return par < PAR_UNKNOWN + 1.0f;
}

static void
complPar(float *parA, int n, BOOL *setP)
{
  int valI, i, k;
  float parInc;

  valI = -1;

  if (n > 1) {
    /* hack to avoid artifacts when two beta sheets directly follow
       each other */
    if (parA[0] < parA[1] - MAX_BULGE ||
	parA[0] > parA[1] + MAX_BULGE)
      parA[0] = PAR_UNKNOWN;
    if (parA[n - 1] > parA[n - 2] + MAX_BULGE ||
	parA[n - 1] < parA[n - 2] - MAX_BULGE)
      parA[n - 1] = PAR_UNKNOWN;
  }

  for (i = 0; i < n; i++) {
    if (isUnknown(parA[i]))
      continue;
    
    if (valI >= 0 && i - valI > 1) {
      if (parA[i] > parA[valI] - MAX_BULGE &&
	  parA[i] < parA[valI] + MAX_BULGE) {
	parInc = (parA[i] - parA[valI]) / (i - valI);
	for (k = valI + 1; k < i; k++)
	  parA[k] = parA[k - 1] + parInc;
      }
    }

    valI = i;
  }

  if (valI == -1)  /* all unknown */
    *setP = FALSE;
}

static void
getAtomRange(float startPar, float endPar, int atomNo,
    int *startIP, int *endIP)
{
  int startI, endI;

  startI = (int) startPar;
  if (startI < 0)
    startI = 0;
  if (startI >= atomNo)
    startI = atomNo - 1;

  endI = (int) endPar + 1;
  if (endI < 0)
    endI = 0;
  if (endI >= atomNo)
    endI = atomNo - 1;

  *startIP = startI;
  *endIP = endI;
}

static void
calcNeighPar(Ribbon *ribbonP, PrimObjP primP,
    float **leftParAP, float **rightParAP)
{
  float *par1A, *par2A;
  int atomI, startI, endI, parNo;
  DhResP resP, lastResP, res1P, res2P;
  BOOL set1, set2;
  int start1I, end1I, start2I, end2I;

  getAtomRange(primP->u.ribbon.startPar, primP->u.ribbon.endPar,
      ribbonP->atomNo, &startI, &endI);
  parNo = endI - startI + 1;
  par1A = malloc(parNo * sizeof(*par1A));
  par2A = malloc(parNo * sizeof(*par2A));

  set1 = FALSE;
  set2 = FALSE;
  lastResP = NULL;

  for (atomI = startI; atomI <= endI; atomI++) {
    resP = DhAtomGetRes(ribbonP->atomA[atomI]);

    if (resP == lastResP) {
      par1A[atomI - startI] = par1A[atomI - startI - 1];
      par2A[atomI - startI] = par2A[atomI - startI - 1];
      continue;
    }

    lastResP = resP;

    res1P = DhResGetNeigh(resP, NC_LEFT);
    res2P = DhResGetNeigh(resP, NC_RIGHT);

    getResAtoms(ribbonP, res1P, &start1I, &end1I);
    getResAtoms(ribbonP, res2P, &start2I, &end2I);

    if (start1I >= 0) {
      par1A[atomI - startI] = 0.5f * (end1I + start1I);
      set1 = TRUE;
    } else {
      par1A[atomI - startI] = PAR_UNKNOWN;
    }

    if (start2I >= 0) {
      par2A[atomI - startI] = 0.5f * (end2I + start2I);
      set2 = TRUE;
    } else {
      par2A[atomI - startI] = PAR_UNKNOWN;
    }
  }

  if (set1)
    complPar(par1A, parNo, &set1);
  
  if (set2)
    complPar(par2A, parNo, &set2);

  if (! set1) {
    free(par1A);
    par1A = NULL;
  }

  if (! set2) {
    free(par2A);
    par2A = NULL;
  }

  *leftParAP = par1A;
  *rightParAP = par2A;
}

static void
getOneNeighV(Ribbon *ribbonP, Vec3 x, float par,
    float *parA, int parNo, int startI, int parI, Vec3 neighV)
{
  int par1I, inc;
  Vec3 x1;

  if (parI >= parNo - 1 || isUnknown(parA[parI]) ||
      isUnknown(parA[parI + 1])) {
    inc = 0;
    for (;;) {
      par1I = parI - inc;
      if (par1I >= 0 && ! isUnknown(parA[par1I]))
	break;

      par1I = parI + inc;
      if (par1I < parNo && ! isUnknown(parA[par1I]))
	break;

      inc++;
    }

    getSplinePoint(ribbonP, (float) (startI + par1I), x1, NULL);
    getSplinePoint(ribbonP, parA[par1I], neighV, NULL);
    Vec3Sub(neighV, x1);
  } else {
    par = parA[parI] + (par - (startI + parI)) *
	(parA[parI + 1] - parA[parI]);
    getSplinePoint(ribbonP, par, neighV, NULL);
    Vec3Sub(neighV, x);
  }
}

static void
getNeighV(Ribbon *ribbonP, PrimObjP primP, Vec3 x, Vec3 dz, float par,
    float *leftParA, float *rightParA, Vec3 neighV)
{
  int startI, endI, parNo, parI;
  Vec3 v1, v2;

  if (leftParA == NULL && rightParA == NULL) {
    neighV[0] = dz[1];
    neighV[1] = - dz[0];
    neighV[2] = 0.0f;
    Vec3Norm(neighV);
    return;
  }

  getAtomRange(primP->u.ribbon.startPar, primP->u.ribbon.endPar,
      ribbonP->atomNo, &startI, &endI);
  parNo = endI - startI + 1;

  parI = (int) par;
  if (parI < startI)
    parI = startI;
  else if (parI > endI)
    parI = endI;
  
  parI -= startI;

  if (leftParA != NULL)
    getOneNeighV(ribbonP, x, par, leftParA, parNo, startI, parI, v1);
  else
    Vec3Zero(v1);

  if (rightParA != NULL)
    getOneNeighV(ribbonP, x, par, rightParA, parNo, startI, parI, v2);
  else
    Vec3Zero(v2);
  
  Vec3Copy(neighV, v1);
  Vec3Sub(neighV, v2);
  Vec3Norm(neighV);
}

static float
smoothVal(float y1, float y2, int pointI, int tabInc)
{
  return 0.5f * (y1 + y2 + (y1 - y2) * CosTab[pointI * tabInc]);
}

static float
smoothDer(float y1, float y2, int pointI, int tabInc)
{
  return 0.5f * (y2 - y1) * SinTab[pointI * tabInc];
}

static void
calcStrips(PrimObjP primP, PrimObjP prevP)
{
  PrimRibbonPart *partP;
  Ribbon *ribbonP;
  RadCoeff *radCoeffA;
  int breakCount;
  int sPNo, mPNo, ePNo, smoothInc, pointNo, pointI, eI;
  int traceNo, traceInc, traceI;
  float maxLen, sLen, eLen, sPar, ePar;
  int polyNo, polyI, stripI;
  float *xProf, *yProf, *xNorm, *yNorm;
  BOOL *inside;
  float *parA, *leftParA, *rightParA, par;
  Vec3 *xA, *dzA, startV, axis, dx, dy, dz, dxN, x, nv;
  float xc, yc, xp, yp, xn, yn, rad, dRad;

  partP = &primP->u.ribbon;
  ribbonP = partP->ribbonP;

  if (hasVarRad(primP))
    radCoeffA = calcRadSpline(ribbonP->radA, ribbonP->atomNo,
	partP->startPar, partP->endPar);
  else
    radCoeffA = NULL;

  maxLen = 0.4f * (partP->endPar - partP->startPar);

  getSplinePoint(ribbonP, partP->startPar, x, dz);
  sLen = partP->startLen / Vec3Abs(dz);
  if (sLen < MIN_LEN)
    sLen = MIN_LEN;
  else if (sLen > maxLen)
    sLen = maxLen;

  if (partP->startStyle == RE_SOFT) {
    sPNo = 2 * (1 << partP->prec);
    smoothInc = MAX_POINT_NO / (2 * sPNo);
    sPar = partP->startPar + sLen;
  } else {
    sPNo = 0;
    sPar = partP->startPar;
  }

  getSplinePoint(ribbonP, partP->endPar, x, dz);
  eLen = partP->endLen / Vec3Abs(dz);
  if (eLen < MIN_LEN)
    eLen = MIN_LEN;
  else if (eLen > maxLen)
    eLen = maxLen;

  if (partP->endStyle == RE_SOFT) {
    ePNo = 2 * (1 << partP->prec);
    smoothInc = MAX_POINT_NO / (2 * ePNo);
    ePar = partP->endPar - eLen;
  } else if (partP->endStyle == RE_ARROW) {
    ePNo = (int) (eLen * (1 << partP->prec)) + 2;
    ePar = partP->endPar - eLen;
  } else {
    ePNo = 0;
    ePar = partP->endPar;
  }

  mPNo = (int) ((ePar - sPar) * (1 << partP->prec)) + 2;
  pointNo = sPNo + mPNo + ePNo;

  parA = malloc(pointNo * sizeof(*parA));
  xA = malloc(pointNo * sizeof(*xA));
  dzA = malloc(pointNo * sizeof(*dzA));

  getSplinePoints(partP, ribbonP, partP->startPar, sPar, sPNo + 1,
      parA, xA, dzA);
  getSplinePoints(partP, ribbonP, sPar, ePar, mPNo,
      parA + sPNo, xA + sPNo, dzA + sPNo);

  if (partP->endStyle == RE_ARROW)
    getSplinePoints(partP, ribbonP, ePar, partP->endPar, ePNo,
	parA + sPNo + mPNo, xA + sPNo + mPNo, dzA + sPNo + mPNo);
  else
    getSplinePoints(partP, ribbonP, ePar, partP->endPar, ePNo + 1,
	parA + sPNo + mPNo - 1, xA + sPNo + mPNo - 1, dzA + sPNo + mPNo - 1);

  if ((partP->style == RS_ROUND && partP->midRad == 0.0f) ||
      BreakInterrupted()) {
    /* draw only one line */
    partP->traceNo = 1;
    partP->traceA = malloc(sizeof(*partP->traceA));
    partP->traceA[0].xA = xA;
    partP->traceA[0].nvA = NULL;

    partP->stripNo = 1;
    partP->stripA = malloc(sizeof(*partP->stripA));
    partP->stripA[0].pointNo = pointNo;
    partP->stripA[0].parA = parA;

    free(dzA);

    return;
  }

  if (partP->orient == RO_NEIGH) {
    calcNeighPar(ribbonP, primP, &leftParA, &rightParA);
  } else {
    leftParA = NULL;
    rightParA = NULL;
  }

  traceNo = getProf(partP, &xProf, &yProf, &xNorm, &yNorm, &inside);
  traceInc = MAX_POINT_NO / traceNo;

  if (partP->endStyle == RE_ARROW) {
    partP->traceNo = 2 * traceNo;
    partP->traceA = malloc(2 * traceNo * sizeof(*partP->traceA));
    for (traceI = 0; traceI < traceNo; traceI++) {
      partP->traceA[traceI].xA = malloc((sPNo + mPNo) * sizeof(Vec3));
      partP->traceA[traceI].nvA = malloc((sPNo + mPNo) * sizeof(Vec3));
    }
    for (traceI = traceNo; traceI < 2 * traceNo; traceI++) {
      partP->traceA[traceI].xA = malloc(ePNo * sizeof(Vec3));
      partP->traceA[traceI].nvA = malloc(ePNo * sizeof(Vec3));
    }
  } else {
    partP->traceNo = traceNo;
    partP->traceA = malloc(traceNo * sizeof(*partP->traceA));
    for (traceI = 0; traceI < traceNo; traceI++) {
      partP->traceA[traceI].xA = malloc(pointNo * sizeof(Vec3));
      partP->traceA[traceI].nvA = malloc(pointNo * sizeof(Vec3));
    }
  }
  
  polyNo = 0;
  if (partP->startStyle == RE_SHARP)
    polyNo++;
  if (partP->endStyle == RE_SHARP || partP->endStyle == RE_ARROW)
    polyNo++;

  partP->polyNo = polyNo;
  if (polyNo > 0)
    partP->polyA = malloc(polyNo * sizeof(*partP->polyA));
  
  for (polyI = 0; polyI < polyNo; polyI++) {
    partP->polyA[polyI].xA = malloc(traceNo * sizeof(Vec3));
    partP->polyA[polyI].pointNo = traceNo;
  }

  polyI = 0;
  breakCount = 10000 / traceNo;

  for (pointI = 0; pointI < pointNo; pointI++) {
    if (pointI > 1 && partP->endStyle != RE_ARROW &&
	BreakCheck(breakCount)) {
      /* interrupting arrow makes it hard to guarantee a
	 consistant state, it's hardly worth the effort */
      pointNo = pointI;
      partP->polyNo = polyI;
      break;
    }

    par = parA[pointI];

    Vec3Copy(dz, dzA[pointI]);
    Vec3Norm(dz);

    if (partP->orient == RO_FREE) {
      if (pointI == 0) {
	getStartV(partP, dz, prevP, FALSE, startV);
	Vec3Copy(dx, startV);
      }
      Vec3ScaleSub(dx, Vec3Scalar(dx, dz), dz);
      Vec3Norm(dx);
    } else if (partP->orient == RO_CYLIND || partP->orient == RO_SPACING) {
      if (pointI == 0) {
	getAxisDirV(partP, ribbonP, axis);
	Vec3Copy(startV, axis);
	getStartV(partP, dz, prevP, TRUE, startV);
      }
      Vec3Copy(dx, axis);
    } else if (partP->orient == RO_NEIGH) {
      getNeighV(ribbonP, primP, xA[pointI], dz, parA[pointI], 
	  leftParA, rightParA, dx);
      if (pointI == 0) {
	Vec3Copy(startV, dx);
	getStartV(partP, dz, prevP, TRUE, startV);
      }
    }

    if (par < partP->startPar + sLen) {
      Vec3Scale(dx, (par - partP->startPar) / sLen);
      Vec3ScaleAdd(dx, (sLen - (par - partP->startPar)) / sLen, startV);
      Vec3Norm(dx);
    }

    Vec3Copy(dy, dz);
    Vec3Cross(dy, dx);
    Vec3Norm(dy);

    Vec3Copy(dxN, dy);
    Vec3Cross(dxN, dz);

    if (pointI == pointNo - ePNo && partP->endStyle == RE_ARROW)
      for (traceI = 0; traceI < traceNo; traceI++) {
	xProf[traceI] *= partP->arrowWidth;
	yNorm[traceI] *= partP->arrowWidth;
      }

    for (traceI = 0; traceI < traceNo; traceI++) {
      Vec3Copy(x, xA[pointI]);
      Vec3Zero(nv);

      if (radCoeffA != NULL) {
	getRadSplinePoint(ribbonP->atomNo, radCoeffA,
	    partP->startPar, partP->endPar, par, &rad, &dRad);
	Vec3ScaleAdd(x, rad * xProf[traceI], dx);
	Vec3ScaleAdd(x, rad * yProf[traceI], dy);

	Vec3ScaleAdd(nv, rad * xNorm[traceI], dxN);
	Vec3ScaleAdd(nv, rad * yNorm[traceI], dy);
	Vec3ScaleSub(nv, rad * dRad, dz);
	Vec3Norm(nv);

	Vec3Copy(partP->traceA[traceI].xA[pointI], x);
	Vec3Copy(partP->traceA[traceI].nvA[pointI], nv);
      } else if (pointI < sPNo) {
	xc = partP->startRad * CosTab[traceInc * traceI];
	yc = partP->startRad * SinTab[traceInc * traceI];

	xp = smoothVal(xc, xProf[traceI], pointI, smoothInc);
	yp = smoothVal(yc, yProf[traceI], pointI, smoothInc);

	Vec3ScaleAdd(x, xp, dx);
	Vec3ScaleAdd(x, yp, dy);

	xn = smoothVal(xc, xNorm[traceI], pointI, smoothInc);
	yn = smoothVal(yc, yNorm[traceI], pointI, smoothInc);

	Vec3ScaleAdd(nv, xn, dxN);
	Vec3ScaleAdd(nv, yn, dy);
	Vec3ScaleSub(nv,
	    xn * smoothDer(xc, xProf[traceI], pointI, smoothInc) +
	    yn * smoothDer(yc, yProf[traceI], pointI, smoothInc), dz);
	Vec3Norm(nv);

	Vec3Copy(partP->traceA[traceI].xA[pointI], x);
	Vec3Copy(partP->traceA[traceI].nvA[pointI], nv);
      } else if (pointI >= sPNo + mPNo && partP->endStyle == RE_SOFT) {
	eI = pointI - (sPNo + mPNo - 1);

	xc = partP->endRad * CosTab[traceInc * traceI];
	yc = partP->endRad * SinTab[traceInc * traceI];

	xp = smoothVal(xProf[traceI], xc, eI, smoothInc);
	yp = smoothVal(yProf[traceI], yc, eI, smoothInc);

	Vec3ScaleAdd(x, xp, dx);
	Vec3ScaleAdd(x, yp, dy);

	xn = smoothVal(xNorm[traceI], xc, eI, smoothInc);
	yn = smoothVal(yNorm[traceI], yc, eI, smoothInc);

	Vec3ScaleAdd(nv, xn, dxN);
	Vec3ScaleAdd(nv, yn, dy);
	Vec3ScaleAdd(nv,
	    xn * smoothDer(xc, xProf[traceI], eI, smoothInc) +
	    yn * smoothDer(yc, yProf[traceI], eI, smoothInc), dz);
	Vec3Norm(nv);

	Vec3Copy(partP->traceA[traceI].xA[pointI], x);
	Vec3Copy(partP->traceA[traceI].nvA[pointI], nv);
      } else if (pointI >= sPNo + mPNo && partP->endStyle == RE_ARROW) {
	eI = pointI - (sPNo + mPNo);

	xc = partP->endRad * CosTab[traceInc * traceI];
	yc = partP->endRad * SinTab[traceInc * traceI];

	xp = xProf[traceI] + (xc - xProf[traceI]) * (par - ePar) / eLen;
	yp = yProf[traceI] + (yc - yProf[traceI]) * (par - ePar) / eLen;

	Vec3ScaleAdd(x, xp, dx);
	Vec3ScaleAdd(x, yp, dy);

	xn = xNorm[traceI] + (xc - xNorm[traceI]) * (par - ePar) / eLen;
	yn = yNorm[traceI] + (yc - yNorm[traceI]) * (par - ePar) / eLen;

	Vec3ScaleAdd(nv, xn, dxN);
	Vec3ScaleAdd(nv, yn, dy);
	Vec3ScaleSub(nv,
	    xn * (xc - xProf[traceI]) / eLen +
	    yn * (yc - yProf[traceI]) / eLen, dz);
	Vec3Norm(nv);

	Vec3Copy(partP->traceA[traceNo + traceI].xA[eI], x);
	Vec3Copy(partP->traceA[traceNo + traceI].nvA[eI], nv);
      } else {
	Vec3ScaleAdd(x, xProf[traceI], dx);
	Vec3ScaleAdd(x, yProf[traceI], dy);

	Vec3ScaleAdd(nv, xNorm[traceI], dxN);
	Vec3ScaleAdd(nv, yNorm[traceI], dy);

	Vec3Copy(partP->traceA[traceI].xA[pointI], x);
	Vec3Copy(partP->traceA[traceI].nvA[pointI], nv);
      }

      if ((pointI == 0 && partP->startStyle == RE_SHARP) ||
	  (pointI == pointNo - ePNo && partP->endStyle == RE_ARROW))
	Vec3Copy(partP->polyA[polyI].xA[traceNo - 1 - traceI], x);
      else if (pointI == pointNo - 1 && partP->endStyle == RE_SHARP)
	Vec3Copy(partP->polyA[polyI].xA[traceI], x);
    }

    if ((pointI == 0 && partP->startStyle == RE_SHARP) ||
	(pointI == pointNo - ePNo && partP->endStyle == RE_ARROW)) {
      Vec3Copy(x, dy);
      Vec3Cross(x, dx);
      Vec3Copy(partP->polyA[polyI].nv, x);
      partP->polyA[polyI].par = parA[pointI];
      polyI++;
    } else if (pointI == pointNo - 1 && partP->endStyle == RE_SHARP) {
      Vec3Copy(x, dx);
      Vec3Cross(x, dy);
      Vec3Copy(partP->polyA[polyI].nv, x);
      partP->polyA[polyI].par = parA[pointI];
      polyI++;
    }
  }

  Vec3Copy(partP->endRefV, dx);

  if (partP->style == RS_LENSE)
    partP->stripNo = traceNo - 2;
  else
    partP->stripNo = traceNo;

  if (partP->endStyle == RE_ARROW) {
    partP->stripNo *= 2;
    pointNo = sPNo + mPNo;
  }

  partP->stripA = malloc(partP->stripNo * sizeof(*partP->stripA));

  if (partP->style == RS_LENSE) {
    for (traceI = 0; traceI < traceNo / 2 - 1; traceI++) {
      partP->stripA[traceI].traceLeftI = traceI;
      partP->stripA[traceI].traceRightI = traceI + 1;
      partP->stripA[traceI].pointNo = pointNo;
      partP->stripA[traceI].flat = FALSE;
      partP->stripA[traceI].secColor = inside[traceI];
      partP->stripA[traceI].parA = malloc(pointNo * sizeof(float));
      for (pointI = 0; pointI < pointNo; pointI++)
	partP->stripA[traceI].parA[pointI] = parA[pointI];
    }
    for (traceI = traceNo / 2; traceI < traceNo - 1; traceI++) {
      partP->stripA[traceI - 1].traceLeftI = traceI;
      partP->stripA[traceI - 1].traceRightI = traceI + 1;
      partP->stripA[traceI - 1].pointNo = pointNo;
      partP->stripA[traceI - 1].flat = FALSE;
      partP->stripA[traceI - 1].secColor = inside[traceI];
      partP->stripA[traceI - 1].parA = malloc(pointNo * sizeof(float));
      for (pointI = 0; pointI < pointNo; pointI++)
	partP->stripA[traceI - 1].parA[pointI] = parA[pointI];
    }
  } else {
    for (traceI = 0; traceI < traceNo; traceI++) {
      partP->stripA[traceI].traceLeftI = traceI;
      partP->stripA[traceI].traceRightI = (traceI + 1) % traceNo;
      partP->stripA[traceI].pointNo = pointNo;
      partP->stripA[traceI].flat = (partP->style == RS_RECT);
      partP->stripA[traceI].secColor = inside[traceI];
      partP->stripA[traceI].parA = malloc(pointNo * sizeof(float));
      for (pointI = 0; pointI < pointNo; pointI++)
	partP->stripA[traceI].parA[pointI] = parA[pointI];
    }
  }

  if (partP->endStyle == RE_ARROW)
    for (stripI = partP->stripNo / 2; stripI < partP->stripNo; stripI++) {
      partP->stripA[stripI].traceLeftI =
	  partP->stripA[stripI - partP->stripNo / 2].traceLeftI + traceNo;
      partP->stripA[stripI].traceRightI =
	  partP->stripA[stripI - partP->stripNo / 2].traceRightI + traceNo;
      partP->stripA[stripI].pointNo = ePNo;
      partP->stripA[stripI].flat = FALSE;
      partP->stripA[stripI].secColor =
	  partP->stripA[stripI - partP->stripNo / 2].secColor;
      partP->stripA[stripI].parA = malloc(ePNo * sizeof(float));
      for (pointI = 0; pointI < ePNo; pointI++)
	partP->stripA[stripI].parA[pointI] = parA[pointNo + pointI];
    }

  if (radCoeffA != NULL)
    free(radCoeffA);

  free(xProf);
  free(yProf);
  free(xNorm);
  free(yNorm);
  free(inside);

  free(parA);
  free(xA);
  free(dzA);

  if (leftParA != NULL)
    free(leftParA);
  if (rightParA != NULL)
    free(rightParA);
}

static float
getEndRad(Ribbon *ribbonP, float par)
{
  int ind;
  
  ind = getParInd(par, ribbonP->atomNo);
  return DhAtomGetAttr(ribbonP->atomA[ind])->radius;
}

void
RibbonCalc(PrimObjP primP)
{
  PrimRibbonPart *partP;
  Ribbon *ribbonP;
  int prec;
  LINLIST primL;
  PrimObjP prevP, nextP;
  float startRad, midRad, endRad, rad;
  BOOL radChanged;
  int startI, endI, i;

  partP = &primP->u.ribbon;
  ribbonP = partP->ribbonP;

  if (! ribbonP->splineValid) {
    calcSpline(ribbonP, primP);
    ribbonP->splineValid = TRUE;
  }

  prec = ParGetIntVal(PN_DRAW_PREC);

  primL = PrimListGet();
  prevP = ListPrev(primL, primP);
  if (! isRibbonPart(prevP, ribbonP))
    prevP = NULL;
  nextP = ListNext(primL, primP);
  if (! isRibbonPart(nextP, ribbonP))
    nextP = NULL;

  if (partP->startStyle == RE_SOFT) {
    if (prevP == NULL)
      startRad = 0.0f;
    else if (hasVarRad(prevP))
      startRad = getEndRad(ribbonP, partP->startPar);
    else
      startRad = prevP->attrP->radius;
  } else {
    startRad = primP->attrP->radius;
  }
  if (startRad < MIN_RAD)
    startRad = MIN_RAD;

  if (hasVarRad(primP))
    midRad = 1.0f;
  else
    midRad = primP->attrP->radius;

  if (partP->endStyle == RE_SOFT || partP->endStyle == RE_ARROW) {
    if (nextP == NULL)
      endRad = 0.0f;
    else if (hasVarRad(nextP))
      endRad = getEndRad(ribbonP, partP->endPar);
    else
      endRad = nextP->attrP->radius;
  } else {
    endRad = primP->attrP->radius;
  }
  if (endRad < MIN_RAD)
    endRad = MIN_RAD;

  radChanged = FALSE;
  if (ribbonP->radA == NULL) {
    ribbonP->radA = malloc(ribbonP->atomNo * sizeof(*ribbonP->radA));
    for (i = 0; i < ribbonP->atomNo; i++) {
      ribbonP->radA[i] = DhAtomGetAttr(ribbonP->atomA[i])->radius;
      if (ribbonP->radA[i] < MIN_RAD)
	ribbonP->radA[i] = MIN_RAD;
    }
  } else if (hasVarRad(primP)) {
    startI = getParInd(partP->startPar, ribbonP->atomNo);
    endI = getParInd(partP->endPar, ribbonP->atomNo);
    for (i = startI; i <= endI; i++) {
      rad = DhAtomGetAttr(ribbonP->atomA[i])->radius;
      if (rad < MIN_RAD)
	rad = MIN_RAD;
      if (rad != ribbonP->radA[i]) {
	ribbonP->radA[i] = rad;
	radChanged = TRUE;
      }
    }
  }

  if (! partP->pointValid || prec != partP->prec ||
      startRad != partP->startRad ||
      (partP->style == RS_ROUND && midRad != partP->midRad) ||
      endRad != partP->endRad || radChanged) {
    for (i = 0; i < partP->traceNo; i++) {
      free(partP->traceA[i].xA);
      if (partP->traceA[i].nvA != NULL)
	free(partP->traceA[i].nvA);
    }
    if (partP->traceA != NULL) {
      free(partP->traceA);
      partP->traceA = NULL;
      partP->traceNo = 0;
    }

    for (i = 0; i < partP->stripNo; i++)
      free(partP->stripA[i].parA);
    if (partP->stripA != NULL) {
      free(partP->stripA);
      partP->stripA = NULL;
      partP->stripNo = 0;
    }

    for (i = 0; i < partP->polyNo; i++)
      free(partP->polyA[i].xA);
    if (partP->polyA != NULL) {
      free(partP->polyA);
      partP->polyA = NULL;
      partP->polyNo = 0;
    }

    if (partP->style != RS_INVISIBLE) {
      partP->prec = prec;
      partP->startRad = startRad;
      partP->midRad = midRad;
      partP->endRad = endRad;

      calcStrips(primP, prevP);

      partP->pointValid = TRUE;
    }
  }
}
