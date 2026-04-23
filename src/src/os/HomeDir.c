/*
************************************************************************
*
*   HomeDir.c - get home directory of user
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
*   Date of last modification : 99/10/30
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/src/os/SCCS/s.HomeDir.c
*   SCCS identification       : 1.6
*
************************************************************************
*/

#include <home_dir.h>

#include <stdlib.h>
#ifdef WIN32
#include <string.h>
#include <shlobj.h>
#else
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#endif

static char *HomeDir = NULL;

char *
HomeDirGet(void)
{
#ifdef WIN32
  LPITEMIDLIST pidl;
  char path[_MAX_PATH];
  HRESULT hr;
  BOOL succ;
  IMalloc *mallocInterfP;
  char *drive, *dir;
  int len, driveLen, dirLen, i;
#else
  uid_t id;
  struct passwd *pwP;
#endif

  if (HomeDir != NULL)
    return HomeDir;

#ifdef WIN32
  hr = SHGetSpecialFolderLocation(NULL, CSIDL_APPDATA, &pidl);
  if (hr == NOERROR)
    succ = SHGetPathFromIDList(pidl, path);
  else
    succ = FALSE;

  if (succ) {
    hr = SHGetMalloc(&mallocInterfP);
    if (hr == NOERROR) {
      mallocInterfP->lpVtbl->Free(mallocInterfP, pidl);
      mallocInterfP->lpVtbl->Release(mallocInterfP);
    }
    len = strlen(path);
    if (len > 0 && path[len - 1] == '\\')
       len--;
    HomeDir = malloc(len + 1);
    (void) strncpy(HomeDir, path, len);
  } else {
    drive = getenv("HOMEDRIVE");
    dir = getenv("HOMEPATH");
    if (drive != NULL && dir != NULL) {
      driveLen = strlen(drive);
      dirLen = strlen(dir);
      if (dirLen > 0 && dir[dirLen - 1] == '\\')
        dirLen--;
      len = driveLen + dirLen;
      HomeDir = malloc(len + 1);
      (void) strncpy(HomeDir, drive, driveLen);
      (void) strncpy(HomeDir + driveLen, dir, dirLen);
      succ = TRUE;
    }
  }

  if (succ) {
    HomeDir[len] = '\0';
    for (i = 0; i < len; i++)
      if (HomeDir[i] == '\\')
        HomeDir[i] = '/';
  }
#else
  id = getuid();
  pwP = getpwuid(id);
  if (pwP != NULL) {
    HomeDir = pwP->pw_dir;
  } else {
    HomeDir = getenv("HOME");
  }
#endif

  if (HomeDir == NULL) {
    HomeDir = malloc(1);
    HomeDir[0] = '\0';
  }

  return HomeDir;
}
