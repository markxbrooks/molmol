/*
************************************************************************
*
*   PuExtIn.c - Pu wrapper functions for external input
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
*   Date of last modification : 94/12/19
*   Pathname of SCCS file     : /sgiext/molmol/src/pudev/SCCS/s.PuExtIn.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <pu_dev.h>

void
PuAddExtInput(int fd, PuExtInput extInpF, void *clientData, PuFreeCB freeCB)
{
  CurrPuDevP->addExtInput(fd, extInpF, clientData, freeCB);
}

void
PuRemoveExtInput(int fd, PuExtInput extInpF, void *clientData)
{
  CurrPuDevP->removeExtInput(fd, extInpF, clientData);
}
