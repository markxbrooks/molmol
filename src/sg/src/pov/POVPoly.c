/*
************************************************************************
*
*   POVPoly.c - POV polygon drawing
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/pov/SCCS/s.POVPoly.c
*   SCCS identification       : 1.13
*
************************************************************************
*/

#include "pov_poly.h"

#include <stdio.h>

#include <sg.h>
#include <sg_map.h>
#include <sg_plot.h>
#include "pov_util.h"
#include "pov_color.h"
#include "pov_clip.h"

static BOOL InMesh = FALSE, NewMesh, ClipMesh;

static void
plotPoint2D(float x[2])
{
  float xm[3];

  POVMap2D(xm, x);
  POVPlotVect(xm);
}

static void
plotPoint(float xm[3], float x[3])
{
  SgMapModelPoint(xm, x);
  POVPlotVect(xm);
}

static void
plotVect(float v[3])
{
  float vm[3];

  SgMapModelVect(vm, v);
  POVPlotVect(vm);
}

void
SgPOVDrawPolygon2D(float x[][2], int n)
{
  int i;

  POVUpdateColor();

  if (n > 3)
    SgPlotOutputStr("mesh {\n");

  /* split into triangles, simple because all polygons must be convex */
  for (i = 1; i < n - 1; i++) {
    SgPlotOutputStr("triangle {\n");
    plotPoint2D(x[0]);
    SgPlotOutputStr(", ");
    plotPoint2D(x[i]);
    SgPlotOutputStr(", ");
    plotPoint2D(x[i + 1]);
    SgPlotOutputStr("\n");
    if (n <= 3)
      POVPlotColor();
    SgPlotOutputStr("}\n");
  }

  if (n > 3) {
    POVPlotColor();
    SgPlotOutputStr("}\n");
  }
}

void
SgPOVDrawPolygon(float x[][3], int n, float nv[3])
{
  float x1m[3], x2m[3], x3m[3];
  BOOL clip;
  int i;

  POVUpdateColor();

  if (n > 3)
    SgPlotOutputStr("mesh {\n");

  for (i = 1; i < n - 1; i++) {
    SgPlotOutputStr("triangle {\n");
    plotPoint(x1m, x[0]);
    if (n > 3 && i == 1)
      clip = ! POVClipTest(x1m, 0.0f);
    SgPlotOutputStr(", ");
    plotPoint(x2m, x[i]);
    SgPlotOutputStr(", ");
    plotPoint(x3m, x[i + 1]);
    if (! clip && n > 3 && i == n - 2)
      clip = ! POVClipTest(x3m, 0.0f);
    SgPlotOutputStr("\n");
    if (n > 3) {
      if (! (clip || POVClipTest(x2m, 0.0f)))
	clip = TRUE;
    } else {
      POVPlotColor();
      (void) (POVClip(x1m, 0.0f) && POVClip(x2m, 0.0f) && POVClip(x3m, 0.0f));
    }
    SgPlotOutputStr("}\n");
  }

  if (n > 3) {
    POVPlotColor();
    if (clip)
      POVClipOutput();
    SgPlotOutputStr("}\n");
  }
}

void
SgPOVStartSurface(SgSurfaceType type)
{
  if (type != SG_SURF_UNIFORM)
    return;

  InMesh = TRUE;
  /* delay output of "mesh" until texture definition is updated */
  NewMesh = TRUE;
  ClipMesh = FALSE;
}

static void
drawMesh(float x[][3], float nv[][3], float col[][3], int n)
{
  float x1m[3], x2m[3], x3m[3];
  int i;

  if (col == NULL)
    POVUpdateColor();

  if (NewMesh) {
    SgPlotOutputStr("mesh {\n");
    NewMesh = FALSE;
  }

  if (InMesh)
    for (i = 0; i < n; i++) {
      if (ClipMesh)
	break;
      SgMapModelPoint(x1m, x[i]);
      ClipMesh = ! POVClipTest(x1m, 0.0f);
    }

  for (i = 0; i < n - 2; i += 2) {
    if (col != NULL) {
      SgSetColor(col[i + 2][0], col[i + 2][1], col[i + 2][2]);
      POVUpdateColor();
    }
    SgPlotOutputStr("smooth_triangle {\n");
    plotPoint(x1m, x[i]);
    SgPlotOutputStr(", ");
    plotVect(nv[i]);
    SgPlotOutputStr(", ");
    plotPoint(x2m, x[i + 1]);
    SgPlotOutputStr(", ");
    plotVect(nv[i + 1]);
    SgPlotOutputStr(", ");
    plotPoint(x3m, x[i + 2]);
    SgPlotOutputStr(", ");
    plotVect(nv[i + 2]);
    SgPlotOutputStr("\n");
    if (! InMesh) {
      POVPlotColor();
      (void) (POVClip(x1m, 0.0f) && POVClip(x2m, 0.0f) && POVClip(x3m, 0.0f));
    }
    SgPlotOutputStr("}\n");

    if (i + 3 == n)
      break;

    if (col != NULL) {
      SgSetColor(col[i + 3][0], col[i + 3][1], col[i + 3][2]);
      POVUpdateColor();
    }
    SgPlotOutputStr("smooth_triangle {\n");
    plotPoint(x1m, x[i + 2]);
    SgPlotOutputStr(", ");
    plotVect(nv[i + 2]);
    SgPlotOutputStr(", ");
    plotPoint(x2m, x[i + 1]);
    SgPlotOutputStr(", ");
    plotVect(nv[i + 1]);
    SgPlotOutputStr(", ");
    plotPoint(x3m, x[i + 3]);
    SgPlotOutputStr(", ");
    plotVect(nv[i + 3]);
    SgPlotOutputStr("\n");
    if (! InMesh) {
      POVPlotColor();
      (void) (POVClip(x1m, 0.0f) && POVClip(x2m, 0.0f) && POVClip(x3m, 0.0f));
    }
    SgPlotOutputStr("}\n");
  }
}

void
SgPOVDrawTriMesh(float x[][3], float nv[][3], int n)
{
  drawMesh(x, nv, NULL, n);
}

void
SgPOVDrawColorTriMesh(float x[][3], float nv[][3], float col[][3], int n)
{
  drawMesh(x, nv, col, n);
}

static void
drawStrip(float x1[][3], float x2[][3],
    float nv1[][3], float nv2[][3], float col[][3], int n)
{
  float x1m[3], x2m[3], x3m[3];
  int i;

  if (col == NULL)
    POVUpdateColor();
  
  if (NewMesh) {
    SgPlotOutputStr("mesh {\n");
    NewMesh = FALSE;
  }

  if (InMesh)
    for (i = 0; i < n; i++) {
      if (ClipMesh)
	break;
      SgMapModelPoint(x1m, x1[i]);
      SgMapModelPoint(x2m, x2[i]);
      ClipMesh = ! (POVClipTest(x1m, 0.0f) && POVClipTest(x2m, 0.0f));
    }

  if (nv2 == NULL)
    nv2 = nv1;

  for (i = 0; i < n - 1; i++) {
    if (col != NULL) {
      SgSetColor(col[i][0], col[i][1], col[i][2]);
      POVUpdateColor();
    }

    SgPlotOutputStr("smooth_triangle {\n");
    plotPoint(x1m, x1[i]);
    SgPlotOutputStr(", ");
    plotVect(nv1[i]);
    SgPlotOutputStr(", ");
    plotPoint(x2m, x2[i]);
    SgPlotOutputStr(", ");
    plotVect(nv2[i]);
    SgPlotOutputStr(", ");
    plotPoint(x3m, x1[i + 1]);
    SgPlotOutputStr(", ");
    plotVect(nv1[i + 1]);
    SgPlotOutputStr("\n");
    if (! InMesh) {
      POVPlotColor();
      (void) (POVClip(x1m, 0.0f) && POVClip(x2m, 0.0f) && POVClip(x3m, 0.0f));
    }
    SgPlotOutputStr("}\n");

    SgPlotOutputStr("smooth_triangle {\n");
    plotPoint(x1m, x2[i]);
    SgPlotOutputStr(", ");
    plotVect(nv2[i]);
    SgPlotOutputStr(", ");
    plotPoint(x2m, x2[i + 1]);
    SgPlotOutputStr(", ");
    plotVect(nv2[i + 1]);
    SgPlotOutputStr(", ");
    plotPoint(x3m, x1[i + 1]);
    SgPlotOutputStr(", ");
    plotVect(nv1[i + 1]);
    SgPlotOutputStr("\n");
    if (! InMesh) {
      POVPlotColor();
      (void) (POVClip(x1m, 0.0f) && POVClip(x2m, 0.0f) && POVClip(x3m, 0.0f));
    }
    SgPlotOutputStr("}\n");
  }
}

void
SgPOVDrawStrip(float x1[][3], float x2[][3],
    float nv1[][3], float nv2[][3], int n)
{
  drawStrip(x1, x2, nv1, nv2, NULL, n);
}

void
SgPOVDrawColorStrip(float x1[][3], float x2[][3],
    float nv1[][3], float nv2[][3],
    float col1[][3], float col2[][3], int n)
{
  drawStrip(x1, x2, nv1, nv2, col1, n);
}

void
SgPOVEndSurface(void)
{
  if (! InMesh)
    return;

  POVPlotColor();
  if (ClipMesh)
    POVClipOutput();

  SgPlotOutputStr("}\n");
  InMesh = FALSE;
}
