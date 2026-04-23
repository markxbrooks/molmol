/*
************************************************************************
*
*   xgl_line.h - XGL line drawing
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
*   Date of last modification : 94/12/30
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/xgl/SCCS/s.xgl_line.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <sg_types.h>

extern void SgXGLDrawLine2D(float[2], float[2]);

extern void SgXGLDrawPolyline2D(float[][2], int);

extern void SgXGLDrawLine(float[3], float[3]);

extern void SgXGLDrawPolyline(float[][3], int);

extern void SgXGLSetLineWidth(float);

extern void SgXGLSetLineStyle(SgLineStyle);
