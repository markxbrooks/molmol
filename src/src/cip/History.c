/*
************************************************************************
*
*   History.c - history management
*
*   Copyright (c) 1994-98
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
*   Date of last modification : 98/08/17
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cip/SCCS/s.History.c
*   SCCS identification       : 1.9
*
************************************************************************
*/

#include <history.h>

#include <stdio.h>
#include <string.h>

#include <bool.h>
#include <dstr.h>
#include <linlist.h>
#include <pu.h>
#include <user_file.h>
#include <cip.h>

#define MAX_KEEP_NO 50
#define MAX_LOG_WIN 200
#define DEL_LOG_WIN 10

#define ERROR_MARK "#ERROR: "
#define CANCEL_MARK "#CANCELLED: "

typedef struct {
  HistoryCB historyCB;
  void *clientData;
} HistoryCBData;

static LINLIST HistoryCBList = NULL;

static GFile HistoryFile;
static PuGizmo LogWin, LogWinList;
static int LogWinSize = 0;
static BOOL Switch = TRUE;
static DSTR ArgStr = NULL, LineStr;
static DSTR LastTab[MAX_KEEP_NO];
static int KeepNo = 0, LastInd = -1;

static void
selectCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  PuSelectCBStruc *callP = callData;

  if (! callP->onOff)
    return;

  if (strncmp(name, ERROR_MARK, strlen(ERROR_MARK)) == 0)
    CipExecCmd(name + strlen(ERROR_MARK));
  else if (strncmp(name, CANCEL_MARK, strlen(CANCEL_MARK)) == 0)
    CipExecCmd(name + strlen(CANCEL_MARK));
  else
    CipExecCmd(name);
}

static void
closeHistory(void *clientData)
{
  if (HistoryFile != NULL)
    GFileClose(HistoryFile);
}

static void
popdownCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  PuSwitchGizmo(LogWin, FALSE);
}

static void
createLogWin(void)
{
  PuConstraints con;

  LogWin = PuCreateDialog("Log Window", 1, 1);
  PuSetBool(LogWin, PU_BC_PLACE_OUTSIDE, TRUE);

  LogWinList = PuCreateList(LogWin, "History");
  con.x = 0;
  con.y = 0;
  con.w = 1;
  con.h = 1;
  PuSetConstraints(LogWinList, con);
  PuSetBool(LogWinList, PU_BC_MULT_SEL, FALSE);
  PuSetBool(LogWinList, PU_BC_AUTO_DESEL, TRUE);
  PuSetBool(LogWinList, PU_BC_SHOW_BOTTOM, TRUE);
  PuSetInt(LogWinList, PU_IC_ENTRY_NO, 10);
  PuAddGizmoCB(LogWinList, PU_CT_SELECT, selectCB, NULL, NULL);

  /* hack to enforce a reasonable width */
  PuAddListEntry(LogWinList,
      "                                        ", FALSE);
  PuSwitchGizmo(LogWinList, TRUE);
  PuRemoveListEntries(LogWinList, 0, 1);

  PuAddGizmoCB(LogWin, PU_CT_CLOSE, popdownCB, NULL, NULL);
}

static void
checkInit(void)
{
  int i;

  if (ArgStr == NULL) {
    ArgStr = DStrNew();
    LineStr = DStrNew();

    HistoryFile = UserFileOpenWrite("history", GF_FORMAT_ASCII);
    PuAddQuitCB(closeHistory, NULL, NULL);

    createLogWin();

    for (i = 0; i < MAX_KEEP_NO; i++)
      LastTab[i] = DStrNew();
  }
}

void
HistoryArgIdent(char *s)
{
  checkInit();

  if (Switch) {
    DStrAppStr(ArgStr, s);
    DStrAppChar(ArgStr, ' ');
  }
}

void
HistoryArgStr(char *s)
{
  int len, i;
  char quoteCh;

  checkInit();

  if (Switch) {
    quoteCh = '\'';
    len = strlen(s);
    for (i = 0; i < len; i++)
      if (s[i] == '\'')
	quoteCh = '"';

    DStrAppChar(ArgStr, quoteCh);
    for (i = 0; i < len; i++)
      DStrAppChar(ArgStr, s[i]);
    DStrAppChar(ArgStr, quoteCh);
    DStrAppChar(ArgStr, ' ');
  }
}

void
HistoryArgExpr(char *s)
{
  int len, i;
  BOOL needPara;

  checkInit();

  if (Switch) {
    needPara = FALSE;
    len = strlen(s);
    for (i = 0; i < len; i++)
      if (s[i] == ' ')
	needPara = TRUE;

    if (needPara)
      DStrAppChar(ArgStr, '(');
    for (i = 0; i < len; i++)
      DStrAppChar(ArgStr, s[i]);
    if (needPara)
      DStrAppChar(ArgStr, ')');
    DStrAppChar(ArgStr, ' ');
  }
}

static void
writeLine(BOOL executed)
{
  HistoryCBData *dataP;

  if (HistoryFile != NULL) {
    GFileWriteStr(HistoryFile, DStrToStr(LineStr));
    GFileWriteNL(HistoryFile);
    GFileFlush(HistoryFile);
  }

  if (LogWinSize >= MAX_LOG_WIN) {
    PuRemoveListEntries(LogWinList, 0, DEL_LOG_WIN);
    LogWinSize -= DEL_LOG_WIN;
  }
  PuAddListEntry(LogWinList, DStrToStr(LineStr), FALSE);
  LogWinSize++;

  if (! executed)
    return;

  /* ring buffer */
  LastInd = (LastInd + 1) % MAX_KEEP_NO;
  DStrAssignDStr(LastTab[LastInd], LineStr);
  if (KeepNo < MAX_KEEP_NO)
    KeepNo++;
  
  dataP = ListFirst(HistoryCBList);
  while (dataP != NULL) {
    dataP->historyCB(DStrToStr(LineStr), dataP->clientData);
    dataP = ListNext(HistoryCBList, dataP);
  }
}

void
HistoryEndCmd(char *cmd, HistoryCmdResult res)
{
  checkInit();

  if (Switch) {
    if (res == HCR_EXECUTED)
      DStrAssignStr(LineStr, "");
    else if (res == HCR_CANCELLED)
      DStrAssignStr(LineStr, CANCEL_MARK);
    else
      DStrAssignStr(LineStr, ERROR_MARK);

    DStrAppStr(LineStr, cmd);
    DStrAppChar(LineStr, ' ');
    DStrAppDStr(LineStr, ArgStr);
    writeLine(res == HCR_EXECUTED);

    DStrAssignStr(ArgStr, "");
  }
}

void
HistoryComment(char *s)
{
  checkInit();

  if (Switch) {
    DStrAssignStr(LineStr, "# ");
    DStrAppStr(LineStr, s);
    writeLine(FALSE);
  }
}

char *
HistoryGetCmd(int ind)
/* 0: last command, 1: previous, ... */
{
  if (ind >= KeepNo)
    return NULL;

  return DStrToStr(LastTab[(LastInd - ind + MAX_KEEP_NO) % MAX_KEEP_NO]);
}

void
HistoryAddCB(HistoryCB historyCB, void *clientData)
{
  HistoryCBData data;

  if (HistoryCBList == NULL)
    HistoryCBList = ListOpen(sizeof(HistoryCBData));
  
  data.historyCB = historyCB;
  data.clientData = clientData;

  (void) ListInsertLast(HistoryCBList, &data);
}

void
HistoryRemoveCB(HistoryCB historyCB, void *clientData)
{
  HistoryCBData *dataP;

  dataP = ListFirst(HistoryCBList);
  while (dataP != NULL) {
    if (dataP->historyCB == historyCB && dataP->clientData == clientData) {
      ListRemove(HistoryCBList, dataP);
      break;
    }
    dataP = ListNext(HistoryCBList, dataP);
  }
}

void
HistorySwitch(BOOL onOff)
{
  Switch = onOff;
}

void
HistoryLogWin(BOOL onOff)
{
  checkInit();
  PuSwitchGizmo(LogWin, onOff);
}
