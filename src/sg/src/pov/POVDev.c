/*
************************************************************************
*
*   POVDev.c - POV device
*
*   Copyright (c) 1994-2000
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/pov/SCCS/s.POVDev.c
*   SCCS identification       : 1.14
*
************************************************************************
*/

#include <stdio.h>

#include <sg_dev.h>

#include "pov_init.h"
#include "pov_view.h"
#include "pov_marker.h"
#include "pov_line.h"
#include "pov_poly.h"
#include "pov_text.h"
#include "pov_shape.h"
#include "pov_frame.h"
#include "pov_color.h"
#include "pov_light.h"
#include "mpov_mesh.h"

static SgDev SgPOVDev = {
  FALSE,

  VPH_NORMAL,
  3,

  SgPOVInit,
  SgPOVCleanup,

  NULL,
  NULL,
  NULL,
  NULL,

  NULL,

  NULL,

  NULL,
  SgPOVDrawLine2D,
  NULL,
  SgPOVDrawPolyline2D,
  SgPOVDrawPolygon2D,
  NULL,
  NULL,
  SgPOVDrawDisc2D,
  SgPOVDrawAnnot2D,
  SgPOVDrawText2D,

  SgPOVDrawMarkers,
  SgPOVDrawLine,
  NULL,
  SgPOVDrawPolyline,
  SgPOVDrawPolygon,
  NULL,
  SgPOVDrawDisc,
  SgPOVDrawAnnot,
  SgPOVDrawText,
  SgPOVDrawSphere,
  SgPOVDrawCone,

  SgPOVStartSurface,
  SgPOVDrawTriMesh,
  SgPOVDrawColorTriMesh,
  SgPOVDrawStrip,
  SgPOVDrawColorStrip,
  SgPOVEndSurface,

  NULL,
  NULL,
  NULL,
  SgPOVEndFrame,
  NULL,

  NULL,

  SgPOVSetColor,
  SgPOVSetMatProp,
  SgPOVSetTexture,
  NULL,
  SgPOVSetLineWidth,
  SgPOVSetLineStyle,
  SgPOVSetShadeModel,
  SgPOVSetFogMode,
  SgPOVSetFogPar,
  NULL,
  SgPOVSetFontSize2D,
  SgPOVSetFontSize,

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

  SgPOVSetProjection,
  SgPOVSetViewPoint,
  SgPOVSetViewAngle,
  SgPOVSetNearPlane,
  SgPOVSetFarPlane,
  SgPOVUpdateView,
  NULL,

  NULL,

  SgPOVSetLight,
  SgPOVSetLightPosition
};

static SgDev SgMPOVDev = {
  FALSE,

  VPH_NORMAL,
  3,

  SgMPOVInit,
  SgPOVCleanup,

  NULL,
  NULL,
  NULL,
  NULL,

  NULL,

  NULL,

  NULL,
  SgPOVDrawLine2D,
  NULL,
  SgPOVDrawPolyline2D,
  SgPOVDrawPolygon2D,
  NULL,
  NULL,
  SgPOVDrawDisc2D,
  SgPOVDrawAnnot2D,
  SgPOVDrawText2D,

  SgPOVDrawMarkers,
  SgPOVDrawLine,
  NULL,
  SgPOVDrawPolyline,
  SgPOVDrawPolygon,
  NULL,
  SgPOVDrawDisc,
  SgPOVDrawAnnot,
  SgPOVDrawText,
  SgPOVDrawSphere,
  SgPOVDrawCone,

  NULL,
  SgMPOVDrawTriMesh,
  SgMPOVDrawColorTriMesh,
  SgMPOVDrawStrip,
  SgMPOVDrawColorStrip,
  NULL,

  NULL,
  NULL,
  NULL,
  SgPOVEndFrame,
  NULL,

  NULL,

  SgPOVSetColor,
  SgPOVSetMatProp,
  SgPOVSetTexture,
  NULL,
  SgPOVSetLineWidth,
  SgPOVSetLineStyle,
  SgPOVSetShadeModel,
  SgPOVSetFogMode,
  SgPOVSetFogPar,
  NULL,
  SgPOVSetFontSize2D,
  SgPOVSetFontSize,

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

  SgPOVSetProjection,
  SgPOVSetViewPoint,
  SgPOVSetViewAngle,
  SgPOVSetNearPlane,
  SgPOVSetFarPlane,
  SgPOVUpdateView,
  NULL,

  NULL,

  SgPOVSetLight,
  SgPOVSetLightPosition
};

void
SgPOVSetDev(void)
{
  CurrSgDevP = &SgPOVDev;
}

void
SgMPOVSetDev(void)
{
  CurrSgDevP = &SgMPOVDev;
}
