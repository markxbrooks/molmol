/*
************************************************************************
*
*   MotOGLDev.c - Motif/OpenGL device
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
*   Date of last modification : 95/09/12
*   Pathname of SCCS file     : /sgiext/molmol/src/motogl/SCCS/s.MotOGLDev.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <io_dev.h>

#include "motogl.h"

static IODev IOMotifOGLDev = {
  IOMotifOGLInit,
  IOMotifOGLCleanup,
  IOMotifOGLSetWindow,
  IOMotifOGLEndFrame,
  IOMotifOGLStartDump,
  IOMotifOGLEndDump,
  "Motif",
  "OpenGL"
};

static IODev IOMotifOGLDDev = {
  IOMotifOGLInit,
  IOMotifOGLCleanup,
  IOMotifOGLSetWindow,
  IOMotifOGLEndFrame,
  IOMotifOGLStartDump,
  IOMotifOGLEndDump,
  "Motif",
  "OpenGLD"
};

void
IOMotifOGLSetDev(void)
{
  CurrIODevP = &IOMotifOGLDev;
}

void
IOMotifOGLDSetDev(void)
{
  CurrIODevP = &IOMotifOGLDDev;
}
