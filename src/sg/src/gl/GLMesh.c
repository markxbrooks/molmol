/*
************************************************************************
*
*   GLMesh.c - GL mesh drawing
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
*   Date of last modification : 95/12/14
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/gl/SCCS/s.GLMesh.c
*   SCCS identification       : 1.5
*
************************************************************************
*/

#include "gl_mesh.h"

#include <stdio.h>
#include <gl/gl.h>

#include <sg_get.h>

static float CurrAlpha;

static void
setColor(float c[3])
{
  float ca[4];

  ca[0] = c[0];
  ca[1] = c[1];
  ca[2] = c[2];
  ca[3] = CurrAlpha;
  c4f(ca);
}

void
SgGLDrawTriMesh(float x[][3], float nv[][3], int n)
{
  int i;

  bgntmesh();
  for (i = 0; i < n; i++) {
    n3f(nv[i]);
    v3f(x[i]);
  }
  endtmesh();
}

void
SgGLDrawColorTriMesh(float x[][3], float nv[][3], float col[][3], int n)
{
  int i;

  CurrAlpha = SgGetMatProp(SG_MAT_ALPHA);

  bgntmesh();
  for (i = 0; i < n; i++) {
    setColor(col[i]);
    n3f(nv[i]);
    v3f(x[i]);
  }
  endtmesh();
}

void
SgGLDrawStrip(float x1[][3], float x2[][3],
    float nv1[][3], float nv2[][3], int n)
{
  int i;

  bgntmesh();

  if (nv2 == NULL)
    for (i = 0; i < n; i++) {
      n3f(nv1[i]);
      v3f(x1[i]);
      v3f(x2[i]);
    }
  else
    for (i = 0; i < n; i++) {
      n3f(nv1[i]);
      v3f(x1[i]);
      n3f(nv2[i]);
      v3f(x2[i]);
    }

  endtmesh();
}

void
SgGLDrawColorStrip(float x1[][3], float x2[][3],
    float nv1[][3], float nv2[][3],
    float col1[][3], float col2[][3], int n)
{
  int i;

  CurrAlpha = SgGetMatProp(SG_MAT_ALPHA);

  bgntmesh();
  for (i = 0; i < n; i++) {
    setColor(col1[i]);
    n3f(nv1[i]);
    v3f(x1[i]);
    if (col2 != NULL)
      setColor(col2[i]);
    if (nv2 != NULL)
      n3f(nv2[i]);
    v3f(x2[i]);
  }
  endtmesh();
}
