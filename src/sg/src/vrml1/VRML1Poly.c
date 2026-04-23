/*
************************************************************************
*
*   VRML1Poly.c - VRML1 polygon drawing
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
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/sg/src/vrml1/SCCS/s.VRML1Poly.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include "vrml1_poly.h"

#include <stdio.h>

#include <sg.h>
#include <sg_map.h>
#include <sg_map.h>
#include <sg_plot.h>
#include "vrml1_util.h"
#include "vrml1_color.h"
#include "vrml1_light.h"

void
SgVRML1DrawPolygon2D(float x[][2], int n)
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

  SgPlotOutputStr("Normal {vector 0 0 1}\n");
  SgPlotOutputStr("NormalBinding {value OVERALL}\n");

  SgPlotOutputStr("IndexedFaceSet {\n");
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
SgVRML1DrawPolygon(float x[][3], int n, float nv[3])
{
  float xm[3], nvm[3];
  int i;

  VRML1UpdateLight();
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

  SgPlotOutputStr("Normal {vector ");
  SgMapModelVect(nvm, nv);
  VRML1PlotVect(nvm);
  SgPlotOutputStr("}\n");
  SgPlotOutputStr("NormalBinding {value OVERALL}\n");

  SgPlotOutputStr("IndexedFaceSet {\n");
  SgPlotOutputStr("coordIndex [");
  for (i = 0; i < n; i++) {
    SgPlotOutputInt(i);
    SgPlotOutputStr(", ");
  }
  SgPlotOutputStr("]\n");
  SgPlotOutputStr("}\n");

  SgPlotOutputStr("}\n");
}

static void
plotIndices(int n)
{
  int i;

  for (i = 0; i < n - 2; i += 2) {
    SgPlotOutputInt(i);
    SgPlotOutputStr(", ");
    SgPlotOutputInt(i + 1);
    SgPlotOutputStr(", ");
    SgPlotOutputInt(i + 2);
    SgPlotOutputStr(", -1,\n");

    if (i + 3 == n)
      break;

    SgPlotOutputInt(i + 2);
    SgPlotOutputStr(", ");
    SgPlotOutputInt(i + 1);
    SgPlotOutputStr(", ");
    SgPlotOutputInt(i + 3);
    SgPlotOutputStr(", -1,\n");
  }
}

void
SgVRML1DrawTriMesh(float x[][3], float nv[][3], int n)
{
  float xm[3], nvm[3];
  int i;

  VRML1UpdateLight();
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

  SgPlotOutputStr("Normal {\n");
  SgPlotOutputStr("vector [");

  for (i = 0; i < n; i++) {
    SgMapModelVect(nvm, nv[i]);
    VRML1PlotVect(nvm);
    SgPlotOutputStr(",\n");
  }

  SgPlotOutputStr("]\n");
  SgPlotOutputStr("}\n");

  SgPlotOutputStr("IndexedFaceSet {\n");

  SgPlotOutputStr("coordIndex [");
  plotIndices(n);
  SgPlotOutputStr("]\n");

  SgPlotOutputStr("normalIndex [");
  plotIndices(n);
  SgPlotOutputStr("]\n");

  SgPlotOutputStr("}\n");

  SgPlotOutputStr("}\n");
}

void
SgVRML1DrawStrip(float x1[][3], float x2[][3],
    float nv1[][3], float nv2[][3], int n)
{
  float xm[3], nvm[3];
  int i;

  if (nv2 == NULL)
    nv2 = nv1;

  VRML1UpdateLight();
  SgPlotOutputStr("Separator {\n");
  VRML1PlotColor();

  SgPlotOutputStr("Coordinate3 {\n");
  SgPlotOutputStr("point [\n");

  for (i = 0; i < n; i++) {
    SgMapModelPoint(xm, x1[i]);
    VRML1PlotVect(xm);
    SgPlotOutputStr(", ");
    SgMapModelPoint(xm, x2[i]);
    VRML1PlotVect(xm);
    SgPlotOutputStr(",\n");
  }

  SgPlotOutputStr("]\n");
  SgPlotOutputStr("}\n");

  SgPlotOutputStr("Normal {\n");
  SgPlotOutputStr("vector [");

  for (i = 0; i < n; i++) {
    SgMapModelVect(nvm, nv1[i]);
    VRML1PlotVect(nvm);
    SgPlotOutputStr(", ");
    SgMapModelVect(nvm, nv2[i]);
    VRML1PlotVect(nvm);
    SgPlotOutputStr(",\n");
  }

  SgPlotOutputStr("]\n");
  SgPlotOutputStr("}\n");

  SgPlotOutputStr("IndexedFaceSet {\n");

  SgPlotOutputStr("coordIndex [");
  plotIndices(2 * n);
  SgPlotOutputStr("]\n");

  SgPlotOutputStr("normalIndex [");
  plotIndices(2 * n);
  SgPlotOutputStr("]\n");

  SgPlotOutputStr("}\n");

  SgPlotOutputStr("}\n");
}
