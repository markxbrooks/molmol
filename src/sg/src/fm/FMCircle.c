/*
************************************************************************
*
*   FMCircle.c - FrameMaker circles
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/fm/SCCS/s.FMCircle.c
*   SCCS identification       : 1.5
*
************************************************************************
*/

#include "fm_circle.h"

#include <sg_map.h>
#include <sg_plot.h>

static void
plotCirc(float x[2], float r, BOOL filled)
{
  float xm[2];

  SgPlotOutputStr("<Ellipse <GroupID 1>\n");
  if (filled)
    SgPlotOutputStr("  <Fill 0>\n");
  else
    SgPlotOutputStr("  <Fill 15>\n");

  SgVpMapPoint(xm, x);
  r = SgVpMapSize(r);

  SgPlotOutputStr("  <BRect ");
  SgPlotOutputFloat(xm[0] - r);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(xm[1] - r);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(2.0f * r);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(2.0f * r);
  SgPlotOutputStr(">\n");

  SgPlotOutputStr(">\n");
}

void
SgFMDrawCircle(float x[2], float r)
{
  plotCirc(x, r, FALSE);
}

void
SgFMDrawDisc(float x[2], float r)
{
  plotCirc(x, r, TRUE);
}
