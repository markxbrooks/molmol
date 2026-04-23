/*
************************************************************************
*
*   PuInit.c - Pu wrapper functions for init/cleanup
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
*   Pathname of SCCS file     : /sgiext/molmol/src/pudev/SCCS/s.PuInit.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <pu_dev.h>

#include <pu_cb.h>

PuRes
PuInit(char *appName, int *argcP, char *argv[])
{
  PuInitCallbacks();
  return CurrPuDevP->init(appName, argcP, argv);
}

PuRes
PuCleanup(void)
{
  PuRes res;

  res = CurrPuDevP->cleanup();
  PuCallQuitCB();
  return res;
}
