/*
************************************************************************
*
*   OGLPoly.c - OpenGL polygon drawing
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
*   Date of last modification : 97/02/17
*   Pathname of SCCS file     : /local/home/kor/molmol/sg/src/ogl/SCCS/s.OGLPoly.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include "ogl_poly.h"

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

#include "ogl_util.h"

void
SgOGLDrawPolygon2D(float x[][2], int n)
{
  int i;

  OGLStart2D();
  glBegin(GL_POLYGON);
  for (i = 0; i < n; i++)
    glVertex2fv(x[i]);
  glEnd();
  OGLEnd2D();
}

void
SgOGLDrawPolygon(float x[][3], int n, float nv[3])
{
  int i;

  glBegin(GL_POLYGON);
  glNormal3fv(nv);
  for (i = 0; i < n; i++)
    glVertex3fv(x[i]);
  glEnd();
}

void
SgOGLSetShadeModel(SgShadeModel model)
{
  switch (model) {
    case SG_SHADE_FLAT:
      glShadeModel(GL_FLAT);
      break;
    case SG_SHADE_GOURAUD:
    case SG_SHADE_PHONG:
      glShadeModel(GL_SMOOTH);
      break;
  }
}
