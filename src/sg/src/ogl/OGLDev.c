/*
************************************************************************
*
*   OGLDev.c - OpenGL device
*
*   Copyright (c) 1994-2001
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/ogl/SCCS/s.OGLDev.c
*   SCCS identification       : 1.16
*
************************************************************************
*/

#include <stdio.h>

#include <sg_dev.h>

#include "ogl_init.h"
#include "ogl_view.h"
#include "ogl_obj.h"
#include "ogl_clear.h"
#include "ogl_marker.h"
#include "ogl_line.h"
#include "ogl_poly.h"
#include "ogl_text.h"
#include "ogl_mesh.h"
#include "ogl_feature.h"
#include "ogl_color.h"
#include "ogl_transf.h"
#include "ogl_stereo.h"
#include "ogl_light.h"

#define USE_VECTOR_FONTS 1

static SgDev SgOGLDev = {
  FALSE,

  VPH_NORMAL,
  3,

  SgOGLInit,
  SgOGLCleanup,

  NULL,
  NULL,
  NULL,
  NULL,

  SgOGLClear,

  NULL,

  SgOGLDrawMarkers2D,
  SgOGLDrawLine2D,
  NULL,
  SgOGLDrawPolyline2D,
  SgOGLDrawPolygon2D,
  NULL,
  NULL,
  NULL,
  SgOGLDrawAnnot2D,
  NULL,

  SgOGLDrawMarkers,
  SgOGLDrawLine,
  NULL,
  SgOGLDrawPolyline,
  SgOGLDrawPolygon,
  NULL,
  NULL,
  SgOGLDrawAnnot,
  NULL,
  NULL,
  NULL,

  NULL,
  SgOGLDrawTriMesh,
  SgOGLDrawColorTriMesh,
  SgOGLDrawStrip,
  SgOGLDrawColorStrip,
  NULL,

  NULL,
  SgOGLSetBuffer,
  SgOGLFlushFrame,
  SgOGLEndFrame,
  NULL,

  SgOGLSetFeature,

  SgOGLSetColor,
  SgOGLSetMatProp,
  NULL,
  NULL,
  SgOGLSetLineWidth,
  SgOGLSetLineStyle,
  SgOGLSetShadeModel,
  SgOGLSetFogMode,
  SgOGLSetFogPar,
  NULL,
  NULL,
  NULL,

  NULL,

  SgOGLRotateX,
  SgOGLRotateY,
  SgOGLRotateZ,
  SgOGLTranslate,
  SgOGLScale,
  SgOGLSetMatrix,
  SgOGLMultMatrix,
  SgOGLPushMatrix,
  SgOGLPopMatrix,

  SgOGLSetStereo,
  SgOGLSetEye,

  SgOGLSetProjection,
  SgOGLSetViewPoint,
  SgOGLSetViewAngle,
  SgOGLSetNearPlane,
  SgOGLSetFarPlane,
  SgOGLUpdateView,
  SgOGLSetViewport,

  SgOGLConvCoord,

  SgOGLSetLight,
  SgOGLSetLightPosition
};

static SgDev SgOGLDDev = {
  FALSE,

  VPH_NORMAL,
  3,

  SgOGLInit,
  SgOGLCleanup,

  SgOGLBeginObj,
  SgOGLEndObj,
  SgOGLReplaceObj,
  SgOGLDestroyObj,

  SgOGLClear,

  SgOGLDrawObj,

  SgOGLDrawMarkers2D,
  SgOGLDrawLine2D,
  NULL,
  SgOGLDrawPolyline2D,
  SgOGLDrawPolygon2D,
  NULL,
  NULL,
  NULL,
  SgOGLDrawAnnot2D,
  NULL,

  SgOGLDrawMarkers,
  SgOGLDrawLine,
  NULL,
  SgOGLDrawPolyline,
  SgOGLDrawPolygon,
  NULL,
  NULL,
  SgOGLDrawAnnot,
  NULL,
  NULL,
  NULL,

  NULL,
  SgOGLDrawTriMesh,
  SgOGLDrawColorTriMesh,
  SgOGLDrawStrip,
  SgOGLDrawColorStrip,
  NULL,

  NULL,
  SgOGLSetBuffer,
  SgOGLFlushFrame,
  SgOGLEndFrame,
  NULL,

  SgOGLSetFeature,

  SgOGLSetColor,
  SgOGLSetMatProp,
  NULL,
  NULL,
  SgOGLSetLineWidth,
  SgOGLSetLineStyle,
  SgOGLSetShadeModel,
  SgOGLSetFogMode,
  SgOGLSetFogPar,
  NULL,
  NULL,
  NULL,

  NULL,

  SgOGLRotateX,
  SgOGLRotateY,
  SgOGLRotateZ,
  SgOGLTranslate,
  SgOGLScale,
  SgOGLSetMatrix,
  SgOGLMultMatrix,
  SgOGLPushMatrix,
  SgOGLPopMatrix,

  SgOGLSetStereo,
  SgOGLSetEye,

  SgOGLSetProjection,
  SgOGLSetViewPoint,
  SgOGLSetViewAngle,
  SgOGLSetNearPlane,
  SgOGLSetFarPlane,
  SgOGLUpdateView,
  SgOGLSetViewport,

  SgOGLConvCoord,

  SgOGLSetLight,
  SgOGLSetLightPosition
};

void
SgOGLSetDev(void)
{
  CurrSgDevP = &SgOGLDev;
}

void
SgOGLDSetDev(void)
{
  CurrSgDevP = &SgOGLDDev;
}
