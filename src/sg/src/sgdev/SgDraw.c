/*
************************************************************************
*
*   SgDraw.c - Sg wrappers for drawing functions
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/sgdev/SCCS/s.SgDraw.c
*   SCCS identification       : 1.18
*
************************************************************************
*/

#include <sg_dev.h>
#include "sg_set.h"

#include <stdio.h>

#include <sg_get.h>
#include <sg_shade.h>

static SgShadeModel CurrShadeModel = SG_SHADE_FLAT;
static SgLightState CurrLightState = SG_LIGHT_OFF;
static SgLightState ReqLightState = SG_LIGHT_OFF;
static float CurrLightPos[3] = {0.0f, 0.0f, 0.0f};

void
SgSetShadeModel(SgShadeModel model)
{
  if (model == CurrShadeModel)
    return;

  CurrShadeModel = model;

  if (model == SG_SHADE_DOTS || model == SG_SHADE_LINES) {
    if (CurrLightState != SG_LIGHT_OFF) {
      CurrSgDevP->setLight(SG_LIGHT_OFF);
      CurrLightState = SG_LIGHT_OFF;
    }
  } else {
    CurrSgDevP->setShadeModel(model);
    if (CurrLightState != ReqLightState) {
      CurrSgDevP->setLight(ReqLightState);
      CurrLightState = ReqLightState;
    }
  }
}

void
SgSetLight(SgLightState state)
{
  if (state == ReqLightState)
    return;

  ReqLightState = state;

  if (CurrShadeModel == SG_SHADE_DOTS || CurrShadeModel == SG_SHADE_LINES)
    return;

  CurrSgDevP->setLight(state);
  CurrLightState = state;
}

void
SgSetLightPosition(float x[3])
{
  CurrSgDevP->setLightPosition(x);
  CurrLightPos[0] = x[0];
  CurrLightPos[1] = x[1];
  CurrLightPos[2] = x[2];
}

void
SgSetShadeAttr(void)
{
  CurrSgDevP->setShadeModel(CurrShadeModel);
  CurrSgDevP->setLight(CurrLightState);
  CurrSgDevP->setLightPosition(CurrLightPos);
}

void
SgDrawMarkers2D(float x[][2], int n)
{
  int i;

  for (i = 0; i < n / SG_POLY_SIZE; i++)
    CurrSgDevP->drawMarkers2D(x + i * SG_POLY_SIZE, SG_POLY_SIZE);

  n -= i * SG_POLY_SIZE;
  if (n > 0)
    CurrSgDevP->drawMarkers2D(x + i * SG_POLY_SIZE, n);
}

void
SgDrawLine2D(float x0[2], float x1[2])
{
  CurrSgDevP->drawLine2D(x0, x1);
}

void
SgDrawArrow2D(float x0[2], float x1[2])
{
  CurrSgDevP->drawArrow2D(x0, x1);
}

void
SgDrawPolyline2D(float x[][2], int n)
{
  int i;

  for (i = 0; i < (n - 1) / (SG_POLY_SIZE - 1); i++)
    CurrSgDevP->drawPolyline2D(x + i * (SG_POLY_SIZE - 1), SG_POLY_SIZE);
  
  n -= i * (SG_POLY_SIZE - 1);
  if (n > 0)
    CurrSgDevP->drawPolyline2D(x + i * (SG_POLY_SIZE - 1), n);
}

void
SgDrawPolygon2D(float x[][2], int n)
{
  if (n > SG_POLY_SIZE - 1)
    n = SG_POLY_SIZE - 1;

  CurrSgDevP->drawPolygon2D(x, n);
}

void
SgDrawShadedPolygon2D(float x[][2], float col[][3], int n)
{
  if (n > SG_POLY_SIZE - 1)
    n = SG_POLY_SIZE - 1;

  CurrSgDevP->drawShadedPolygon2D(x, col, n);
}

BOOL
SgHas2DShading(void)
{
  return CurrSgDevP->drawShadedPolygon2D != NULL;
}

void
SgDrawCircle2D(float x[2], float r)
{
  CurrSgDevP->drawCircle2D(x, r);
}

void
SgDrawDisc2D(float x[2], float r)
{
  CurrSgDevP->drawDisc2D(x, r);
}

void
SgDrawMarkers(float x[][3], int n)
{
  int i;

  for (i = 0; i < n / SG_POLY_SIZE; i++)
    CurrSgDevP->drawMarkers(x + i * SG_POLY_SIZE, SG_POLY_SIZE);

  n -= i * SG_POLY_SIZE;
  if (n > 0)
    CurrSgDevP->drawMarkers(x + i * SG_POLY_SIZE, n);
}

void
SgDrawLine(float x0[3], float x1[3])
{
  CurrSgDevP->drawLine(x0, x1);
}

void
SgDrawArrow(float x0[3], float x1[3])
{
  CurrSgDevP->drawArrow(x0, x1);
}

void
SgDrawPolyline(float x[][3], int n)
{
  int i;

  for (i = 0; i < (n - 1) / (SG_POLY_SIZE - 1); i++)
    CurrSgDevP->drawPolyline(x + i * (SG_POLY_SIZE - 1), SG_POLY_SIZE);
  
  n -= i * (SG_POLY_SIZE - 1);
  if (n > 0)
    CurrSgDevP->drawPolyline(x + i * (SG_POLY_SIZE - 1), n);
}

void
SgDrawPolygon(float x[][3], int n, float nv[3])
{
  if (n > SG_POLY_SIZE - 1)
    n = SG_POLY_SIZE - 1;

  if (CurrShadeModel == SG_SHADE_DOTS) {
    CurrSgDevP->drawMarkers(x, n);
  } else if (CurrShadeModel == SG_SHADE_LINES) {
    CurrSgDevP->drawPolyline(x, n);
    CurrSgDevP->drawLine(x[n - 1], x[0]);
  } else {
    CurrSgDevP->drawPolygon(x, n, nv);
  }
}

void
SgDrawCircle(float x[3], float v[3], float r)
{
  CurrSgDevP->drawCircle(x, v, r);
}

void
SgDrawDisc(float x[3], float v[3], float r)
{
  CurrSgDevP->drawDisc(x, v, r);
}

void
SgDrawSphere(float x[3], float r)
{
  CurrSgDevP->drawSphere(x, r);
}

void
SgDrawCone(float x0[3], float x1[3], float r1, float r2,
    SgConeEnd end0, SgConeEnd end1)
{
  CurrSgDevP->drawCone(x0, x1, r1, r2, end0, end1);
}

void
SgStartSurface(SgSurfaceType type)
{
  CurrSgDevP->startSurface(type);
}

void
SgDrawTriMesh(float x[][3], float nv[][3], int n)
{
  int i;

  if (n < 3)
    return;

  if (CurrShadeModel == SG_SHADE_DOTS) {
    /* draw only half of the points to avoid drawing every
       point twice when meshes are side by side */
    for (i = 0; i < n; i += 2)
      CurrSgDevP->drawMarkers(x + i, 1);
  } else if (CurrShadeModel == SG_SHADE_LINES) {
    CurrSgDevP->drawPolyline(x, n);
    for (i = 2; i < n; i += 2)
      CurrSgDevP->drawLine(x[i - 2], x[i]);
  } else {
    CurrSgDevP->drawTriMesh(x, nv, n);
  }
}

void
SgDrawColorTriMesh(float x[][3], float nv[][3], float col[][3], int n)
{
  float r, g, b;
  int i;

  if (n < 3)
    return;

  /* devices may change current color, reset it at the end so
     they don't have to care about it */
  SgGetColor(&r, &g, &b);

  if (CurrShadeModel == SG_SHADE_DOTS) {
    for (i = 0; i < n; i += 2) {
      SgSetColor(col[i][0], col[i][1], col[i][2]);
      CurrSgDevP->drawMarkers(x + i, 1);
    }
  } else if (CurrShadeModel == SG_SHADE_LINES) {
    for (i = 0; i < n - 2; i += 2) {
      SgSetColor(col[i][0], col[i][1], col[i][2]);
      CurrSgDevP->drawLine(x[i], x[i + 1]);
      SgSetColor(col[i + 1][0], col[i + 1][1], col[i + 1][2]);
      CurrSgDevP->drawLine(x[i + 1], x[i + 2]);
      SgSetColor(col[i + 2][0], col[i + 2][1], col[i + 2][2]);
      CurrSgDevP->drawLine(x[i], x[i + 2]);
    }
  } else {
    CurrSgDevP->drawColorTriMesh(x, nv, col, n);
  }

  SgSetColor(r, g, b);
  SgUpdateColor();
}

void
SgDrawStrip(float x1[][3], float x2[][3],
    float nv1[][3], float nv2[][3], int n)
{
  int i;

  if (n < 2)
    return;

  if (CurrShadeModel == SG_SHADE_DOTS) {
    CurrSgDevP->drawMarkers(x1, n);
  } else if (CurrShadeModel == SG_SHADE_LINES) {
    CurrSgDevP->drawPolyline(x1, n);
    for (i = 0; i < n; i++)
      CurrSgDevP->drawLine(x1[i], x2[i]);
  } else {
    CurrSgDevP->drawStrip(x1, x2, nv1, nv2, n);
  }
}

void
SgDrawColorStrip(float x1[][3], float x2[][3],
    float nv1[][3], float nv2[][3],
    float col1[][3], float col2[][3], int n)
{
  float r, g, b;
  int i;

  if (n < 2)
    return;

  SgGetColor(&r, &g, &b);

  if (CurrShadeModel == SG_SHADE_DOTS) {
    for (i = 0; i < n; i++) {
      SgSetColor(col1[i][0], col1[i][1], col1[i][2]);
      CurrSgDevP->drawMarkers(x1 + i, 1);
    }
  } else if (CurrShadeModel == SG_SHADE_LINES) {
    for (i = 0; i < n - 1; i++) {
      SgSetColor(col1[i][0], col1[i][1], col1[i][2]);
      CurrSgDevP->drawLine(x1[i], x1[i + 1]);
      CurrSgDevP->drawLine(x1[i], x2[i]);
    }
    SgSetColor(col1[n - 1][0], col1[n - 1][1], col1[n - 1][2]);
    CurrSgDevP->drawLine(x1[n - 1], x2[n - 1]);
  } else {
    CurrSgDevP->drawColorStrip(x1, x2, nv1, nv2, col1, col2, n);
  }

  SgSetColor(r, g, b);
  SgUpdateColor();
}

void
SgEndSurface(void)
{
  CurrSgDevP->endSurface();
}
