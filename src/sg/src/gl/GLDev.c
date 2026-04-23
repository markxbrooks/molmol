/*
************************************************************************
*
*   GLDev.c - GL device
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/gl/SCCS/s.GLDev.c
*   SCCS identification       : 1.16
*
************************************************************************
*/

#include <stdio.h>

#include <sg_dev.h>

#include "gl_init.h"
#include "gl_view.h"
#include "gl_obj.h"
#include "gl_clear.h"
#include "gl_marker.h"
#include "gl_line.h"
#include "gl_poly.h"
#include "gl_text.h"
#include "gl_mesh.h"
#include "gl_feature.h"
#include "gl_color.h"
#include "gl_transf.h"
#include "gl_light.h"

static SgDev SgGLDev = {
  FALSE,

  VPH_NORMAL,
  3,

  SgGLInit,
  SgGLCleanup,

  NULL,
  NULL,
  NULL,
  NULL,

  SgGLClear,

  NULL,

  SgGLDrawMarkers2D,
  SgGLDrawLine2D,
  NULL,
  SgGLDrawPolyline2D,
  SgGLDrawPolygon2D,
  NULL,
  NULL,
  NULL,
  SgGLDrawAnnot2D,
  NULL,

  SgGLDrawMarkers,
  SgGLDrawLine,
  NULL,
  SgGLDrawPolyline,
  SgGLDrawPolygon,
  NULL,
  NULL,
  SgGLDrawAnnot,
  NULL,
  NULL,
  NULL,

  NULL,
  SgGLDrawTriMesh,
  SgGLDrawColorTriMesh,
  SgGLDrawStrip,
  SgGLDrawColorStrip,
  NULL,

  NULL,
  SgGLSetBuffer,
  SgGLFlushFrame,
  SgGLEndFrame,
  NULL,

  SgGLSetFeature,

  SgGLSetColor,
  SgGLSetMatProp,
  NULL,
  NULL,
  SgGLSetLineWidth,
  SgGLSetLineStyle,
  SgGLSetShadeModel,
  SgGLSetFogMode,
  SgGLSetFogPar,
  NULL,
  NULL,
  NULL,

  NULL,

  SgGLRotateX,
  SgGLRotateY,
  SgGLRotateZ,
  SgGLTranslate,
  SgGLScale,
  SgGLSetMatrix,
  SgGLMultMatrix,
  SgGLPushMatrix,
  SgGLPopMatrix,

  SgGLSetStereo,
  SgGLSetEye,

  SgGLSetProjection,
  SgGLSetViewPoint,
  SgGLSetViewAngle,
  SgGLSetNearPlane,
  SgGLSetFarPlane,
  SgGLUpdateView,
  SgGLSetViewport,

  SgGLConvCoord,

  SgGLSetLight,
  SgGLSetLightPosition
};

static SgDev SgGLDDev = {
  FALSE,

  VPH_NORMAL,
  3,

  SgGLInit,
  SgGLCleanup,

  SgGLBeginObj,
  SgGLEndObj,
  SgGLReplaceObj,
  SgGLDestroyObj,

  SgGLClear,

  SgGLDrawObj,

  SgGLDrawMarkers2D,
  SgGLDrawLine2D,
  NULL,
  SgGLDrawPolyline2D,
  SgGLDrawPolygon2D,
  NULL,
  NULL,
  NULL,
  SgGLDrawAnnot2D,
  NULL,

  SgGLDrawMarkers,
  SgGLDrawLine,
  NULL,
  SgGLDrawPolyline,
  SgGLDrawPolygon,
  NULL,
  NULL,
  SgGLDrawAnnot,
  NULL,
  NULL,
  NULL,

  NULL,
  SgGLDrawTriMesh,
  SgGLDrawColorTriMesh,
  SgGLDrawStrip,
  SgGLDrawColorStrip,
  NULL,

  NULL,
  SgGLSetBuffer,
  SgGLFlushFrame,
  SgGLEndFrame,
  NULL,

  SgGLSetFeature,

  SgGLSetColor,
  SgGLSetMatProp,
  NULL,
  NULL,
  SgGLSetLineWidth,
  SgGLSetLineStyle,
  SgGLSetShadeModel,
  SgGLSetFogMode,
  SgGLSetFogPar,
  NULL,
  NULL,
  NULL,

  NULL,

  SgGLRotateX,
  SgGLRotateY,
  SgGLRotateZ,
  SgGLTranslate,
  SgGLScale,
  SgGLSetMatrix,
  SgGLMultMatrix,
  SgGLPushMatrix,
  SgGLPopMatrix,

  SgGLSetStereo,
  SgGLSetEye,

  SgGLSetProjection,
  SgGLSetViewPoint,
  SgGLSetViewAngle,
  SgGLSetNearPlane,
  SgGLSetFarPlane,
  SgGLUpdateView,
  SgGLSetViewport,

  SgGLConvCoord,

  SgGLSetLight,
  SgGLSetLightPosition
};

void
SgGLSetDev(void)
{
  CurrSgDevP = &SgGLDev;
}

void
SgGLDSetDev(void)
{
  CurrSgDevP = &SgGLDDev;
}
