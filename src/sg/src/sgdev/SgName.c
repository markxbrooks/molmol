/*
************************************************************************
*
*   SgName.c - set and get device name
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
*   Date of last modification : 95/11/21
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/sgdev/SCCS/s.SgName.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <sg.h>
#include "sg_name.h"

static char *CurrName;

char *
SgGetDeviceName(void)
{
  return CurrName;
}

void
SgSetDeviceName(char *name)
{
  CurrName = name;
}
