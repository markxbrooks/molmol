/*
************************************************************************
*
*   GLMarker.c - GL maker drawing
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
*   Date of last modification : 95/01/12
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/gl/SCCS/s.GLMarker.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include "gl_marker.h"

#include <gl/gl.h>
#include "gl_util.h"

void
SgGLDrawMarkers2D(float x[][2], int n)
{
  int i;

  GLStart2D();
  bgnpoint();
  for (i = 0; i < n; i++)
    v2f(x[i]);
  endpoint();
  GLEnd2D();
}

void
SgGLDrawMarkers(float x[][3], int n)
{
  int i;

  bgnpoint();
  for (i = 0; i < n; i++)
    v3f(x[i]);
  endpoint();
}
