/*
************************************************************************
*
*   xgl_poly.h - XGL polygons
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
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/xgl/SCCS/s.xgl_poly.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <sg_types.h>

extern void SgXGLDrawPolygon2D(float[][2], int);

extern void SgXGLDrawPolygon(float[][3], int, float[3]);
