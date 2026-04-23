/*
************************************************************************
*
*   ogl_transf.h - OpenGL transformations
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
*   Date of last modification : 94/08/16
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/ogl/SCCS/s.ogl_transf.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

extern void SgOGLRotateX(float);

extern void SgOGLRotateY(float);

extern void SgOGLRotateZ(float);

extern void SgOGLTranslate(float[3]);

extern void SgOGLScale(float, float, float);

extern void SgOGLSetMatrix(float[4][4]);

extern void SgOGLMultMatrix(float[4][4]);

extern void SgOGLPushMatrix(void);

extern void SgOGLPopMatrix(void);
