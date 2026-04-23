/*
************************************************************************
*
*   ogl_light.h - OpenGL light source
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
*   Date of last modification : 95/08/24
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/ogl/SCCS/s.ogl_light.h
*   SCCS identification       : 1.5
*
************************************************************************
*/

#include <sg_types.h>

extern void SgOGLSetMatProp(SgMatProp, float);

extern void SgOGLSetLight(SgLightState);

extern void SgOGLSetLightPosition(float[3]);

extern void OGLSetMaterialColor(float[4]);
