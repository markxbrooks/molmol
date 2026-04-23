/*
************************************************************************
*
*   xgl_light.h - XGL light source
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
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/xgl/SCCS/s.xgl_light.h
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <sg_types.h>

extern void SgXGLSetLight(SgLightState);

extern void SgXGLSetLightPosition(float[3]);

extern void SgXGLSetShadeModel(SgShadeModel);
