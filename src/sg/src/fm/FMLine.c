/*
************************************************************************
*
*   FMLine.c - FrameMaker line drawing
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
*   Date of last modification : 01/05/27
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/fm/SCCS/s.FMLine.c
*   SCCS identification       : 1.5
*
************************************************************************
*/

#include "fm_line.h"

#include <sg_map.h>
#include <sg_plot.h>

#define MIN_LINE_WIDTH 0.24f  /* one dot at 300 dpi */

void
SgFMDrawLine(float x0[2], float x1[2])
{
  float xm[2];

  SgPlotOutputStr("<PolyLine <GroupID 1>\n");
  SgPlotOutputStr("  <NumPoints 2>\n");

  SgVpMapPoint(xm, x0);
  SgPlotOutputStr("  <Point ");
  SgPlotOutputFloat(xm[0]);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(xm[1]);
  SgPlotOutputStr(">\n");

  SgVpMapPoint(xm, x1);
  SgPlotOutputStr("  <Point ");
  SgPlotOutputFloat(xm[0]);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(xm[1]);
  SgPlotOutputStr(">\n");

  SgPlotOutputStr(">\n");
}

void
SgFMDrawPolyline(float x[][2], int n)
{
  float xm[2];
  int i;

  SgPlotOutputStr("<PolyLine <GroupID 1>\n");
  SgPlotOutputStr("  <Fill 15>\n");
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

void
SgFMSetLineWidth(float w)
{
  w *= 0.72f;  /* 1/100 inch -> point */
  if (w < MIN_LINE_WIDTH)
    /* use a minimal line width, otherwise lines get too thin
       on high resolution printers */
    w = MIN_LINE_WIDTH;

  SgPlotOutputStr("<PenWidth ");
  SgPlotOutputFloat(w);
  SgPlotOutputStr(">\n");
}

void
SgFMSetLineStyle(SgLineStyle lineStyle)
{
  if (lineStyle == SG_LINE_SOLID)
    SgPlotOutputStr("<Pen 0>\n");
  else
    SgPlotOutputStr("<Pen 10>\n");
}
