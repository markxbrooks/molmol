/*
************************************************************************
*
*   MotGLDev.c - Motif/GL device
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
*   Pathname of SCCS file     : /sgiext/molmol/src/motgl/SCCS/s.MotGLDev.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <io_dev.h>

#include <stdio.h>

#include "motgl.h"

static IODev IOMotifGLDev = {
  IOMotifGLInit,
  IOMotifGLCleanup,
  IOMotifGLSetWindow,
  IOMotifGLEndFrame,
  NULL,
  NULL,
  "Motif",
  "GL"
};

static IODev IOMotifGLDDev = {
  IOMotifGLInit,
  IOMotifGLCleanup,
  IOMotifGLSetWindow,
  IOMotifGLEndFrame,
  NULL,
  NULL,
  "Motif",
  "GLD"
};

void
IOMotifGLSetDev(void)
{
  CurrIODevP = &IOMotifGLDev;
}

void
IOMotifGLDSetDev(void)
{
  CurrIODevP = &IOMotifGLDDev;
}
