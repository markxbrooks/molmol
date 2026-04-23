/*
************************************************************************
*
*   pick.h - picking functions
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
*   Date of last modification : 96/06/06
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/pick/SCCS/s.pick.h
*   SCCS identification       : 1.6
*
************************************************************************
*/

#include <sg_types.h>

extern SgRes SgPickInit(int *, char *[]);

extern SgRes SgPickCleanup(void);

extern void SgPickDrawMarkers2D(float[][2], int);

extern void SgPickDrawLine2D(float[2], float[2]);

extern void SgPickDrawPolyline2D(float[][2], int);

extern void SgPickDrawPolygon2D(float[][2], int);

extern void SgPickDrawCircle2D(float[2], float);

extern void SgPickDrawDisc2D(float[2], float);

extern void SgPickDrawAnnot2D(float[2], char *);

extern void SgPickDrawText2D(float[2], char *);

extern void SgPickDrawMarkers(float[][3], int);

extern void SgPickDrawLine(float[3], float[3]);

extern void SgPickDrawPolyline(float[][3], int);

extern void SgPickDrawPolygon(float[][3], int, float[3]);

extern void SgPickDrawAnnot(float[3], char *);

extern void SgPickDrawText(float[3], char *);

extern void SgPickDrawSphere(float[3], float);

extern void SgPickDrawCone(float[3], float[3], float, float,
    SgConeEnd, SgConeEnd);
