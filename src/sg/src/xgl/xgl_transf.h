/*
************************************************************************
*
*   xgl_transf.h - XGL transformations
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
*   Date of last modification : 94/12/30
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/xgl/SCCS/s.xgl_transf.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

extern void SgXGLRotateX(float);

extern void SgXGLRotateY(float);

extern void SgXGLRotateZ(float);

extern void SgXGLTranslate(float[3]);

extern void SgXGLScale(float, float, float);

extern void SgXGLSetMatrix(float[4][4]);

extern void SgXGLMultMatrix(float[4][4]);

extern void SgXGLPushMatrix(void);

extern void SgXGLPopMatrix(void);
