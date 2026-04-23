/*
************************************************************************
*
*   MotXGLDev.c - Motif/XGL device
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
*   Date of last modification : 95/11/23
*   Pathname of SCCS file     : /sgiext/molmol/src/motxgl/SCCS/s.MotXGLDev.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <io_dev.h>

#include <stdio.h>

#include "motxgl.h"

static IODev IOMotifXGLDev = {
  IOMotifXGLInit,
  IOMotifXGLCleanup,
  IOMotifXGLSetWindow,
  IOMotifXGLEndFrame,
  IOMotifXGLStartDump,
  IOMotifXGLEndDump,
  "Motif",
  "XGL"
};

void
IOMotifXGLSetDev(void)
{
  CurrIODevP = &IOMotifXGLDev;
}
