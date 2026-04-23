/*
************************************************************************
*
*   xgl_feature.h - XGL enable/disable features
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
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/xgl/SCCS/s.xgl_feature.h
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <sg_types.h>

extern void SgXGLSetFogMode(SgFogMode);

extern void SgXGLSetFogPar(SgFogPar, float);

extern void SgXGLSetFeature(SgFeature, BOOL);
