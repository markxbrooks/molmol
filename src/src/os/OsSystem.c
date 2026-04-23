/*
************************************************************************
*
*   OsSystem.c - execute system command
*
*   Copyright (c) 1996-97
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
*   Date of last modification : 97/02/17
*   Pathname of SCCS file     : /local/home/kor/molmol/src/os/SCCS/s.OsSystem.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <os_system.h>

#include <stdlib.h>

BOOL
OsSystem(char *cmd)
{
  return system(cmd) == 0;
}
