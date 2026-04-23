/*
************************************************************************
*
*   DefLine.c - default line drawing for Sg
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
*   Date of last modification : 96/03/05
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/def/SCCS/s.DefLine.c
*   SCCS identification       : 1.6
*
************************************************************************
*/

#include <sg_def.h>

#include <sg.h>
#include <sg_get.h>
#include <sg_map.h>
#include "def_hidden.h"
#include "def_shade.h"

void
SgDefDrawArrow2D(float x0[2], float x1[2])
{
  SgDrawLine2D(x0, x1);
}

void
SgDefDrawLine(float x0[3], float x1[3])
{
  float x0m[3], x1m[3], xEye[3];
  BOOL hidden, fog;
  float r, g, b, fr, fg, fb;

  if (SgMapLine(x0m, x1m, x0, x1) == 0)
    return;

  hidden = SgGetFeature(SG_FEATURE_DEPTH_SORT) ||
      SgGetFeature(SG_FEATURE_HIDDEN);
  fog = SgGetFeature(SG_FEATURE_SHADE) && DefFogEnabled();

  if (fog) {
    SgGetColor(&r, &g, &b);
    fr = r;
    fg = g;
    fb = b;
    SgMapEyePoint(xEye, x0);
    DefFog(&fr, &fg, &fb, xEye[2]);
  } else if (hidden) {
    SgGetColor(&fr, &fg, &fb);
  }

  if (hidden) {
    DefHiddenLine(x0m, x1m, fr, fg, fb);
  } else if (fog) {
    SgSetColor(fr, fg, fb);
    SgDrawLine2D(x0m, x1m);
    SgSetColor(r, g, b);
  } else {
    SgDrawLine2D(x0m, x1m);
  }
}

void
SgDefDrawArrow(float x0[3], float x1[3])
{
  SgDrawLine(x0, x1);
}

void
SgDefDrawPolyline(float x[][3], int n)
{
  float xm[SG_POLY_SIZE][3], xEye[3];
  BOOL hidden, fog;
  float xm2[SG_POLY_SIZE][2];
  float r, g, b, fr, fg, fb;
  int i;

  n = SgMapPoly(xm, x, n);
  if (n == 0)
    return;

  hidden = SgGetFeature(SG_FEATURE_DEPTH_SORT) ||
      SgGetFeature(SG_FEATURE_HIDDEN);
  fog = SgGetFeature(SG_FEATURE_SHADE) && DefFogEnabled();

  if (fog) {
    SgGetColor(&r, &g, &b);
    fr = r;
    fg = g;
    fb = b;
    SgMapEyePoint(xEye, x[0]);
    DefFog(&fr, &fg, &fb, xEye[2]);
  } else if (hidden) {
    SgGetColor(&fr, &fg, &fb);
  }

  if (hidden) {
    DefHiddenPolyline(xm, n, fr, fg, fb);
    return;
  }

  for (i = 0; i < n; i++) {
    xm2[i][0] = xm[i][0];
    xm2[i][1] = xm[i][1];
  }

  if (fog) {
    SgSetColor(fr, fg, fb);
    SgDrawPolyline2D(xm2, n);
    SgSetColor(r, g, b);
  } else {
    SgDrawPolyline2D(xm2, n);
  }
}

void
SgDefSetLineWidth(float w)
{
}

void
SgDefSetLineStyle(SgLineStyle style)
{
}
