/*
************************************************************************
*
*   SurfaceTrim.c - surface trimming
*
*   Copyright (c) 1997
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/prim/SCCS/s.SurfaceTrim.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <stdio.h>
#include <stdlib.h>

#include "prim_struc.h"

void
dotSurfaceTrim(PrimObjP primP,
    PrimSurfaceTrimFunc trimF, void *clientData)
{
  PrimDotSurface *surfaceP;
  PrimDotSurfacePatch *patchP;
  int patchI, patchKeepI;
  int pointI, pointKeepI;
  float dist;

  surfaceP = &primP->u.dotSurface;

  patchKeepI = 0;
  for (patchI = 0; patchI < surfaceP->size; patchI++) {
    patchP = surfaceP->p + patchI;

    pointKeepI = 0;
    for (pointI = 0; pointI < patchP->size; pointI++) {
      trimF(&dist, patchP->p[pointI], clientData);
      if (dist > 0.0f) {
	Vec3Copy(patchP->p[pointKeepI], patchP->p[pointI]);
	pointKeepI++;
      }
    }

    if (pointKeepI > 0) {
      patchP->size = pointKeepI;
      patchP->p = realloc(patchP->p, patchP->size * sizeof(*patchP->p));
      surfaceP->p[patchKeepI] = *patchP;
      patchKeepI++;
    } else {
      free(patchP->p);
    }
  }

  surfaceP->size = patchKeepI;
  surfaceP->p = realloc(surfaceP->p, surfaceP->size * sizeof(*surfaceP->p));
}

void
surfaceTrim(PrimObjP primP,
    PrimSurfaceTrimFunc trimF, void *clientData)
{
  PrimSurface *surfaceP;
  PrimSurfaceMesh *meshP, *newMeshP;
  float *distA;
  int meshI, meshKeepI;
  int pointI, pointKeepI;
  int newMeshNo;
  BOOL isFirst;
  int indA[3], firstIndA[3], p;

  surfaceP = &primP->u.surface;

  distA = malloc(surfaceP->pointNo * sizeof(*distA));
  for (pointI = 0; pointI < surfaceP->pointNo; pointI++)
    trimF(distA + pointI, surfaceP->xA[pointI], clientData);

  /* first eliminate meshes that are trimmed away completely */
  meshKeepI = 0;
  for (meshI = 0; meshI < surfaceP->meshNo; meshI++) {
    meshP = surfaceP->meshA + meshI;

    for (pointI = 2; pointI < meshP->size; pointI++)
      if (distA[meshP->indA[pointI - 2]] > 0.0f &&
	  distA[meshP->indA[pointI - 1]] > 0.0f &&
	  distA[meshP->indA[pointI]] > 0.0f)
	break;

    if (pointI < meshP->size) {
      surfaceP->meshA[meshKeepI] = *meshP;
      meshKeepI++;
    } else {
      free(meshP->indA);
    }
  }

  surfaceP->meshNo = meshKeepI;
  surfaceP->meshA = realloc(surfaceP->meshA,
      surfaceP->meshNo * sizeof(*surfaceP->meshA));

  /* clip the meshes that are trimmed away partly */
  newMeshNo = surfaceP->meshNo;
  for (meshI = 0; meshI < surfaceP->meshNo; meshI++) {
    meshP = surfaceP->meshA + meshI;

    pointKeepI = 0;
    for (pointI = 0; pointI < meshP->size; pointI++)
      if (distA[meshP->indA[pointI]] > 0.0f)
	pointKeepI++;

    if (pointKeepI == meshP->size)  /* not trimmed at all */
      continue;

    /* create only single triangles to make things simpler */
    isFirst = TRUE;
    indA[0] = meshP->indA[0];
    indA[1] = meshP->indA[1];
    p = 0;
    for (pointI = 2; pointI < meshP->size; pointI++) {
      indA[2] = meshP->indA[pointI];

      if (distA[indA[0]] > 0.0f && distA[indA[1]] > 0.0f &&
	  distA[indA[2]] > 0.0f) {
	if (isFirst) {
	  firstIndA[0] = indA[0];
	  firstIndA[1] = indA[1];
	  firstIndA[2] = indA[2];
	  isFirst = FALSE;
	} else {
	  newMeshNo++;
	  surfaceP->meshA = realloc(surfaceP->meshA,
	      newMeshNo * sizeof(*surfaceP->meshA));
	  newMeshP = surfaceP->meshA + newMeshNo - 1;
	  newMeshP->indA = malloc(3 * sizeof(*newMeshP->indA));
	  newMeshP->indA[0] = indA[0];
	  newMeshP->indA[1] = indA[1];
	  newMeshP->indA[2] = indA[2];
	  newMeshP->size = 3;
	  meshP = surfaceP->meshA + meshI;  /* can move in realloc */
	}
      }

      indA[p] = indA[2];
      p = 1 - p;
    }

    meshP->indA = realloc(meshP->indA, 3 * sizeof(*meshP->indA));
    meshP->indA[0] = firstIndA[0];
    meshP->indA[1] = firstIndA[1];
    meshP->indA[2] = firstIndA[2];
    meshP->size = 3;
  }

  surfaceP->meshNo = newMeshNo;

  free(distA);
}

void
PrimSurfaceTrim(PrimObjP primP,
    PrimSurfaceTrimFunc trimF, void *clientData)
{
  if (primP->type == PT_DOT_SURFACE)
    dotSurfaceTrim(primP, trimF, clientData);
  else
    surfaceTrim(primP, trimF, clientData);
}
