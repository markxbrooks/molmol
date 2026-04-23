/*
************************************************************************
*
*   VRML1Dev.c - VRML1 device
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
*   Date of last modification : 01/06/02
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/vrml1/SCCS/s.VRML1Dev.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <stdio.h>

#include <sg_dev.h>

#include "vrml1_init.h"
#include "vrml1_marker.h"
#include "vrml1_line.h"
#include "vrml1_poly.h"
#include "vrml1_text.h"
#include "vrml1_shape.h"
#include "vrml1_frame.h"
#include "vrml1_color.h"
#include "vrml1_light.h"

static SgDev SgVRML1Dev = {
  FALSE,

  VPH_NORMAL,
  3,

  SgVRML1Init,
  SgVRML1Cleanup,

  NULL,
  NULL,
  NULL,
  NULL,

  NULL,

  NULL,

  NULL,
  SgVRML1DrawLine2D,
  NULL,
  SgVRML1DrawPolyline2D,
  SgVRML1DrawPolygon2D,
  NULL,
  NULL,
  NULL,
  SgVRML1DrawAnnot2D,
  SgVRML1DrawText2D,

  SgVRML1DrawMarkers,
  SgVRML1DrawLine,
  NULL,
  SgVRML1DrawPolyline,
  SgVRML1DrawPolygon,
  NULL,
  NULL,
  SgVRML1DrawAnnot,
  SgVRML1DrawText,
  SgVRML1DrawSphere,
  SgVRML1DrawCone,

  NULL,
  SgVRML1DrawTriMesh,
  NULL,
  SgVRML1DrawStrip,
  NULL,
  NULL,

  NULL,
  NULL,
  NULL,
  SgVRML1EndFrame,
  NULL,

  NULL,

  SgVRML1SetColor,
  SgVRML1SetMatProp,
  SgVRML1SetTexture,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  SgVRML1SetFontSize2D,
  SgVRML1SetFontSize,

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

  SgVRML1SetLight,
  SgVRML1SetLightPosition
};

void
SgVRML1SetDev(void)
{
  CurrSgDevP = &SgVRML1Dev;
}
