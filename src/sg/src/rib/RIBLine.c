/*
************************************************************************
*
*   RIBLine.c - RIB line drawing
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/rib/SCCS/s.RIBLine.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include "rib_line.h"

#include <math.h>

#include <mat_vec.h>
#include <sg_map.h>
#include <sg_plot.h>
#include "rib_util.h"
#include "rib_color.h"

static SgLineStyle CurrStyle;
static float CurrWidth;

static void
plotVect(float x, float y, float z)
{
  float v[3];

  v[0] = x;
  v[1] = y;
  v[2] = z;

  RIBPlotVect(v);
}

static void
plotRect(float x0[3], float x1[3])
{
  float vx, vy, a;

  vx = x1[0] - x0[0];
  vy = x1[1] - x0[1];
  a = sqrtf(vx * vx + vy * vy);

  vx *= CurrWidth / a;
  vy *= CurrWidth / a;

  SgPlotOutputStr("Polygon \"P\" [");
  plotVect(x0[0] - vy, x0[1] + vx, x0[2]);
  SgPlotOutputStr(" ");
  plotVect(x0[0] + vy, x0[1] - vx, x0[2]);
  SgPlotOutputStr(" ");
  plotVect(x1[0] + vy, x1[1] - vx, x1[2]);
  SgPlotOutputStr(" ");
  plotVect(x1[0] - vy, x1[1] + vx, x1[2]);
  SgPlotOutputStr("]\n");
}

static void
plotLine(float x0[3], float x1[3])
{
  float v[3], len;
  float xp0[3], xp1[3];
  int steps, i;

  if (CurrStyle == SG_LINE_SOLID) {
    plotRect(x0, x1);
  } else {
    Vec3Copy(v, x1);
    Vec3Sub(v, x0);
    len = Vec3Abs(v);
    steps = (int) (len / 0.2f);
    Vec3Scale(v, 1.0f / (2 * steps + 1));

    Vec3Copy(xp0, x0);
    for (i = 0; i <= steps; i++) {
      Vec3Copy(xp1, xp0);
      Vec3Add(xp1, v);
      plotRect(xp0, xp1);

      Vec3Copy(xp0, xp1);
      Vec3Add(xp0, v);
    }
  }
}

static void
plotCirc(float x[3])
{
  SgPlotOutputStr("Translate ");
  RIBPlotVect(x);
  SgPlotOutputStr("\n");
  SgPlotOutputStr("Disk 0.0 ");
  SgPlotOutputFloat(CurrWidth);
  SgPlotOutputStr(" 360\n");
  SgPlotOutputStr("Identity\n");
}

void
SgRIBDrawLine2D(float x0[2], float x1[2])
{
  float x0m[3], x1m[3];

  RIBUpdateMaterial();

  RIBMap2D(x0m, x0);
  RIBMap2D(x1m, x1);

  plotLine(x0m, x1m);
  plotCirc(x0m);
  plotCirc(x1m);
}

void
SgRIBDrawPolyline2D(float x[][2], int n)
{
  float x0m[3], x1m[3];
  int i;

  RIBUpdateMaterial();

  RIBMap2D(x0m, x[0]);

  plotCirc(x0m);

  for (i = 1; i < n; i++) {
    RIBMap2D(x1m, x[i]);

    plotLine(x0m, x1m);
    plotCirc(x1m);

    Vec3Copy(x0m, x1m);
  }
}

void
SgRIBDrawLine(float x0[3], float x1[3])
{
  float x0m[3], x1m[3];

  RIBUpdateMaterial();

  SgMapModelPoint(x0m, x0);
  SgMapModelPoint(x1m, x1);

  plotLine(x0m, x1m);
  plotCirc(x0m);
  plotCirc(x1m);
}

void
SgRIBDrawPolyline(float x[][3], int n)
{
  float x0m[3], x1m[3];
  int i;

  RIBUpdateMaterial();

  SgMapModelPoint(x0m, x[0]);

  plotCirc(x0m);

  for (i = 1; i < n; i++) {
    SgMapModelPoint(x1m, x[i]);

    plotLine(x0m, x1m);
    plotCirc(x1m);

    Vec3Copy(x0m, x1m);
  }
}

void
SgRIBSetLineWidth(float w)
{
  if (w == 0.0f)
    w = 1.0f;

  CurrWidth = 0.005f * w;
}

void
SgRIBSetLineStyle(SgLineStyle lineStyle)
{
  CurrStyle = lineStyle;
}
