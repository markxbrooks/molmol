/*
************************************************************************
*
*   VRML2Line.c - VRML2 line drawing
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
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/sg/src/vrml2/SCCS/s.VRML2Line.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include "vrml2_line.h"

#include <sg_map.h>
#include <sg_plot.h>
#include "vrml2_util.h"
#include "vrml2_color.h"

static void
plotLine(float x0[3], float x1[3])
{
  SgPlotOutputStr("Shape {\n");
  VRML2PlotColor();

  SgPlotOutputStr("geometry IndexedLineSet {\n");
  SgPlotOutputStr("coord Coordinate {\n");
  SgPlotOutputStr("point [");
  VRML2PlotVect(x0);
  SgPlotOutputStr(", ");
  VRML2PlotVect(x1);
  SgPlotOutputStr("]\n");
  SgPlotOutputStr("}\n");
  SgPlotOutputStr("coordIndex [0 1]\n");
  SgPlotOutputStr("}\n");

  SgPlotOutputStr("}\n");
}

void
SgVRML2DrawLine2D(float x0[2], float x1[2])
{
  float x0m[3], x1m[3];

  VRML2Map2D(x0m, x0);
  VRML2Map2D(x1m, x1);

  plotLine(x0m, x1m);
}

void
SgVRML2DrawPolyline2D(float x[][2], int n)
{
  float xm[3];
  int i;

  SgPlotOutputStr("Shape {\n");
  VRML2PlotColor();

  SgPlotOutputStr("geometry IndexedLineSet {\n");
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

  SgPlotOutputStr("coordIndex [");
  for (i = 0; i < n; i++) {
    SgPlotOutputInt(i);
    SgPlotOutputStr(" ");
  }
  SgPlotOutputStr("]\n");
  SgPlotOutputStr("}\n");

  SgPlotOutputStr("}\n");
}

void
SgVRML2DrawLine(float x0[3], float x1[3])
{
  float x0m[3], x1m[3];

  SgMapModelPoint(x0m, x0);
  SgMapModelPoint(x1m, x1);

  plotLine(x0m, x1m);
}

void
SgVRML2DrawPolyline(float x[][3], int n)
{
  float xm[3];
  int i;

  SgPlotOutputStr("Shape {\n");
  VRML2PlotColor();

  SgPlotOutputStr("geometry IndexedLineSet {\n");
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

  SgPlotOutputStr("coordIndex [");
  for (i = 0; i < n; i++) {
    SgPlotOutputInt(i);
    SgPlotOutputStr(" ");
  }
  SgPlotOutputStr("]\n");
  SgPlotOutputStr("}\n");

  SgPlotOutputStr("}\n");
}
