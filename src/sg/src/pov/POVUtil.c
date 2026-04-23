/*
************************************************************************
*
*   POVUtil.c - POV utility functions
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/pov/SCCS/s.POVUtil.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include "pov_util.h"

#include <sg_get.h>
#include <sg_plot.h>

static float Fact;

void
POVInit2D(void)
{
  Fact = SgGetOrthoHeight();
}

void
POVMap2D(float xOut[3], float xIn[2])
{
  xOut[0] = xIn[0] * Fact;
  xOut[1] = xIn[1] * Fact;
  xOut[2] = 0.0f;
}

void
POVPlotVect(float x[3])
{
  SgPlotOutputStr("<");
  SgPlotOutputFloat(x[0]);
  SgPlotOutputStr(", ");
  SgPlotOutputFloat(x[1]);
  SgPlotOutputStr(", ");
  SgPlotOutputFloat(x[2]);
  SgPlotOutputStr(">");
}
