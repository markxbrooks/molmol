/*
************************************************************************
*
*   XGLView.c - XGL viewing parameters
*
*   Copyright (c) 1994-97
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
*   Date of last modification : 97/07/17
*   Pathname of SCCS file     : /local/home/kor/molmol/sg/src/xgl/SCCS/s.XGLView.c
*   SCCS identification       : 1.10
*
************************************************************************
*/

#include "xgl_view.h"
#include "xgl_proj.h"

#include <stdio.h>
#include <math.h>
#include <xgl/xgl.h>

#include <mat_vec.h>
#include <sg_get.h>
#include "xgl_access.h"

static SgProjection Proj;
static float VX, VY, VZ;
static float ViewAng;
static float Near, Far;
static float VpX, VpY, VpW, VpH;
static float AspRat;
static Xgl_matrix_f3d ViewM, ProjM;
static Xgl_trans ViewT, ProjT;
static BOOL ViewUpToDate = FALSE, ProjUpToDate = FALSE;

static void
updateView(void)
{
  Mat4 m;
  float dx, dy, dz;
  float a, axz;
  float c, s;

  dx = - VX;
  dy = - VY;
  dz = - VZ;
  a = sqrtf(dx * dx + dy * dy + dz * dz);
  axz = sqrtf(dx * dx + dz * dz);

  Mat4Ident(ViewM);
  c = axz / a;
  s = - dy / a;
  ViewM[1][1] = c;
  ViewM[2][2] = c;
  ViewM[1][2] = s;
  ViewM[2][1] = -s;

  Mat4Ident(m);
  c = - dz / axz;
  s = dx / axz;
  m[0][0] = c;
  m[2][2] = c;
  m[0][2] = -s;
  m[2][0] = s;
  Mat4Mult(ViewM, m);

  Mat4Ident(m);
  m[3][0] = - VX;
  m[3][1] = - VY;
  m[3][2] = - VZ;
  Mat4Mult(ViewM, m);

  if (ViewT == NULL)
    ViewT = xgl_object_create(XGLGetSysState(), XGL_TRANS, NULL, NULL);

  xgl_transform_write_specific(ViewT, ViewM, XGL_TRANS_MEMBER_LENGTH_PRESERV);

  ViewUpToDate = TRUE;
}

void
updateProj(void)
{
  float b, t;

  Mat4Ident(ProjM);
  if (Proj == SG_PROJ_ORTHO) {
    b = SgGetOrthoHeight();
    ProjM[0][0] = 1.0 / b;
    ProjM[1][1] = 1.0 / b;
    ProjM[2][2] = - 1.0 / (Far - Near);
    ProjM[3][2] = - Near / (Far - Near);
    /* for completely mysterious reasons, the sign of the light
       positions has to be inverted with orthogonal projection,
       detailed research didn't reveal any reasons why this happens */
    xgl_object_set(XGLGetContext3D(),
	XGL_3D_CTX_SURF_NORMAL_FLIP, TRUE,
	NULL);
    if (SgGetFeature(SG_FEATURE_BACKFACE))
      xgl_object_set(XGLGetContext3D(),
	  XGL_3D_CTX_SURF_FACE_CULL, XGL_CULL_FRONT,
	  NULL);
  } else {
    t = tanf(ViewAng / 2.0);
    ProjM[0][0] = 1.0 / t;
    ProjM[1][1] = 1.0 / t;
    ProjM[2][2] = - Far / (Far - Near);
    ProjM[2][3] = - 1.0;
    ProjM[3][2] = - Far * Near / (Far - Near);
    ProjM[3][3] = 0.0;
    xgl_object_set(XGLGetContext3D(),
	XGL_3D_CTX_SURF_NORMAL_FLIP, FALSE,
	NULL);
    if (SgGetFeature(SG_FEATURE_BACKFACE))
      xgl_object_set(XGLGetContext3D(),
	  XGL_3D_CTX_SURF_FACE_CULL, XGL_CULL_BACK,
	  NULL);
  }

  if (ProjT == NULL)
    ProjT = xgl_object_create(XGLGetSysState(), XGL_TRANS, NULL, NULL);

  xgl_transform_write_specific(ProjT, ProjM, XGL_TRANS_MEMBER_LIM_PERSPECTIVE);

  ProjUpToDate = TRUE;
}

#if DEBUG
static void
printTrans(char *title, Xgl_trans trans)
{
  Xgl_matrix_f3d mat;
  int i, k;

  xgl_transform_read(trans, mat);
  (void) printf("%s\n", title);
  for (i = 0; i < 4; i++) {
    for (k = 0; k < 4; k++)
      (void) printf("%7.3f ", mat[i][k]);
    (void) printf("\n");
  }
  (void) printf("%7.3f\n",
      (8.99 * mat[2][2] + mat[3][2]) / (8.99 * mat[2][3] + mat[3][3]));
  (void) printf("\n");
}
#endif

void
SgXGLUpdateView(void)
{
  Xgl_trans trans;

  if (! ViewUpToDate)
    updateView();
  if (! ProjUpToDate)
    updateProj();

  xgl_object_get(XGLGetContext3D(), XGL_CTX_VIEW_TRANS, &trans);
  xgl_transform_multiply(trans, ViewT, ProjT);
}

void
SgXGLSetProjection(SgProjection proj)
{
  Proj = proj;
  ProjUpToDate = FALSE;
}

SgProjection
XGLGetProjection(void)
{
  return Proj;
}

void
SgXGLSetViewPoint(float x[3])
{
  VX = x[0];
  VY = x[1];
  VZ = x[2];
  ViewUpToDate = FALSE;
}

void
SgXGLSetViewAngle(float ang)
{
  ViewAng = ang;
  ProjUpToDate = FALSE;
}

void
SgXGLSetNearPlane(float d, BOOL local)
{
  if (local)
    return;

  Near = d;
  ProjUpToDate = FALSE;
}

float
XGLGetNearPlane(void)
{
  return Near;
}

void
SgXGLSetFarPlane(float d, BOOL local)
{
  if (local)
    return;

  Far = d;
  ProjUpToDate = FALSE;
}

float
XGLGetFarPlane(void)
{
  return Far;
}

static void
setViewport(void)
{
  Xgl_win_ras ras;
  Xgl_obj_type rasType;
  Xgl_pt_d3d max;
  Xgl_bounds_d2d bounds2D;
  Xgl_bounds_d3d bounds3D;

  xgl_object_get(XGLGetContext3D(), XGL_CTX_DEVICE, &ras);
  xgl_object_get(ras, XGL_OBJ_TYPE, &rasType);
  if (rasType == XGL_WIN_RAS)
    xgl_window_raster_resize(ras);

  xgl_object_get(ras, XGL_DEV_MAXIMUM_COORDINATES, &max);

  bounds2D.xmin = - AspRat * (2.0 * VpX + VpW) / VpW;
  bounds2D.xmax = AspRat * (2.0 * (max.x - VpX) - VpW) / VpW;
  bounds2D.ymin = - (2.0 * VpY + VpH) / VpH;
  bounds2D.ymax = (2.0 * (max.y - VpY) - VpH) / VpH;

  xgl_object_set(XGLGetContext2D(),
      XGL_CTX_VDC_WINDOW, &bounds2D,
      NULL);

  bounds3D.xmin = bounds2D.xmin;
  bounds3D.xmax = bounds2D.xmax;
  bounds3D.ymin = bounds2D.ymin;
  bounds3D.ymax = bounds2D.ymax;
  bounds3D.zmin = 0.0;
  bounds3D.zmax = 1.0;

  xgl_object_set(XGLGetContext3D(),
      XGL_CTX_VDC_WINDOW, &bounds3D,
      NULL);
}

void
SgXGLSetViewport(float x, float y, float width, float height)
{
  VpX = x;
  VpY = y;
  VpW = width;
  VpH = height;
  AspRat = width / height;

  setViewport();
}

void
SgXGLSetBuffer(SgBuffer buf)
{
  Xgl_win_ras ras;
  Xgl_usgn32 displayBuf, drawBuf, bufNo;

  xgl_object_get(XGLGetContext3D(), XGL_CTX_DEVICE, &ras);
  xgl_object_get(ras, XGL_WIN_RAS_BUF_DISPLAY, &displayBuf);

  if (buf == SG_BUF_FRONT) {
    drawBuf = displayBuf;
  } else {
    xgl_object_get(ras, XGL_WIN_RAS_BUFFERS_ALLOCATED, &bufNo);
    if (bufNo > 1)
      drawBuf = 1 - displayBuf;
    else
      drawBuf = 0;
  }

  xgl_object_set(ras,
      XGL_WIN_RAS_BUF_DRAW, drawBuf,
      NULL);
}

void
SgXGLEndFrame(void)
{
  Xgl_ctx ctx = XGLGetContext3D();
  Xgl_ctx_new_frame_action oldAction;

  setViewport();  /* may have been changed for stereo display */

  xgl_object_get(ctx, XGL_CTX_NEW_FRAME_ACTION, &oldAction);
  xgl_object_set(ctx,
      XGL_CTX_NEW_FRAME_ACTION, XGL_CTX_NEW_FRAME_SWITCH_BUFFER,
      NULL);
  xgl_context_new_frame(ctx);
  xgl_object_set(ctx,
      XGL_CTX_NEW_FRAME_ACTION, oldAction,
      NULL);
}
