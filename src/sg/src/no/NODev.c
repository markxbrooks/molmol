/*
************************************************************************
*
*   NODev.c - NO device (all empty functions)
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
*   Date of last modification : 01/06/02
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/no/SCCS/s.NODev.c
*   SCCS identification       : 1.8
*
************************************************************************
*/

#include <stdio.h>

#include <sg_dev.h>

#include "no.h"

static SgDev SgNODev = {
  FALSE,

  VPH_NORMAL,
  3,

  SgNOInit,
  SgNOCleanup,

  SgNOBeginObj,
  SgNOEndObj,
  SgNOReplaceObj,
  SgNODestroyObj,

  NULL,

  SgNODrawObj,

  SgNODrawMarkers2D,
  SgNODrawLine2D,
  SgNODrawArrow2D,
  SgNODrawPolyline2D,
  SgNODrawPolygon2D,
  NULL,
  SgNODrawCircle2D,
  SgNODrawDisc2D,
  SgNODrawAnnot2D,
  SgNODrawText2D,

  SgNODrawMarkers,
  SgNODrawLine,
  SgNODrawArrow,
  SgNODrawPolyline,
  SgNODrawPolygon,
  SgNODrawCircle,
  SgNODrawDisc,
  SgNODrawAnnot,
  SgNODrawText,
  SgNODrawSphere,
  SgNODrawCone,

  NULL,
  SgNODrawTriMesh,
  SgNODrawColorTriMesh,
  SgNODrawStrip,
  SgNODrawColorStrip,
  NULL,

  NULL,
  NULL,
  NULL,
  NULL,
  NULL,

  NULL,

  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,

  NULL,

  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,

  NULL,
  NULL,

  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,

  NULL,

  NULL,
  NULL
};

void
SgNOSetDev(void)
{
  CurrSgDevP = &SgNODev;
}
