/*
************************************************************************
*
*   xgl_mesh.h - XGL mesh
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
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/xgl/SCCS/s.xgl_mesh.h
*   SCCS identification       : 1.3
*
************************************************************************
*/

extern void SgXGLDrawTriMesh(float[][3], float[][3], int);

extern void SgXGLDrawColorTriMesh(float[][3], float[][3], float[][3], int);

extern void SgXGLDrawStrip(float[][3], float[][3],
    float[][3], float[][3], int);

extern void SgXGLDrawColorStrip(float[][3], float[][3],
    float[][3], float[][3], float[][3], float[][3], int);
