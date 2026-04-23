/*
************************************************************************
*
*   PuCmdField.c - Pu wrapper functions for command fields
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
*   Date of last modification : 94/08/11
*   Pathname of SCCS file     : /sgiext/molmol/src/pudev/SCCS/s.PuCmdField.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <pu_dev.h>

PuCmdField
PuCreateCmdField(char *name,
    PuCmdFieldCB cmdFieldCB, void *clientData, PuFreeCB freeCB)
{
  return CurrPuDevP->createCmdField(name,
      cmdFieldCB, clientData, freeCB);
}

void
PuSwitchCmdFields(BOOL onOff)
{
  CurrPuDevP->switchCmdFields(onOff);
}
