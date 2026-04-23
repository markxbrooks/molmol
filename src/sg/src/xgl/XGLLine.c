/*
************************************************************************
*
*   XGLLine.c - XGL line drawing
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
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/xgl/SCCS/s.XGLLine.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include "xgl_line.h"

#include <stdio.h>
#include <xgl/xgl.h>

#include "xgl_access.h"

void
SgXGLDrawLine2D(float x0[2], float x1[2])
{
  Xgl_pt_list pl;
  Xgl_pt_f2d f2d[2];

  pl.pt_type = XGL_PT_F2D;
  pl.bbox = NULL;
  pl.num_pts = 2;
  pl.pts.f2d = f2d;

  f2d[0].x = x0[0];
  f2d[0].y = x0[1];
  f2d[1].x = x1[0];
  f2d[1].y = x1[1];

  xgl_multipolyline(XGLGetContext2D(), NULL, 1, &pl);
}

void
SgXGLDrawPolyline2D(float x[][2], int n)
{
  Xgl_pt_list pl;
  Xgl_pt_f2d f2d[SG_POLY_SIZE];
  int i;

  pl.pt_type = XGL_PT_F2D;
  pl.bbox = NULL;
  pl.num_pts = n;
  pl.pts.f2d = f2d;

  for (i = 0; i < n; i++) {
    f2d[i].x = x[i][0];
    f2d[i].y = x[i][1];
  }

  xgl_multipolyline(XGLGetContext2D(), NULL, 1, &pl);
}

void
SgXGLDrawLine(float x0[3], float x1[3])
{
  Xgl_pt_list pl;
  Xgl_pt_f3d f3d[2];

  pl.pt_type = XGL_PT_F3D;
  pl.bbox = NULL;
  pl.num_pts = 2;
  pl.pts.f3d = f3d;

  f3d[0].x = x0[0];
  f3d[0].y = x0[1];
  f3d[0].z = x0[2];
  f3d[1].x = x1[0];
  f3d[1].y = x1[1];
  f3d[1].z = x1[2];

  xgl_multipolyline(XGLGetContext3D(), NULL, 1, &pl);
}

void
SgXGLDrawPolyline(float x[][3], int n)
{
  Xgl_pt_list pl;
  Xgl_pt_f3d f3d[SG_POLY_SIZE];
  int i;

  pl.pt_type = XGL_PT_F3D;
  pl.bbox = NULL;
  pl.num_pts = n;
  pl.pts.f3d = f3d;

  for (i = 0; i < n; i++) {
    f3d[i].x = x[i][0];
    f3d[i].y = x[i][1];
    f3d[i].z = x[i][2];
  }

  xgl_multipolyline(XGLGetContext3D(), NULL, 1, &pl);
}

void
SgXGLSetLineWidth(float w)
{
  xgl_object_set(XGLGetContext2D(),
      XGL_CTX_LINE_WIDTH_SCALE_FACTOR, w,
      NULL);
  xgl_object_set(XGLGetContext3D(),
      XGL_CTX_LINE_WIDTH_SCALE_FACTOR, w,
      NULL);
}

void
SgXGLSetLineStyle(SgLineStyle lineStyle)
{
  if (lineStyle == SG_LINE_SOLID) {
    xgl_object_set(XGLGetContext2D(),
	XGL_CTX_LINE_STYLE, XGL_LINE_SOLID,
	NULL);
    xgl_object_set(XGLGetContext3D(),
	XGL_CTX_LINE_STYLE, XGL_LINE_SOLID,
	NULL);
  } else {
    xgl_object_set(XGLGetContext2D(),
	XGL_CTX_LINE_PATTERN, xgl_lpat_dashed,
	XGL_CTX_LINE_STYLE, XGL_LINE_PATTERNED,
	NULL);
    xgl_object_set(XGLGetContext3D(),
	XGL_CTX_LINE_PATTERN, xgl_lpat_dashed,
	XGL_CTX_LINE_STYLE, XGL_LINE_PATTERNED,
	NULL);
  }
}
