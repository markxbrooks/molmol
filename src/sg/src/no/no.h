/*
************************************************************************
*
*   no.h - NO functions
*
*   Copyright (c) 1995
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
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/no/SCCS/s.no.h
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <sg_types.h>

extern SgRes SgNOInit(int *, char *[]);

extern SgRes SgNOCleanup(void);

extern SgObjId SgNOBeginObj(void);

extern void SgNOEndObj(void);

extern void SgNOReplaceObj(SgObjId);

extern void SgNODestroyObj(SgObjId);

extern void SgNODrawObj(SgObjId);

extern void SgNODrawMarkers2D(float[][2], int);

extern void SgNODrawLine2D(float[2], float[2]);

extern void SgNODrawArrow2D(float[2], float[2]);

extern void SgNODrawPolyline2D(float[][2], int);

extern void SgNODrawPolygon2D(float[][2], int);

extern void SgNODrawCircle2D(float[2], float);

extern void SgNODrawDisc2D(float[2], float);

extern void SgNODrawAnnot2D(float[2], char *);

extern void SgNODrawText2D(float[2], char *);

extern void SgNODrawMarkers(float[][3], int);

extern void SgNODrawLine(float[3], float[3]);

extern void SgNODrawArrow(float[3], float[3]);

extern void SgNODrawPolyline(float[][3], int);

extern void SgNODrawPolygon(float[][3], int, float[3]);

extern void SgNODrawCircle(float[3], float[3], float);

extern void SgNODrawDisc(float[3], float[3], float);

extern void SgNODrawAnnot(float[3], char *);

extern void SgNODrawText(float[3], char *);

extern void SgNODrawSphere(float[3], float);

extern void SgNODrawCone(float[3], float[3], float, float,
    SgConeEnd, SgConeEnd);

extern void SgNODrawTriMesh(float[][3], float[][3], int);

extern void SgNODrawColorTriMesh(float[][3], float[][3], float[][3], int);

extern void SgNODrawStrip(float[][3], float[][3],
    float[][3], float[][3], int);

extern void SgNODrawColorStrip(float[][3], float[][3],
    float[][3], float[][3], float[][3], float[][3], int);
