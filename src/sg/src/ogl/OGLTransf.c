/*
************************************************************************
*
*   OGLTransf.c - OpenGL transformations
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/ogl/SCCS/s.OGLTransf.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include "ogl_transf.h"

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

#include "ogl_util.h"

void
SgOGLRotateX(float ang)
{
  glRotatef(Rad2OGLAng(ang), 1.0f, 0.0f, 0.0f);
}

void
SgOGLRotateY(float ang)
{
  glRotatef(Rad2OGLAng(ang), 0.0f, 1.0f, 0.0f);
}

void
SgOGLRotateZ(float ang)
{
  glRotatef(Rad2OGLAng(ang), 0.0f, 0.0f, 1.0f);
}

void
SgOGLTranslate(float dx[3])
{
  glTranslatef(dx[0], dx[1], dx[2]);
}

void
SgOGLScale(float sx, float sy, float sz)
{
  glScalef(sx, sy, sz);
}

void
SgOGLSetMatrix(float m[4][4])
{
  glLoadMatrixf((GLfloat *) m);
}

void
SgOGLMultMatrix(float m[4][4])
{
  glMultMatrixf((GLfloat *) m);
}

void
SgOGLPushMatrix(void)
{
  glPushMatrix();
}

void
SgOGLPopMatrix(void)
{
  glPopMatrix();
}
