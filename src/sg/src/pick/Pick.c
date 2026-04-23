/*
************************************************************************
*
*   Pick.c - picking
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
*   Date of last modification : 01/05/27
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/pick/SCCS/s.Pick.c
*   SCCS identification       : 1.7
*
************************************************************************
*/

#include "pick.h"

#include <values.h>
#include <math.h>

#include <mat_vec.h>
#include <sg_map.h>
#include <sg_pick.h>

#define Z_2D -1.0f  /* z value for 2D objects, closer than all 3D */

static float PickP[2];

SgRes
SgPickInit(int *argcP, char *argv[])
{
  SgPickGetPoint(PickP);

  return SG_RES_OK;
}

SgRes
SgPickCleanup(void)
{
  return SG_RES_OK;
}

static float
pointDist(float x[2])
{
  return (x[0] - PickP[0]) * (x[0] - PickP[0]) +
      (x[1] - PickP[1]) * (x[1] - PickP[1]);
}

void
SgPickDrawMarkers2D(float x[][2], int n)
{
  int i;

  for (i = 0; i < n; i++)
    SgPickDist(pointDist(x[i]), Z_2D, 0);
}

void
SgPickDrawMarkers(float x[][3], int n)
{
  float xm[3];
  int i;

  for (i = 0; i < n; i++)
    if (SgMapPoint(xm, x[i]) > 0)
      SgPickDist(pointDist(xm), xm[2], 0);
}

void
pickLine(float x0m[3], float x1m[3])
{
  float v1[2], v2[2];
  float a, s, d;
  int closeInd;

  v1[0] = x1m[0] - x0m[0];
  v1[1] = x1m[1] - x0m[1];
  v2[0] = PickP[0] - x0m[0];
  v2[1] = PickP[1] - x0m[1];

  s = v1[0] * v2[0] + v1[1] * v2[1];
  if (s < 0.0f) {
    SgPickDist(pointDist(x0m), x0m[2], 0);
    return;
  }

  a = v1[0] * v1[0] + v1[1] * v1[1];
  if (s > a) {
    SgPickDist(pointDist(x1m), x1m[2], 1);
    return;
  }

  if (s < 0.25f * a)
    closeInd = 0;
  else
    closeInd = 1;

  d = v1[0] * v2[1] - v1[1] * v2[0];
  if (d == 0.0f)
    /* calculate depth */
    SgPickDist(0.0f, x0m[2] + sqrtf(s / a) * (x1m[2] - x0m[2]), closeInd);
  else
    SgPickDist(d * d / a, 0.0f, closeInd);
}

void
SgPickDrawLine2D(float x0[2], float x1[2])
{
  float x0m[3], x1m[3];

  x0m[0] = x0[0];
  x0m[1] = x0[1];
  x0m[2] = Z_2D;
  x1m[0] = x1[0];
  x1m[1] = x1[1];
  x1m[2] = Z_2D;
  pickLine(x0m, x1m);
}

void
SgPickDrawLine(float x0[3], float x1[3])
{
  float x0m[3], x1m[3];

  if (SgMapLine(x0m, x1m, x0, x1) == 0)
    return;
  
  pickLine(x0m, x1m);
}

void
SgPickDrawPolyline2D(float x[][2], int n)
{
  int i;

  for (i = 0; i < n - 1; i++)
    SgPickDrawLine2D(x[i], x[i + 1]);
}

void
SgPickDrawPolyline(float x[][3], int n)
{
  int i;

  for (i = 0; i < n - 1; i++)
    SgPickDrawLine(x[i], x[i + 1]);
}

static float
edgeDist(float x0[2], float x1[2])
{
  float v1[2], v2[2];
  float a, s, d;
  
  v1[0] = x1[0] - x0[0];
  v1[1] = x1[1] - x0[1];
  v2[0] = PickP[0] - x0[0];
  v2[1] = PickP[1] - x0[1];

  d = v1[0] * v2[1] - v1[1] * v2[0];
  if (d >= 0.0f)
    return 0.0f;
  
  s = v1[0] * v2[0] + v1[1] * v2[1];
  if (s < 0.0f)
    return v2[0] * v2[0] + v2[1] * v2[1];

  a = v1[0] * v1[0] + v1[1] * v1[1];
  if (s > a)
    return pointDist(x1);

  return d * d / a;
}

void
pickPolygon(float xm[][3], int n)
{
  int closeInd;
  float v1[3], v2[3];
  BOOL clockWise, isInside;
  float minDist, det, d;
  float px, py, s1, s2;
  int i;

  minDist = MAXFLOAT;
  for (i = 0; i < n; i++) {
    d = pointDist(xm[i]);
    if (d < minDist) {
      minDist = d;
      closeInd = i;
    }
  }

  minDist = MAXFLOAT;

  if (n == SG_POLY_SIZE)
    n--;
  Vec3Copy(xm[n], xm[0]);

  /* assume convex polygon */
  Vec3Copy(v1, xm[1]);
  Vec3Sub(v1, xm[0]);
  Vec3Copy(v2, xm[2]);
  Vec3Sub(v2, xm[0]);
  det = v1[0] * v2[1] - v1[1] * v2[0];
  clockWise = det < 0.0f;
  isInside = TRUE;
  for (i = 0; i < n; i++) {
    if (clockWise)
      d = edgeDist(xm[i + 1], xm[i]);
    else
      d = edgeDist(xm[i], xm[i + 1]);

    if (d > 0.0f) {
      isInside = FALSE;
      if (d < minDist)
	minDist = d;
    }
  }

  if (isInside) {
    if (det == 0.0f) {  /* will probably never happen, but test anyway... */
      SgPickDist(0.0f, 0.0f, closeInd);
    } else {
      px = PickP[0] - xm[0][0];
      py = PickP[1] - xm[0][1];
      s1 = (px * v2[1] - py * v2[0]) / det;
      s2 = (v1[0] * py - v1[1] * px) / det;
      SgPickDist(0.0f, xm[0][2] +
	  s1 * (xm[1][2] - xm[0][2]) +
	  s2 * (xm[2][2] - xm[0][2]),
	  closeInd);
    }
  } else {
    SgPickDist(minDist, 0.0f, closeInd);
  }
}

void
SgPickDrawPolygon2D(float x[][2], int n)
{
  float xm[SG_POLY_SIZE][3];
  int i;

  for (i = 0; i < n; i++) {
    xm[i][0] = x[i][0];
    xm[i][1] = x[i][1];
    xm[i][2] = Z_2D;
  }

  pickPolygon(xm, n);
}

void
SgPickDrawPolygon(float x[][3], int n, float nv[3])
{
  float xm[SG_POLY_SIZE][3];

  n = SgMapPoly(xm, x, n);
  if (n == 0)
    return;

  pickPolygon(xm, n);
}

void
SgPickDrawCircle2D(float x[2], float r)
{
  float d;

  d = sqrtf(pointDist(x));
  SgPickDist((d - r) * (d - r), Z_2D, 0);
}

void
SgPickDrawDisc2D(float x[2], float r)
{
  float d;

  d = pointDist(x);
  if (d < r * r) {
    SgPickDist(0.0f, Z_2D, 0);
  } else {
    d = sqrtf(d);
    SgPickDist((d - r) * (d - r), Z_2D, 0);
  }
}

void
SgPickDrawAnnot2D(float x[2], char *str)
{
  SgPickDist(pointDist(x), Z_2D, 0);
}

void
SgPickDrawText2D(float x[2], char *str)
{
  SgPickDist(pointDist(x), Z_2D, 0);
}

void
SgPickDrawAnnot(float x[3], char *str)
{
  float xm[3];

  if (SgMapPoint(xm, x) == 0)
    return;

  SgPickDist(pointDist(xm), xm[2], 0);
}

void
SgPickDrawText(float x[3], char *str)
{
  float xm[3];

  if (SgMapPoint(xm, x) == 0)
    return;

  SgPickDist(pointDist(xm), xm[2], 0);
}

void
SgPickDrawSphere(float x[3], float r)
{
  float xm[3], ry, rz, d;

  if (SgMapSphere(xm, &ry, &rz, x, r) == 0)
    return;
  d = pointDist(xm);
  if (d < ry * ry)
    SgPickDist(0.0f, xm[2] - rz / ry * sqrtf(ry * ry - d), 0);
  else
    SgPickDist(d + ry * ry - 2.0f * ry * sqrtf(d), 0.0f, 0);
}

void
SgPickDrawCone(float x0[3], float x1[3], float r0, float r1,
    SgConeEnd end0, SgConeEnd end1)
{
  float xm[5][3];
  float rz0, rz1;
  BOOL isInside;
  float minDist = MAXFLOAT, d;
  int closeInd;
  float v1[3], v2[3], det, px, py, s1, s2, c;
  int i;

  if (SgMapCone(xm, &rz0, &rz1, x0, x1, r0, r1) == 0)
    return;
  
  if (pointDist(xm[0]) < pointDist(xm[2]))
    closeInd = 0;
  else
    closeInd = 1;

  xm[4][0] = xm[0][0];
  xm[4][1] = xm[0][1];

  isInside = TRUE;
  for (i = 0; i < 4; i++) {
    d = edgeDist(xm[i], xm[i + 1]);

    if (d > 0.0f) {
      isInside = FALSE;
      if (d < minDist)
	minDist = d;
    }
  }

  if (isInside) {
    Vec3Copy(v1, xm[1]);
    Vec3Sub(v1, xm[0]);
    Vec3Copy(v2, xm[3]);
    Vec3Sub(v2, xm[0]);
    det = v1[0] * v2[1] - v1[1] * v2[0];
    px = PickP[0] - xm[0][0];
    py = PickP[1] - xm[0][1];
    s1 = (px * v2[1] - py * v2[0]) / det;
    s2 = (v1[0] * py - v1[1] * px) / det;
    c = 2.0f * sqrtf(s1 * (1.0f - s1));
    SgPickDist(0.0f, xm[0][2] +
	s1 * (xm[1][2] - xm[0][2]) +
	s2 * (xm[3][2] - xm[0][2]) -
	(1.0f - s2) * c * rz0 + s2 * c * rz1,
	closeInd);
  } else {
    SgPickDist(minDist, 0.0f, closeInd);
  }
}
