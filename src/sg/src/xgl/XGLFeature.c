/*
************************************************************************
*
*   XGLFeature.c - XGL enable/disable features
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
*   Date of last modification : 96/03/07
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/xgl/SCCS/s.XGLFeature.c
*   SCCS identification       : 1.8
*
************************************************************************
*/

#include "xgl_feature.h"

#include <stdio.h>
#include <values.h>
#include <xgl/xgl.h>

#include "xgl_access.h"
#include "xgl_proj.h"

static float FogStart, FogEnd;

void
SgXGLSetFogMode(SgFogMode mode)
{
  float cueVal[2] = {0.0, 1.0};

  if (mode == SG_FOG_MODE_OFF) {
    xgl_object_set(XGLGetContext3D(),
	XGL_3D_CTX_DEPTH_CUE_MODE, XGL_DEPTH_CUE_OFF,
	NULL);
  } else {
    xgl_object_set(XGLGetContext3D(),
	XGL_3D_CTX_DEPTH_CUE_MODE, XGL_DEPTH_CUE_SCALED,
	XGL_3D_CTX_DEPTH_CUE_SCALE_FACTORS, cueVal,
	NULL);
  }
}

void
SgXGLSetFogPar(SgFogPar par, float val)
{
  float near, far;
  double planes[2];

  switch (par) {
    case SG_FOG_START:
      FogStart = val;
      break;
    case SG_FOG_END:
      FogEnd = val;
      break;
    default:
      /* not supported */
      return;
  }

  near = XGLGetNearPlane();
  far = XGLGetFarPlane();
  planes[0] = (near - FogEnd) / (near - far);
  planes[1] = (near - FogStart) / (near - far);

  xgl_object_set(XGLGetContext3D(),
      XGL_3D_CTX_DEPTH_CUE_REF_PLANES, planes,
      NULL);
}

void
SgXGLSetFeature(SgFeature feature, BOOL onOff)
{
  Xgl_hlhsr_data hlhsrData;

  switch (feature) {
    case SG_FEATURE_TRANSPARENCY:
      /* handled in XGLColor.c */
      break;
    case SG_FEATURE_ANTI_ALIASING:
      if (onOff) {
	xgl_object_set(XGLGetContext2D(),
	    XGL_CTX_LINE_AA_BLEND_EQ, XGL_BLEND_ARBITRARY_BG,
	    XGL_CTX_LINE_AA_FILTER_WIDTH, 1,
	    NULL);
	xgl_object_set(XGLGetContext3D(),
	    XGL_CTX_LINE_AA_BLEND_EQ, XGL_BLEND_ARBITRARY_BG,
	    XGL_CTX_LINE_AA_FILTER_WIDTH, 1,
	    NULL);
      } else {
	xgl_object_set(XGLGetContext2D(),
	    XGL_CTX_LINE_AA_BLEND_EQ, XGL_BLEND_NONE,
	    NULL);
	xgl_object_set(XGLGetContext3D(),
	    XGL_CTX_LINE_AA_BLEND_EQ, XGL_BLEND_NONE,
	    NULL);
      }
      break;
    case SG_FEATURE_BACKFACE:
      if (onOff) {
	if (XGLGetProjection() == SG_PROJ_ORTHO)
	  /* see comment in XGLView.c */
	  xgl_object_set(XGLGetContext3D(),
	      XGL_3D_CTX_SURF_FACE_CULL, XGL_CULL_FRONT,
	      NULL);
	else
	  xgl_object_set(XGLGetContext3D(),
	      XGL_3D_CTX_SURF_FACE_CULL, XGL_CULL_BACK,
	      NULL);
      } else {
	xgl_object_set(XGLGetContext3D(),
	    XGL_3D_CTX_SURF_FACE_CULL, XGL_CULL_OFF,
	    NULL);
      }
      break;
    case SG_FEATURE_HIDDEN:
      if (onOff) {
	hlhsrData.z_buffer.z_value = 0.0;
	xgl_object_set(XGLGetContext3D(),
	    XGL_3D_CTX_HLHSR_MODE, XGL_HLHSR_Z_BUFFER,
	    XGL_3D_CTX_HLHSR_DATA, &hlhsrData,
	    XGL_CTX_NEW_FRAME_ACTION,
	        XGL_CTX_NEW_FRAME_CLEAR | XGL_CTX_NEW_FRAME_HLHSR_ACTION,
	    NULL);
      } else {
	xgl_object_set(XGLGetContext3D(),
	    XGL_3D_CTX_HLHSR_MODE, XGL_HLHSR_NONE,
	    XGL_CTX_NEW_FRAME_ACTION, XGL_CTX_NEW_FRAME_CLEAR,
	    NULL);
      }
      break;
  }
}
