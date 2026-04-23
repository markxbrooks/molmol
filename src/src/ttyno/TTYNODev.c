/*
************************************************************************
*
*   TTYNODev.c - TTY/NO device
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
*   Date of last modification : 95/09/12
*   Pathname of SCCS file     : /sgiext/molmol/src/ttyno/SCCS/s.TTYNODev.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <io_dev.h>

#include <stdio.h>

#include "ttyno.h"

static IODev IOTTYNODev = {
  IOTTYNOInit,
  IOTTYNOCleanup,
  IOTTYNOSetWindow,
  IOTTYNOEndFrame,
  NULL,
  NULL,
  "TTY",
  "NO"
};

void
IOTTYNOSetDev(void)
{
  CurrIODevP = &IOTTYNODev;
}
