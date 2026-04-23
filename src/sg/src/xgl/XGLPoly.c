/*
************************************************************************
*
*   XGLPoly.c - XGL polygon drawing
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
*   Date of last modification : 95/11/23
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/xgl/SCCS/s.XGLPoly.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include "xgl_poly.h"

#include <stdio.h>
#include <xgl/xgl.h>

#include "xgl_access.h"

void
SgXGLDrawPolygon2D(float x[][2], int n)
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

  xgl_polygon(XGLGetContext2D(), XGL_FACET_NONE, NULL, NULL, 1, &pl);
}

void
SgXGLDrawPolygon(float x[][3], int n, float nv[3])
{
  Xgl_pt_list pl;
  Xgl_pt_f3d f3d[SG_POLY_SIZE];
  Xgl_facet facet;
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

  facet.normal_facet.normal.x = nv[0];
  facet.normal_facet.normal.y = nv[1];
  facet.normal_facet.normal.z = nv[2];

  xgl_polygon(XGLGetContext3D(), XGL_FACET_NORMAL, &facet, NULL, 1, &pl);
}
