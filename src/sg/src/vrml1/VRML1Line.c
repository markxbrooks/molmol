/*
************************************************************************
*
*   VRML1Line.c - VRML1 line drawing
*
*   Copyright (c) 1996
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
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/sg/src/vrml1/SCCS/s.VRML1Line.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include "vrml1_line.h"

#include <sg_map.h>
#include <sg_plot.h>
#include "vrml1_util.h"
#include "vrml1_color.h"

static void
plotLine(float x0[3], float x1[3])
{
  SgPlotOutputStr("Separator {\n");
  VRML1PlotColor();

  SgPlotOutputStr("Coordinate3 {\n");
  SgPlotOutputStr("point [");
  VRML1PlotVect(x0);
  SgPlotOutputStr(", ");
  VRML1PlotVect(x1);
  SgPlotOutputStr("]\n");
  SgPlotOutputStr("}\n");

  SgPlotOutputStr("IndexedLineSet {\n");
  SgPlotOutputStr("coordIndex [0, 1]\n");
  SgPlotOutputStr("}\n");

  SgPlotOutputStr("}\n");
}

void
SgVRML1DrawLine2D(float x0[2], float x1[2])
{
  float x0m[3], x1m[3];

  VRML1Map2D(x0m, x0);
  VRML1Map2D(x1m, x1);

  plotLine(x0m, x1m);
}

void
SgVRML1DrawPolyline2D(float x[][2], int n)
{
  float xm[3];
  int i;

  SgPlotOutputStr("Separator {\n");
  VRML1PlotColor();

  SgPlotOutputStr("Coordinate3 {\n");
  SgPlotOutputStr("point [\n");

  for (i = 0; i < n; i++) {
    VRML1Map2D(xm, x[i]);
    VRML1PlotVect(xm);
    SgPlotOutputStr(",\n");
  }

  SgPlotOutputStr("]\n");
  SgPlotOutputStr("}\n");

  SgPlotOutputStr("IndexedLineSet {\n");
  SgPlotOutputStr("coordIndex [");
  for (i = 0; i < n; i++) {
    SgPlotOutputInt(i);
    SgPlotOutputStr(", ");
  }
  SgPlotOutputStr("]\n");
  SgPlotOutputStr("}\n");

  SgPlotOutputStr("}\n");
}

void
SgVRML1DrawLine(float x0[3], float x1[3])
{
  float x0m[3], x1m[3];

  SgMapModelPoint(x0m, x0);
  SgMapModelPoint(x1m, x1);

  plotLine(x0m, x1m);
}

void
SgVRML1DrawPolyline(float x[][3], int n)
{
  float xm[3];
  int i;

  SgPlotOutputStr("Separator {\n");
  VRML1PlotColor();

  SgPlotOutputStr("Coordinate3 {\n");
  SgPlotOutputStr("point [\n");

  for (i = 0; i < n; i++) {
    SgMapModelPoint(xm, x[i]);
    VRML1PlotVect(xm);
    SgPlotOutputStr(",\n");
  }

  SgPlotOutputStr("]\n");
  SgPlotOutputStr("}\n");

  SgPlotOutputStr("IndexedLineSet {\n");
  SgPlotOutputStr("coordIndex [");
  for (i = 0; i < n; i++) {
    SgPlotOutputInt(i);
    SgPlotOutputStr(", ");
  }
  SgPlotOutputStr("]\n");
  SgPlotOutputStr("}\n");

  SgPlotOutputStr("}\n");
}
