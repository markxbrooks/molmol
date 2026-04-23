/*
************************************************************************
*
*   PickDev.c - picking device
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/pick/SCCS/s.PickDev.c
*   SCCS identification       : 1.17
*
************************************************************************
*/

#include <stdio.h>

#include <sg_dev.h>

#include "pick.h"

static SgDev SgPickDev = {
  FALSE,

  VPH_NORMAL,
  3,

  SgPickInit,
  SgPickCleanup,

  NULL,
  NULL,
  NULL,
  NULL,

  NULL,

  NULL,

  SgPickDrawMarkers2D,
  SgPickDrawLine2D,
  NULL,
  SgPickDrawPolyline2D,
  SgPickDrawPolygon2D,
  NULL,
  SgPickDrawCircle2D,
  SgPickDrawDisc2D,
  SgPickDrawAnnot2D,
  SgPickDrawText2D,

  SgPickDrawMarkers,
  SgPickDrawLine,
  NULL,
  SgPickDrawPolyline,
  SgPickDrawPolygon,
  NULL,
  NULL,
  SgPickDrawAnnot,
  SgPickDrawText,
  SgPickDrawSphere,
  SgPickDrawCone,

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
  NULL,
  NULL,
  NULL,

  NULL,

  NULL,
  NULL
};

void
SgPickSetDev(void)
{
  CurrSgDevP = &SgPickDev;
}
