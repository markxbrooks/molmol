/*
************************************************************************
*
*   FMPoly.c - FrameMaker polygon drawing
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
*   Date of last modification : 96/01/25
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/fm/SCCS/s.FMPoly.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include "fm_poly.h"

#include <sg_map.h>
#include <sg_plot.h>

void
SgFMDrawPolygon(float x[][2], int n)
{
  float xm[2];
  int i;

  SgPlotOutputStr("<Polygon <GroupID 1>\n");
  SgPlotOutputStr("  <Fill 0>\n");
  SgPlotOutputStr("  <NumPoints ");
  SgPlotOutputInt(n);
  SgPlotOutputStr(">\n");

  for (i = 0; i < n; i++) {
    SgVpMapPoint(xm, x[i]);
    SgPlotOutputStr("  <Point ");
    SgPlotOutputFloat(xm[0]);
    SgPlotOutputStr(" ");
    SgPlotOutputFloat(xm[1]);
    SgPlotOutputStr(">\n");
  }

  SgPlotOutputStr(">\n");
}
