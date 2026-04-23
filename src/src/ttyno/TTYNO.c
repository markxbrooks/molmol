/*
************************************************************************
*
*   TTYNO.c - TTY/NO device
*
*   Copyright (c) 1994-95
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
*   Date of last modification : 95/06/15
*   Pathname of SCCS file     : /sgiext/molmol/src/ttyno/SCCS/s.TTYNO.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include "ttyno.h"

#include <sg.h>
#include <pu.h>

IORes
IOTTYNOInit(char *appName, int *argcP, char *argv[])
{
  (void) PuInit(appName, argcP, argv);
  (void) SgInit(argcP, argv);

  return IO_RES_OK;
}

IORes
IOTTYNOCleanup(void)
{
  (void) SgCleanup();
  (void) PuCleanup();

  return IO_RES_OK;
}

IORes
IOTTYNOSetWindow(PuWindow win)
{
  return IO_RES_OK;
}

void
IOTTYNOEndFrame(void)
{
}
