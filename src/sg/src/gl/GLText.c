/*
************************************************************************
*
*   GLText.c - GL text drawing
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
*   Date of last modification : 95/02/07
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/gl/SCCS/s.GLText.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include "gl_text.h"

#include <gl/gl.h>
#include "gl_util.h"

void
SgGLDrawAnnot2D(float x[2], char *str)
{
  GLStart2D();
  cmov2(x[0], x[1]);
  charstr(str);
  GLEnd2D();
}

void
SgGLDrawAnnot(float x[3], char *str)
{
  cmov(x[0], x[1], x[2]);
  charstr(str);
}
