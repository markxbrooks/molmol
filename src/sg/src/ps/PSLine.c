/*
************************************************************************
*
*   PSLine.c - PostScript line drawing
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/ps/SCCS/s.PSLine.c
*   SCCS identification       : 1.6
*
************************************************************************
*/

#include "ps_line.h"

#include <sg_map.h>
#include <sg_plot.h>
#include "ps_color.h"

#define MIN_LINE_WIDTH 0.0033333f  /* one dot at 300 dpi */
#define DASH_LEN 0.02f

static float CurrW = 0.0f;
static SgLineStyle CurrStyle = SG_LINE_SOLID;

void
SgPSDrawLine(float x0[2], float x1[2])
{
  float xm[2];

  PSUpdateColor();

  SgVpMapPoint(xm, x0);
  SgPlotOutputFloat(xm[0]);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(xm[1]);
  SgPlotOutputStr(" ");

  SgVpMapPoint(xm, x1);
  SgPlotOutputFloat(xm[0]);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(xm[1]);
  SgPlotOutputStr(" ls\n");
}

void
SgPSDrawPolyline(float x[][2], int n)
{
  float xm[2];
  int i;

  PSUpdateColor();

  SgVpMapPoint(xm, x[0]);
  SgPlotOutputStr("np\n");
  SgPlotOutputFloat(xm[0]);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(xm[1]);
  SgPlotOutputStr(" mt\n");
  for (i = 1; i < n; i++) {
    SgVpMapPoint(xm, x[i]);
    SgPlotOutputFloat(xm[0]);
    SgPlotOutputStr(" ");
    SgPlotOutputFloat(xm[1]);
    SgPlotOutputStr(" lt\n");
  }
  SgPlotOutputStr("st\n");
}

static void
dashOn(float w)
{
  SgPlotOutputFloat(DASH_LEN);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(DASH_LEN + w);
  SgPlotOutputStr(" do\n");
}

void
SgPSSetLineWidth(float w)
{
  w /= 100.0f;
  if (w < MIN_LINE_WIDTH)
    /* use a minimal line width, otherwise lines get too thin
       on high resolution printers */
    w = MIN_LINE_WIDTH;

  SgPlotOutputFloat(w);
  SgPlotOutputStr(" lw\n");
  CurrW = w;

  if (CurrStyle != SG_LINE_SOLID)
    dashOn(w);
}

void
SgPSSetLineStyle(SgLineStyle lineStyle)
{
  if (lineStyle == SG_LINE_SOLID)
    SgPlotOutputStr("df\n");
  else
    dashOn(CurrW);
  
  CurrStyle = lineStyle;
}
