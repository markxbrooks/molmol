/*
************************************************************************
*
*   XGLMarker.c - XGL marker drawing
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
*   Date of last modification : 95/01/12
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/xgl/SCCS/s.XGLMarker.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include "xgl_marker.h"

#include <stdio.h>
#include <xgl/xgl.h>

#include <sg_types.h>
#include "xgl_access.h"

void
SgXGLDrawMarkers2D(float x[][2], int n)
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

  xgl_multimarker(XGLGetContext2D(), &pl);
}

void
SgXGLDrawMarkers(float x[][3], int n)
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

  xgl_multimarker(XGLGetContext3D(), &pl);
}
