/*
************************************************************************
*
*   pov_color.h - POV color setting
*
*   Copyright (c) 1994-2000
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
*   Date of last modification : 00/02/26
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/pov/SCCS/s.pov_color.h
*   SCCS identification       : 1.6
*
************************************************************************
*/

#include <sg_types.h>

extern void SgPOVSetColor(float, float, float, float, BOOL);

extern void SgPOVSetMatProp(SgMatProp, float);

extern void SgPOVSetTexture(char *);

extern void SgPOVSetShadeModel(SgShadeModel);

extern void SgPOVSetFogMode(SgFogMode);

extern void SgPOVSetFogPar(SgFogPar, float);

extern void POVUpdateColor(void);

extern void POVPlotColor(void);

extern void POVPlotTexture(void);
