/*
************************************************************************
*
*   gl_color.h - GL set color
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
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/gl/SCCS/s.gl_color.h
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include <sg_types.h>

extern void SgGLSetColor(float, float, float, float, BOOL);

extern void SgGLSetFogMode(SgFogMode);

extern void SgGLSetFogPar(SgFogPar, float);

extern void GLSetFG(void);

extern void GLSetBG(void);
