/*
************************************************************************
*
*   XGLClear.c - XGL clear window
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
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/xgl/SCCS/s.XGLClear.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include "xgl_clear.h"

#include <xgl/xgl.h>

#include "xgl_access.h"

void
SgXGLClear(void)
{
  xgl_context_new_frame(XGLGetContext3D());
}
