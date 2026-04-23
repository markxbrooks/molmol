/*
************************************************************************
*
*   ogl_mesh.h - OpenGL mesh
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
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/ogl/SCCS/s.ogl_mesh.h
*   SCCS identification       : 1.4
*
************************************************************************
*/

extern void SgOGLDrawTriMesh(float[][3], float[][3], int);

extern void SgOGLDrawColorTriMesh(float[][3], float[][3], float[][3], int);

extern void SgOGLDrawStrip(float[][3], float[][3],
    float[][3], float[][3], int);

extern void SgOGLDrawColorStrip(float[][3], float[][3],
    float[][3], float[][3], float[][3], float[][3], int);
