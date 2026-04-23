/*
************************************************************************
*
*   gl_obj.h - GL display lists
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
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/gl/SCCS/s.gl_obj.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <sg_types.h>

extern SgObjId SgGLBeginObj(void);

extern void SgGLEndObj(void);

extern void SgGLReplaceObj(SgObjId);

extern void SgGLDestroyObj(SgObjId);

extern void SgGLDrawObj(SgObjId);
