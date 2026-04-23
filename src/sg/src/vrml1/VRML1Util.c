/*
************************************************************************
*
*   VRML1Util.c - VRML1 utility functions
*
*   Copyright (c) 1996
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/vrml1/SCCS/s.VRML1Util.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include "vrml1_util.h"

#include <sg_get.h>
#include <sg_plot.h>

static float Fact;

void
VRML1Init2D(void)
{
  Fact = SgGetOrthoHeight();
}

void
VRML1Map2D(float xOut[3], float xIn[2])
{
  xOut[0] = xIn[0] * Fact;
  xOut[1] = xIn[1] * Fact;
  xOut[2] = 0.0f;
}

void
VRML1PlotVect(float x[3])
{
  SgPlotOutputFloat(x[0]);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(x[1]);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(x[2]);
}
