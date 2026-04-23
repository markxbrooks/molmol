/*
************************************************************************
*
*   vrml2_color.h - VRML2 color setting
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
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/sg/src/vrml2/SCCS/s.vrml2_color.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <sg_types.h>

extern void SgVRML2SetColor(float, float, float, float, BOOL);

extern void SgVRML2SetMatProp(SgMatProp, float);

extern void SgVRML2SetTexture(char *);

extern void VRML2ColorSetLight(BOOL);

extern void VRML2PlotColor(void);
