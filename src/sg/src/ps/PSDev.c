/*
************************************************************************
*
*   PSDev.c - PostScript device
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/ps/SCCS/s.PSDev.c
*   SCCS identification       : 1.15
*
************************************************************************
*/

#include <stdio.h>

#include <sg_dev.h>

#include "ps_init.h"
#include "ps_line.h"
#include "ps_poly.h"
#include "ps_circle.h"
#include "ps_text.h"
#include "ps_frame.h"
#include "ps_color.h"
#include "ps_stereo.h"

static SgDev SgPSDev = {
  FALSE,

  VPH_NORMAL,
  2,

  SgPSInit,
  SgPSCleanup,

  NULL,
  NULL,
  NULL,
  NULL,

  NULL,

  NULL,

  NULL,
  SgPSDrawLine,
  NULL,
  SgPSDrawPolyline,
  SgPSDrawPolygon,
  NULL,
  SgPSDrawCircle,
  SgPSDrawDisc,
  NULL,
  SgPSDrawText,

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
  SgPSEndFrame,
  NULL,

  NULL,

  NULL,
  NULL,
  NULL,
  NULL,
  SgPSSetLineWidth,
  SgPSSetLineStyle,
  NULL,
  NULL,
  NULL,
  NULL,
  SgPSSetFontSize,
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

  SgPSSetStereo,
  SgPSSetEye,

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

static SgDev SgCPSDev = {
  FALSE,

  VPH_NORMAL,
  2,

  SgPSInit,
  SgPSCleanup,

  NULL,
  NULL,
  NULL,
  NULL,

  NULL,

  NULL,

  NULL,
  SgPSDrawLine,
  NULL,
  SgPSDrawPolyline,
  SgPSDrawPolygon,
  SgPSDrawShadedPolygon,
  SgPSDrawCircle,
  SgPSDrawDisc,
  NULL,
  SgPSDrawText,

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
  SgPSEndFrame,
  NULL,

  NULL,

  SgPSSetColor,
  NULL,
  NULL,
  NULL,
  SgPSSetLineWidth,
  SgPSSetLineStyle,
  NULL,
  NULL,
  NULL,
  NULL,
  SgPSSetFontSize,
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

  SgPSSetStereo,
  SgPSSetEye,

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
SgPSSetDev(void)
{
  CurrSgDevP = &SgPSDev;
}

void
SgCPSSetDev(void)
{
  CurrSgDevP = &SgCPSDev;
}
