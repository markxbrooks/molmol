/*
************************************************************************
*
*   ExHelpMouse.c - HelpMouse command
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
*   Date of last modification : 95/04/11
*   Pathname of SCCS file     : /sgiext/molmol/src/cmdmisc/SCCS/s.ExHelpMouse.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <cmd_misc.h>

#include <stdio.h>

#include <cip.h>

ErrCode
ExHelpMouse(char *cmd)
{
  CipShowHelpFile(NULL, "Mouse");

  return EC_OK;
}
