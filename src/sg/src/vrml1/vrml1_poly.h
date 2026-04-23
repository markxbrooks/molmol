/*
************************************************************************
*
*   vrml1_poly.h - VRML1 polygons
*
*   Copyright (c) 1996
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
*   Date of last modification : 99/10/31
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/sg/src/vrml1/SCCS/s.vrml1_poly.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <sg_types.h>

extern void SgVRML1DrawPolygon2D(float[][2], int);

extern void SgVRML1DrawPolygon(float[][3], int, float[3]);

void SgVRML1DrawTriMesh(float[][3], float[][3], int);

void SgVRML1DrawStrip(float[][3], float[][3],
    float[][3], float[][3], int);
