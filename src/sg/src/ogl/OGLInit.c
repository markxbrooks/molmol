/*
************************************************************************
*
*   OGLInit.c - init OpenGL device
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/ogl/SCCS/s.OGLInit.c
*   SCCS identification       : 1.6
*
************************************************************************
*/

#include "ogl_init.h"

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

SgRes
SgOGLInit(int *argcP, char *argv[])
{
  GLfloat v[4];

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_NORMALIZE);
  glMatrixMode(GL_MODELVIEW);
  glDrawBuffer(GL_BACK);

  /* Amount of specular reflectance is set by modifying the
     specular term of the light source, we keep the material
     property constant. */
  v[0] = 1.0f;
  v[1] = 1.0f;
  v[2] = 1.0f;
  v[3] = 1.0f;
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, v);

  return SG_RES_OK;
}

SgRes
SgOGLCleanup(void)
{
  return SG_RES_OK;
}
