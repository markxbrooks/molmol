/*
************************************************************************
*
*   OGLObj.c - OpenGL display lists
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
*   Pathname of SCCS file     : /local/home/kor/molmol/sg/src/ogl/SCCS/s.OGLObj.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include "ogl_obj.h"

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

SgObjId
SgOGLBeginObj(void)
{
  GLuint obj;

  obj = glGenLists(1);
  glNewList(obj, GL_COMPILE);

  return (SgObjId) obj;
}

void
SgOGLEndObj(void)
{
  glEndList();
}

void
SgOGLReplaceObj(SgObjId obj)
{
  glNewList((GLuint) obj, GL_COMPILE);
}

void
SgOGLDestroyObj(SgObjId obj)
{
  glDeleteLists((GLuint) obj, 1);
}

void
SgOGLDrawObj(SgObjId obj)
{
  glCallList((GLuint) obj);
}
