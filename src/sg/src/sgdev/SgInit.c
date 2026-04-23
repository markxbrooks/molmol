/*
************************************************************************
*
*   SgInit.c - Sg init
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
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/sgdev/SCCS/s.SgInit.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <sg_dev.h>

#include "sg_set.h"
#include "sg_view.h"

SgRes
SgInit(int *argcP, char *argv[])
{
  SgRes res;

  res = CurrSgDevP->init(argcP, argv);
  if (res == SG_RES_OK) {
    SgSetAttr();
    SgSetView();
  }
  return res;
}

SgRes
SgCleanup(void)
{
  return CurrSgDevP->cleanup();
}
