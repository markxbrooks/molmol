/*
************************************************************************
*
*   gl_light.h - GL light source
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
*   Date of last modification : 95/12/10
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/gl/SCCS/s.gl_light.h
*   SCCS identification       : 1.5
*
************************************************************************
*/

#include <sg_types.h>

extern void SgGLSetMatProp(SgMatProp, float);

extern void SgGLSetLight(SgLightState);

extern void SgGLSetLightPosition(float[3]);

extern void GLSetMaterialColor(float[4]);

extern long GLGetColorMode(void);
