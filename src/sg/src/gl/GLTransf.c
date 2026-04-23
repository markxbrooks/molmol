/*
************************************************************************
*
*   GLTransf.c - GL transformations
*
*   Copyright (c) 1994
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
*   Date of last modification : 94/06/02
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/gl/SCCS/s.GLTransf.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include "gl_transf.h"

#include <gl/gl.h>
#include "gl_util.h"

void
SgGLRotateX(float ang)
{
  rotate(Rad2GLAng(ang), 'x');
}

void
SgGLRotateY(float ang)
{
  rotate(Rad2GLAng(ang), 'y');
}

void
SgGLRotateZ(float ang)
{
  rotate(Rad2GLAng(ang), 'z');
}

void
SgGLTranslate(float dx[3])
{
  translate(dx[0], dx[1], dx[2]);
}

void
SgGLScale(float sx, float sy, float sz)
{
  scale(sx, sy, sz);
}

void
SgGLSetMatrix(float m[4][4])
{
  loadmatrix(m);
}

void
SgGLMultMatrix(float m[4][4])
{
  multmatrix(m);
}

void
SgGLPushMatrix(void)
{
  pushmatrix();
}

void
SgGLPopMatrix(void)
{
  popmatrix();
}
