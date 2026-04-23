/*
************************************************************************
*
*   XGLInit.c - init XGL device
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
*   Date of last modification : 96/01/18
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/xgl/SCCS/s.XGLInit.c
*   SCCS identification       : 1.10
*
************************************************************************
*/

#include "xgl_init.h"

#include <stdio.h>
#include <xgl/xgl.h>

#include <sg_xgl_p.h>
#include "xgl_access.h"

static Xgl_sys_state SysState;
static Xgl_object Context2D = NULL, Context3D = NULL;
static Xgl_object Raster;

SgRes
SgXGLInit(int *argcP, char *argv[])
{
  /* xgl_open is called in IO*XGLInit */

  Context2D = xgl_object_create(SysState, XGL_2D_CTX, NULL,
      XGL_CTX_DEVICE, Raster,
      XGL_CTX_VDC_MAP, XGL_VDC_MAP_ALL,
      XGL_CTX_VDC_ORIENTATION, XGL_Y_UP_Z_TOWARD,
      XGL_CTX_MARKER, xgl_marker_dot,
      XGL_CTX_LINE_CAP, XGL_CAP_ROUND,
      XGL_CTX_LINE_JOIN, XGL_JOIN_ROUND,
      NULL);

  Context3D = xgl_object_create(SysState, XGL_3D_CTX, NULL,
      XGL_CTX_DEVICE, Raster,
      XGL_CTX_VDC_MAP, XGL_VDC_MAP_ALL,
      XGL_CTX_CLIP_PLANES, XGL_CLIP_ZMIN | XGL_CLIP_ZMAX,
      XGL_CTX_VDC_ORIENTATION, XGL_Y_UP_Z_TOWARD,
      XGL_3D_CTX_Z_BUFFER_COMP_METHOD, XGL_Z_COMP_GREATER_THAN_OR_EQUAL,
      XGL_CTX_MODEL_TRANS_STACK_SIZE, 24,
      XGL_CTX_MARKER, xgl_marker_dot,
      XGL_CTX_LINE_CAP, XGL_CAP_ROUND,
      XGL_CTX_LINE_JOIN, XGL_JOIN_ROUND,
      XGL_CTX_ATEXT_CHAR_HEIGHT, 0.03,
      XGL_3D_CTX_SURF_TRANSP_BLEND_EQ, XGL_BLEND_ARBITRARY_BG,
      NULL);

  return SG_RES_OK;
}

void
XGLSetRaster(Xgl_object obj)
{
  Raster = obj;

  if (Context2D != NULL)
    xgl_object_set(Context2D,
	XGL_CTX_DEVICE, Raster,
	NULL);

  if (Context3D != NULL)
    xgl_object_set(Context3D,
	XGL_CTX_DEVICE, Raster,
	NULL);
}

SgRes
SgXGLCleanup(void)
{
  xgl_close(SysState);

  return SG_RES_OK;
}

void
XGLSetSysState(Xgl_sys_state sysState)
{
  SysState = sysState;

#if 0
  xgl_object_set(SysState,
      XGL_SYS_ST_ERROR_DETECTION, TRUE,
      NULL);
#endif
}

Xgl_sys_state
XGLGetSysState(void)
{
  return SysState;
}

Xgl_object
XGLGetContext2D(void)
{
  return Context2D;
}

Xgl_object
XGLGetContext3D(void)
{
  return Context3D;
}
