/*
************************************************************************
*
*   rib_poly.h - RIB polygons
*
*   Copyright (c) 1994-98
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
*   Date of last modification : 99/10/16
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/sg/src/rib/SCCS/s.rib_poly.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <sg_types.h>

extern void SgRIBDrawPolygon2D(float[][2], int);

extern void SgRIBDrawPolygon(float[][3], int, float[3]);

extern void SgRIBDrawPolygon(float[][3], int, float[3]);

extern void SgRIBStartSurface(SgSurfaceType);

extern void SgRIBDrawTriMesh(float[][3], float[][3], int);

extern void SgRIBDrawColorTriMesh(float[][3], float[][3], float [][3], int);

extern void SgRIBDrawStrip(float[][3], float[][3],
    float[][3], float[][3], int);

extern void SgRIBDrawColorStrip(float[][3], float[][3],
    float[][3], float [][3], float [][3], float[][3], int);

extern void SgRIBEndSurface(void);
