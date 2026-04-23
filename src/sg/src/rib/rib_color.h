/*
************************************************************************
*
*   rib_color.h - RIB color setting
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
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/sg/src/rib/SCCS/s.rib_color.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <sg_types.h>

extern void SgRIBSetColor(float, float, float, float, BOOL);

extern void SgRIBSetMatProp(SgMatProp, float);

extern void SgRIBSetTexture(char *);

extern void SgRIBSetShadeModel(SgShadeModel);

extern void SgRIBSetFogMode(SgFogMode);

extern void SgRIBSetFogPar(SgFogPar, float);

extern void RIBStartColor(void);

extern void RIBUpdateMaterial(void);
