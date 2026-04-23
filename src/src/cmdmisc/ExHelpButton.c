/*
************************************************************************
*
*   ExHelpButton.c - HelpButton command
*
*   Copyright (c) 1995
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
*   Date of last modification : 95/07/20
*   Pathname of SCCS file     : /sgiext/molmol/src/cmdmisc/SCCS/s.ExHelpButton.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <cmd_misc.h>

#include <stdio.h>
#include <stdlib.h>

#include <pu.h>
#include <par_names.h>
#include <setup_file.h>

ErrCode
ExHelpButton(char *cmd)
{
  char *fileName;

  fileName = SetupGetName(PN_MENU_DIR, "Buttons", TRUE);
  PuShowHelp(NULL, "Buttons", fileName);
  free(fileName);

  return EC_OK;
}
