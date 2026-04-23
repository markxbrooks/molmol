/*
************************************************************************
*
*   ogl_feature.h - OpenGL enable/disable features
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
*   Date of last modification : 96/03/05
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/ogl/SCCS/s.ogl_feature.h
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <sg_types.h>

extern void SgOGLSetFogMode(SgFogMode);

extern void SgOGLSetFogPar(SgFogPar, float);

extern void SgOGLSetFeature(SgFeature, BOOL);
