/*
************************************************************************
*
*   DefCircle.c - default circle and cone drawing
*
*   Copyright (c) 1994-96
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/def/SCCS/s.DefCircle.c
*   SCCS identification       : 1.18
*
************************************************************************
*/

#include <sg_def.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <values.h>

#include <sg.h>
#include <sg_get.h>
#include <sg_map.h>
#include "def_sphere.h"

static const float EpsDist = ((float) 1.0e-6);

static int MaxPrec = -1;

typedef struct {
  int pointNo;
  float (* pointsF)[3];
  float (* pointsR)[3];
  float (* points2D)[2];
  float (* points0)[3];
  float (* points1)[3];
  float (* points2)[3];
} CircData;

static CircData *Circles;

static void
calcCirc(CircData *circP, int prec)
{
  int stepNo;
  double ang, cosAng, sinAng;
  double x0, y0, x1, y1;
  int i;

  stepNo = 1 << prec;
  circP->pointNo = 4 * stepNo + 1;

  circP->pointsF = malloc(circP->pointNo * sizeof(*circP->pointsF));
  circP->pointsR = malloc(circP->pointNo * sizeof(*circP->pointsR));
  circP->points2D = malloc(circP->pointNo * sizeof(*circP->points2D));
  circP->points0 = malloc(circP->pointNo * sizeof(*circP->points0));
  circP->points1 = malloc(circP->pointNo * sizeof(*circP->points1));
  circP->points2 = malloc(circP->pointNo * sizeof(*circP->points2));

  ang = M_PI / (2 * stepNo);
  cosAng = cos(ang);
  sinAng = sin(ang);
  x1 = 1.0;
  y1 = 0.0;
  for (i = 0; i < stepNo; i++) {
    circP->pointsF[i][0] = (float) x1;
    circP->pointsF[i][1] = (float) y1;

    circP->pointsF[stepNo + i][0] = - (float) y1;
    circP->pointsF[stepNo + i][1] = (float) x1;

    circP->pointsF[2 * stepNo + i][0] = - (float) x1;
    circP->pointsF[2 * stepNo + i][1] = - (float) y1;

    circP->pointsF[3 * stepNo + i][0] = (float) y1;
    circP->pointsF[3 * stepNo + i][1] = - (float) x1;

    x0 = x1;
    y0 = y1;
    x1 = x0 * cosAng - y0 * sinAng;
    y1 = x0 * sinAng + y0 * cosAng;
  }

  /* repeat starting point at the end to draw circle as polyline */
  circP->pointsF[4 * stepNo][0] = circP->pointsF[0][0];
  circP->pointsF[4 * stepNo][1] = circP->pointsF[0][1];

  for (i = 0; i < circP->pointNo; i++) {
    circP->pointsF[i][2] = 0.0f;

    circP->pointsR[circP->pointNo - i - 1][0] = circP->pointsF[i][0];
    circP->pointsR[circP->pointNo - i - 1][1] = circP->pointsF[i][1];
    circP->pointsR[circP->pointNo - i - 1][2] = 0.0f;
  }
}

static CircData *
getCirc(int prec)
{
  int i;

  if (prec > MaxPrec) {
    if (MaxPrec < 0)
      Circles = malloc((prec + 1) * sizeof(*Circles));
    else
      Circles = realloc(Circles, (prec + 1) * sizeof(*Circles));

    for (i = MaxPrec + 1; i <= prec; i++)
      calcCirc(Circles + i, i);

    MaxPrec = prec;
  }

  return Circles + prec;
}

void
drawCircle2D(float x[2], float r, BOOL filled)
{
  CircData *circP;
  int i;

  /* Use full precision here because 2D circles normally end up
     larger than 3D circles. They're rarely used anyway, so it
     doesn't matter if we use high precision. */
  circP = getCirc(SgGetPrecision());

  for (i = 0; i < circP->pointNo; i++) {
    circP->points2D[i][0] = r * circP->pointsF[i][0] + x[0];
    circP->points2D[i][1] = r * circP->pointsF[i][1] + x[1];
  }

  if (filled)
    SgDrawPolygon2D(circP->points2D, circP->pointNo - 1);
  else
    SgDrawPolyline2D(circP->points2D, circP->pointNo);
}

void
SgDefDrawCircle2D(float x[2], float r)
{
  drawCircle2D(x, r, FALSE);
}

void
SgDefDrawDisc2D(float x[2], float r)
{
  drawCircle2D(x, r, TRUE);
}

static void
setTransf(float x[3], float v[3])
{
  float a, f, m[4][4];

  a = sqrtf(v[0] * v[0] + v[1] * v[1]);
  f = 1.0f / a;
  if (a > EpsDist) {
    m[0][0] = v[0] * v[2] * f;
    m[0][1] = v[1] * v[2] * f;
    m[0][2] = - a;
    m[1][0] = - v[1] * f;
    m[1][1] = v[0] * f;
    m[1][2] = 0.0f;
    m[2][0] = v[0];
    m[2][1] = v[1];
    m[2][2] = v[2];
  } else if (v[2] < 0.0f) {
    m[0][0] = 1.0f;
    m[0][1] = 0.0f;
    m[0][2] = 0.0f;
    m[1][0] = 0.0f;
    m[1][1] = -1.0f;
    m[1][2] = 0.0f;
    m[2][0] = 0.0f;
    m[2][1] = 0.0f;
    m[2][2] = -1.0f;
  } else {
    m[0][0] = 1.0f;
    m[0][1] = 0.0f;
    m[0][2] = 0.0f;
    m[1][0] = 0.0f;
    m[1][1] = 1.0f;
    m[1][2] = 0.0f;
    m[2][0] = 0.0f;
    m[2][1] = 0.0f;
    m[2][2] = 1.0f;
  }

  m[0][3] = 0.0f;
  m[1][3] = 0.0f;
  m[2][3] = 0.0f;
  m[3][0] = x[0];
  m[3][1] = x[1];
  m[3][2] = x[2];
  m[3][3] = 1.0f;

  SgMultMatrix(m);
}

static void
drawCircle(float x[3], float v[3], float r, BOOL filled)
{
  CircData *circP;
  float f;
  int i;

  circP = getCirc(DefGetSpherePrecision(r));

  f = 1.0f / sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
  v[0] *= f;
  v[1] *= f;
  v[2] *= f;
  SgPushMatrix();
  setTransf(x, v);
  for (i = 0; i < circP->pointNo; i++) {
    circP->points0[i][0] = r * circP->pointsF[i][0];
    circP->points0[i][1] = r * circP->pointsF[i][1];
    circP->points0[i][2] = 0.0f;
  }
  if (filled)
    SgDrawPolygon(circP->points0, circP->pointNo - 1, v);
  else
    SgDrawPolyline(circP->points0, circP->pointNo);
  SgPopMatrix();
}

void
SgDefDrawCircle(float x[3], float v[3], float r)
{
  drawCircle(x, v, r, FALSE);
}

void
SgDefDrawDisc(float x[3], float v[3], float r)
{
  drawCircle(x, v, r, TRUE);
}

void
SgDefDrawCone(float x0[3], float x1[3], float r0, float r1,
    SgConeEnd end0, SgConeEnd end1)
{
  int prec;
  CircData *circP;
  float xm[4][3], xm2[4][2];
  float v[3], t[3], nv[3];
  float a, f, c0, s0, c1, s1, c2, s2;
  BOOL scaled;
  int i;

  prec = DefGetSpherePrecision(r0);

  circP = getCirc(prec);

  if (SgGetFeature(SG_FEATURE_FAST_SPHERE)) {
    if (SgMapCone(xm, NULL, NULL, x0, x1, r0, r1) > 0) {
      for (i = 0; i < 4; i++) {
	xm2[i][0] = xm[i][0];
	xm2[i][1] = xm[i][1];
      }
      SgDrawPolygon2D(xm2, 4);
    }
    return;
  }

  v[0] = x1[0] - x0[0];
  v[1] = x1[1] - x0[1];
  v[2] = x1[2] - x0[2];
  a = sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
  f = 1.0f / a;
  v[0] *= f;
  v[1] *= f;
  v[2] *= f;
  SgPushMatrix();
  setTransf(x0, v);

  if (r0 == r1 || end0 == SG_CONE_CAP_FLAT) {
    c0 = 0.0f;
    s0 = r0;
  } else {
    c0 = (r0 - r1) / a;
    if (c0 > 1.0f || c0 < -1.0f)  /* degenerate */
      c0 = 0.0f;
    s0 = sqrtf(1.0f - c0 * c0);
    c0 *= r0;
    s0 *= r0;
  }

  for (i = 0; i < circP->pointNo; i++) {
    circP->points0[i][0] = s0 * circP->pointsF[i][0];
    circP->points0[i][1] = s0 * circP->pointsF[i][1];
    circP->points0[i][2] = c0;
  }

  if (r0 == r1 || end1 == SG_CONE_CAP_FLAT) {
    c1 = a;
    s1 = r1;
  } else {
    c1 = (r0 - r1) / a;
    if (c1 > 1.0f || c1 < -1.0f)  /* degenerate */
      c1 = 0.0f;
    s1 = sqrtf(1.0f - c1 * c1);
    c1 = a + c1 * r1;
    s1 *= r1;
  }

  for (i = 0; i < circP->pointNo; i++) {
    circP->points1[i][0] = s1 * circP->pointsF[i][0];
    circP->points1[i][1] = s1 * circP->pointsF[i][1];
    circP->points1[i][2] = c1;
  }

  SgStartSurface(SG_SURF_UNIFORM);

  if (r0 == r1) {
    SgDrawStrip(circP->points1, circP->points0,
	circP->pointsF, NULL, circP->pointNo);
  } else {
    c2 = s0 - s1;
    s2 = c1 - c0;
    f = 1.0f / sqrtf(c2 * c2 + s2 * s2);
    c2 *= f;
    s2 *= f;
    for (i = 0; i < circP->pointNo; i++) {
      circP->points2[i][0] = s2 * circP->pointsF[i][0];
      circP->points2[i][1] = s2 * circP->pointsF[i][1];
      circP->points2[i][2] = c2;
    }
    SgDrawStrip(circP->points1, circP->points0,
	circP->points2, NULL, circP->pointNo);
  }

  scaled = FALSE;
  if (end0 == SG_CONE_CAP_FLAT) {
    for (i = 0; i < circP->pointNo; i++) {
      circP->points0[i][0] = r0 * circP->pointsR[i][0];
      circP->points0[i][1] = r0 * circP->pointsR[i][1];
    }
    nv[0] = 0.0f;
    nv[1] = 0.0f;
    nv[2] = -1.0f;
    SgDrawPolygon(circP->points0, circP->pointNo - 1, nv);
  } else if (end0 == SG_CONE_CAP_ROUND) {
    SgScale(r0, r0, r0);
    scaled = TRUE;
    DefDrawHalfSphere(FALSE, prec);
    if (r0 > r1)
      DefDrawHalfSphere(TRUE, prec);
  }

  if (end1 == SG_CONE_CAP_FLAT) {
    nv[0] = 0.0f;
    nv[1] = 0.0f;
    nv[2] = 1.0f;
    SgDrawPolygon(circP->points1, circP->pointNo - 1, nv);
  } else if (end1 == SG_CONE_CAP_ROUND) {
    if (scaled) {
      if (r1 != r0)
	SgScale(r1 / r0, r1 / r0, r1 / r0);
    } else {
      SgScale(r1, r1, r1);
    }
    t[0] = 0.0f;
    t[1] = 0.0f;
    t[2] = a / r1;
    SgTranslate(t);
    DefDrawHalfSphere(TRUE, prec);
    if (r1 > r0)
      DefDrawHalfSphere(FALSE, prec);
  }

  SgEndSurface();

  SgPopMatrix();
}
