/*
************************************************************************
*
*   GLPoly.c - GL polygon drawing
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
*   Date of last modification : 94/06/02
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/gl/SCCS/s.GLPoly.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include "gl_poly.h"

#include <gl/gl.h>
#include "gl_util.h"

void
SgGLDrawPolygon2D(float x[][2], int n)
{
  int i;

  GLStart2D();
  bgnpolygon();
  for (i = 0; i < n; i++)
    v2f(x[i]);
  endpolygon();
  GLEnd2D();
}

void
SgGLDrawPolygon(float x[][3], int n, float nv[3])
{
  int i;

  bgnpolygon();
  n3f(nv);
  for (i = 0; i < n; i++)
    v3f(x[i]);
  endpolygon();
}

void
SgGLSetShadeModel(SgShadeModel model)
{
  switch (model) {
    case SG_SHADE_FLAT:
      shademodel(FLAT);
      break;
    case SG_SHADE_GOURAUD:
    case SG_SHADE_PHONG:
      shademodel(GOURAUD);
      break;
  }
}
