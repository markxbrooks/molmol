/*
************************************************************************
*
*   VRML2Dev.c - VRML2 device
*
*   Copyright (c) 1996-99
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/vrml2/SCCS/s.VRML2Dev.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <stdio.h>

#include <sg_dev.h>

#include "vrml2_init.h"
#include "vrml2_marker.h"
#include "vrml2_line.h"
#include "vrml2_poly.h"
#include "vrml2_text.h"
#include "vrml2_shape.h"
#include "vrml2_frame.h"
#include "vrml2_color.h"
#include "vrml2_light.h"

static SgDev SgVRML2Dev = {
  FALSE,

  VPH_NORMAL,
  3,

  SgVRML2Init,
  SgVRML2Cleanup,

  NULL,
  NULL,
  NULL,
  NULL,

  NULL,

  NULL,

  NULL,
  SgVRML2DrawLine2D,
  NULL,
  SgVRML2DrawPolyline2D,
  SgVRML2DrawPolygon2D,
  NULL,
  NULL,
  NULL,
  SgVRML2DrawAnnot2D,
  SgVRML2DrawText2D,

  SgVRML2DrawMarkers,
  SgVRML2DrawLine,
  NULL,
  SgVRML2DrawPolyline,
  SgVRML2DrawPolygon,
  NULL,
  NULL,
  SgVRML2DrawAnnot,
  SgVRML2DrawText,
  SgVRML2DrawSphere,
  SgVRML2DrawCone,

  NULL,
  SgVRML2DrawTriMesh,
  SgVRML2DrawColorTriMesh,
  SgVRML2DrawStrip,
  SgVRML2DrawColorStrip,
  NULL,

  NULL,
  NULL,
  NULL,
  SgVRML2EndFrame,
  NULL,

  NULL,

  SgVRML2SetColor,
  SgVRML2SetMatProp,
  SgVRML2SetTexture,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  SgVRML2SetFontSize2D,
  SgVRML2SetFontSize,

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

  SgVRML2SetLight,
  SgVRML2SetLightPosition
};

void
SgVRML2SetDev(void)
{
  CurrSgDevP = &SgVRML2Dev;
}
