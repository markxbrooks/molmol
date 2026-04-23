/*
************************************************************************
*
*   XGLMesh.c - XGL mesh drawing
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
*   Date of last modification : 95/12/10
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/xgl/SCCS/s.XGLMesh.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include "xgl_mesh.h"

#include <stdio.h>
#include <xgl/xgl.h>

#include "xgl_access.h"

#define LIST_SIZE 100

void
SgXGLDrawTriMesh(float x[][3], float nv[][3], int n)
{
  Xgl_pt_list pl;
  Xgl_pt_normal_f3d f3d[LIST_SIZE];
  int inInd, outInd;

  pl.pt_type = XGL_PT_NORMAL_F3D;
  pl.bbox = NULL;
  pl.num_pts = LIST_SIZE;
  pl.pts.normal_f3d = f3d;

  inInd = 0;
  outInd = 0;
  while (inInd < n) {
    f3d[outInd].x = x[inInd][0];
    f3d[outInd].y = x[inInd][1];
    f3d[outInd].z = x[inInd][2];
    f3d[outInd].normal.x = nv[inInd][0];
    f3d[outInd].normal.y = nv[inInd][1];
    f3d[outInd].normal.z = nv[inInd][2];
    outInd++;

    if (outInd == LIST_SIZE) {
      xgl_triangle_strip(XGLGetContext3D(), NULL, &pl);
      outInd = 0;
      inInd -= 2;
    }

    inInd++;
  }

  if (outInd > 0) {
    pl.num_pts = outInd;
    xgl_triangle_strip(XGLGetContext3D(), NULL, &pl);
  }
}

void
SgXGLDrawColorTriMesh(float x[][3], float nv[][3], float col[][3], int n)
{
  Xgl_pt_list pl;
  Xgl_pt_color_normal_f3d f3d[LIST_SIZE];
  int inInd, outInd;

  pl.pt_type = XGL_PT_COLOR_NORMAL_F3D;
  pl.bbox = NULL;
  pl.num_pts = LIST_SIZE;
  pl.pts.color_normal_f3d = f3d;

  inInd = 0;
  outInd = 0;
  while (inInd < n) {
    f3d[outInd].x = x[inInd][0];
    f3d[outInd].y = x[inInd][1];
    f3d[outInd].z = x[inInd][2];
    f3d[outInd].color.rgb.r = col[inInd][0];
    f3d[outInd].color.rgb.g = col[inInd][1];
    f3d[outInd].color.rgb.b = col[inInd][2];
    f3d[outInd].normal.x = nv[inInd][0];
    f3d[outInd].normal.y = nv[inInd][1];
    f3d[outInd].normal.z = nv[inInd][2];
    outInd++;

    if (outInd == LIST_SIZE) {
      xgl_triangle_strip(XGLGetContext3D(), NULL, &pl);
      outInd = 0;
      inInd -= 2;
    }

    inInd++;
  }

  if (outInd > 0) {
    pl.num_pts = outInd;
    xgl_triangle_strip(XGLGetContext3D(), NULL, &pl);
  }
}

void
SgXGLDrawStrip(float x1[][3], float x2[][3],
    float nv1[][3], float nv2[][3], int n)
{
  Xgl_pt_list pl;
  Xgl_pt_normal_f3d f3d[LIST_SIZE];
  int inInd, outInd;

  if (nv2 == NULL)
    nv2 = nv1;

  pl.pt_type = XGL_PT_NORMAL_F3D;
  pl.bbox = NULL;
  pl.num_pts = LIST_SIZE;
  pl.pts.normal_f3d = f3d;

  inInd = 0;
  outInd = 0;
  while (inInd < n) {
    f3d[outInd].x = x1[inInd][0];
    f3d[outInd].y = x1[inInd][1];
    f3d[outInd].z = x1[inInd][2];
    f3d[outInd].normal.x = nv1[inInd][0];
    f3d[outInd].normal.y = nv1[inInd][1];
    f3d[outInd].normal.z = nv1[inInd][2];
    outInd++;

    f3d[outInd].x = x2[inInd][0];
    f3d[outInd].y = x2[inInd][1];
    f3d[outInd].z = x2[inInd][2];
    f3d[outInd].normal.x = nv2[inInd][0];
    f3d[outInd].normal.y = nv2[inInd][1];
    f3d[outInd].normal.z = nv2[inInd][2];
    outInd++;

    if (outInd == LIST_SIZE) {
      xgl_triangle_strip(XGLGetContext3D(), NULL, &pl);
      outInd = 0;
      inInd--;
    }

    inInd++;
  }

  if (outInd > 0) {
    pl.num_pts = outInd;
    xgl_triangle_strip(XGLGetContext3D(), NULL, &pl);
  }
}

void
SgXGLDrawColorStrip(float x1[][3], float x2[][3],
    float nv1[][3], float nv2[][3],
    float col1[][3], float col2[][3], int n)
{
  Xgl_pt_list pl;
  Xgl_pt_color_normal_f3d f3d[LIST_SIZE];
  int inInd, outInd;

  if (nv2 == NULL)
    nv2 = nv1;

  if (col2 == NULL)
    col2 = col1;

  pl.pt_type = XGL_PT_COLOR_NORMAL_F3D;
  pl.bbox = NULL;
  pl.num_pts = LIST_SIZE;
  pl.pts.color_normal_f3d = f3d;

  inInd = 0;
  outInd = 0;
  while (inInd < n) {
    f3d[outInd].x = x1[inInd][0];
    f3d[outInd].y = x1[inInd][1];
    f3d[outInd].z = x1[inInd][2];
    f3d[outInd].color.rgb.r = col1[inInd][0];
    f3d[outInd].color.rgb.g = col1[inInd][1];
    f3d[outInd].color.rgb.b = col1[inInd][2];
    f3d[outInd].normal.x = nv1[inInd][0];
    f3d[outInd].normal.y = nv1[inInd][1];
    f3d[outInd].normal.z = nv1[inInd][2];
    outInd++;

    f3d[outInd].x = x2[inInd][0];
    f3d[outInd].y = x2[inInd][1];
    f3d[outInd].z = x2[inInd][2];
    f3d[outInd].color.rgb.r = col2[inInd][0];
    f3d[outInd].color.rgb.g = col2[inInd][1];
    f3d[outInd].color.rgb.b = col2[inInd][2];
    f3d[outInd].normal.x = nv2[inInd][0];
    f3d[outInd].normal.y = nv2[inInd][1];
    f3d[outInd].normal.z = nv2[inInd][2];
    outInd++;

    if (outInd == LIST_SIZE) {
      xgl_triangle_strip(XGLGetContext3D(), NULL, &pl);
      outInd = 0;
      inInd--;
    }

    inInd++;
  }

  if (outInd > 0) {
    pl.num_pts = outInd;
    xgl_triangle_strip(XGLGetContext3D(), NULL, &pl);
  }
}
