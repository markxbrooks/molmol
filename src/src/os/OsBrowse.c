/*
************************************************************************
*
*   OsBrowse.c - display files in web browser
*
*   Copyright (c) 1997-99
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/os/SCCS/s.OsBrowse.c
*   SCCS identification       : 1.8
*
************************************************************************
*/

#include <os_browse.h>

#include <stdlib.h>
#include <string.h>

#include <dstr.h>
#ifdef WIN32
#include <windows.h>
#include <ddeml.h>
#else
#include <os_system.h>
#endif

static BOOL BrowseInit = FALSE;
static BOOL BrowseStarted = FALSE;
static DSTR LastStartCmd;
static DSTR LastSpec;

#ifdef WIN32
static DWORD DdeInst;
static HCONV Conv;
#endif

char *
OsBrowseGetSpecPrompt(void)
{
#ifdef WIN32
  return "DDE Server";
#else
  return "Display Command";
#endif
}

int
OsBrowseGetListSize(void)
{
#ifdef WIN32
  return 2;
#else
  return 1;
#endif
}

void
OsBrowseGetListEntry(int ind, char **nameP, char **startCmdP, char **specP)
{
#ifdef WIN32
  if (ind == 0) {
    *nameP = "Netscape";
    *startCmdP = "netscape";
    *specP = "NSShell";
  } else {
    *nameP = "Internet Explorer";
    *startCmdP = "iexplore";
    *specP = "iexplore";
  }
#else
  *nameP = "Netscape";
  *startCmdP = "netscape file:%f";
  *specP = "netscape -remote openFile\\(%f\\)";
#endif
}

#ifdef WIN32
static HDDEDATA CALLBACK
ddeCB(UINT type, UINT fmt, HCONV conv, HSZ str1, HSZ str2,
    HDDEDATA data, DWORD data1, DWORD data2)
{
  return 0;
}
#else
BOOL execCmd(char *cmd, char *fileName, BOOL background)
{
  DSTR cmdStr;
  BOOL succ;

  cmdStr = DStrNew();
  while (*cmd != '\0') {
    if (cmd[0] == '%' && cmd[1] == 'f') {
      DStrAppStr(cmdStr, fileName);
      cmd++;
    } else {
      DStrAppChar(cmdStr, *cmd);
    }
    cmd++;
  }
  if (background)
    DStrAppStr(cmdStr, " &");
  else
    DStrAppStr(cmdStr, " 2>/dev/null");
  succ = OsSystem(DStrToStr(cmdStr));
  DStrFree(cmdStr);

  return succ;
}
#endif

OsBrowseRes
OsBrowseShow(char *startCmd, char *spec, char *fileName)
{
  BOOL startup;
  BOOL succ;
#ifdef WIN32
  HSZ serv, topic;
  int len, i;
  char *name;
#else
#endif

  if (! BrowseInit) {
#ifdef WIN32
    if (DdeInitialize(&DdeInst, (PFNCALLBACK) ddeCB,
        APPCLASS_STANDARD | APPCMD_CLIENTONLY, 0) != DMLERR_NO_ERROR)
      return OBR_STARTUP_FAILED;
#endif
    BrowseInit = TRUE;
  }

  if (BrowseStarted) {
    if (strcmp(startCmd, DStrToStr(LastStartCmd)) == 0 &&
	strcmp(spec, DStrToStr(LastSpec)) == 0) {
      startup = FALSE;
    } else {
      OsBrowseCleanup();
      startup = TRUE;
    }
  } else {
    startup = TRUE;
  }

#ifdef WIN32
  if (startup) {
    serv = DdeCreateStringHandle(DdeInst, spec, 0);
    topic = DdeCreateStringHandle(DdeInst, "WWW_ShowFile", 0);
    Conv = DdeConnect(DdeInst, serv, topic, NULL);
    if (Conv == 0) {
      WinExec(startCmd, SW_SHOW);
      Conv = DdeConnect(DdeInst, serv, topic, NULL);
    }

    DdeFreeStringHandle(DdeInst, serv);
    DdeFreeStringHandle(DdeInst, topic);

    if (Conv == 0) {
      (void) DdeUninitialize(DdeInst);
      return OBR_STARTUP_FAILED;
    }
  }

  len = strlen(fileName);
  name = malloc(len + 1);
  (void) strcpy(name, fileName);
  for (i = 0; i < len; i++)
    if (name[i] == '/')
      name[i] = '\\';
  succ = (DdeClientTransaction(name, len + 1,
      Conv, 0, 0, XTYP_EXECUTE, TIMEOUT_ASYNC, NULL) != NULL);
  free(name);

  if (! succ) {
    if (startup) {
      (void) DdeDisconnect(Conv);
      (void) DdeUninitialize(DdeInst);
    } else {
      OsBrowseCleanup();
    }
    return OBR_DISPLAY_FAILED;
  }
#else
  succ = execCmd(spec, fileName, FALSE);
  if (! succ) {
    if (startup) {
      if (! execCmd(startCmd, fileName, TRUE))
        return OBR_STARTUP_FAILED;
    } else {
      return OBR_DISPLAY_FAILED;
    }
  }
#endif

  if (startup) {
    LastStartCmd = DStrNew();
    DStrAssignStr(LastStartCmd, startCmd);
    LastSpec = DStrNew();
    DStrAssignStr(LastSpec, spec);

    BrowseStarted = TRUE;
  }

  return OBR_OK;
}

void
OsBrowseCleanup(void)
{
  if (! BrowseStarted)
    return;

#ifdef WIN32
  (void) DdeDisconnect(Conv);
#endif

  DStrFree(LastStartCmd);
  DStrFree(LastSpec);

  BrowseStarted = FALSE;
}
