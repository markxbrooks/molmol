/*
************************************************************************
*
*   vrml1_color.h - VRML1 color setting
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
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/sg/src/vrml1/SCCS/s.vrml1_color.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <sg_types.h>

extern void SgVRML1SetColor(float, float, float, float, BOOL);

extern void SgVRML1SetMatProp(SgMatProp, float);

extern void SgVRML1SetTexture(char *);

extern void VRML1PlotColor(void);
