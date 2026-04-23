/*
************************************************************************
*
*   sg_get.h - get status values, used internally in Sg
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
*   Date of last modification : 96/02/27
*   Pathname of SCCS file     : /sgiext/molmol/sg/include/SCCS/s.sg_get.h
*   SCCS identification       : 1.7
*
************************************************************************
*/

#include <sg_types.h>

extern int SgGetDim(void);

extern BOOL SgGetFeature(SgFeature);

extern void SgGetBackgroundColor(float *, float *, float *);

extern void SgGetColor(float *, float *, float *);

extern float SgGetMatProp(SgMatProp);

extern int SgGetPrecision(void);

extern float SgGetLineWidth(void);

extern float SgGetLineWidth(void);

extern SgLineStyle SgGetLineStyle(void);

extern float SgGetFontSize(void);

extern SgEye SgGetEye(void);

extern float SgGetOrthoHeight(void);
