/*
************************************************************************
*
*   vrml1_line.h - VRML1 line drawing
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
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/sg/src/vrml1/SCCS/s.vrml1_line.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <sg_types.h>

extern void SgVRML1DrawLine2D(float[2], float[2]);

extern void SgVRML1DrawPolyline2D(float[][2], int);

extern void SgVRML1DrawLine(float[3], float[3]);

extern void SgVRML1DrawPolyline(float[][3], int);
