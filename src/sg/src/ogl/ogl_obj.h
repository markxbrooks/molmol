/*
************************************************************************
*
*   ogl_obj.h - OpenGL display lists
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
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/ogl/SCCS/s.ogl_obj.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <sg_types.h>

extern SgObjId SgOGLBeginObj(void);

extern void SgOGLEndObj(void);

extern void SgOGLReplaceObj(SgObjId);

extern void SgOGLDestroyObj(SgObjId);

extern void SgOGLDrawObj(SgObjId);
