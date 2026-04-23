/*
************************************************************************
*
*   sg_map.h - map coordinates, used internally in Sg
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
*   Date of last modification : 97/07/17
*   Pathname of SCCS file     : /local/home/kor/molmol/sg/include/SCCS/s.sg_map.h
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include <bool.h>

extern void SgMapModelInit(void);

extern void SgMapModelPoint(float[3], float[3]);

extern void SgMapModelVect(float[3], float[3]);

extern void SgMapEyePoint(float[3], float[3]);

extern void SgMapEyeVect(float[3], float[3]);

extern BOOL SgBackfacing(float[3], float[3]);

extern int SgMapPoint(float[3], float[3]);

extern int SgMapLine(float[3], float[3], float[3], float[3]);

extern int SgMapPoly(float[][3], float[][3], int);

extern int SgMapSphere(float[3], float *, float *, float[3], float);

extern int SgMapCone(float[4][3], float *, float *,
    float[3], float[3], float, float);

extern void SgVpMapPoint(float[2], float[2]);

extern float SgVpMapSize(float);
