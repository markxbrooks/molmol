/*
************************************************************************
*
*   gl_transf.h - GL transformations
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
*   Date of last modification : 94/06/02
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/gl/SCCS/s.gl_transf.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

extern void SgGLRotateX(float);

extern void SgGLRotateY(float);

extern void SgGLRotateZ(float);

extern void SgGLTranslate(float[3]);

extern void SgGLScale(float, float, float);

extern void SgGLSetMatrix(float[4][4]);

extern void SgGLMultMatrix(float[4][4]);

extern void SgGLPushMatrix(void);

extern void SgGLPopMatrix(void);
