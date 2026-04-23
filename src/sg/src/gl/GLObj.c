/*
************************************************************************
*
*   GLObj.c - GL display lists
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
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/gl/SCCS/s.GLObj.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include "gl_obj.h"

#include <gl/gl.h>

SgObjId
SgGLBeginObj(void)
{
  Object obj;

  obj = genobj();
  makeobj(obj);

  return (SgObjId) obj;
}

void
SgGLEndObj(void)
{
  closeobj();
}

void
SgGLReplaceObj(SgObjId obj)
{
  makeobj((Object) obj);
}

void
SgGLDestroyObj(SgObjId obj)
{
  delobj((Object) obj);
}

void
SgGLDrawObj(SgObjId obj)
{
  callobj((Object) obj);
}
