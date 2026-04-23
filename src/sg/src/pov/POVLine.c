/*
************************************************************************
*
*   POVLine.c - POV line drawing
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/pov/SCCS/s.POVLine.c
*   SCCS identification       : 1.7
*
************************************************************************
*/

#include "pov_line.h"

#include <math.h>

#include <mat_vec.h>
#include <sg_map.h>
#include <sg_plot.h>
#include "pov_util.h"
#include "pov_color.h"
#include "pov_clip.h"

static SgLineStyle CurrStyle;
static float CurrWidth;

static void
plotRect(float x0[3], float x1[3])
{
  float vx, vy, a;

  vx = x1[0] - x0[0];
  vy = x1[1] - x0[1];
  a = sqrtf(vx * vx + vy * vy);

  vx *= CurrWidth / a;
  vy *= CurrWidth / a;

  SgPlotOutputStr("triangle {\n");
  SgPlotOutputStr("<");
  SgPlotOutputFloat(x0[0] - vy);
  SgPlotOutputStr(", ");
  SgPlotOutputFloat(x0[1] + vx);
  SgPlotOutputStr(", ");
  SgPlotOutputFloat(x0[2]);
  SgPlotOutputStr(">, ");
  SgPlotOutputStr("<");
  SgPlotOutputFloat(x0[0] + vy);
  SgPlotOutputStr(", ");
  SgPlotOutputFloat(x0[1] - vx);
  SgPlotOutputStr(", ");
  SgPlotOutputFloat(x0[2]);
  SgPlotOutputStr(">, ");
  SgPlotOutputStr("<");
  SgPlotOutputFloat(x1[0] + vy);
  SgPlotOutputStr(", ");
  SgPlotOutputFloat(x1[1] - vx);
  SgPlotOutputStr(", ");
  SgPlotOutputFloat(x1[2]);
  SgPlotOutputStr(">\n");
  POVPlotColor();
  (void) (POVClip(x0, CurrWidth) && POVClip(x1, CurrWidth));
  SgPlotOutputStr("}\n");

  SgPlotOutputStr("triangle {\n");
  SgPlotOutputStr("<");
  SgPlotOutputFloat(x0[0] - vy);
  SgPlotOutputStr(", ");
  SgPlotOutputFloat(x0[1] + vx);
  SgPlotOutputStr(", ");
  SgPlotOutputFloat(x0[2]);
  SgPlotOutputStr(">, ");
  SgPlotOutputStr("<");
  SgPlotOutputFloat(x1[0] + vy);
  SgPlotOutputStr(", ");
  SgPlotOutputFloat(x1[1] - vx);
  SgPlotOutputStr(", ");
  SgPlotOutputFloat(x1[2]);
  SgPlotOutputStr(">, ");
  SgPlotOutputStr("<");
  SgPlotOutputFloat(x1[0] - vy);
  SgPlotOutputStr(", ");
  SgPlotOutputFloat(x1[1] + vx);
  SgPlotOutputStr(", ");
  SgPlotOutputFloat(x1[2]);
  SgPlotOutputStr(">\n");
  POVPlotColor();
  (void) (POVClip(x0, CurrWidth) && POVClip(x1, CurrWidth));
  SgPlotOutputStr("}\n");
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
    steps = (int) (len / 0.05f);
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
  SgPlotOutputStr("disc {\n");
  POVPlotVect(x);
  SgPlotOutputStr(", z, ");
  SgPlotOutputFloat(CurrWidth);
  SgPlotOutputStr("\n");
  POVPlotColor();
  (void) POVClip(x, CurrWidth);
  SgPlotOutputStr("}\n");
}

void
SgPOVDrawLine2D(float x0[2], float x1[2])
{
  float x0m[3], x1m[3];

  POVUpdateColor();

  POVMap2D(x0m, x0);
  POVMap2D(x1m, x1);

  plotLine(x0m, x1m);
  plotCirc(x0m);
  plotCirc(x1m);
}

void
SgPOVDrawPolyline2D(float x[][2], int n)
{
  float x0m[3], x1m[3];
  int i;

  POVUpdateColor();

  POVMap2D(x0m, x[0]);

  plotCirc(x0m);

  for (i = 1; i < n; i++) {
    POVMap2D(x1m, x[i]);

    plotLine(x0m, x1m);
    plotCirc(x1m);

    Vec3Copy(x0m, x1m);
  }
}

void
SgPOVDrawLine(float x0[3], float x1[3])
{
  float x0m[3], x1m[3];

  POVUpdateColor();

  SgMapModelPoint(x0m, x0);
  SgMapModelPoint(x1m, x1);

  plotLine(x0m, x1m);
  plotCirc(x0m);
  plotCirc(x1m);
}

void
SgPOVDrawPolyline(float x[][3], int n)
{
  float x0m[3], x1m[3];
  int i;

  POVUpdateColor();

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
SgPOVSetLineWidth(float w)
{
  if (w == 0.0f)
    w = 1.0;

  CurrWidth = 0.005f * w;
}

void
SgPOVSetLineStyle(SgLineStyle lineStyle)
{
  CurrStyle = lineStyle;
}
