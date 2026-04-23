/*
************************************************************************
*
*   RIBMarker.c - RIB marker drawing
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
*   Date of last modification : 99/10/16
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/sg/src/rib/SCCS/s.RIBMarker.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include "rib_marker.h"

#include <sg_map.h>
#include <sg_plot.h>
#include "rib_util.h"
#include "rib_color.h"

void
SgRIBDrawMarkers(float x[][3], int n)
{
  float xm[3];
  int i;

  RIBUpdateMaterial();

  for (i = 0; i < n; i++) {
    SgMapModelPoint(xm, x[i]);
    SgPlotOutputStr("Translate ");
    RIBPlotVect(xm);
    SgPlotOutputStr("\n");
    SgPlotOutputStr("Sphere 0.01 -0.01 0.01 360\n");
    SgPlotOutputStr("Identity\n");
  }
}
