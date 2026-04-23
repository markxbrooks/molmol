/*
************************************************************************
*
*   rib_line.h - RIB line drawing
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
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/sg/src/rib/SCCS/s.rib_line.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <sg_types.h>

extern void SgRIBDrawLine2D(float[2], float[2]);

extern void SgRIBDrawPolyline2D(float[][2], int);

extern void SgRIBDrawLine(float[3], float[3]);

extern void SgRIBDrawPolyline(float[][3], int);

extern void SgRIBSetLineWidth(float);

extern void SgRIBSetLineStyle(SgLineStyle);
