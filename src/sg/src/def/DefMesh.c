/*
************************************************************************
*
*   DefMesh.c - default mesh drawing for Sg
*
*   Copyright (c) 1994
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
*   Date of last modification : 96/12/03
*   Pathname of SCCS file     : /local/home/kor/molmol/sg/src/def/SCCS/s.DefMesh.c
*   SCCS identification       : 1.9
*
************************************************************************
*/

#include <sg_def.h>

#include <stdio.h>
#include <stdlib.h>

#include <mat_vec.h>
#include <sg.h>
#include <sg_get.h>
#include <sg_map.h>
#include <sg_shade.h>
#include "def_shade.h"
#include "def_hidden.h"

#define ASSIGNV(v1, v2) {v1[0] = v2[0]; v1[1] = v2[1]; v1[2] = v2[2];}

void
SgDefStartSurface(SgSurfaceType type)
{
}

static BOOL
do2DShade(void)
{
  SgShadeModel model;

  model = DefGetShadeModel();

  return (SgGetFeature(SG_FEATURE_SHADE) && SgHas2DShading() &&
      (model == SG_SHADE_GOURAUD || model == SG_SHADE_PHONG));
}

static void
getCol(float acol[3], float col[][3], int i, float ccol[3],
    Vec3 xe, Vec3 nve, BOOL inv, BOOL fog)
{
  Vec3 nvi;

  if (col == NULL) {
    ASSIGNV(acol, ccol);
  } else {
    ASSIGNV(acol, col[i]);
  }
  
  if (inv) {
    nvi[0] = - nve[0];
    nvi[1] = - nve[1];
    nvi[2] = - nve[2];
    DefShade(acol + 0, acol + 1, acol + 2, xe, nvi);
  } else {
    DefShade(acol + 0, acol + 1, acol + 2, xe, nve);
  }

  if (fog)
    DefFog(acol + 0, acol + 1, acol + 2, xe[2]);
}

void
drawShadedTriMesh(float x[][3], float nv[][3], float col[][3], int n)
{
  Vec3 tri[3], trie[3], trim[3], nve[3], v1, v2;
  float ccol[3], fcol[3][3], bcol[3][3];
  BOOL backface, twoSide, hidden, fog;
  int in[3];
  BOOL drawIt, takeBack;
  float xm2[3][2];
  int p, i;

  if (col == NULL)
    SgGetColor(ccol + 0, ccol + 1, ccol + 2);

  backface = SgGetFeature(SG_FEATURE_BACKFACE);
  twoSide = SgGetFeature(SG_FEATURE_TWO_SIDE);
  hidden = SgGetFeature(SG_FEATURE_DEPTH_SORT) ||
      SgGetFeature(SG_FEATURE_HIDDEN);
  fog = DefFogEnabled();

  ASSIGNV(tri[0], x[0]);
  SgMapEyePoint(trie[0], tri[0]);
  in[0] = SgMapPoint(trim[0], tri[0]);
  SgMapEyeVect(nve[0], nv[0]);
  Vec3Norm(nve[0]);
  getCol(fcol[0], col, 0, ccol, trie[0], nve[0], FALSE, fog);
  if (twoSide)
    getCol(bcol[0], col, 0, ccol, trie[0], nve[0], TRUE, fog);

  ASSIGNV(tri[1], x[1]);
  SgMapEyePoint(trie[1], tri[1]);
  in[1] = SgMapPoint(trim[1], tri[1]);
  SgMapEyeVect(nve[1], nv[1]);
  Vec3Norm(nve[1]);
  getCol(fcol[1], col, 1, ccol, trie[1], nve[1], FALSE, fog);
  if (twoSide)
    getCol(bcol[1], col, 1, ccol, trie[1], nve[1], TRUE, fog);

  p = 0;
  for (i = 2; i < n; i++) {
    ASSIGNV(tri[2], x[i]);
    SgMapEyePoint(trie[2], tri[2]);
    in[2] = SgMapPoint(trim[2], tri[2]);
    SgMapEyeVect(nve[2], nv[i]);
    Vec3Norm(nve[2]);
    getCol(fcol[2], col, i, ccol, trie[2], nve[2], FALSE, fog);
    if (twoSide)
      getCol(bcol[2], col, i, ccol, trie[2], nve[2], TRUE, fog);

    /* we do not perform correct clipping of the triangles here,
       they are just left away as soon as one point is outside */
    drawIt = (in[0] + in[1] + in[2] == 3);
    takeBack = FALSE;

    if (drawIt && (backface || twoSide)) {
      Vec3Copy(v1, tri[1]);
      Vec3Sub(v1, tri[0]);
      Vec3Copy(v2, tri[2]);
      Vec3Sub(v2, tri[0]);
      Vec3Cross(v1, v2);
      SgMapEyeVect(v1, v1);
      if (SgBackfacing(trie[0], v1)) {
	if (backface)
	  drawIt = FALSE;
	else
	  takeBack = TRUE;
      }
    }

    if (drawIt) {
      if (hidden) {
	if (takeBack)
	  DefHiddenShadedPolygon(trim, bcol, 3);
	else
	  DefHiddenShadedPolygon(trim, fcol, 3);
      } else {
	for (i = 0; i < 3; i++) {
	  xm2[i][0] = trim[i][0];
	  xm2[i][1] = trim[i][1];
	}

	if (takeBack)
	  SgDrawShadedPolygon2D(xm2, bcol, 3);
	else
	  SgDrawShadedPolygon2D(xm2, fcol, 3);
      }
    }

    ASSIGNV(tri[p], tri[2]);
    ASSIGNV(trie[p], trie[2]);
    ASSIGNV(trim[p], trim[2]);
    in[p] = in[2];
    ASSIGNV(nve[p], nve[2]);
    ASSIGNV(fcol[p], fcol[2]);
    if (twoSide) {
      ASSIGNV(bcol[p], bcol[2]);
    }

    p = 1 - p;
  }
}

void
SgDefDrawTriMesh(float x[][3], float nv[][3], int n)
{
  float tri[3][3];
  int p, i;

  if (do2DShade()) {
    drawShadedTriMesh(x, nv, NULL, n);
    return;
  }

  ASSIGNV(tri[0], x[0]);
  ASSIGNV(tri[1], x[1]);
  p = 0;
  for (i = 2; i < n; i++) {
    ASSIGNV(tri[2], x[i]);
    SgDrawPolygon(tri, 3, nv[i]);
    ASSIGNV(tri[p], x[i]);
    p = 1 - p;
  }
}

void
SgDefDrawColorTriMesh(float x[][3], float nv[][3], float col[][3], int n)
{
  float tri[3][3];
  int p, i;

  if (do2DShade()) {
    drawShadedTriMesh(x, nv, col, n);
    return;
  }

  ASSIGNV(tri[0], x[0]);
  ASSIGNV(tri[1], x[1]);
  p = 0;
  for (i = 2; i < n; i++) {
    ASSIGNV(tri[2], x[i]);
    SgSetColor(col[i][0], col[i][1], col[i][2]);
    SgDrawPolygon(tri, 3, nv[i]);
    ASSIGNV(tri[p], x[i]);
    p = 1 - p;
  }
}

static void
drawShadedStrip(float x1[][3], float x2[][3],
    float nv1[][3], float nv2[][3],
    float col1[][3], float col2[][3], int n)
{
  float (*x)[3], (*nv)[3], (*col)[3];
  int i;

  if (nv2 == NULL)
    nv2 = nv1;
  
  if (col2 == NULL)
    col2 = col1;
  
  /* convert to TriMesh to save code */

  x = malloc(2 * n * sizeof(*x));
  nv = malloc(2 * n * sizeof(*nv));
  if (col1 == NULL)
    col = NULL;
  else
    col = malloc(2 * n * sizeof(*col));

  for (i = 0; i < n; i++) {
    ASSIGNV(x[2 * i], x1[i]);
    ASSIGNV(x[2 * i + 1], x2[i]);
    ASSIGNV(nv[2 * i], nv1[i]);
    ASSIGNV(nv[2 * i + 1], nv2[i]);
    if (col != NULL) {
      ASSIGNV(col[2 * i], col1[i]);
      ASSIGNV(col[2 * i + 1], col2[i]);
    }
  }

  drawShadedTriMesh(x, nv, col, 2 * n);

  free(x);
  free(nv);
  if (col != NULL)
    free(col);
}

void
SgDefDrawStrip(float x1[][3], float x2[][3],
    float nv1[][3], float nv2[][3], int n)
{
  float tri[3][3];
  int i;

  if (do2DShade()) {
    drawShadedStrip(x1, x2, nv1, nv2, NULL, NULL, n);
    return;
  }

  /* draw as triangles, quads might not be planar */
  for (i = 0; i < n - 1; i++) {
    ASSIGNV(tri[0], x1[i]);
    ASSIGNV(tri[1], x2[i]);
    ASSIGNV(tri[2], x1[i + 1]);
    SgDrawPolygon(tri, 3, nv1[i]);
    ASSIGNV(tri[0], x2[i]);
    ASSIGNV(tri[1], x2[i + 1]);
    SgDrawPolygon(tri, 3, nv1[i]);
  }
}

void
SgDefDrawColorStrip(float x1[][3], float x2[][3],
    float nv1[][3], float nv2[][3],
    float col1[][3], float col2[][3], int n)
{
  float tri[3][3];
  int i;

  if (do2DShade()) {
    drawShadedStrip(x1, x2, nv1, nv2, col1, col2, n);
    return;
  }

  /* draw as triangles, quads might not be planar */
  for (i = 0; i < n - 1; i++) {
    ASSIGNV(tri[0], x1[i]);
    ASSIGNV(tri[1], x2[i]);
    ASSIGNV(tri[2], x1[i + 1]);
    SgSetColor(col1[i][0], col1[i][1], col1[i][2]);
    SgDrawPolygon(tri, 3, nv1[i]);
    ASSIGNV(tri[0], x2[i]);
    ASSIGNV(tri[1], x2[i + 1]);
    SgDrawPolygon(tri, 3, nv1[i]);
  }
}

void
SgDefEndSurface(void)
{
}
