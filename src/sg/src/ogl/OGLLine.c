/*
************************************************************************
*
*   OGLLine.c - OpenGL line drawing
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
*   Date of last modification : 01/05/27
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/ogl/SCCS/s.OGLLine.c
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include "ogl_line.h"

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

#include "ogl_util.h"

void
SgOGLDrawLine2D(float x0[2], float x1[2])
{
  OGLStart2D();
  glBegin(GL_LINES);
  glVertex2fv(x0);
  glVertex2fv(x1);
  glEnd();
  OGLEnd2D();
}

void
SgOGLDrawPolyline2D(float x[][2], int n)
{
  int i;

  OGLStart2D();
  glBegin(GL_LINE_STRIP);
  for (i = 0; i < n; i++)
    glVertex2fv(x[i]);
  glEnd();
  OGLEnd2D();
}

void
SgOGLDrawLine(float x0[3], float x1[3])
{
  glBegin(GL_LINES);
  glVertex3fv(x0);
  glVertex3fv(x1);
  glEnd();
}

void
SgOGLDrawPolyline(float x[][3], int n)
{
  int i;

  glBegin(GL_LINE_STRIP);
  for (i = 0; i < n; i++)
    glVertex3fv(x[i]);
  glEnd();
}

void
SgOGLSetLineWidth(float w)
{
  if (w == 0.0f)  /* 0.0 is illegal */
    w = 0.001f;

  glLineWidth(w);
}

void
SgOGLSetLineStyle(SgLineStyle lineStyle)
{
  if (lineStyle == SG_LINE_SOLID) {
    glDisable(GL_LINE_STIPPLE);
  } else {
    glLineStipple(1, 0xF0F0);
    glEnable(GL_LINE_STIPPLE);
  }
}
