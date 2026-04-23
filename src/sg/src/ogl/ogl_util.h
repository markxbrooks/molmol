/*
************************************************************************
*
*   ogl_util.h - OpenGL utility functions
*
*   Copyright (c) 1994-97
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
*   Date of last modification : 97/02/17
*   Pathname of SCCS file     : /local/home/kor/molmol/sg/src/ogl/SCCS/s.ogl_util.h
*   SCCS identification       : 1.2
*
************************************************************************
*/

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

extern GLfloat Rad2OGLAng(float);

extern void OGLStart2D(void);

extern void OGLEnd2D(void);
