/*
************************************************************************
*
*   pov_light.h - POV light source
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
*   Date of last modification : 95/05/21
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/pov/SCCS/s.pov_light.h
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <sg_types.h>

extern void SgPOVSetLight(SgLightState);

extern void SgPOVSetLightPosition(float[3]);

extern void POVPlotLightSource(void);
