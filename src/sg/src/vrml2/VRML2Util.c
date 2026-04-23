/*
************************************************************************
*
*   VRML2Util.c - VRML2 utility functions
*
*   Copyright (c) 1996-99
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/vrml2/SCCS/s.VRML2Util.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include "vrml2_util.h"

#include <sg_get.h>
#include <sg_plot.h>

static float Fact;

void
VRML2Init2D(void)
{
  Fact = SgGetOrthoHeight();
}

void
VRML2Map2D(float xOut[3], float xIn[2])
{
  xOut[0] = xIn[0] * Fact;
  xOut[1] = xIn[1] * Fact;
  xOut[2] = 0.0f;
}

void
VRML2PlotVect(float x[3])
{
  SgPlotOutputFloat(x[0]);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(x[1]);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(x[2]);
}
