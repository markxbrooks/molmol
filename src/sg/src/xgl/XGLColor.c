/*
************************************************************************
*
*   XGLColor.c - XGL color setting
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
*   Date of last modification : 95/05/21
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/xgl/SCCS/s.XGLColor.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include "xgl_color.h"

#include <stdio.h>
#include <xgl/xgl.h>

#include "xgl_access.h"

void
SgXGLSetColor(float r, float g, float b, float a, BOOL fg)
{
  Xgl_color col;

  col.rgb.r = r;
  col.rgb.g = g;
  col.rgb.b = b;

  if (fg) {
    xgl_object_set(XGLGetContext2D(),
	XGL_CTX_MARKER_COLOR, &col,
	XGL_CTX_LINE_COLOR, &col,
	XGL_CTX_STEXT_COLOR, &col,
	XGL_CTX_SURF_FRONT_COLOR, &col,
	NULL);
    xgl_object_set(XGLGetContext3D(),
	XGL_CTX_MARKER_COLOR, &col,
	XGL_CTX_LINE_COLOR, &col,
	XGL_CTX_STEXT_COLOR, &col,
	XGL_CTX_SURF_FRONT_COLOR, &col,
	XGL_3D_CTX_SURF_FRONT_SPECULAR_COLOR, &col,
	XGL_3D_CTX_SURF_FRONT_TRANSP, 1.0 - a,
	NULL);
    if (a < 1.0)
      xgl_object_set(XGLGetContext3D(),
	  XGL_3D_CTX_SURF_TRANSP_METHOD, XGL_TRANSP_BLENDED,
	  NULL);
    else
      xgl_object_set(XGLGetContext3D(),
	  XGL_3D_CTX_SURF_TRANSP_METHOD, XGL_TRANSP_NONE,
	  NULL);
  } else {
    xgl_object_set(XGLGetContext3D(),
	XGL_CTX_BACKGROUND_COLOR, &col,
	XGL_3D_CTX_DEPTH_CUE_COLOR, &col,
	NULL);
  }
}

void
SgXGLSetMatProp(SgMatProp prop, float val)
{
  switch (prop) {
    case SG_MAT_AMBIENT_FACTOR:
      xgl_object_set(XGLGetContext3D(),
	  XGL_3D_CTX_SURF_FRONT_AMBIENT, val,
	  NULL);
      break;
    case SG_MAT_DIFFUSE_FACTOR:
      xgl_object_set(XGLGetContext3D(),
	  XGL_3D_CTX_SURF_FRONT_DIFFUSE, val,
	  NULL);
      break;
    case SG_MAT_SPECULAR_FACTOR:
      xgl_object_set(XGLGetContext3D(),
	  XGL_3D_CTX_SURF_FRONT_SPECULAR, val,
	  NULL);
      if (val > 0.0)
	xgl_object_set(XGLGetContext3D(),
	    XGL_3D_CTX_SURF_FRONT_LIGHT_COMPONENT,
	        XGL_LIGHT_ENABLE_COMP_AMBIENT |
		XGL_LIGHT_ENABLE_COMP_DIFFUSE |
		XGL_LIGHT_ENABLE_COMP_SPECULAR,
	    NULL);
      else
	xgl_object_set(XGLGetContext3D(),
	    XGL_3D_CTX_SURF_FRONT_LIGHT_COMPONENT,
	        XGL_LIGHT_ENABLE_COMP_AMBIENT |
		XGL_LIGHT_ENABLE_COMP_DIFFUSE,
	    NULL);
      break;
    case SG_MAT_SHININESS:
      xgl_object_set(XGLGetContext3D(),
	  XGL_3D_CTX_SURF_FRONT_SPECULAR_POWER, val,
	  NULL);
      break;
  }
}
