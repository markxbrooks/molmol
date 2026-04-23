/*
************************************************************************
*
*   MotX11Dev.c - Motif/X11 device
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
*   Pathname of SCCS file     : /sgiext/molmol/src/motx11/SCCS/s.MotX11Dev.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <io_dev.h>

#include <stdio.h>

#include "motx11.h"

static IODev IOMotifX11Dev = {
  IOMotifX11Init,
  IOMotifX11Cleanup,
  IOMotifX11SetWindow,
  IOMotifX11EndFrame,
  NULL,
  NULL,
  "Motif",
  "X11"
};

void
IOMotifX11SetDev(void)
{
  CurrIODevP = &IOMotifX11Dev;
}
