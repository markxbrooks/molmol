/*
************************************************************************
*
*   XGLText.c - XGL text drawing
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
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/xgl/SCCS/s.XGLText.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include "xgl_text.h"

#include <stdio.h>
#include <xgl/xgl.h>

#include "xgl_access.h"

void
SgXGLDrawAnnot2D(float x[2], char *str)
{
  Xgl_pt_f2d ref;
  Xgl_trans trans;

  ref.x = x[0];
  ref.y = x[1];

  xgl_stroke_text(XGLGetContext2D(), str, &ref, NULL);
}

void
SgXGLDrawAnnot(float x[3], char *str)
{
  Xgl_pt_f3d ref, ann;

  ref.x = x[0];
  ref.y = x[1];
  ref.z = x[2];
  ann.x = 0.0;
  ann.y = 0.0;
  ann.z = 0.0;

  xgl_annotation_text(XGLGetContext3D(), str, &ref, &ann);
}
