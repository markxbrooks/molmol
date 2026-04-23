/*
************************************************************************
*
*   SetupFile.c - management of setup files
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
*   Date of last modification : 96/04/11
*   Pathname of SCCS file     : /sgiext/molmol/src/fileio/SCCS/s.SetupFile.c
*   SCCS identification       : 1.6
*
************************************************************************
*/

#include <setup_file.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <prog_dir.h>
#include <par_hand.h>

static char* SetupDir = NULL;
static int SetupDirLen;

void
SetupSetDir(char *subDir)
{
  char *homeDir;
  int homeLen, subLen;

  if (SetupDir != NULL)
    free(SetupDir);

  homeDir = ProgDirGet();
  homeLen = strlen(homeDir);
  subLen = strlen(subDir);
  SetupDirLen = homeLen + subLen + 2;
  
  SetupDir = malloc(SetupDirLen + 1);

  (void) strcpy(SetupDir, homeDir);
  SetupDir[homeLen] = '/';
  (void) strcpy(SetupDir + homeLen + 1, subDir);
  SetupDir[SetupDirLen - 1] = '/';
  SetupDir[SetupDirLen] = '\0';
}

char *
SetupGetDir(void)
{
  return SetupDir;
}

void
openFile(GFile *gfP, char **fullNameP,
    char *parName, char *fileName, BOOL dirOnly)
{
  GFile gf;
  char *parVal, *fullName;
  int parLen;

  gf = NULL;
  fullName = NULL;

  if (ParDefined(parName)) {
    parVal = ParGetStrVal(parName);
    parLen = strlen(parVal);

    if (strcmp(parVal, "0") == 0) {
      fullName = malloc(SetupDirLen + 6);
      (void) strcpy(fullName, SetupDir);
      (void) strcpy(fullName + SetupDirLen, "empty");
    } else if (dirOnly) {
      /* use parameter as directory, append fileName */
      fullName = malloc(parLen + strlen(fileName) + 2);
      (void) strcpy(fullName, parVal);
      fullName[parLen] = '/';
      (void) strcpy(fullName + parLen + 1, fileName);
    } else {
      /* use parameter as full filename */
      fullName = malloc(parLen + 1);
      (void) strcpy(fullName, parVal);
    }

    gf = GFileOpenRead(fullName);
  } 
  
  if (gf == NULL) {
    if (fullName == NULL)
      fullName = malloc(SetupDirLen + strlen(fileName) + 1);
    else
      fullName = realloc(fullName, SetupDirLen + strlen(fileName) + 1);

    (void) strcpy(fullName, SetupDir);
    (void) strcpy(fullName + SetupDirLen, fileName);
    gf = GFileOpenRead(fullName);
  }

  *gfP = gf;
  *fullNameP = fullName;
}

char *
SetupGetName(char *parName, char *fileName, BOOL dirOnly)
{
  GFile gf;
  char *fullName;

  openFile(&gf, &fullName, parName, fileName, dirOnly);
  if (gf != NULL)
    GFileClose(gf);

  return fullName;
}

GFile
SetupOpen(char *parName, char *fileName, BOOL dirOnly)
{
  GFile gf;
  char *fullName;

  openFile(&gf, &fullName, parName, fileName, dirOnly);
  free(fullName);

  return gf;
}
