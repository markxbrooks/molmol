/*
************************************************************************
*
*   pov_poly.h - POV polygons
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
*   Date of last modification : 96/12/03
*   Pathname of SCCS file     : /local/home/kor/molmol/sg/src/pov/SCCS/s.pov_poly.h
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include <sg_types.h>

extern void SgPOVDrawPolygon2D(float[][2], int);

extern void SgPOVDrawPolygon(float[][3], int, float[3]);

extern void SgPOVDrawPolygon(float[][3], int, float[3]);

extern void SgPOVStartSurface(SgSurfaceType);

extern void SgPOVDrawTriMesh(float[][3], float[][3], int);

extern void SgPOVDrawColorTriMesh(float[][3], float[][3], float [][3], int);

extern void SgPOVDrawStrip(float[][3], float[][3],
    float[][3], float[][3], int);

extern void SgPOVDrawColorStrip(float[][3], float[][3],
    float[][3], float [][3], float [][3], float[][3], int);

extern void SgPOVEndSurface(void);
