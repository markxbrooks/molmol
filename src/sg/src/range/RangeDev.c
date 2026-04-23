/*
************************************************************************
*
*   RangeDev.c - range determination device
*
*   Copyright (c) 1994-99
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/range/SCCS/s.RangeDev.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <stdio.h>

#include <sg_dev.h>

#include "range.h"

static SgDev SgRangeDev = {
  FALSE,

  VPH_NORMAL,
  3,

  SgRangeInit,
  SgRangeCleanup,

  NULL,
  NULL,
  NULL,
  NULL,

  NULL,

  NULL,

  SgRangeDrawMarkers2D,
  SgRangeDrawLine2D,
  NULL,
  SgRangeDrawPolyline2D,
  SgRangeDrawPolygon2D,
  NULL,
  SgRangeDrawCircle2D,
  SgRangeDrawDisc2D,
  SgRangeDrawAnnot2D,
  SgRangeDrawText2D,

  SgRangeDrawMarkers,
  SgRangeDrawLine,
  NULL,
  SgRangeDrawPolyline,
  SgRangeDrawPolygon,
  NULL,
  NULL,
  SgRangeDrawAnnot,
  SgRangeDrawText,
  SgRangeDrawSphere,
  SgRangeDrawCone,

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
SgRangeSetDev(void)
{
  CurrSgDevP = &SgRangeDev;
}
