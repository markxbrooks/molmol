/*
************************************************************************
*
*   POVMarker.c - POV marker drawing
*
*   Copyright (c) 1994-95
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/pov/SCCS/s.POVMarker.c
*   SCCS identification       : 1.5
*
************************************************************************
*/

#include "pov_marker.h"

#include <sg_map.h>
#include <sg_plot.h>
#include "pov_util.h"
#include "pov_color.h"
#include "pov_clip.h"

void
SgPOVDrawMarkers(float x[][3], int n)
{
  float xm[3];
  int i;

  POVUpdateColor();

  for (i = 0; i < n; i++) {
    SgMapModelPoint(xm, x[i]);
    SgPlotOutputStr("sphere {\n");
    POVPlotVect(xm);
    SgPlotOutputStr(", 0.01\n");
    POVPlotColor();
    (void) POVClip(xm, 0.01f);
    SgPlotOutputStr("}\n");
  }
}
