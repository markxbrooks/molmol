/*
************************************************************************
*
*   VRML1Marker.c - VRML1 marker drawing
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
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/sg/src/vrml1/SCCS/s.VRML1Marker.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include "vrml1_marker.h"

#include <sg_map.h>
#include <sg_plot.h>
#include "vrml1_util.h"
#include "vrml1_color.h"

void
SgVRML1DrawMarkers(float x[][3], int n)
{
  float xm[3];
  int i;

  SgPlotOutputStr("Separator {\n");
  VRML1PlotColor();

  SgPlotOutputStr("Coordinate3 {\n");
  SgPlotOutputStr("point [\n");

  for (i = 0; i < n; i++) {
    if (i > 0)
      SgPlotOutputStr(",\n");
    SgMapModelPoint(xm, x[i]);
    VRML1PlotVect(xm);
  }

  SgPlotOutputStr("]\n");
  SgPlotOutputStr("}\n");

  SgPlotOutputStr("}\n");
}
