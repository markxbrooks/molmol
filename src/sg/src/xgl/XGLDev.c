/*
************************************************************************
*
*   XGLDev.c - XGL device
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/xgl/SCCS/s.XGLDev.c
*   SCCS identification       : 1.12
*
************************************************************************
*/

#include <stdio.h>

#include <sg_dev.h>

#include "xgl_init.h"
#include "xgl_view.h"
#include "xgl_clear.h"
#include "xgl_marker.h"
#include "xgl_line.h"
#include "xgl_poly.h"
#include "xgl_text.h"
#include "xgl_mesh.h"
#include "xgl_feature.h"
#include "xgl_color.h"
#include "xgl_transf.h"
#include "xgl_light.h"

static SgDev SgXGLDev = {
  FALSE,

  VPH_NORMAL,
  3,

  SgXGLInit,
  SgXGLCleanup,

  NULL,
  NULL,
  NULL,
  NULL,

  SgXGLClear,

  NULL,

  SgXGLDrawMarkers2D,
  SgXGLDrawLine2D,
  NULL,
  SgXGLDrawPolyline2D,
  SgXGLDrawPolygon2D,
  NULL,
  NULL,
  NULL,
  SgXGLDrawAnnot2D,
  NULL,

  SgXGLDrawMarkers,
  SgXGLDrawLine,
  NULL,
  SgXGLDrawPolyline,
  SgXGLDrawPolygon,
  NULL,
  NULL,
  SgXGLDrawAnnot,
  NULL,
  NULL,
  NULL,

  NULL,
  SgXGLDrawTriMesh,
  SgXGLDrawColorTriMesh,
  SgXGLDrawStrip,
  SgXGLDrawColorStrip,
  NULL,

  NULL,
  SgXGLSetBuffer,
  NULL,
  SgXGLEndFrame,
  NULL,

  SgXGLSetFeature,

  SgXGLSetColor,
  SgXGLSetMatProp,
  NULL,
  NULL,
  SgXGLSetLineWidth,
  SgXGLSetLineStyle,
  SgXGLSetShadeModel,
  SgXGLSetFogMode,
  SgXGLSetFogPar,
  NULL,
  NULL,
  NULL,

  NULL,

  SgXGLRotateX,
  SgXGLRotateY,
  SgXGLRotateZ,
  SgXGLTranslate,
  SgXGLScale,
  SgXGLSetMatrix,
  SgXGLMultMatrix,
  SgXGLPushMatrix,
  SgXGLPopMatrix,

  NULL,
  NULL,

  SgXGLSetProjection,
  SgXGLSetViewPoint,
  SgXGLSetViewAngle,
  SgXGLSetNearPlane,
  SgXGLSetFarPlane,
  SgXGLUpdateView,
  SgXGLSetViewport,

  NULL,

  SgXGLSetLight,
  SgXGLSetLightPosition
};

void
SgXGLSetDev(void)
{
  CurrSgDevP = &SgXGLDev;
}
