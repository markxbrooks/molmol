/*
************************************************************************
*
*   X11Marker.c - X11 marker drawing
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
*   Date of last modification : 95/01/12
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/x11/SCCS/s.X11Marker.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include "x11_marker.h"

#include <X11/Xlib.h>

#include <sg_types.h>
#include <sg_map.h>
#include "x11_access.h"

void
SgX11DrawMarkers(float x[][2], int n)
{
  float xm[2];
  XPoint pointArr[SG_POLY_SIZE];
  int i;

  for (i = 0; i < n; i++) {
    SgVpMapPoint(xm, x[i]);
    pointArr[i].x = (short) xm[0];
    pointArr[i].y = (short) xm[1];
  }

  XDrawPoints(X11GetDisplay(), X11GetDrawable(), X11GetGC(),
      pointArr, n, CoordModeOrigin);
}
