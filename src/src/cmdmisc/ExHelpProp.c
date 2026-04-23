/*
************************************************************************
*
*   ExHelpProp.c - HelpProp command
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
*   Pathname of SCCS file     : /sgiext/molmol/src/cmdmisc/SCCS/s.ExHelpProp.c
*   SCCS identification       : 1.3
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
ExHelpProp(char *cmd)
{
  char *fileName;

  fileName = SetupGetName(PN_PROP_DEF, "PropDef", FALSE);
  PuShowHelp(NULL, "Properties", fileName);
  free(fileName);

  return EC_OK;
}
