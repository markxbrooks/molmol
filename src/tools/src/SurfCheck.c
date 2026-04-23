/*
************************************************************************
*
*   SurfCheck.c - check for points inside surface
*
*   Copyright (c) 1996
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/tools/src/SCCS/s.SurfCheck.c
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include <surf_check.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <values.h>

#include <surface.h>
#include <grid.h>

struct SurfaceCheckDescrS {
  float solvRad;
  float maxDist;
  PatchDescr *patchA;
  int patchNo;
  GRID pointGrid;
  GRID insideGrid;
  GRID outsideGrid;
  BOOL inside;
};

SurfaceCheckDescrP
SurfaceCheckPrepareContact(PotAtomDescr atomDescrA[], int atomNo,
    float solvRad, int prec)
{
  SurfaceCheckDescrP surfDescrP;
  SphereDescr *sphereDescrA;
  PatchDescr *patchA;
  int patchNo;
  float maxRad, d;
  int atomI, patchI, pointI;

  surfDescrP = malloc(sizeof(*surfDescrP));

  surfDescrP->solvRad = solvRad;

  sphereDescrA = malloc(atomNo * sizeof(*sphereDescrA));

  maxRad = 0.0f;
  for (atomI = 0; atomI < atomNo; atomI++) {
    Vec3Copy(sphereDescrA[atomI].cent, atomDescrA[atomI].cent);
    sphereDescrA[atomI].rad = atomDescrA[atomI].rad + solvRad;
    if (atomDescrA[atomI].rad > maxRad)
      maxRad = atomDescrA[atomI].rad;
    sphereDescrA[atomI].userData = NULL;
  }

  SurfaceDots(sphereDescrA, atomNo, 0.0f, prec, FALSE,
      &patchA, &patchNo);
  surfDescrP->patchA = patchA;
  surfDescrP->patchNo = patchNo;

  free(sphereDescrA);

  surfDescrP->pointGrid = GridNew();
  for (patchI = 0; patchI < patchNo; patchI++)
    for (pointI = 0; pointI < patchA[patchI].pointNo; pointI++)
      GridPrepareAddEntry(surfDescrP->pointGrid,
	  patchA[patchI].pointA[pointI]);

  /* approximate half distance between two sphere points */
  d = 0.25f * (float) M_PI / (1 << prec) * (maxRad + solvRad);
  surfDescrP->maxDist = sqrtf(solvRad * solvRad + d * d);
  GridInsertInit(surfDescrP->pointGrid, surfDescrP->maxDist);
  for (patchI = 0; patchI < patchNo; patchI++)
    for (pointI = 0; pointI < patchA[patchI].pointNo; pointI++)
      GridInsertEntry(surfDescrP->pointGrid,
	  patchA[patchI].pointA[pointI], patchA[patchI].pointA[pointI]);

  surfDescrP->insideGrid = GridNew();
  surfDescrP->outsideGrid = GridNew();
  for (atomI = 0; atomI < atomNo; atomI++) {
    GridPrepareAddEntry(surfDescrP->insideGrid, atomDescrA[atomI].cent);
    GridPrepareAddEntry(surfDescrP->outsideGrid, atomDescrA[atomI].cent);
  }
  GridInsertInit(surfDescrP->insideGrid, maxRad);
  GridInsertInit(surfDescrP->outsideGrid, maxRad + solvRad);
  for (atomI = 0; atomI < atomNo; atomI++) {
    GridInsertEntry(surfDescrP->insideGrid,
	atomDescrA[atomI].cent, atomDescrA + atomI);
    GridInsertEntry(surfDescrP->outsideGrid,
	atomDescrA[atomI].cent, atomDescrA + atomI);
  }

  return surfDescrP;
}

SurfaceCheckDescrP
SurfaceCheckPrepareSolvent(PotAtomDescr atomDescrA[], int atomNo,
    float solvRad)
{
  SurfaceCheckDescrP surfDescrP;
  float maxRad;
  int atomI;

  surfDescrP = malloc(sizeof(*surfDescrP));

  surfDescrP->solvRad = solvRad;
  surfDescrP->outsideGrid = NULL;
  surfDescrP->patchA = NULL;
  surfDescrP->patchNo = 0;
  surfDescrP->pointGrid = NULL;

  maxRad = 0.0f;
  surfDescrP->insideGrid = GridNew();
  for (atomI = 0; atomI < atomNo; atomI++) {
    GridPrepareAddEntry(surfDescrP->insideGrid, atomDescrA[atomI].cent);
    if (atomDescrA[atomI].rad > maxRad)
      maxRad = atomDescrA[atomI].rad;
  }

  GridInsertInit(surfDescrP->insideGrid, maxRad + solvRad);

  for (atomI = 0; atomI < atomNo; atomI++)
    GridInsertEntry(surfDescrP->insideGrid,
	atomDescrA[atomI].cent, atomDescrA + atomI);

  return surfDescrP;
}

static BOOL
checkInside(void *entryP, float *x, void *clientData)
{
  PotAtomDescr *atomDescrP = entryP;
  SurfaceCheckDescrP surfDescrP = clientData;
  float rad;
  Vec3 dx;

  Vec3Copy(dx, x);
  Vec3Sub(dx, atomDescrP->cent);

  if (surfDescrP->outsideGrid == NULL)
    rad = atomDescrP->rad + surfDescrP->solvRad;
  else
    rad = atomDescrP->rad;

  if (dx[0] * dx[0] + dx[1] * dx[1] + dx[2] * dx[2] < rad * rad) {
    surfDescrP->inside = TRUE;
    return FALSE;
  }

  return TRUE;
}

static BOOL
checkOutside(void *entryP, float *x, void *clientData)
{
  PotAtomDescr *atomDescrP = entryP;
  SurfaceCheckDescrP surfDescrP = clientData;
  Vec3 dx;
  float rad;

  Vec3Copy(dx, x);
  Vec3Sub(dx, atomDescrP->cent);
  rad = atomDescrP->rad + surfDescrP->solvRad;
  if (dx[0] * dx[0] + dx[1] * dx[1] + dx[2] * dx[2] < rad * rad) {
    surfDescrP->inside = TRUE;
    return FALSE;
  }

  return TRUE;
}

static BOOL
getPointDist(void *entryP, float *x, void *clientData)
{
  float *p = entryP;
  float *distP = clientData;
  Vec3 dx;
  float dS;

  Vec3Copy(dx, x);
  Vec3Sub(dx, p);
  dS = dx[0] * dx[0] + dx[1] * dx[1] + dx[2] * dx[2];
  if (dS < *distP)
    *distP = dS;

  return TRUE;
}

BOOL
SurfaceCheckPoint(SurfaceCheckDescrP surfDescrP, Vec3 x)
{
  float dist;

  surfDescrP->inside = FALSE;
  GridFind(surfDescrP->insideGrid, x, 1, checkInside, surfDescrP);
  if (surfDescrP->inside)
    return TRUE;

  if (surfDescrP->outsideGrid == NULL)
    return FALSE;

  GridFind(surfDescrP->outsideGrid, x, 1, checkOutside, surfDescrP);
  if (! surfDescrP->inside)
    return FALSE;

  dist = MAXFLOAT;
  GridFind(surfDescrP->pointGrid, x, 1, getPointDist, &dist);

  return dist > surfDescrP->maxDist * surfDescrP->maxDist;
}

void
SurfaceCheckFree(SurfaceCheckDescrP surfDescrP)
{
  int patchI;

  for (patchI = 0; patchI < surfDescrP->patchNo; patchI++)
    free(surfDescrP->patchA[patchI].pointA);
  if (surfDescrP->patchA != NULL)
    free(surfDescrP->patchA);

  if (surfDescrP->pointGrid != NULL)
    GridDestroy(surfDescrP->pointGrid);
  GridDestroy(surfDescrP->insideGrid);
  if (surfDescrP->outsideGrid != NULL)
    GridDestroy(surfDescrP->outsideGrid);

  free(surfDescrP);
}
