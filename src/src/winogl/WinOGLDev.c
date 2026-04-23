/*
************************************************************************
*
*   WinOGLDev.c - Windows/OpenGL device
*
*   Copyright (c) 1996
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
*   Date of last modification : 96/05/09
*   Pathname of SCCS file     : /local/home/kor/molmol/src/winogl/SCCS/s.WinOGLDev.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <io_dev.h>

#include "winogl.h"

static IODev IOWinOGLDev = {
  IOWinOGLInit,
  IOWinOGLCleanup,
  IOWinOGLSetWindow,
  IOWinOGLEndFrame,
  IOWinOGLStartDump,
  IOWinOGLEndDump,
  "Windows",
  "OpenGL"
};

static IODev IOWinOGLDDev = {
  IOWinOGLInit,
  IOWinOGLCleanup,
  IOWinOGLSetWindow,
  IOWinOGLEndFrame,
  IOWinOGLStartDump,
  IOWinOGLEndDump,
  "Windows",
  "OpenGLD"
};

void
IOWinOGLSetDev(void)
{
  CurrIODevP = &IOWinOGLDev;
}

void
IOWinOGLDSetDev(void)
{
  CurrIODevP = &IOWinOGLDDev;
}
