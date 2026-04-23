/*
************************************************************************
*
*   GLLine.c - GL line drawing
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
*   Date of last modification : 01/05/27
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/gl/SCCS/s.GLLine.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include "gl_line.h"

#include <gl/gl.h>
#include "gl_util.h"

void
SgGLDrawLine2D(float x0[2], float x1[2])
{
  GLStart2D();
  bgnline();
  v2f(x0);
  v2f(x1);
  endline();
  GLEnd2D();
}

void
SgGLDrawPolyline2D(float x[][2], int n)
{
  int i;

  GLStart2D();
  bgnline();
  for (i = 0; i < n; i++)
    v2f(x[i]);
  endline();
  GLEnd2D();
}

void
SgGLDrawLine(float x0[3], float x1[3])
{
  bgnline();
  v3f(x0);
  v3f(x1);
  endline();
}

void
SgGLDrawPolyline(float x[][3], int n)
{
  int i;

  bgnline();
  for (i = 0; i < n; i++)
    v3f(x[i]);
  endline();
}

void
SgGLSetLineWidth(float w)
{
  linewidth((short) (w + 0.5f));
}

void
SgGLSetLineStyle(SgLineStyle lineStyle)
{
  if (lineStyle == SG_LINE_SOLID)
    setlinestyle(0);
  else
    setlinestyle((int) lineStyle);
}
