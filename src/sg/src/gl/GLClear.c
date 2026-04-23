/*
************************************************************************
*
*   GLClear.c - GL clear window
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
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/gl/SCCS/s.GLClear.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include "gl_clear.h"

#include <gl/gl.h>

#include "gl_color.h"

void
SgGLClear(void)
{
  GLSetBG();
  clear();
  zclear();
  GLSetFG();
}
