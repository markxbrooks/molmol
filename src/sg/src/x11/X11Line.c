/*
************************************************************************
*
*   X11Line.c - X11 line drawing
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
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/x11/SCCS/s.X11Line.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include "x11_line.h"

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
SgX11DrawLine(float x0[2], float x1[2])
{
  float x0m[2], x1m[2];

  SgVpMapPoint(x0m, x0);
  SgVpMapPoint(x1m, x1);

  prepareDrawing();
  XDrawLine(CurrDisplay, CurrDrawable, CurrGC,
      (int) x0m[0], (int) x0m[1], (int) x1m[0], (int) x1m[1]);
}

void
SgX11DrawPolyline(float x[][2], int n)
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
  XDrawLines(CurrDisplay, CurrDrawable, CurrGC,
      pointArr, n, CoordModeOrigin);
}

void
SgX11SetLineWidth(float w)
{
  XGCValues gcv;

  prepareDrawing();
  gcv.line_width = (int) w;
  XChangeGC(CurrDisplay, CurrGC, GCLineWidth, &gcv);
}

void
SgX11SetLineStyle(SgLineStyle lineStyle)
{
  XGCValues gcv;

  prepareDrawing();
  if (lineStyle == SG_LINE_DASHED)
    gcv.line_style = LineOnOffDash;
  else
    gcv.line_style = LineSolid;
  XChangeGC(CurrDisplay, CurrGC, GCLineStyle, &gcv);
}
