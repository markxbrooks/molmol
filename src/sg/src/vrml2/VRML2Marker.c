/*
************************************************************************
*
*   VRML2Marker.c - VRML2 marker drawing
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
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/sg/src/vrml2/SCCS/s.VRML2Marker.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include "vrml2_marker.h"

#include <sg_map.h>
#include <sg_plot.h>
#include "vrml2_util.h"
#include "vrml2_color.h"

void
SgVRML2DrawMarkers(float x[][3], int n)
{
  float xm[3];
  int i;

  SgPlotOutputStr("Shape {\n");
  VRML2PlotColor();

  SgPlotOutputStr("geometry PointSet {\n");
  SgPlotOutputStr("coord Coordinate {\n");
  SgPlotOutputStr("point [\n");

  for (i = 0; i < n; i++) {
    if (i > 0)
      SgPlotOutputStr(",\n");
    SgMapModelPoint(xm, x[i]);
    VRML2PlotVect(xm);
  }

  SgPlotOutputStr("]\n");
  SgPlotOutputStr("}\n");
  SgPlotOutputStr("}\n");

  SgPlotOutputStr("}\n");
}
