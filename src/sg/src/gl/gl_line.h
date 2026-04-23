/*
************************************************************************
*
*   gl_line.h - GL line drawing
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
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/gl/SCCS/s.gl_line.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <sg_types.h>

extern void SgGLDrawLine2D(float[2], float[2]);

extern void SgGLDrawPolyline2D(float[][2], int);

extern void SgGLDrawLine(float[3], float[3]);

extern void SgGLDrawPolyline(float[][3], int);

extern void SgGLSetLineWidth(float);

extern void SgGLSetLineStyle(SgLineStyle);
