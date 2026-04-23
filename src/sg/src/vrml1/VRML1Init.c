/*
************************************************************************
*
*   VRML1Init.c - init VRML1 device
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
*   Date of last modification : 99/10/31
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/sg/src/vrml1/SCCS/s.VRML1Init.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include "vrml1_init.h"

#include <sg_plot.h>
#include <sg_get.h>
#include "vrml1_util.h"

SgRes
SgVRML1Init(int *argcP, char *argv[])
{
  SgPlotOutputStr("#VRML V1.0 ascii\n\n");
  SgPlotOutputStr("Separator {\n");

  VRML1Init2D();

  return SG_RES_OK;
}

SgRes
SgVRML1Cleanup(void)
{
  SgPlotOutputStr("ShapeHints {\n");
  SgPlotOutputStr("vertexOrdering COUNTERCLOCKWISE\n");
  if (SgGetFeature(SG_FEATURE_BACKFACE))
    SgPlotOutputStr("shapeType SOLID\n");
  SgPlotOutputStr("faceType CONVEX\n");
  SgPlotOutputStr("}\n");

  SgPlotOutputStr("}\n");

  return SG_RES_OK;
}
