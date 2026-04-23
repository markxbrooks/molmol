/*
************************************************************************
*
*   VRML2Poly.c - VRML2 polygon drawing
*
*   Copyright (c) 1996-99
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
*   Date of last modification : 99/10/31
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/sg/src/vrml2/SCCS/s.VRML2Poly.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include "vrml2_poly.h"

#include <stdio.h>

#include <sg.h>
#include <sg_map.h>
#include <sg_map.h>
#include <sg_plot.h>
#include <sg_get.h>
#include "vrml2_util.h"
#include "vrml2_color.h"
#include "vrml2_light.h"

void
SgVRML2DrawPolygon2D(float x[][2], int n)
{
  float xm[3];
  int i;

  SgPlotOutputStr("Shape {\n");
  VRML2PlotColor();

  SgPlotOutputStr("geometry IndexedFaceSet {\n");
  SgPlotOutputStr("coord Coordinate {\n");
  SgPlotOutputStr("point [\n");

  for (i = 0; ; i++) {
    VRML2Map2D(xm, x[i]);
    VRML2PlotVect(xm);
    if (i == n - 1)
      break;
    SgPlotOutputStr(",\n");
  }
  SgPlotOutputStr("\n");

  SgPlotOutputStr("]\n");
  SgPlotOutputStr("}\n");

  SgPlotOutputStr("normalPerVertex FALSE\n");
  SgPlotOutputStr("normal Normal {vector [0 0 1]}\n");

  SgPlotOutputStr("coordIndex [");
  for (i = 0; i < n; i++) {
    SgPlotOutputInt(i);
    SgPlotOutputStr(" ");
  }
  SgPlotOutputStr("]\n");

  if (SgGetFeature(SG_FEATURE_BACKFACE))
    SgPlotOutputStr("solid FALSE\n");

  SgPlotOutputStr("}\n");

  SgPlotOutputStr("}\n");
}

void
SgVRML2DrawPolygon(float x[][3], int n, float nv[3])
{
  float xm[3], nvm[3];
  int i;

  VRML2UpdateLight();
  SgPlotOutputStr("Shape {\n");
  VRML2PlotColor();

  SgPlotOutputStr("geometry IndexedFaceSet {\n");
  SgPlotOutputStr("coord Coordinate {\n");
  SgPlotOutputStr("point [\n");

  for (i = 0; ; i++) {
    SgMapModelPoint(xm, x[i]);
    VRML2PlotVect(xm);
    if (i == n - 1)
      break;
    SgPlotOutputStr(",\n");
  }
  SgPlotOutputStr("\n");

  SgPlotOutputStr("]\n");
  SgPlotOutputStr("}\n");

  SgPlotOutputStr("normalPerVertex FALSE\n");
  SgPlotOutputStr("normal Normal {vector [\n");
  SgMapModelVect(nvm, nv);
  VRML2PlotVect(nvm);
  SgPlotOutputStr("]}\n");

  SgPlotOutputStr("coordIndex [");
  for (i = 0; i < n; i++) {
    SgPlotOutputInt(i);
    SgPlotOutputStr(" ");
  }
  SgPlotOutputStr("]\n");

  if (SgGetFeature(SG_FEATURE_BACKFACE))
    SgPlotOutputStr("solid FALSE\n");

  SgPlotOutputStr("}\n");

  SgPlotOutputStr("}\n");
}

static void
plotIndices(int n)
{
  int i;

  for (i = 0; i < n - 2; i += 2) {
    SgPlotOutputInt(i);
    SgPlotOutputStr(" ");
    SgPlotOutputInt(i + 1);
    SgPlotOutputStr(" ");
    SgPlotOutputInt(i + 2);
    SgPlotOutputStr(" -1\n");

    if (i + 3 == n)
      break;

    SgPlotOutputInt(i + 2);
    SgPlotOutputStr(" ");
    SgPlotOutputInt(i + 1);
    SgPlotOutputStr(" ");
    SgPlotOutputInt(i + 3);
    SgPlotOutputStr(" -1\n");
  }
}

static void
drawMesh(float x[][3], float nv[][3], float col[][3], int n)
{
  float xm[3], nvm[3];
  int i;

  VRML2UpdateLight();
  SgPlotOutputStr("Shape {\n");
  VRML2PlotColor();

  SgPlotOutputStr("geometry IndexedFaceSet {\n");
  SgPlotOutputStr("coord Coordinate {\n");
  SgPlotOutputStr("point [\n");

  for (i = 0; ; i++) {
    SgMapModelPoint(xm, x[i]);
    VRML2PlotVect(xm);
    if (i == n - 1)
      break;
    SgPlotOutputStr(",\n");
  }
  SgPlotOutputStr("\n");

  SgPlotOutputStr("]\n");
  SgPlotOutputStr("}\n");

  SgPlotOutputStr("normal Normal {\n");
  SgPlotOutputStr("vector [\n");

  for (i = 0; ; i++) {
    SgMapModelVect(nvm, nv[i]);
    VRML2PlotVect(nvm);
    if (i == n - 1)
      break;
    SgPlotOutputStr(",\n");
  }
  SgPlotOutputStr("\n");

  SgPlotOutputStr("]\n");
  SgPlotOutputStr("}\n");

  if (col != NULL) {
    SgPlotOutputStr("color Color {\n");
    SgPlotOutputStr("color [\n");

    for (i = 0; ; i++) {
      VRML2PlotVect(col[i]);
      if (i == n - 1)
        break;
      SgPlotOutputStr(",\n");
    }
    SgPlotOutputStr("\n");

    SgPlotOutputStr("]\n");
    SgPlotOutputStr("}\n");
  }

  SgPlotOutputStr("coordIndex [\n");
  plotIndices(n);
  SgPlotOutputStr("]\n");

  if (SgGetFeature(SG_FEATURE_BACKFACE))
    SgPlotOutputStr("solid FALSE\n");

  SgPlotOutputStr("}\n");

  SgPlotOutputStr("}\n");
}

void
SgVRML2DrawTriMesh(float x[][3], float nv[][3], int n)
{
  drawMesh(x, nv, NULL, n);
}

void
SgVRML2DrawColorTriMesh(float x[][3], float nv[][3], float col[][3], int n)
{
  drawMesh(x, nv, col, n);
}

static void
drawStrip(float x1[][3], float x2[][3],
    float nv1[][3], float nv2[][3],
    float col1[][3], float col2[][3], int n)
{
  float xm[3], nvm[3];
  int i;

  if (nv2 == NULL)
    nv2 = nv1;

  VRML2UpdateLight();
  SgPlotOutputStr("Shape {\n");
  VRML2PlotColor();

  SgPlotOutputStr("geometry IndexedFaceSet {\n");
  SgPlotOutputStr("coord Coordinate {\n");
  SgPlotOutputStr("point [\n");

  for (i = 0; ; i++) {
    SgMapModelPoint(xm, x1[i]);
    VRML2PlotVect(xm);
    SgPlotOutputStr(",\n");
    SgMapModelPoint(xm, x2[i]);
    VRML2PlotVect(xm);
    if (i == n - 1)
      break;
    SgPlotOutputStr(",\n");
  }
  SgPlotOutputStr("\n");

  SgPlotOutputStr("]\n");
  SgPlotOutputStr("}\n");

  SgPlotOutputStr("normal Normal {\n");
  SgPlotOutputStr("vector [\n");

  for (i = 0; ; i++) {
    SgMapModelVect(nvm, nv1[i]);
    VRML2PlotVect(nvm);
    SgPlotOutputStr(",\n");
    SgMapModelVect(nvm, nv2[i]);
    VRML2PlotVect(nvm);
    if (i == n - 1)
      break;
    SgPlotOutputStr(",\n");
  }
  SgPlotOutputStr("\n");

  SgPlotOutputStr("]\n");
  SgPlotOutputStr("}\n");

  if (col1 != NULL) {
    SgPlotOutputStr("color Color {\n");
    SgPlotOutputStr("color [\n");

    for (i = 0; ; i++) {
      VRML2PlotVect(col1[i]);
      SgPlotOutputStr(",\n");
      if (col2 != NULL)
        VRML2PlotVect(col2[i]);
      else
        VRML2PlotVect(col1[i]);
      if (i == n - 1)
	break;
      SgPlotOutputStr(",\n");
    }
    SgPlotOutputStr("\n");

    SgPlotOutputStr("]\n");
    SgPlotOutputStr("}\n");
  }

  SgPlotOutputStr("coordIndex [\n");
  plotIndices(2 * n);
  SgPlotOutputStr("]\n");

  if (SgGetFeature(SG_FEATURE_BACKFACE))
    SgPlotOutputStr("solid FALSE\n");

  SgPlotOutputStr("}\n");

  SgPlotOutputStr("}\n");
}

void
SgVRML2DrawStrip(float x1[][3], float x2[][3],
    float nv1[][3], float nv2[][3], int n)
{
  drawStrip(x1, x2, nv1, nv2, NULL, NULL, n);
}

void
SgVRML2DrawColorStrip(float x1[][3], float x2[][3],
    float nv1[][3], float nv2[][3],
    float col1[][3], float col2[][3], int n)
{
  drawStrip(x1, x2, nv1, nv2, col1, col2, n);
}
