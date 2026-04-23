/*
************************************************************************
*
*   ProgDir.c - program home directory
*
*   Copyright (c) 1994-99
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
*   Date of last modification : 99/10/24
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/src/os/SCCS/s.ProgDir.c
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include <prog_dir.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <windows.h>
#endif

static char *ProgDir;

void
ProgDirSet(char *envVar, char *defVal)
{
#ifdef WIN32
  char *dir;
  char exePath[500];
  int len, i;

  dir = getenv(envVar);
  if (dir == NULL) {
    len = GetModuleFileName(NULL, exePath, sizeof(exePath));
    len--;
    while (len > 0 && exePath[len] != '\\')
      len--;
    exePath[len] = '\0';
    dir = exePath;
  } else {
    len = strlen(dir);
  }

  ProgDir = malloc(len + 1);
  (void) strcpy(ProgDir, dir);

  for (i = 0; i < len; i++)
    if (ProgDir[i] == '\\')
      ProgDir[i] = '/';
#else
  char *dir;
  char *exePath;
  int len;
  dir = getenv(envVar);
  if (dir == NULL) {
    exePath = defVal;
    len = strlen(exePath);
    len--;
    while (len > 0 && exePath[len] != '/')
      len--;
    dir = exePath;
  } else {
    len = strlen(dir);
  }

  ProgDir = malloc(len + 1);
  (void) strncpy(ProgDir, dir, len);

#endif
}

char *
ProgDirGet(void)
{
  return ProgDir;
}
