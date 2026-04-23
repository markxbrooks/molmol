/*
************************************************************************
*
*   OGLUtil.c - OpenGL utility functions
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/ogl/SCCS/s.OGLUtil.c
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include "ogl_util.h"

#include <math.h>
#include <values.h>
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

GLfloat
Rad2OGLAng(float ang)
{
  /* argument is in rad, OpenGL angles are in degrees */
  return ang * 180.0f / (float) M_PI;
}
