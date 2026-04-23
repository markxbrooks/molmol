/*
************************************************************************
*
*   X11Dev.c - X11 device
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/x11/SCCS/s.X11Dev.c
*   SCCS identification       : 1.15
*
************************************************************************
*/

#include <stdio.h>

#include <sg_dev.h>

#include "x11_init.h"
#include "x11_clear.h"
#include "x11_marker.h"
#include "x11_line.h"
#include "x11_poly.h"
#include "x11_circle.h"
#include "x11_text.h"
#include "x11_color.h"

static SgDev SgX11Dev = {
  FALSE,

  VPH_UPSIDE_DOWN,
  2,

  SgX11Init,
  SgX11Cleanup,

  NULL,
  NULL,
  NULL,
  NULL,

  SgX11Clear,

  NULL,

  SgX11DrawMarkers,
  SgX11DrawLine,
  NULL,
  SgX11DrawPolyline,
  SgX11DrawPolygon,
  NULL,
  SgX11DrawCircle,
  SgX11DrawDisc,
  SgX11DrawAnnot,
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
  SgX11SetBuffer,
  SgX11FlushFrame,
  SgX11EndFrame,
  SgX11Refresh,

  SgX11SetFeature,

  SgX11SetColor,
  NULL,
  NULL,
  NULL,
  SgX11SetLineWidth,
  SgX11SetLineStyle,
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
SgX11SetDev(void)
{
  CurrSgDevP = &SgX11Dev;
}
