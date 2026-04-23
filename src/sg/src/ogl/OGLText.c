/*
************************************************************************
*
*   OGLText.c - OpenGL text drawing
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
*   Pathname of SCCS file     : /local/home/kor/molmol/sg/src/ogl/SCCS/s.OGLText.c
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include "ogl_text.h"

#include <string.h>
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

#include "ogl_util.h"
#include "ogl_light.h"

void
SgOGLDrawAnnot2D(float x[2], char *str)
{
  OGLStart2D();

  glRasterPos2fv(x);
  glCallLists(strlen(str), GL_UNSIGNED_BYTE, str);

  OGLEnd2D();
}

void
SgOGLDrawAnnot(float x[3], char *str)
{
  glRasterPos3fv(x);
  glCallLists(strlen(str), GL_UNSIGNED_BYTE, str);
}
