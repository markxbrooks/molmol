/*
************************************************************************
*
*   X11Poly.c - X11 polygon drawing
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
*   Date of last modification : 94/08/11
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/x11/SCCS/s.X11Poly.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include "x11_poly.h"

#include <X11/Xlib.h>

#include <sg_map.h>
#include "x11_access.h"

static Display *CurrDisplay;
static Drawable CurrDrawable;
static GC CurrGC;

static void
prepareDrawing(void)
{
  CurrDisplay = X11GetDisplay();
  CurrDrawable = X11GetDrawable();
  CurrGC = X11GetGC();
}

void
SgX11DrawPolygon(float x[][2], int n)
{
  int i;
  float xm[2];
  XPoint pointArr[SG_POLY_SIZE];

  prepareDrawing();
  for (i = 0; i < n; i++) {
    SgVpMapPoint(xm, x[i]);
    pointArr[i].x = (short) xm[0];
    pointArr[i].y = (short) xm[1];
  }
  XFillPolygon(CurrDisplay, CurrDrawable, CurrGC,
      pointArr, n, Convex, CoordModeOrigin);
}
