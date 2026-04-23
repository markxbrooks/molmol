/*
************************************************************************
*
*   SurfaceSolid.c - molecule solid surface calculation
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/tools/src/SCCS/s.SurfaceSolid.c
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
#include <iso_surface.h>

typedef struct {
  Vec3 c;
  float rr, ro, roS, riS;
} Sphere, *SphereP;

typedef struct {
  SphereP sphereP;
  SphereP *neighA;
  int neighNo, size;
} NeighList;

typedef struct {
  Vec3 x;
  SphereP sphereP;
} Point;

typedef struct {
  float dist;
  SphereP sphereP;
} DistData;

static Vec3 *SpherePoints;
static int SpherePointNo;

static Point *PointA;
static int PointNo, PointSize;

static BOOL
fillNeigh(void *entryP, Vec3 c, void *clientData)
{
  SphereP nSphereP = entryP;
  NeighList *neighListP = clientData;
  Vec3 dx;
  float rs;

  if (nSphereP == neighListP->sphereP)
    return TRUE;

  Vec3Copy(dx, c);
  Vec3Sub(dx, nSphereP->c);
  rs = neighListP->sphereP->ro + nSphereP->ro;

  if (dx[0] * dx[0] + dx[1] * dx[1] + dx[2] * dx[2] > rs * rs)
    return TRUE;

  if (neighListP->neighNo == neighListP->size) {
    if (neighListP->size == 0) {
      neighListP->size = 5;
      neighListP->neighA = malloc(
	  neighListP->size * sizeof(*neighListP->neighA));
    } else {
      neighListP->size *= 2;
      neighListP->neighA = realloc(neighListP->neighA,
	  neighListP->size * sizeof(*neighListP->neighA));
    }
  }

  neighListP->neighA[neighListP->neighNo++] = nSphereP;

  return TRUE;
}

static void
calcPoints(GRID grid, SphereP sphereP)
{
  NeighList neighList;
  int pointI, sphereI;
  SphereP nSphereP;
  Vec3 x, dx;

  neighList.sphereP = sphereP;
  neighList.neighNo = 0;
  neighList.size = 0;
  GridFind(grid, sphereP->c, 1, fillNeigh, &neighList);
  for (pointI = 0; pointI < SpherePointNo; pointI++) {
    Vec3Copy(x, sphereP->c);
    Vec3ScaleAdd(x, sphereP->ro, SpherePoints[pointI]);

    for (sphereI = 0; sphereI < neighList.neighNo; sphereI++) {
      nSphereP = neighList.neighA[sphereI];

      Vec3Copy(dx, x);
      Vec3Sub(dx, nSphereP->c);
      if (dx[0] * dx[0] + dx[1] * dx[1] + dx[2] * dx[2] < nSphereP->roS)
	break;
    }

    if (sphereI == neighList.neighNo) {
      if (PointNo == PointSize) {
	PointSize *= 2;
	PointA = realloc(PointA, PointSize * sizeof(*PointA));
      }

      Vec3Copy(PointA[PointNo].x, x);
      PointA[PointNo].sphereP = sphereP;
      PointNo++;
    }
  }

  if (neighList.size > 0)
    free(neighList.neighA);
}

static BOOL
checkInside(void *entryP, float *x, void *clientData)
{
  SphereP sphereP = entryP;
  BOOL *insideP = clientData;
  Vec3 dx;

  Vec3Copy(dx, x);
  Vec3Sub(dx, sphereP->c);
  if (dx[0] * dx[0] + dx[1] * dx[1] + dx[2] * dx[2] < sphereP->riS) {
    *insideP = TRUE;
    return FALSE;
  }

  return TRUE;
}


static BOOL
getSphereDist(void *entryP, float *x, void *clientData)
{
  SphereP sphereP = entryP;
  DistData *dataP = clientData;
  Vec3 dx;
  float d;

  Vec3Copy(dx, x);
  Vec3Sub(dx, sphereP->c);
  d = sphereP->rr - Vec3Abs(dx);
  if (d > dataP->dist) {
    dataP->dist = d;
    dataP->sphereP = sphereP;
  }

  return TRUE;
}

static BOOL
checkOutside(void *entryP, float *x, void *clientData)
{
  SphereP sphereP = entryP;
  BOOL *insideP = clientData;
  Vec3 dx;

  Vec3Copy(dx, x);
  Vec3Sub(dx, sphereP->c);
  if (dx[0] * dx[0] + dx[1] * dx[1] + dx[2] * dx[2] < sphereP->roS) {
    *insideP = TRUE;
    return FALSE;
  }

  return TRUE;
}

static BOOL
getPointDist(void *entryP, float *x, void *clientData)
{
  Point *p = entryP;
  DistData *dataP = clientData;
  Vec3 dx;
  float dS;

  Vec3Copy(dx, x);
  Vec3Sub(dx, p->x);
  dS = dx[0] * dx[0] + dx[1] * dx[1] + dx[2] * dx[2];
  if (dS < dataP->dist) {
    dataP->dist = dS;
    dataP->sphereP = p->sphereP;
  }

  return TRUE;
}

static void
fillVdwVal(float *valA, int valSize[3], float vMin[3], float gridW,
    float maxDist, GRID insideGrid, GRID outsideGrid)
{
  int breakCount, xi, yi, zi, ind;
  Vec3 x;
  BOOL inside;
  DistData distData;

  breakCount = 1000 / valSize[0];

  ind = -1;
  for (zi = 0; zi < valSize[2]; zi++) {
    x[2] = vMin[2] + zi * gridW;
    for (yi = 0; yi < valSize[1]; yi++) {
      if (BreakCheck(breakCount))
	break;

      x[1] = vMin[1] + yi * gridW;
      for (xi = 0; xi < valSize[0]; xi++) {
	x[0] = vMin[0] + xi * gridW;
	ind++;

	inside = FALSE;
	GridFind(insideGrid, x, 1, checkInside, &inside);
	if (inside) {
	  valA[ind] = maxDist;
	  continue;
	}

	distData.dist = - maxDist;
	GridFind(outsideGrid, x, 1, getSphereDist, &distData);
	valA[ind] = distData.dist;
      }
    }
  }
}

static void
fillSolvVal(float *valA, int valSize[3], float vMin[3], float gridW,
    float maxDist, GRID insideGrid, GRID outsideGrid, GRID pointGrid)
{
  int breakCount, xi, yi, zi, ind;
  Vec3 x;
  BOOL inside;
  DistData distData;

  breakCount = 1000 / valSize[0];

  ind = -1;
  for (zi = 0; zi < valSize[2]; zi++) {
    x[2] = vMin[2] + zi * gridW;
    for (yi = 0; yi < valSize[1]; yi++) {
      if (BreakCheck(breakCount))
	break;

      x[1] = vMin[1] + yi * gridW;
      for (xi = 0; xi < valSize[0]; xi++) {
	x[0] = vMin[0] + xi * gridW;
	ind++;

	inside = FALSE;
	GridFind(insideGrid, x, 1, checkInside, &inside);
	if (inside) {
	  valA[ind] = maxDist;
	  continue;
	}

	GridFind(outsideGrid, x, 1, checkOutside, &inside);
	if (! inside) {
	  valA[ind] = 0.0f;
	  continue;
	}

	distData.dist = maxDist * maxDist;
	GridFind(pointGrid, x, 1, getPointDist, &distData);
	valA[ind] = sqrtf(distData.dist);
      }
    }
  }
}

void
SurfaceSolid(SphereDescr *descrA, int descrNo,
    float solvRad, int prec,
    IsoSurface **surfAP, int *surfNoP)
{
  float rMin, rMax;
  float vMin[3], vMax[3];
  Sphere *sphereA;
  int breakCount, sphereNo, sI;
  GRID neighGrid, pointGrid, insideGrid, outsideGrid;
  float cellW, cellD, maxDist, excess;
  int valSize[3], totSize;
  float *valA;
  IsoSurface *surfA;
  int surfNo;
  int i, k;
#if DEBUG
  clock_t t0;
#endif

#if DEBUG
  t0 = clock();
#endif

  for (k = 0; k < 3; k++) {
    vMin[k] = MAXFLOAT;
    vMax[k] = - MAXFLOAT;
  }

  rMin = MAXFLOAT;
  rMax = - MAXFLOAT;

  sphereNo = 0;
  for (i = 0; i < descrNo; i++) {
    if (descrA[i].rad <= 0.0f)
      continue;

    for (k = 0; k < 3; k++) {
      if (descrA[i].cent[k] - descrA[i].rad < vMin[k])
	vMin[k] = descrA[i].cent[k] - descrA[i].rad;
      if (descrA[i].cent[k] + descrA[i].rad > vMax[k])
	vMax[k] = descrA[i].cent[k] + descrA[i].rad;
    }

    if (descrA[i].rad < rMin)
      rMin = descrA[i].rad;
    if (descrA[i].rad > rMax)
      rMax = descrA[i].rad;

    sphereNo++;
  }

  if (sphereNo == 0)
    return;

  /* make cell spacing approximately equal to spacing of dots
     on sphere  with radius 1.5 */
  cellW = (float) M_PI / (1 << prec);

  /* distance of solvent sphere centers approximately equal
     to cell size */
  while (prec > 0 &&
      (float) M_PI * (rMax + solvRad) < (1 << (prec + 1)) * cellW)
    prec--;

  /* number larger than diagonal of cell */
  cellD = 1.8f * cellW;

  /* solvent radius may not be too small, otherwise the
     algorithm produces artifacts */
  if (solvRad != 0.0f && solvRad < cellD)
    solvRad = cellD;

  sphereA = malloc(sphereNo * sizeof(*sphereA));

  sI = 0;
  for (i = 0; i < descrNo; i++) {
    if (descrA[i].rad <= 0.0f)
      continue;

    Vec3Copy(sphereA[sI].c, descrA[i].cent);
    sphereA[sI].rr = descrA[i].rad;
    if (solvRad == 0.0f)
      sphereA[sI].ro = sphereA[sI].rr + cellD;
    else
      sphereA[sI].ro = sphereA[sI].rr + solvRad;
    sphereA[sI].roS = sphereA[sI].ro * sphereA[sI].ro;
    sphereA[sI].riS = descrA[i].rad - cellD;
    sphereA[sI].riS *= sphereA[sI].riS;

    sI++;
  }

  if (solvRad > 0.0f) {
    neighGrid = GridNew();
    for (i = 0; i < sphereNo; i++)
      GridPrepareAddEntry(neighGrid, sphereA[i].c);
    GridInsertInit(neighGrid, 2.0f * (rMax + solvRad));
    for (i = 0; i < sphereNo; i++)
      GridInsertEntry(neighGrid, sphereA[i].c, sphereA + i);

    SphereCalcPoints(prec, &SpherePoints, &SpherePointNo);

    breakCount = 1000 / SpherePointNo;

    PointSize = 10;
    PointA = malloc(PointSize * sizeof(*PointA));
    PointNo = 0;

    for (i = 0; i < sphereNo; i++) {
      if (BreakCheck(breakCount))
	break;
      calcPoints(neighGrid, sphereA + i);
    }
      
    GridDestroy(neighGrid);

    if (PointNo > 0)
      PointA = realloc(PointA, PointNo * sizeof(*PointA));

    maxDist = solvRad + cellD;
    pointGrid = GridNew();
    for (i = 0; i < PointNo; i++)
      GridPrepareAddEntry(pointGrid, PointA[i].x);
    GridInsertInit(pointGrid, maxDist);
    for (i = 0; i < PointNo; i++)
      GridInsertEntry(pointGrid, PointA[i].x, PointA + i);
  }

  insideGrid = GridNew();
  outsideGrid = GridNew();
  for (i = 0; i < sphereNo; i++) {
    GridPrepareAddEntry(insideGrid, sphereA[i].c);
    GridPrepareAddEntry(outsideGrid, sphereA[i].c);
  }

  /* we could take rMax - cellD as grid size here, but that gives a
     small number if cellD is large (low precision), which results
     in a huge grid */
  GridInsertInit(insideGrid, rMax);

  if (solvRad == 0.0f)
    GridInsertInit(outsideGrid, rMax + cellD);
  else
    GridInsertInit(outsideGrid, rMax + solvRad);

  for (i = 0; i < sphereNo; i++) {
    GridInsertEntry(insideGrid, sphereA[i].c, sphereA + i);
    GridInsertEntry(outsideGrid, sphereA[i].c, sphereA + i);
  }

  totSize = 1;
  for (k = 0; k < 3; k++) {
    valSize[k] = (int) ceilf((vMax[k] - vMin[k]) / cellW) + 1;
    excess = (valSize[k] - 1) * cellW - (vMax[k] - vMin[k]);
    vMin[k] -= 0.5f * excess;
    vMax[k] += 0.5f * excess;
    totSize *= valSize[k];
  }

  valA = malloc(totSize * sizeof(*valA));
  /* this can be very large, check return value */
  if (valA != NULL) {
    for (i = 0; i < totSize; i++)
      valA[i] = 0.0f;  /* for the case that calculation is interrupted */

    if (solvRad == 0.0f)
      fillVdwVal(valA, valSize, vMin, cellW,
	  cellD, insideGrid, outsideGrid);
    else
      fillSolvVal(valA, valSize, vMin, cellW,
	  maxDist, insideGrid, outsideGrid, pointGrid);
    IsoSurfaceCalc(valA, valSize[0], valSize[1], valSize[2],
	vMin[0], vMin[1], vMin[2],
	vMax[0] - vMin[0], vMax[1] - vMin[1], vMax[2] - vMin[2],
	solvRad, TRUE,
	&surfA, &surfNo);
    free(valA);
  } else {
    surfA = NULL;
    surfNo = 0;
  }

  GridDestroy(insideGrid);
  GridDestroy(outsideGrid);

  if (solvRad > 0.0f) {
    free(PointA);
    GridDestroy(pointGrid);
    free(SpherePoints);
  }

  free(sphereA);

#if DEBUG
  (void) printf("time: %d\n", clock() - t0);
#endif

  *surfAP = surfA;
  *surfNoP = surfNo;
}
