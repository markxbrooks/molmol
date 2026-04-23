/*
************************************************************************
*
*   OGLClear.c - OpenGL clear window
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
*   Pathname of SCCS file     : /local/home/kor/molmol/sg/src/ogl/SCCS/s.OGLClear.c
*   SCCS identification       : 1.5
*
************************************************************************
*/

#include "ogl_clear.h"

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

void
SgOGLClear(void)
{
  GLint currBuf;

  glGetIntegerv(GL_DRAW_BUFFER, &currBuf);
  if (currBuf != GL_BACK && currBuf != GL_FRONT) {
    /* clear left AND right buffer in stereo mode! */
    glDrawBuffer(GL_BACK);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawBuffer(currBuf);
  } else {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }
}
