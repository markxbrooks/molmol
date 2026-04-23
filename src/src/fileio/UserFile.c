/*
************************************************************************
*
*   UserFile.c - management of user files
*
*   Copyright (c) 1994-2001
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/fileio/SCCS/s.UserFile.c
*   SCCS identification       : 1.5
*
************************************************************************
*/

#include <user_file.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <os_dir.h>
#include <home_dir.h>

static char* UserDir = NULL;
static int UserDirLen;

void
UserFileSetDir(char *defHome, char *subDir)
{
  char *homeDir;
  int homeLen, subLen;
  struct stat st;

  if (UserDir != NULL)
    free(UserDir);

  homeDir = HomeDirGet();
  if (homeDir == NULL)
    homeDir = defHome;

  homeLen = strlen(homeDir);
  subLen = strlen(subDir);
  UserDirLen = homeLen + subLen + 2;
  
  UserDir = malloc(UserDirLen + 1);

  (void) strcpy(UserDir, homeDir);
  UserDir[homeLen] = '/';
  (void) strcpy(UserDir + homeLen + 1, subDir);

  if (stat(UserDir, &st) == -1 && errno == ENOENT)
    (void) OsDirCreate(UserDir);

  UserDir[UserDirLen - 1] = '/';
  UserDir[UserDirLen] = '\0';
}

char *
UserFileGetName(char *fileName)
{
  char *fullName;

  fullName = malloc(UserDirLen + strlen(fileName) + 1);
  (void) strcpy(fullName, UserDir);
  (void) strcpy(fullName + UserDirLen, fileName);

  return fullName;
}

GFile
UserFileOpenRead(char *fileName)
{
  GFile gf;
  char *fullName;

  fullName = UserFileGetName(fileName);
  gf = GFileOpenRead(fullName);
  free(fullName);

  return gf;
}

GFile
UserFileOpenWrite(char *fileName, GFileFormat format)
{
  GFile gf;
  char *fullName;

  fullName = UserFileGetName(fileName);
  gf = GFileOpenNew(fullName, format);
  free(fullName);

  return gf;
}

GFile
UserFileOpenAppend(char *fileName)
{
  GFile gf;
  char *fullName;

  fullName = UserFileGetName(fileName);
  gf = GFileOpenAppend(fullName);
  free(fullName);

  return gf;
}
