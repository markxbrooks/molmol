/*
************************************************************************
*
*   SgClear.c - Sg wrapper for clear
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
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/sgdev/SCCS/s.SgClear.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <sg_dev.h>

void
SgClear(void)
{
  CurrSgDevP->clear();
}
