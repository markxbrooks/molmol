/*
************************************************************************
*
*   OGLMesh.c - OpenGL mesh drawing
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
*   Pathname of SCCS file     : /local/home/kor/molmol/sg/src/ogl/SCCS/s.OGLMesh.c
*   SCCS identification       : 1.6
*
************************************************************************
*/

#include "ogl_mesh.h"

#include <stdio.h>
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

#include <sg_get.h>

static float CurrAlpha;

static void
setColor(float c[3])
{
  glColor4f(c[0], c[1], c[2], CurrAlpha);
}

void
SgOGLDrawTriMesh(float x[][3], float nv[][3], int n)
{
  int i;

  glBegin(GL_TRIANGLE_STRIP);
  for (i = 0; i < n; i++) {
    glNormal3fv(nv[i]);
    glVertex3fv(x[i]);
  }
  glEnd();
}

void
SgOGLDrawColorTriMesh(float x[][3], float nv[][3], float col[][3], int n)
{
  int i;

  CurrAlpha = SgGetMatProp(SG_MAT_ALPHA);

  glBegin(GL_TRIANGLE_STRIP);
  for (i = 0; i < n; i++) {
    setColor(col[i]);
    glNormal3fv(nv[i]);
    glVertex3fv(x[i]);
  }
  glEnd();
}

void
SgOGLDrawStrip(float x1[][3], float x2[][3],
    float nv1[][3], float nv2[][3], int n)
{
  int i;

  glBegin(GL_TRIANGLE_STRIP);

  if (nv2 == NULL)
    for (i = 0; i < n; i++) {
      glNormal3fv(nv1[i]);
      glVertex3fv(x1[i]);
      glVertex3fv(x2[i]);
    }
  else
    for (i = 0; i < n; i++) {
      glNormal3fv(nv1[i]);
      glVertex3fv(x1[i]);
      glNormal3fv(nv2[i]);
      glVertex3fv(x2[i]);
    }

  glEnd();
}

void
SgOGLDrawColorStrip(float x1[][3], float x2[][3],
    float nv1[][3], float nv2[][3],
    float col1[][3], float col2[][3], int n)
{
  int i;

  CurrAlpha = SgGetMatProp(SG_MAT_ALPHA);

  glBegin(GL_TRIANGLE_STRIP);
  for (i = 0; i < n; i++) {
    setColor(col1[i]);
    glNormal3fv(nv1[i]);
    glVertex3fv(x1[i]);
    if (col2 != NULL)
      setColor(col2[i]);
    if (nv2 != NULL)
      glNormal3fv(nv2[i]);
    glVertex3fv(x2[i]);
  }
  glEnd();
}
