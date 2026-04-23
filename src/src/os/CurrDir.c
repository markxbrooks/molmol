/*
************************************************************************
*
*   CurrDir.c - get current directory
*
*   Copyright (c) 1994-97
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
*   Date of last modification : 97/03/17
*   Pathname of SCCS file     : /local/home/kor/molmol/src/os/SCCS/s.CurrDir.c
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include <curr_dir.h>

#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <direct.h>
#else
#include <unistd.h>
#include <errno.h>
#endif

static char *CurrDir = NULL;

void
CurrDirSet(char *dir)
{
  CurrDir = malloc(strlen(dir) + 1);
  (void) strcpy(CurrDir, dir);
}

char *
CurrDirGet(void)
{
  return CurrDir;
}
