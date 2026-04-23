/*
************************************************************************
*
*   vrml2_poly.h - VRML2 polygons
*
*   Copyright (c) 1996-99
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
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/sg/src/vrml2/SCCS/s.vrml2_poly.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <sg_types.h>

extern void SgVRML2DrawPolygon2D(float[][2], int);

extern void SgVRML2DrawPolygon(float[][3], int, float[3]);

void SgVRML2DrawTriMesh(float[][3], float[][3], int);

void SgVRML2DrawColorTriMesh(float[][3], float[][3], float[][3], int);

void SgVRML2DrawStrip(float[][3], float[][3],
    float[][3], float[][3], int);

void SgVRML2DrawColorStrip(float[][3], float[][3],
    float[][3], float [][3], float[][3], float[][3], int);
