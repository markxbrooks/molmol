/*
************************************************************************
*
*   ogl_poly.h - OpenGL polygons
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
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/ogl/SCCS/s.ogl_poly.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <sg_types.h>

extern void SgOGLDrawPolygon2D(float[][2], int);

extern void SgOGLDrawPolygon(float[][3], int, float[3]);

extern void SgOGLSetShadeModel(SgShadeModel);
