/*
************************************************************************
*
*   GLInit.c - init GL device
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
*   Date of last modification : 94/12/28
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/gl/SCCS/s.GLInit.c
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include "gl_init.h"

#include <gl/gl.h>

SgRes
SgGLInit(int *argcP, char *argv[])
{
  subpixel(TRUE);
  zbuffer(TRUE);
  mmode(MVIEWING);

  deflinestyle((int) SG_LINE_DASHED, 0xF0F0);

  return SG_RES_OK;
}

SgRes
SgGLCleanup(void)
{
  return SG_RES_OK;
}
