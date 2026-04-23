/*
************************************************************************
*
*   RIBPoly.c - RIB polygon drawing
*
*   Copyright (c) 1994-99
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/rib/SCCS/s.RIBPoly.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include "rib_poly.h"

#include <stdio.h>

#include <sg.h>
#include <sg_map.h>
#include <sg_plot.h>
#include "rib_util.h"
#include "rib_color.h"

static void
plotPoint2D(float x[2])
{
  float xm[3];

  RIBMap2D(xm, x);
  RIBPlotVect(xm);
}

static void
plotPoint(float x[3])
{
  float xm[3];

  SgMapModelPoint(xm, x);
  RIBPlotVect(xm);
}

static void
plotVect(float v[3])
{
  float vm[3];

  SgMapModelVect(vm, v);
  RIBPlotVect(vm);
}

void
SgRIBDrawPolygon2D(float x[][2], int n)
{
  int i;

  RIBUpdateMaterial();

  SgPlotOutputStr("Polygon \"P\" [");
  for (i = 0; i < n; i++) {
    plotPoint2D(x[i]);
    SgPlotOutputStr(" ");
  }
  SgPlotOutputStr("]\n");
}

void
SgRIBDrawPolygon(float x[][3], int n, float nv[3])
{
  int i;

  RIBUpdateMaterial();

  SgPlotOutputStr("Polygon \"P\" [");
  for (i = 0; i < n; i++) {
    plotPoint(x[i]);
    SgPlotOutputStr(" ");
  }
  SgPlotOutputStr("] \"Np\" [");
  plotVect(nv);
  SgPlotOutputStr("]\n");
}

void
SgRIBStartSurface(SgSurfaceType type)
{
}

static void
drawMesh(float x[][3], float nv[][3], float col[][3], int n)
{
  int i;

  RIBUpdateMaterial();

  SgPlotOutputStr("PointsPolygons [");
  for (i = 0; i < n - 3; i++)
    SgPlotOutputStr("3 ");
  SgPlotOutputStr("3] [");

  for (i = 0; i < n - 2; i += 2) {
    SgPlotOutputInt(i);
    SgPlotOutputStr(" ");
    SgPlotOutputInt(i + 1);
    SgPlotOutputStr(" ");
    SgPlotOutputInt(i + 2);
    SgPlotOutputStr(" ");

    if (i + 3 == n)
      break;

    SgPlotOutputInt(i + 2);
    SgPlotOutputStr(" ");
    SgPlotOutputInt(i + 1);
    SgPlotOutputStr(" ");
    SgPlotOutputInt(i + 3);
    SgPlotOutputStr(" ");
  }

  SgPlotOutputStr("] \"P\" [");
  for (i = 0; i < n; i++) {
    plotPoint(x[i]);
    SgPlotOutputStr(" ");
  }

  SgPlotOutputStr("] \"N\" [");
  for (i = 0; i < n; i++) {
    plotVect(nv[i]);
    SgPlotOutputStr(" ");
  }
  SgPlotOutputStr("]");

  if (col != NULL) {
    SgPlotOutputStr(" \"Cs\" [");
    for (i = 0; i < n; i++) {
      plotVect(col[i]);
      SgPlotOutputStr(" ");
    }
    SgPlotOutputStr("]");
  }

  SgPlotOutputStr("\n");
}

void
SgRIBDrawTriMesh(float x[][3], float nv[][3], int n)
{
  drawMesh(x, nv, NULL, n);
}

void
SgRIBDrawColorTriMesh(float x[][3], float nv[][3], float col[][3], int n)
{
  drawMesh(x, nv, col, n);
}

static void
drawStrip(float x1[][3], float x2[][3],
    float nv1[][3], float nv2[][3], float col[][3], int n)
{
  int i;

  RIBUpdateMaterial();

  SgPlotOutputStr("PointsPolygons [");
  for (i = 0; i < 2 * n - 3; i++)
    SgPlotOutputStr("3 ");
  SgPlotOutputStr("3] [");

  for (i = 0; i < n - 1; i++) {
    SgPlotOutputInt(2 * i);
    SgPlotOutputStr(" ");
    SgPlotOutputInt(2 * i + 1);
    SgPlotOutputStr(" ");
    SgPlotOutputInt(2 * i + 2);
    SgPlotOutputStr(" ");
    SgPlotOutputInt(2 * i + 2);
    SgPlotOutputStr(" ");
    SgPlotOutputInt(2 * i + 1);
    SgPlotOutputStr(" ");
    SgPlotOutputInt(2 * i + 3);
    SgPlotOutputStr(" ");
  }

  SgPlotOutputStr("] \"P\" [");
  for (i = 0; i < n; i++) {
    plotPoint(x1[i]);
    SgPlotOutputStr(" ");
    plotPoint(x2[i]);
    SgPlotOutputStr(" ");
  }

  SgPlotOutputStr("] \"N\" [");
  for (i = 0; i < n; i++) {
    plotVect(nv1[i]);
    SgPlotOutputStr(" ");
    plotVect(nv2[i]);
    SgPlotOutputStr(" ");
  }
  SgPlotOutputStr("]");

  if (col != NULL) {
    SgPlotOutputStr(" \"Cs\" [");
    for (i = 0; i < n; i++) {
      plotVect(col[i]);
      SgPlotOutputStr(" ");
      plotVect(col[i]);
      SgPlotOutputStr(" ");
    }
    SgPlotOutputStr("]");
  }

  SgPlotOutputStr("\n");
}

void
SgRIBDrawStrip(float x1[][3], float x2[][3],
    float nv1[][3], float nv2[][3], int n)
{
  drawStrip(x1, x2, nv1, nv2, NULL, n);
}

void
SgRIBDrawColorStrip(float x1[][3], float x2[][3],
    float nv1[][3], float nv2[][3],
    float col1[][3], float col2[][3], int n)
{
  drawStrip(x1, x2, nv1, nv2, col1, n);
}

void
SgRIBEndSurface(void)
{
}
