/*
************************************************************************
*
*   OGLColor.c - OpenGL color setting
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/ogl/SCCS/s.OGLColor.c
*   SCCS identification       : 1.5
*
************************************************************************
*/

#include "ogl_color.h"

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

#include <sg.h>
#include "ogl_light.h"

void
SgOGLSetColor(float r, float g, float b, float a, BOOL fg)
{
  float col[4];

  col[0] = r;
  col[1] = g;
  col[2] = b;
  col[3] = a;

  if (fg) {
    OGLSetMaterialColor(col);
    glColor4fv(col);
  } else {
    glClearColor(r, g, b, 1.0f);
    glFogfv(GL_FOG_COLOR, col);
  }
}
