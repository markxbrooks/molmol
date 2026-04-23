/*
************************************************************************
*
*   RIBDev.c - RIB device
*
*   Copyright (c) 1994-98
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/rib/SCCS/s.RIBDev.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <stdio.h>

#include <sg_dev.h>

#include "rib_init.h"
#include "rib_view.h"
#include "rib_marker.h"
#include "rib_line.h"
#include "rib_poly.h"
#include "rib_shape.h"
#include "rib_frame.h"
#include "rib_color.h"
#include "rib_light.h"

static SgDev SgRIBDev = {
  FALSE,

  VPH_NORMAL,
  3,

  SgRIBInit,
  SgRIBCleanup,

  NULL,
  NULL,
  NULL,
  NULL,

  NULL,

  NULL,

  NULL,
  SgRIBDrawLine2D,
  NULL,
  SgRIBDrawPolyline2D,
  SgRIBDrawPolygon2D,
  NULL,
  NULL,
  SgRIBDrawDisc2D,
  NULL,
  NULL,

  SgRIBDrawMarkers,
  SgRIBDrawLine,
  NULL,
  SgRIBDrawPolyline,
  SgRIBDrawPolygon,
  NULL,
  SgRIBDrawDisc,
  NULL,
  NULL,
  SgRIBDrawSphere,
  SgRIBDrawCone,

  SgRIBStartSurface,
  SgRIBDrawTriMesh,
  SgRIBDrawColorTriMesh,
  SgRIBDrawStrip,
  SgRIBDrawColorStrip,
  SgRIBEndSurface,

  NULL,
  NULL,
  NULL,
  SgRIBEndFrame,
  NULL,

  NULL,

  SgRIBSetColor,
  SgRIBSetMatProp,
  SgRIBSetTexture,
  NULL,
  SgRIBSetLineWidth,
  SgRIBSetLineStyle,
  SgRIBSetShadeModel,
  SgRIBSetFogMode,
  SgRIBSetFogPar,
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

  SgRIBSetProjection,
  SgRIBSetViewPoint,
  SgRIBSetViewAngle,
  SgRIBSetNearPlane,
  SgRIBSetFarPlane,
  SgRIBUpdateView,
  NULL,

  NULL,

  SgRIBSetLight,
  SgRIBSetLightPosition
};

void
SgRIBSetDev(void)
{
  CurrSgDevP = &SgRIBDev;
}
