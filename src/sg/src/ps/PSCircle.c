/*
************************************************************************
*
*   PSCircle.c - PostScript circle drawing
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
*   Date of last modification : 95/07/06
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/ps/SCCS/s.PSCircle.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include "ps_circle.h"

#include <sg_map.h>
#include <sg_plot.h>
#include "ps_color.h"

static void
plotCirc(float x[2], float r, BOOL filled)
{
  float xm[2];

  SgVpMapPoint(xm, x);
  r = SgVpMapSize(r);

  SgPlotOutputFloat(xm[0]);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(xm[1]);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(r);
  if (filled)
    SgPlotOutputStr(" di\n");
  else
    SgPlotOutputStr(" ci\n");
}

void
SgPSDrawCircle(float x[2], float r)
{
  PSUpdateColor();
  plotCirc(x, r, FALSE);
}

void
SgPSDrawDisc(float x[2], float r)
{
  PSUpdateColor();
  plotCirc(x, r, TRUE);
}
