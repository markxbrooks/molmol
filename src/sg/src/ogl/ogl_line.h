/*
************************************************************************
*
*   ogl_line.h - OpenGL line drawing
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
*   Date of last modification : 94/08/16
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/ogl/SCCS/s.ogl_line.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <sg_types.h>

extern void SgOGLDrawLine2D(float[2], float[2]);

extern void SgOGLDrawPolyline2D(float[][2], int);

extern void SgOGLDrawLine(float[3], float[3]);

extern void SgOGLDrawPolyline(float[][3], int);

extern void SgOGLSetLineWidth(float);

extern void SgOGLSetLineStyle(SgLineStyle);
