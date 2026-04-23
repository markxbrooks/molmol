/*
************************************************************************
*
*   PuHelp.c - Pu wrapper function for online help
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
*   Pathname of SCCS file     : /sgiext/molmol/src/pudev/SCCS/s.PuHelp.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <pu_dev.h>

void
PuShowHelp(PuGizmo parent, char *title, char *fileName)
{
  CurrPuDevP->showHelp(parent, title, fileName);
}
