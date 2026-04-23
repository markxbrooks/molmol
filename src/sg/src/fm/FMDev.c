/*
************************************************************************
*
*   FMDev.c - FrameMaker device
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/fm/SCCS/s.FMDev.c
*   SCCS identification       : 1.15
*
************************************************************************
*/

#include <stdio.h>

#include <sg_dev.h>

#include "fm_init.h"
#include "fm_line.h"
#include "fm_poly.h"
#include "fm_circle.h"
#include "fm_text.h"
#include "fm_frame.h"
#include "fm_color.h"

static SgDev SgFM3Dev = {
  FALSE,

  VPH_UPSIDE_DOWN,
  2,

  SgFM3Init,
  SgFMCleanup,

  NULL,
  NULL,
  NULL,
  NULL,

  NULL,

  NULL,

  NULL,
  SgFMDrawLine,
  NULL,
  SgFMDrawPolyline,
  SgFMDrawPolygon,
  NULL,
  SgFMDrawCircle,
  SgFMDrawDisc,
  NULL,
  SgFMDrawText,

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
  SgFMEndFrame,
  NULL,

  NULL,

  SgFM3SetColor,
  NULL,
  NULL,
  NULL,
  SgFMSetLineWidth,
  SgFMSetLineStyle,
  NULL,
  NULL,
  NULL,
  NULL,
  SgFMSetFontSize,
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

static SgDev SgFM4Dev = {
  FALSE,

  VPH_UPSIDE_DOWN,
  2,

  SgFM4Init,
  SgFMCleanup,

  NULL,
  NULL,
  NULL,
  NULL,

  NULL,

  NULL,

  NULL,
  SgFMDrawLine,
  NULL,
  SgFMDrawPolyline,
  SgFMDrawPolygon,
  NULL,
  SgFMDrawCircle,
  SgFMDrawDisc,
  NULL,
  SgFMDrawText,

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
  SgFMEndFrame,
  NULL,

  NULL,

  SgFM4SetColor,
  NULL,
  NULL,
  NULL,
  SgFMSetLineWidth,
  SgFMSetLineStyle,
  NULL,
  NULL,
  NULL,
  NULL,
  SgFMSetFontSize,
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
SgFM3SetDev(void)
{
  CurrSgDevP = &SgFM3Dev;
}

void
SgFM4SetDev(void)
{
  CurrSgDevP = &SgFM4Dev;
}
