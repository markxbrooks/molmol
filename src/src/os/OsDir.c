/*
************************************************************************
*
*   OsDir.c - create and read directory
*
*   Copyright (c) 1997
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
*   Date of last modification : 01/05/27
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/os/SCCS/s.OsDir.c
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include <os_dir.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#include <windows.h>
#include <direct.h>
#else
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include <strmatch.h>

void
OsDirCreate(char *dirName)
{
#ifdef WIN32
  (void) _mkdir(dirName);
#else
  (void) mkdir(dirName, 0777);
#endif
}

void
OsDirGet(char *dirName, char *namePat, char ***nameAP, int *nameNoP)
{
  char **nameA, *name;
  int nameNo, dirLen;
#ifdef WIN32
  HANDLE hand;
  char *path;
  WIN32_FIND_DATA findData;
#else
  DIR *dirP;
  struct dirent *entP;
#endif

  nameA = NULL;
  nameNo = 0;

  dirLen = strlen(dirName);
  if (dirName[dirLen - 1] == '/')
    dirLen--;

#ifdef WIN32
  path = malloc(dirLen + 1 + strlen(namePat) + 1);
  (void) strcpy(path, dirName);
  path[dirLen] = '/';
  (void) strcpy(path + dirLen + 1, namePat);
  hand = FindFirstFile(path, &findData);
  free(path);

  if (hand != INVALID_HANDLE_VALUE) {
    for (;;) {
      name = findData.cFileName;
#else
  dirP = opendir(dirName);
  if (dirP != NULL) {
    for (;;) {
      entP = readdir(dirP);
      if (entP == NULL)
	break;
      
      name = entP->d_name;

      if (! StrMatch(name, namePat))
	continue;

      if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
	continue;
#endif

      if (nameNo == 0)
	nameA = malloc(sizeof(*nameA));
      else
	nameA = realloc(nameA, (nameNo + 1) * sizeof(*nameA));

      nameA[nameNo] = malloc(dirLen + 1 + strlen(name) + 1);
      (void) strcpy(nameA[nameNo], dirName);
      nameA[nameNo][dirLen] = '/';
      (void) strcpy(nameA[nameNo] + dirLen + 1, name);

      nameNo++;

#ifdef WIN32
      if (! FindNextFile(hand, &findData))
	break;
#endif
    }

#ifdef WIN32
    (void) FindClose(hand);
#else
    (void) closedir(dirP);
#endif
  }

  *nameAP = nameA;
  *nameNoP = nameNo;
}

void
OsDirFree(char **nameA, int nameNo)
{
  int nameI;

  if (nameNo == 0)
    return;

  for (nameI = 0; nameI < nameNo; nameI++)
    free(nameA[nameI]);
  
  free(nameA);
}
