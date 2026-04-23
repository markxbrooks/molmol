/*
************************************************************************
*
*   OGLMarker.c - OpenGL marker drawing
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
*   Pathname of SCCS file     : /local/home/kor/molmol/sg/src/ogl/SCCS/s.OGLMarker.c
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include "ogl_marker.h"

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

#include "ogl_util.h"

void
SgOGLDrawMarkers2D(float x[][2], int n)
{
  int i;

  OGLStart2D();
  glBegin(GL_POINTS);
  for (i = 0; i < n; i++)
    glVertex2fv(x[i]);
  glEnd();
  OGLEnd2D();
}

void
SgOGLDrawMarkers(float x[][3], int n)
{
  int i;

  glBegin(GL_POINTS);
  for (i = 0; i < n; i++)
    glVertex3fv(x[i]);
  glEnd();
}
