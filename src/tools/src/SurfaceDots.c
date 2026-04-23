/*
************************************************************************
*
*   SurfaceDots.c - molecule dot surface calculation
*
*   Copyright (c) 1995
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/tools/src/SCCS/s.SurfaceDots.c
*   SCCS identification       : 1.6
*
************************************************************************
*/

#include <surface.h>

#define DEBUG 0

#include <stdio.h>
#include <stdlib.h>
#include <values.h>
#include <math.h>

#if DEBUG
#include <time.h>
#endif

#include <break.h>
#include <mat_vec.h>
#include <sphere.h>
#include <grid.h>

typedef struct SphereS *SphereP;

typedef struct {
  SphereP sphereP;
  int patchI;
} Neighbour;

typedef struct SphereS {
  void *userData;
  Vec3 c;
  float rr, re, reS;
  int neighNo;
  int size;
  int patchI;
  Neighbour *neighA;
} Sphere;

typedef struct {
  SphereP sphereP;
  int patchI;
} ProblPatch;

static float SolventRad, SolventRadS, IntersRadFract;
static BOOL SurfFast;

static Sphere *SphereA;
static int SphereNo;

static int SpherePointNo;
static Vec3 *SpherePoints;

static int CircPointNo;
static float (*CircPoints)[2];

static PatchDescr *PatchA;
static int PatchNo;

static ProblPatch *ProblPatchA;
static int ProblPatchNo, ProblPatchSize;
static Vec3 *ProblPointA;
static int ProblPointNo, ProblPointSize;

static BOOL
fillNeighbour(void *entryP, Vec3 c, void *clientData)
{
  SphereP nSphereP = entryP;
  SphereP sphereP = clientData;
  Vec3 dx;
  float rs;

  if (nSphereP == sphereP)
    return TRUE;

  Vec3Copy(dx, c);
  Vec3Sub(dx, nSphereP->c);
  rs = sphereP->re + nSphereP->re;

  if (dx[0] * dx[0] + dx[1] * dx[1] + dx[2] * dx[2] > rs * rs)
    return TRUE;
  
  if (sphereP->neighNo == sphereP->size) {
    if (sphereP->size == 0) {
      sphereP->size = 5;
      sphereP->neighA = malloc(sphereP->size * sizeof(Neighbour));
    } else {
      sphereP->size *= 2;
      sphereP->neighA = realloc(sphereP->neighA,
	  sphereP->size * sizeof(Neighbour));
    }
  }

  sphereP->neighA[sphereP->neighNo].sphereP = nSphereP;
  sphereP->neighA[sphereP->neighNo].patchI = -1;

  sphereP->neighNo++;

  return TRUE;
}

static void
testSphere(SphereP sphereP)
{
  Vec3 *surfPoints;
  int surfPointNo;
  int pointI, sphereI;
  SphereP nSphereP;
  Vec3 x, dx;

  surfPointNo = 0;

  for (pointI = 0; pointI < SpherePointNo; pointI++) {
    Vec3Copy(x, sphereP->c);
    Vec3ScaleAdd(x, sphereP->re, SpherePoints[pointI]);

    for (sphereI = 0; sphereI < sphereP->neighNo; sphereI++) {
      nSphereP = sphereP->neighA[sphereI].sphereP;

      Vec3Copy(dx, x);
      Vec3Sub(dx, nSphereP->c);

      if (dx[0] * dx[0] + dx[1] * dx[1] + dx[2] * dx[2] < nSphereP->reS)
	break;
    }

    if (sphereI == sphereP->neighNo) {
      if (surfPointNo == 0)
	surfPoints = malloc(SpherePointNo * sizeof(*surfPoints));

      if (sphereP->re > sphereP->rr) {
	Vec3Copy(surfPoints[surfPointNo], sphereP->c);
	Vec3ScaleAdd(surfPoints[surfPointNo],
	    sphereP->rr, SpherePoints[pointI]);
      } else {
	Vec3Copy(surfPoints[surfPointNo], x);
      }

      surfPointNo++;
    }
  }

  if (surfPointNo > 0) {
    if (PatchNo == 0)
      PatchA = malloc(sizeof(*PatchA));
    else
      PatchA = realloc(PatchA, (PatchNo + 1) * sizeof(*PatchA));
    
    PatchA[PatchNo].pointA = realloc(surfPoints,
	surfPointNo * sizeof(*surfPoints));
    PatchA[PatchNo].pointNo = surfPointNo;
    PatchA[PatchNo].userData = sphereP->userData;

    sphereP->patchI = PatchNo;

    PatchNo++;
  } else {
    sphereP->patchI = -1;
  }
}

static void
calcCircPoints(int prec)
{
  int stepNo;
  double ang, cosAng, sinAng;
  double x0, y0, x1, y1;
  int i;

  stepNo = 1 << prec;
  CircPointNo = 4 * stepNo;
  CircPoints = malloc(CircPointNo * sizeof(*CircPoints));

  ang = M_PI / (2 * stepNo);
  cosAng = cos(ang);
  sinAng = sin(ang);
  x1 = 1.0;
  y1 = 0.0;

  for (i = 0; i < stepNo; i++) {
    CircPoints[i][0] = (float) x1;
    CircPoints[i][1] = (float) y1;

    CircPoints[stepNo + i][0] = - (float) y1;
    CircPoints[stepNo + i][1] = (float) x1;

    CircPoints[2 * stepNo + i][0] = - (float) x1;
    CircPoints[2 * stepNo + i][1] = - (float) y1;

    CircPoints[3 * stepNo + i][0] = (float) y1;
    CircPoints[3 * stepNo + i][1] = - (float) x1;

    x0 = x1;
    y0 = y1;
    x1 = x0 * cosAng - y0 * sinAng;
    y1 = x0 * sinAng + y0 * cosAng;
  }
}

static void
addProblPatch(SphereP sphereP, int patchI)
{
  if (ProblPatchNo == ProblPatchSize) {
    if (ProblPatchSize == 0) {
      ProblPatchSize = 5;
      ProblPatchA = malloc(ProblPatchSize * sizeof(*ProblPatchA));
    } else {
      ProblPatchSize *= 2;
      ProblPatchA = realloc(ProblPatchA,
	  ProblPatchSize * sizeof(*ProblPatchA));
    }
  }

  ProblPatchA[ProblPatchNo].sphereP = sphereP;
  ProblPatchA[ProblPatchNo].patchI = patchI;

  ProblPatchNo++;
}

static void
addProblPoint(Vec3 x)
{
  if (ProblPointNo == ProblPointSize) {
    if (ProblPointSize == 0) {
      ProblPointSize = 50;
      ProblPointA = malloc(ProblPointSize * sizeof(*ProblPointA));
    } else {
      ProblPointSize *= 2;
      ProblPointA = realloc(ProblPointA,
	  ProblPointSize * sizeof(*ProblPointA));
    }
  }

  Vec3Copy(ProblPointA[ProblPointNo], x);
  ProblPointNo++;
}

static void
getTorusPoint(Vec3 x, int pointI, int i, float r, Mat3 m, Vec3 t)
{
  float r0;

  r0 = 1.0f + CircPoints[i][0] * r;
  x[0] = r0 * CircPoints[pointI][0];
  x[1] = r0 * CircPoints[pointI][1];
  x[2] = CircPoints[i][1] * r;

  Mat3VecMult(x, m);
  Vec3Add(x, t);
}

static void
testSaddle(Neighbour *neighP, SphereP s1P, SphereP s2P)
{
  Vec3 c1, c2;
  float r1S, r2S;
  float d, t, a1S, a2S, rcS, rc;
  Vec3 c;
  float a1, a2;
  float v1x, v1y, v2x, v2y, v2z;
  Mat3 m;
  Vec3 *surfPoints;
  int surfPointNo;
  int pointI, sphereI, minI, maxI;
  SphereP nSphereP;
  Vec3 x, dx;
  float rt;
  BOOL probl;
  int i;

  Vec3Copy(c1, s1P->c);
  r1S = s1P->reS;

  Vec3Copy(c2, s2P->c);
  Vec3Sub(c2, c1);
  r2S = s2P->reS;

  a1S = c2[0] * c2[0] + c2[1] * c2[1];
  a2S = a1S + c2[2] * c2[2];
  if (a2S == 0.0f)
    return;

  d = 0.5f * (r1S - r2S + a2S);
  t = d / a2S;
  rcS = r1S - t * t * a2S;
  if (rcS < 0.0f)
    return;

  Vec3Copy(c, c1);
  Vec3ScaleAdd(c, t, c2);
  rc = sqrtf(rcS);

  a1 = sqrtf(a1S);
  a2 = sqrtf(a2S);

  v1x = - c2[1] / a1;
  v1y = c2[0] / a1;

  v2x = c2[0] / a2;
  v2y = c2[1] / a2;
  v2z = c2[2] / a2;

  /* matrix for rotating circle to correct orientation */
  m[0][0] = rc * v1y * v2z;
  m[0][1] = - rc * v1x * v2z;
  m[0][2] = rc * (v1x * v2y - v1y * v2x);
  m[1][0] = rc * v1x;
  m[1][1] = rc * v1y;
  m[1][2] = 0.0f;
  m[2][0] = rc * v2x;
  m[2][1] = rc * v2y;
  m[2][2] = rc * v2z;

  surfPointNo = 0;

  for (pointI = 0; pointI < CircPointNo; pointI++) {
    x[0] = m[0][0] * CircPoints[pointI][0] + m[1][0] * CircPoints[pointI][1];
    x[1] = m[0][1] * CircPoints[pointI][0] + m[1][1] * CircPoints[pointI][1];
    x[2] = m[0][2] * CircPoints[pointI][0] + m[1][2] * CircPoints[pointI][1];

    Vec3Add(x, c);

    for (sphereI = 0; sphereI < s1P->neighNo; sphereI++) {
      nSphereP = s1P->neighA[sphereI].sphereP;

      if (nSphereP == s2P)
	continue;

      Vec3Copy(dx, x);
      Vec3Sub(dx, nSphereP->c);

      if (dx[0] * dx[0] + dx[1] * dx[1] + dx[2] * dx[2] < 
	  IntersRadFract * nSphereP->reS)
	break;
    }

    if (sphereI == s1P->neighNo) {
      if (surfPointNo == 0) {
	rt = SolventRad / rc;
	probl = (rt > 1.0f);

	minI = CircPointNo / 4;
	while (CircPoints[minI][0] * (1.0f - t) * a2 +
	    CircPoints[minI][1] * rc > 0.0f)
	  minI++;

	maxI = minI;
	while (CircPoints[maxI][0] * t * a2 -
	    CircPoints[maxI][1] * rc <= 0.0f)
	  maxI++;

	surfPoints = malloc(CircPointNo * (maxI - minI + 1) *
	    sizeof(*surfPoints));
      }

      if (probl)
	addProblPoint(x);

      for (i = minI; i <= maxI; i++) {
	getTorusPoint(surfPoints[surfPointNo], pointI, i, rt, m, c);
	surfPointNo++;
      }
    }
  }

  if (surfPointNo > 0) {
    PatchA = realloc(PatchA, (PatchNo + 1) * sizeof(*PatchA));
    
    PatchA[PatchNo].pointA = realloc(surfPoints,
	surfPointNo * sizeof(*surfPoints));
    PatchA[PatchNo].pointNo = surfPointNo;
    PatchA[PatchNo].userData = NULL;

    neighP->patchI = PatchNo;

    if (probl)
      addProblPatch(s1P, PatchNo);

    PatchNo++;
  }
}

static void
pairSphere(SphereP sphereP)
{
  int sphereI;
  SphereP nSphereP;

  if (SurfFast && sphereP->patchI == -1)
    return;

  for (sphereI = 0; sphereI < sphereP->neighNo; sphereI++) {
    nSphereP = sphereP->neighA[sphereI].sphereP;

    if (SurfFast && nSphereP->patchI == -1)
      continue;

    if (nSphereP < sphereP)
      continue;
    
    testSaddle(sphereP->neighA + sphereI, sphereP, nSphereP);
  }
}

static BOOL
calcSphereInters(Vec3 x1, Vec3 x2, BOOL *problP,
    SphereP s1P, SphereP s2P, SphereP s3P)
{
  Vec3 c1, c2, c3;
  float r1S, r2S, r3S;
  float a2S, a3S, p23, d, b1, b2, t2, t3, rS, r;
  Vec3 m, cp;

  Vec3Copy(c1, s1P->c);
  r1S = s1P->reS;

  Vec3Copy(c2, s2P->c);
  Vec3Sub(c2, c1);
  r2S = s2P->reS;

  Vec3Copy(c3, s3P->c);
  Vec3Sub(c3, c1);
  r3S = s3P->reS;

  a2S = c2[0] * c2[0] + c2[1] * c2[1] + c2[2] * c2[2];
  a3S = c3[0] * c3[0] + c3[1] * c3[1] + c3[2] * c3[2];
  p23 = Vec3Scalar(c2, c3);

  d = 4.0f * (a2S * a3S - p23 * p23);
  if (d == 0.0f)
    return FALSE;
  
  b1 = r1S - r2S + a2S;
  b2 = r1S - r3S + a3S;

  t2 = (b1 * 2.0f * a3S - b2 * 2.0f * p23) / d;
  t3 = (b2 * 2.0f * a2S - b1 * 2.0f * p23) / d;

  /* point in the middle of the two intersection points */
  Vec3Zero(m);
  Vec3ScaleAdd(m, t2, c2);
  Vec3ScaleAdd(m, t3, c3);

  rS = r1S - (m[0] * m[0] + m[1] * m[1] + m[2] * m[2]);
  if (rS < 0.0f)
    return FALSE;

  r = sqrtf(rS);
  *problP = (r < SolventRad);

  Vec3Add(m, c1);

  Vec3Copy(cp, c2);
  Vec3Cross(cp, c3);
  Vec3Norm(cp);
  Vec3Scale(cp, r);

  Vec3Copy(x1, m);
  Vec3Add(x1, cp);

  Vec3Copy(x2, m);
  Vec3Sub(x2, cp);

  return TRUE;
}

static void
testTriangle(SphereP s1P, SphereP s2P, SphereP s3P, Vec3 c, BOOL probl)
{
  Mat3 m;
  Vec3 *surfPoints;
  int surfPointNo;
  int pointI, sphereI;
  SphereP nSphereP;
  Vec3 x, dx;

  for (sphereI = 0; sphereI < s1P->neighNo; sphereI++) {
    nSphereP = s1P->neighA[sphereI].sphereP;

    if (nSphereP == s2P || nSphereP == s3P)
      continue;

    Vec3Copy(dx, c);
    Vec3Sub(dx, nSphereP->c);

    if (dx[0] * dx[0] + dx[1] * dx[1] + dx[2] * dx[2] < 
	IntersRadFract * nSphereP->reS)
      return;
  }

  Vec3Copy(m[0], s1P->c);
  Vec3Sub(m[0], c);

  Vec3Copy(m[1], s2P->c);
  Vec3Sub(m[1], c);

  Vec3Copy(m[2], s3P->c);
  Vec3Sub(m[2], c);

  Mat3Inv(m);

  surfPointNo = 0;

  for (pointI = 0; pointI < SpherePointNo; pointI++) {
    Vec3Copy(x, SpherePoints[pointI]);
    Mat3VecMult(x, m);

    if (x[0] >= 0.0f && x[1] >= 0.0f && x[2] >= 0.0f) {
      if (surfPointNo == 0)
	surfPoints = malloc(SpherePointNo * sizeof(*surfPoints));

      Vec3Copy(x, c);
      Vec3ScaleAdd(x, SolventRad, SpherePoints[pointI]);
      Vec3Copy(surfPoints[surfPointNo], x);
      surfPointNo++;
    }
  }

  if (surfPointNo > 0) {
    PatchA = realloc(PatchA, (PatchNo + 1) * sizeof(*PatchA));
    
    PatchA[PatchNo].pointA = realloc(surfPoints,
	surfPointNo * sizeof(*surfPoints));
    PatchA[PatchNo].pointNo = surfPointNo;
    PatchA[PatchNo].userData = NULL;

    if (probl) {
      addProblPatch(s1P, PatchNo);
      addProblPoint(c);
    }

    PatchNo++;
  }
}

static void
tripleSphere(SphereP sphereP)
{
  int s2I, s3I;
  SphereP s2P, s3P;
  Vec3 x1, x2;
  BOOL probl;

  if (SurfFast && sphereP->patchI == -1)
    return;

  for (s2I = 0; s2I < sphereP->neighNo - 1; s2I++) {
    s2P = sphereP->neighA[s2I].sphereP;

    if (SurfFast && s2P->patchI == -1)
      continue;

    if (s2P < sphereP)
      continue;
    
    for (s3I = s2I + 1; s3I < sphereP->neighNo; s3I++) {
      s3P = sphereP->neighA[s3I].sphereP;

      if (SurfFast && s3P->patchI == -1)
	continue;

      if (s3P < sphereP)
	continue;
    
      if (calcSphereInters(x1, x2, &probl, sphereP, s2P, s3P)) {
	testTriangle(sphereP, s2P, s3P, x1, probl);
	testTriangle(sphereP, s2P, s3P, x2, probl);
      }
    }
  }
}

static BOOL
testProbl(void *entryP, Vec3 x, void *clientData)
{
  float *c = entryP;
  BOOL *insideP = clientData;
  Vec3 dx;
  BOOL inside;

  Vec3Copy(dx, x);
  Vec3Sub(dx, c);

  inside = (dx[0] * dx[0] + dx[1] * dx[1] + dx[2] * dx[2] <
      (1.0f - (float) 1.0e-5) * SolventRadS);

  *insideP = inside;

  return ! inside;
}

static void
fixupProbl(GRID grid)
{
  int patchI, pointI, outI;
  PatchDescr *patchP;
  BOOL inside;

  for (patchI = 0; patchI < ProblPatchNo; patchI++) {
    if (BreakCheck(1))
      break;

    patchP = PatchA + ProblPatchA[patchI].patchI;

    outI = 0;
    for (pointI = 0; pointI < patchP->pointNo; pointI++) {
      inside = FALSE;
      GridFind(grid, patchP->pointA[pointI], 1, testProbl, &inside);

      if (! inside) {
	if (pointI > outI)
	  Vec3Copy(patchP->pointA[outI], patchP->pointA[pointI]);
	outI++;
      }
    }

    if (outI < patchP->pointNo) {
      patchP->pointNo = outI;
      if (outI == 0) {
	free(patchP->pointA);
	patchP->pointA = NULL;
      } else {
	patchP->pointA = realloc(patchP->pointA,
	    patchP->pointNo * sizeof(Vec3));
      }
    }
  }
}

void
SurfaceDots(SphereDescr *descrA, int sphereNo,
    float solvRad, int prec, BOOL fast,
    PatchDescr **patchAP, int *patchNoP)
{
  GRID sphereGrid, problGrid;
  float rMax, phi;
  int sphereI, breakCount, i;
#if DEBUG
  clock_t t0;
  int pointNo;
#endif

  SolventRad = solvRad;
  SurfFast = fast;

#if DEBUG
  (void) printf("preparation\n");
  t0 = clock();
#endif

  SolventRadS = SolventRad * SolventRad;

  SphereA = malloc(sphereNo * sizeof(*SphereA));

  sphereI = 0;
  for (i = 0; i < sphereNo; i++) {
    if (descrA[i].rad <= 0.0f)
      continue;

    SphereA[sphereI].userData = descrA[i].userData;
    Vec3Copy(SphereA[sphereI].c, descrA[i].cent);
    SphereA[sphereI].rr = descrA[i].rad;
    SphereA[sphereI].re = SphereA[sphereI].rr + SolventRad;
    SphereA[sphereI].reS = SphereA[sphereI].re * SphereA[sphereI].re;

    sphereI++;
  }
  SphereNo = sphereI;

#if DEBUG
  (void) printf("time: %d\n", clock() - t0);
  (void) printf("build grid\n");
#endif

  sphereGrid = GridNew();
  rMax = - MAXFLOAT;
  for (i = 0; i < SphereNo; i++) {
    GridPrepareAddEntry(sphereGrid, SphereA[i].c);
    if (SphereA[i].re > rMax)
      rMax = SphereA[i].re;
  }

  GridInsertInit(sphereGrid, 2.0f * rMax + (float) 1.0e-6);
  for (i = 0; i < SphereNo; i++)
    GridInsertEntry(sphereGrid, SphereA[i].c, SphereA + i);

#if DEBUG
  (void) printf("time: %d\n", clock() - t0);
  (void) printf("find neighbours\n");
#endif

  for (i = 0; i < SphereNo; i++) {
    SphereA[i].neighNo = 0;
    SphereA[i].size = 0;
    GridFind(sphereGrid, SphereA[i].c, 1, fillNeighbour, SphereA + i);
  }

  GridDestroy(sphereGrid);

  SphereCalcPoints(prec, &SpherePoints, &SpherePointNo);

#if DEBUG
  (void) printf("time: %d\n", clock() - t0);
  (void) printf("test 1\n");
#endif

  breakCount = 1000 / SpherePointNo;

  PatchNo = 0;
  for (i = 0; i < SphereNo; i++) {
    if (BreakCheck(breakCount))
      break;
    testSphere(SphereA + i);
  }

  if (SolventRad > 0.0f && ! BreakInterrupted()) {
    /* approximate half angle between two sphere points */
    phi = 0.25f * (float) M_PI / (1 << prec);
    /* approximation for 1.0 - maximal intersection of solvent
       sphere with sphere that did not have surface points */
    IntersRadFract = 1.0f - 0.5f * phi * phi * (1.0f + rMax / SolventRad);

    ProblPatchNo = 0;
    ProblPatchSize = 0;
    ProblPointNo = 0;
    ProblPointSize = 0;

    calcCircPoints(prec);

#if DEBUG
    (void) printf("time: %d\n", clock() - t0);
    (void) printf("test 2\n");
#endif

    for (i = 0; i < SphereNo; i++) {
      if (BreakCheck(1))
	break;
      pairSphere(SphereA + i);
    }

#if DEBUG
    (void) printf("time: %d\n", clock() - t0);
    (void) printf("ProblPatchNo: %d\n", ProblPatchNo);
    (void) printf("ProblPointNo: %d\n", ProblPointNo);
    (void) printf("test 3\n");
#endif

    for (i = 0; i < SphereNo; i++) {
      if (BreakCheck(1))
	break;
      tripleSphere(SphereA + i);
    }
  }

  for (i = 0; i < SphereNo; i++)
    if (SphereA[i].neighNo > 0)
      free(SphereA[i].neighA);

  free(SphereA);
  free(SpherePoints);

  if (SolventRad > 0.0f && ! BreakInterrupted()) {
#if DEBUG
    (void) printf("time: %d\n", clock() - t0);
    (void) printf("ProblPatchNo: %d\n", ProblPatchNo);
    (void) printf("ProblPointNo: %d\n", ProblPointNo);
    (void) printf("build grid\n");
#endif

    problGrid = GridNew();
    for (i = 0; i < ProblPointNo; i++)
      GridPrepareAddEntry(problGrid, ProblPointA[i]);

    GridInsertInit(problGrid, SolventRad);
    for (i = 0; i < ProblPointNo; i++)
      GridInsertEntry(problGrid, ProblPointA[i], ProblPointA[i]);

#if DEBUG
    (void) printf("time: %d\n", clock() - t0);
    (void) printf("fixup\n");
#endif
    fixupProbl(problGrid);
#if DEBUG
    (void) printf("time: %d\n", clock() - t0);
#endif

    GridDestroy(problGrid);

    if (ProblPatchSize > 0)
      free(ProblPatchA);
    if (ProblPointSize > 0)
      free(ProblPointA);
  }

#if DEBUG
  pointNo = 0;
  for (i = 0; i < PatchNo; i++)
    pointNo += PatchA[i].pointNo;
  (void) printf("point number: %d\n", pointNo);
#endif

  *patchAP = PatchA;
  *patchNoP = PatchNo;
}
