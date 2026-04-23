/*
************************************************************************
*
*   range.h - range determination functions
*
*   Copyright (c) 1994-99
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
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/sg/src/range/SCCS/s.range.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <sg_types.h>

extern SgRes SgRangeInit(int *, char *[]);

extern SgRes SgRangeCleanup(void);

extern void SgRangeDrawMarkers2D(float[][2], int);

extern void SgRangeDrawLine2D(float[2], float[2]);

extern void SgRangeDrawPolyline2D(float[][2], int);

extern void SgRangeDrawPolygon2D(float[][2], int);

extern void SgRangeDrawCircle2D(float[2], float);

extern void SgRangeDrawDisc2D(float[2], float);

extern void SgRangeDrawAnnot2D(float[2], char *);

extern void SgRangeDrawText2D(float[2], char *);

extern void SgRangeDrawMarkers(float[][3], int);

extern void SgRangeDrawLine(float[3], float[3]);

extern void SgRangeDrawPolyline(float[][3], int);

extern void SgRangeDrawPolygon(float[][3], int, float[3]);

extern void SgRangeDrawAnnot(float[3], char *);

extern void SgRangeDrawText(float[3], char *);

extern void SgRangeDrawSphere(float[3], float);

extern void SgRangeDrawCone(float[3], float[3], float, float,
    SgConeEnd, SgConeEnd);
