/*
************************************************************************
*
*   Cip.c - command interpreter
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
*   Date of last modification : 01/05/27
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cip/SCCS/s.Cip.c
*   SCCS identification       : 1.46
*
************************************************************************
*/

#include <cip.h>
#include <cmd_help.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <break.h>
#include <strmatch.h>
#include <linlist.h>
#include <os_browse.h>
#include <pu.h>
#include <g_file.h>
#include <home_dir.h>
#include <par_names.h>
#include <par_hand.h>
#include <arg.h>
#include <graph_draw.h>
#include <history.h>
#include "sel_history.h"
#include "text_cmd.h"
#include "drawobj_cmd.h"
#include "cmd_tab.h"
#include "arg_p.h"

typedef enum {
  CS_CMD_INPUT,
  CS_ARG_INPUT,
  CS_EXEC
} CipState;

typedef struct {
  DhMolListCB updateCB;
  void *clientData;
} UpdateData;

typedef struct {
  char *cmd;
  char *menu;
} MenuEntry;

static CmdTabEntry *CmdTab;
static int CmdTabSize = 0;

static LINLIST UpdateList = NULL;
static char HelpDir[200];
static BOOL HelpMode = FALSE, FirstHelp = TRUE;
static LINLIST MenuEntryList = NULL;
static CipMenuAddCB MenuAddCB = NULL;
static CipState CurrState;
static DSTR ErrStr = NULL;
static int MacroLevel = 0;
static int CmdDefLevel = 0;
static int SeqLevel = 0;
static DSTR ExtCmd = NULL;

void
CipAddUpdateCB(CipUpdateCB updateCB, void *clientData)
{
  UpdateData entry;

  entry.updateCB = updateCB;
  entry.clientData = clientData;

  if (UpdateList == NULL)
    UpdateList = ListOpen(sizeof(UpdateData));

  (void) ListInsertLast(UpdateList, &entry);
}

void
CipRemoveUpdateCB(CipUpdateCB updateCB, void *clientData)
{
  UpdateData *entryP;

  entryP = ListFirst(UpdateList);
  while (entryP != NULL) {
    if (entryP->updateCB == updateCB && entryP->clientData == clientData) {
      ListRemove(UpdateList, entryP);
      return;
    }
    entryP = ListNext(UpdateList, entryP);
  }
}

void
callUpdateCB(void)
{
  UpdateData *entryP, *nextEntryP;

  entryP = ListFirst(UpdateList);
  while (entryP != NULL) {
    /* entryP could be removed from callback */
    nextEntryP = ListNext(UpdateList, entryP);
    entryP->updateCB(entryP->clientData);
    entryP = nextEntryP;
  }
}

ErrCode
ExHelpCmd(char *cmd)
{
  HelpMode = TRUE;

  PuSwitchTextField(PU_TF_PROMPT, TRUE);

  if (FirstHelp) {
    PuSetTextField(PU_TF_PROMPT, "Help");
    FirstHelp = FALSE;
  }

  return EC_OK;
}

static void
helpOff(void)
{
  HelpMode = FALSE;

  PuSwitchTextField(PU_TF_PROMPT, FALSE);
}

static int
findCmd(DSTR cmd)
{
  char *cmdP = DStrToStr(cmd);
  int left, right, middle;

  /* binary search */
  left = 0;
  right = CmdTabSize;
  while (left < right) {
    middle = (left + right) / 2;
    if (strcmp(CmdTab[middle].cmd, cmdP) < 0)
      left = middle + 1;
    else
      right = middle;
  }

  return left;
}

static CmdTabEntry *
getCmd(DSTR cmd)
{
  int ind;

  ind = findCmd(cmd);
  if (ind < CmdTabSize && strcmp(CmdTab[ind].cmd, DStrToStr(cmd)) == 0)
    return CmdTab + ind;
  else
    return NULL;
}

void
CipShowHelpFile(PuGizmo g, char *cmd)
{
  DSTR cmdStr, fileName;
  BOOL useBrowser;
  OsBrowseRes browseRes;
  CmdTabEntry *entryP;

  cmdStr = DStrNew();
  DStrAssignStr(cmdStr, cmd);
  entryP = getCmd(cmdStr);
  DStrFree(cmdStr);

  if (entryP == NULL || entryP->defP == NULL) {
    useBrowser = (ParDefined(PN_HELP_VIEWER) &&
        strcmp(ParGetStrVal(PN_HELP_VIEWER), "browser") == 0);

    if (useBrowser && ! ParDefined(PN_BROWSE_CMD))
      useBrowser = FALSE;

    if (useBrowser && ! ParDefined(PN_BROWSE_SPEC))
      useBrowser = FALSE;

    fileName = DStrNew();

    if (useBrowser) {
      DStrAssignStr(fileName, HelpDir);
      DStrAppStr(fileName, "html/");
      DStrAppStr(fileName, cmd);
      DStrAppStr(fileName, ".html");

      browseRes = OsBrowseShow(
          ParGetStrVal(PN_BROWSE_CMD),
	  ParGetStrVal(PN_BROWSE_SPEC),
	  DStrToStr(fileName));
      if (browseRes != OBR_OK) {
	PuShowMessage(PU_MT_WARNING, "Help", "web browser display failed");
        ParSetStrVal(PN_HELP_VIEWER, "builtin");
        useBrowser = FALSE;
      }
    }

    if (! useBrowser) {
      DStrAssignStr(fileName, HelpDir);
      DStrAppStr(fileName, "ascii/");
      DStrAppStr(fileName, cmd);
      PuShowHelp(g, "Help", DStrToStr(fileName));
    }

    DStrFree(fileName);
  } else {
    PuShowHelp(g, "Help", DStrToStr(entryP->defP->fileName));
  }
}

static void
initCmdTab(void)
{
  int i;

  if (CmdTabSize > 0)
    return;

  CmdTabSize = sizeof(BuiltinCmdTab) / sizeof(*BuiltinCmdTab);
  CmdTab = malloc(CmdTabSize * sizeof(*CmdTab));
  for (i = 0; i < CmdTabSize; i++)
    CmdTab[i] = BuiltinCmdTab[i];
}

void
CipSetMenu(char *cmdP, char *menu)
{
  DSTR cmdStr = DStrNew();
  MenuEntry entry;
  int ind, i;

  i = 0;
  while (isalnum(cmdP[i])) {
    DStrAppChar(cmdStr, cmdP[i]);
    i++;
  }

  initCmdTab();
  ind = findCmd(cmdStr);
  if (ind < CmdTabSize &&
      strcmp(CmdTab[ind].cmd, DStrToStr(cmdStr)) == 0) {
    if (CmdTab[ind].menu == NULL) {
      CmdTab[ind].menu = malloc(strlen(menu) + 1);
      (void) strcpy(CmdTab[ind].menu, menu);
    }
  } else {
    entry.cmd = malloc(strlen(cmdP) + 1);
    (void) strcpy(entry.cmd, cmdP);
    entry.menu = malloc(strlen(menu) + 1);
    (void) strcpy(entry.menu, menu);

    if (MenuEntryList == NULL)
      MenuEntryList = ListOpen(sizeof(MenuEntry));

    (void) ListInsertLast(MenuEntryList, &entry);
  }

  DStrFree(cmdStr);
}

void
CipSetMenuAddCB(CipMenuAddCB menuAddCB)
{
  MenuAddCB = menuAddCB;
}

static char *
getHelpLine(CmdTabEntryP entryP)
{
  DSTR fileName;
  GFile gf;
  GFileRes res;
  char buf[100], *str;
  int len;

  if (entryP->helpLine != NULL)
    return entryP->helpLine;

  entryP->helpLine = " ";
  len = strlen(entryP->cmd);

  fileName = DStrNew();
  DStrAssignStr(fileName, HelpDir);
  DStrAppStr(fileName, "ascii/");
  DStrAppStr(fileName, entryP->cmd);

  gf = GFileOpenRead(DStrToStr(fileName));
  DStrFree(fileName);
  if (gf == NULL)
    return entryP->helpLine;

  while (! GFileEOF(gf)) {
    res = GFileReadLine(gf, buf, sizeof(buf));
    if (res != GF_RES_OK)
      break;
    
    if (strcmp(buf, "SYNOPSIS") == 0)  /* not found */
      break;
    
    str = buf;
    while (str[0] != '\0' && isspace(str[0]))
      str++;
    
    if (strncmp(str, entryP->cmd, len) == 0) {
      entryP->helpLine = malloc(strlen(str) + 1);
      (void) strcpy(entryP->helpLine, str);
      break;
    }
  }

  GFileClose(gf);

  return entryP->helpLine;
}

static char *
getLowCopy(char *str)
{
  char *strLow;
  int len, i;

  len = strlen(str);
  strLow = malloc(len + 1);
  for (i = 0; i < len; i++)
    if (isupper(str[i]))
      strLow[i] = tolower(str[i]);
    else
      strLow[i] = str[i];
  strLow[len] = '\0';

  return strLow;
}

void
CipHelpApropos(char *pat)
{
  PuTextWindow textW;
  char *patLow, *help, *helpLow;
  DSTR patStr, helpStr;
  int ind;

  textW = PuCreateTextWindow("HelpApropos");

  patLow = getLowCopy(pat);

  patStr = DStrNew();
  DStrAssignStr(patStr, "*");
  DStrAppStr(patStr, patLow);
  DStrAppStr(patStr, "*");

  free(patLow);

  for (ind = 0; ind < CmdTabSize; ind++) {
    help = getHelpLine(CmdTab + ind);
    helpLow = getLowCopy(help);
    if (StrMatch(helpLow, DStrToStr(patStr))) {
      if (CmdTab[ind].menu == NULL) {
	PuWriteStr(textW, help);
      } else {
	helpStr = DStrNew();
	while (isalnum(*help)) {
	  DStrAppChar(helpStr, *help);
	  help++;
	}
	DStrAppStr(helpStr, " (");
	DStrAppStr(helpStr, CmdTab[ind].menu);
	DStrAppStr(helpStr, ")");
	DStrAppStr(helpStr, help);
	PuWriteStr(textW, DStrToStr(helpStr));
	DStrFree(helpStr);
      }

      PuWriteStr(textW, "\n");
    }
    free(helpLow);
  }

  DStrFree(patStr);
}

void
CipShowHelpLine(char *cmdP)
{
  int len;
  DSTR cmdStr = DStrNew();
  int ind, i;

  DStrAssignStr(cmdStr, cmdP);
  len = DStrLen(cmdStr);

  for (i = 0; i < len; i++)
    if (cmdP[i] == ' ')
      break;
  
  if (i < len) {  /* command with arguments, display as is */
    PuSetTextField(PU_TF_STATUS, cmdP);
  } else {
    ind = findCmd(cmdStr);
    if (ind < CmdTabSize && strcmp(CmdTab[ind].cmd, cmdP) == 0)
      PuSetTextField(PU_TF_STATUS, getHelpLine(CmdTab + ind));
  }

  DStrFree(cmdStr);
}

static ErrCode
replaceArg(DSTR inStr, LINLIST argList, DSTR outStr)
{
  ErrCode errCode;
  char *str;
  DSTR varName;
  BOOL inBrace;
  ArgDef *argDefP;

  DStrAssignStr(outStr, "");
  varName = DStrNew();

  errCode = EC_OK;
  str = DStrToStr(inStr);
  while (*str != '\0') {
    if (*str == '$') {
      str++;
      DStrAssignStr(varName, "");

      inBrace = (*str == '{');
      if (inBrace)
	str++;
      
      while (isalnum(*str) || *str == '_') {
	DStrAppChar(varName, *str);
	str++;
      }
      
      if (inBrace) {
	if (*str == '}') {
	  str++;
	} else {
	  CipSetError("error in argument reference");
	  errCode = EC_ERROR;
	  break;
	}
      }

      argDefP = ListFirst(argList);
      while (argDefP != NULL) {
	if (DStrCmp(argDefP->name, varName) == 0)
	  break;
	argDefP = ListNext(argList, argDefP);
      }

      if (argDefP == NULL) {
	CipSetError("unknown argument reference");
	errCode = EC_ERROR;
	break;
      }

      DStrAppDStr(outStr, argDefP->val);
    } else {
      if (*str == '\\' && str[1] == '$')
	str++;
      DStrAppChar(outStr, *str);
      str++;
    }
  }

  DStrFree(varName);

  return errCode;
}

static ErrCode
execDef(CmdTabEntry *entryP)
{
  int argNum, argI;
  ArgDescrP argA;
  ArgDef *argDefP;
  ErrCode errCode;
  DSTR *strP, cmdStr;

  argNum = ListSize(entryP->defP->argList);
  argA = malloc(argNum * sizeof(*argA));

  for (argI = 0; argI < argNum; argI++)
    argA[argI].type = AT_STR;
  
  ArgInit(argA, argNum);

  argDefP = ListFirst(entryP->defP->argList);
  argI = 0;
  while (argDefP != NULL) {
    argA[argI].prompt = DStrToStr(argDefP->prompt);
    if (DStrLen(argDefP->defaultVal) > 0)
      DStrAssignDStr(argA[argI].v.strVal, argDefP->defaultVal);
    else
      DStrAssignDStr(argA[argI].v.strVal, argDefP->val);

    argDefP = ListNext(entryP->defP->argList, argDefP);
    argI++;
  }

  errCode = ArgGet(argA, argNum);
  if (errCode == EC_OK) {
    argDefP = ListFirst(entryP->defP->argList);
    argI = 0;
    while (argDefP != NULL) {
      DStrAssignDStr(argDefP->val, argA[argI].v.strVal);
      argDefP = ListNext(entryP->defP->argList, argDefP);
      argI++;
    }

    CmdDefLevel++;
    HistorySwitch(FALSE);

    cmdStr = DStrNew();

    strP = ListFirst(entryP->defP->cmdSeq);
    while (strP != NULL) {
      errCode = replaceArg(*strP, entryP->defP->argList, cmdStr);
      if (errCode != EC_OK)
	break;

      errCode = CipExecCmd(DStrToStr(cmdStr));
      if (errCode != EC_OK)
	break;

      strP = ListNext(entryP->defP->cmdSeq, strP);
    }

    CmdDefLevel--;
    if (CmdDefLevel == 0)
      HistorySwitch(TRUE);

    DStrFree(cmdStr);
  }

  ArgCleanup(argA, argNum);
  free(argA);

  return errCode;
}

ErrCode
CipExecCmd(char *cmdP)
{
  int totLen, cmdLen, argLen;
  char *argP;
  DSTR cmdStr, argStr;
  int ind;
  ErrCode res;
  BOOL interrupted;

  if (cmdP[0] == '\0')  /* allow empty line */
    return EC_OK;

  if (cmdP[0] == '#')  /* comment in macro */
    return EC_OK;

  if (SeqLevel > 0 && BreakInterrupted())
    return EC_WARNING;

  cmdStr = DStrNew();
  argStr = DStrNew();

  DStrAssignStr(cmdStr, cmdP);
  totLen = DStrLen(cmdStr);
  cmdLen = 0;
  while (isalnum(cmdP[cmdLen]))
    cmdLen++;
  DStrPart(cmdStr, 0, cmdLen - 1);
  
  argP = cmdP + cmdLen;
  argLen = totLen - cmdLen;
  while (isspace(argP[0])) {
    argP++;
    argLen--;
  }
  DStrAssignStr(argStr, argP);
  while (isspace(argP[argLen - 1]))
    argLen--;
  DStrPart(argStr, 0, argLen - 1);

  ind = findCmd(cmdStr);
  cmdP = DStrToStr(cmdStr);
  if (ind == CmdTabSize || strcmp(CmdTab[ind].cmd, cmdP) != 0) {
    PuShowMessage(PU_MT_ERROR, DStrToStr(cmdStr), "unknown command");
    DStrFree(cmdStr);
    DStrFree(argStr);
    return EC_ERROR;
  }

  if (HelpMode) {
    CipShowHelpFile(NULL, DStrToStr(cmdStr));
    helpOff();
    DStrFree(cmdStr);
    DStrFree(argStr);
    return EC_OK;
  }

  if (ErrStr == NULL)
    ErrStr = DStrNew();
  DStrAssignStr(ErrStr, "");

  ArgSetCmd(cmdStr);
  ArgSet(argStr);
  GraphRedrawEnable(FALSE);
  PuSetTextField(PU_TF_STATUS, " ");

  if (MacroLevel == 0 && CmdDefLevel == 0 && SeqLevel == 0) {
    PuSetBusyCursor(TRUE);
    UndoSave(CmdTab[ind].undo);
  }

  BreakReset();

  if (CmdTab[ind].func == NULL)
    res = execDef(CmdTab + ind);
  else
    res = CmdTab[ind].func(DStrToStr(cmdStr));

  interrupted = BreakInterrupted();

  if (MacroLevel == 0 && CmdDefLevel == 0 && SeqLevel == 0)
    PuSetBusyCursor(FALSE);

  GraphRedrawEnable(TRUE);
  if (res == EC_WARNING)
    PuShowMessage(PU_MT_WARNING, DStrToStr(cmdStr), DStrToStr(ErrStr));
  else if (res == EC_ERROR)
    PuShowMessage(PU_MT_ERROR, DStrToStr(cmdStr), DStrToStr(ErrStr));
  else if (interrupted)
    PuShowMessage(PU_MT_WARNING, DStrToStr(cmdStr), "command interrupted");

  if (res == EC_ERROR)
    HistoryEndCmd(DStrToStr(cmdStr), HCR_ERROR);
  else if (res == EC_CANCEL)
    HistoryEndCmd(DStrToStr(cmdStr), HCR_CANCELLED);
  else
    HistoryEndCmd(DStrToStr(cmdStr), HCR_EXECUTED);

  DStrFree(cmdStr);
  DStrFree(argStr);

  if (MacroLevel == 0 && CmdDefLevel == 0 && SeqLevel == 0)
    callUpdateCB();

  return res;
}

static CipMatch
completeCmd(DSTR cmdStr, char newCh, DSTR appStr)
{
  char *cmdP, *firstCmdP;
  DSTR searchStr;
  int len, complLen;
  int ind;
  BOOL unique;

  cmdP = DStrToStr(cmdStr);
  len = DStrLen(cmdStr);
  ind = findCmd(cmdStr);
  if (isupper(CmdTab[ind].cmd[len]))
    newCh = toupper(newCh);

  searchStr = DStrNew();
  DStrAssignDStr(searchStr, cmdStr);
  DStrAppChar(searchStr, newCh);
  cmdP = DStrToStr(searchStr);
  len++;
  ind = findCmd(searchStr);
  if (ind == CmdTabSize || strncmp(CmdTab[ind].cmd, cmdP, len) != 0) {
    DStrPart(appStr, 0, 0);
    DStrFree(searchStr);
    return CM_NONE;
  }

  firstCmdP = CmdTab[ind].cmd;
  complLen = len;
  while (islower(firstCmdP[complLen]))
    complLen++;

  ind++;
  unique = TRUE;
  while (ind < CmdTabSize &&
      strncmp(CmdTab[ind].cmd, cmdP, len) == 0) {
    if (strncmp(CmdTab[ind].cmd, firstCmdP, complLen) != 0) {
      unique = FALSE;
      break;
    }
    ind++;
  }

  if (! unique)
    complLen = len;

  DStrAssignStr(appStr, firstCmdP);
  DStrPart(appStr, len - 1, complLen - 1);

  DStrFree(searchStr);

  if (complLen == (int) strlen(firstCmdP))
    return CM_UNIQUE;
  else
    return CM_MULTIPLE;
}

static int
blankPos(char *s)
{
  int blankPos;

  blankPos = 0;
  while (*s != '\0') {
    if (*s == ' ')
      return blankPos;
    s++;
    blankPos++;
  }
  
  return 0;
}

static void
cmdCB(PuWindow win, void *clientData, PuTextCBStruc *callP)
{
  DSTR oldStr, appStr, cmdStr;
  int ind;

  if (CurrState == CS_EXEC) {
    /* editing command line during execution leads to a crash,
       this is the easiest fix */
    callP->doIt = FALSE;
    return;
  }

  if (callP->action == PU_CA_CR) {
    CurrState = CS_EXEC;
    (void) CipExecCmd(callP->newText);
    CurrState = CS_CMD_INPUT;
    return;
  }

  if (CurrState == CS_CMD_INPUT &&
      callP->action == PU_CA_APPEND &&
      strlen(callP->newPart) == 1) {
    oldStr = DStrNew();
    appStr = DStrNew();
    DStrAssignStr(oldStr, callP->oldText);

    if (completeCmd(oldStr, callP->newPart[0], appStr) == CM_UNIQUE) {
      DStrAppDStr(oldStr, appStr);
      PuSetTextField(PU_TF_STATUS,
	  getHelpLine(CmdTab + findCmd(oldStr)));

      DStrAppChar(appStr, ' ');
      CurrState = CS_ARG_INPUT;
    }

    callP->newPart = realloc(callP->newPart, DStrLen(appStr) + 1);
    (void) strcpy(callP->newPart, DStrToStr(appStr));

    DStrFree(oldStr);
    DStrFree(appStr);
  } else if (CurrState == CS_CMD_INPUT &&
      callP->action == PU_CA_APPEND) {
    if (blankPos(callP->newText) > 0)
      CurrState = CS_ARG_INPUT;
  } else if (callP->action == PU_CA_DELETE &&
      callP->oldPartStart <= blankPos(callP->oldText) &&
      callP->oldPartEnd < callP->oldLen) {
    callP->doIt = FALSE;
  } else if (CurrState == CS_ARG_INPUT &&
      callP->action == PU_CA_DELETE) {
    if (blankPos(callP->newText) == 0) {
      cmdStr = DStrNew();
      DStrAssignStr(cmdStr, callP->newText);
      ind = findCmd(cmdStr);
      DStrFree(cmdStr);
      if (ind == CmdTabSize ||
	  strcmp(CmdTab[ind].cmd, callP->newText) != 0) {
        PuSetTextField(PU_TF_STATUS, " ");
        CurrState = CS_CMD_INPUT;
      }
    }
  } else if (callP->action == PU_CA_INSERT &&
      callP->newPartStart <= blankPos(callP->newText)) {
    callP->doIt = FALSE;
  } else if (callP->action == PU_CA_REPLACE &&
      callP->oldPartStart <= blankPos(callP->newText)) {
    callP->doIt = FALSE;
  }
}

static void
freeCmdDef(CmdTabEntry *entryP)
{
  ArgDef *argDefP;
  DSTR *strP;

  free(entryP->cmd);

  if (entryP->helpLine != NULL)
    free(entryP->helpLine);

  DStrFree(entryP->defP->fileName);

  argDefP = ListFirst(entryP->defP->argList);
  while (argDefP != NULL) {
    DStrFree(argDefP->name);
    DStrFree(argDefP->prompt);
    DStrFree(argDefP->defaultVal);
    DStrFree(argDefP->val);
    argDefP = ListNext(entryP->defP->argList, argDefP);
  }

  strP = ListFirst(entryP->defP->cmdSeq);
  while (strP != NULL) {
    DStrFree(*strP);
    strP = ListNext(entryP->defP->cmdSeq, strP);
  }

  free(entryP->defP);
}

static char *
skipSpace(char *str)
{
  while (isspace(*str))
    str++;

  return str;
}

static char *
getStr(char *inpStr, DSTR dstr)
{
  char quoteCh;

  inpStr = skipSpace(inpStr);

  quoteCh = inpStr[0];
  if (quoteCh == '"' || quoteCh == '\'') {
    inpStr++;
    while (inpStr[0] != '\0' && inpStr[0] != quoteCh) {
      if (inpStr[0] == '\\' && inpStr[1] == quoteCh)
	inpStr++;
      DStrAppChar(dstr, inpStr[0]);
      inpStr++;
    }
    if (inpStr[0] == quoteCh)
      inpStr++;
  } else {
    while (inpStr[0] != '\0' && ! isspace(inpStr[0])) {
      DStrAppChar(dstr, inpStr[0]);
      inpStr++;
    }
  }

  return inpStr;
}

static ErrCode
getCmdDef(char *fileName, GFile gf, char *cmdP)
{
  DSTR cmdStr;
  CmdTabEntry *entryP;
  CmdTabEntry entry;
  ErrCode errCode;
  GFileRes gRes;
  BOOL argMode;
  char buf[500], *str;
  ArgDef argDef, *argDefP;
  MenuEntry *menuEntryP;
  int cmdLen, insertInd, ind;

  cmdStr = DStrNew();
  DStrAssignStr(cmdStr, cmdP);
  entryP = getCmd(cmdStr);
  DStrFree(cmdStr);
  if (entryP != NULL && entryP->defP == NULL) {
    CipSetError("cannot override builtin command");
    return EC_ERROR;
  }

  cmdLen = strlen(cmdP);
  entry.cmd = malloc(cmdLen + 1);
  (void) strcpy(entry.cmd, cmdP);

  entry.func = NULL;
  entry.undo = US_ALL;
  entry.menu = NULL;
  entry.helpLine = NULL;

  entry.defP = malloc(sizeof(*entry.defP));
  entry.defP->fileName = DStrNew();
  DStrAssignStr(entry.defP->fileName, fileName);
  entry.defP->argList = ListOpen(sizeof(ArgDef));
  entry.defP->cmdSeq = ListOpen(sizeof(DSTR));

  errCode = EC_OK;
  argMode = TRUE;
  for (;;) {
    if (GFileEOF(gf)) {
      CipSetError("missing end in command definition");
      errCode = EC_ERROR;
      break;
    }

    gRes = GFileReadLine(gf, buf, sizeof(buf));
    if (gRes != GF_RES_OK) {
      errCode = EC_ERROR;
      break;
    }

    str = skipSpace(buf);

    if (str[0] == '\0' || str[0] == '#')
      continue;

    if (strncmp(str, "end", 3) == 0) {
      if (argMode) {
	CipSetError("empty command definition");
	errCode = EC_ERROR;
      }

      break;
    }

    if (strncmp(str, "help", 4) == 0) {
      str += 5;
      entry.helpLine = malloc(cmdLen + 3 + strlen(str) + 1);
      (void) strcpy(entry.helpLine, cmdP);
      (void) strcpy(entry.helpLine + cmdLen, " - ");
      (void) strcpy(entry.helpLine + cmdLen + 3, str);
    } else if (str[0] == '$') {
      if (! argMode) {
	CipSetError("argument not at start of command definition");
	errCode = EC_ERROR;
	break;
      }

      argDef.name = DStrNew();
      argDef.prompt = DStrNew();
      argDef.defaultVal = DStrNew();
      argDef.val = DStrNew();
      argDefP = ListInsertLast(entry.defP->argList, &argDef);

      str++;
      while (isalnum(*str) || *str == '_') {
	DStrAppChar(argDefP->name, *str);
	str++;
      }

      str = skipSpace(str);
      if (str[0] == '=')
	str++;
      else
	DStrAppDStr(argDefP->prompt, argDefP->name);

      str = getStr(str, argDefP->defaultVal);

      str = getStr(str, argDefP->prompt);
      if (DStrLen(argDefP->prompt) == 0)
	DStrAssignDStr(argDefP->prompt, argDefP->name);
    } else {
      cmdStr = DStrNew();
      (void) getStr(str, cmdStr);
      entryP = getCmd(cmdStr);
      if (entryP == NULL) {
	DStrFree(cmdStr);
	CipSetError("unknown command in command definition");
	errCode = EC_ERROR;
	break;
      }

      if (entryP->undo > entry.undo)
	entry.undo = entryP->undo;

      DStrAssignStr(cmdStr, str);
      (void) ListInsertLast(entry.defP->cmdSeq, &cmdStr);

      argMode = FALSE;
    }
  }

  if (errCode == EC_OK) {
    if (entry.helpLine == NULL) {
      entry.helpLine = malloc(cmdLen + 1);
      (void) strcpy(entry.helpLine, cmdP);
    }

    menuEntryP = ListFirst(MenuEntryList);
    while (menuEntryP != NULL) {
      if (strcmp(cmdP, menuEntryP->cmd) == 0)
	break;
      menuEntryP = ListNext(MenuEntryList, menuEntryP);
    }

    if (menuEntryP == NULL) {
      if (MenuAddCB != NULL)
	MenuAddCB(cmdP, ListSize(entry.defP->argList) > 0);
    } else {
      entry.menu = menuEntryP->menu;
      free(menuEntryP->cmd);
      ListRemove(MenuEntryList, menuEntryP);
    }

    cmdStr = DStrNew();
    DStrAssignStr(cmdStr, cmdP);

    entryP = getCmd(cmdStr);
    if (entryP == NULL) {
      insertInd = findCmd(cmdStr);

      CmdTabSize++;
      CmdTab = realloc(CmdTab, CmdTabSize * sizeof(*CmdTab));
      for (ind = CmdTabSize - 1; ind > insertInd; ind--)
	CmdTab[ind] = CmdTab[ind - 1];

      entryP = CmdTab + insertInd;
    } else {
      freeCmdDef(entryP);
    }

    DStrFree(cmdStr);

    *entryP = entry;
  } else {
    freeCmdDef(&entry);
  }

  return errCode;
}

ErrCode
CipExecFile(char *fileName)
{
  GFile gf;
  GFileRes gRes;
  char cmdBuf[500];
  ErrCode errCode;

  gf = GFileOpenRead(fileName);
  if (gf == NULL)
    return EC_ERROR;
  
  MacroLevel++;
  HistorySwitch(FALSE);
  BreakActivate(TRUE);

  errCode = EC_OK;
  for (;;) {
    if (GFileEOF(gf))
      break;
    
    if (BreakCheck(1)) {
      CipSetError("macro interrupted");
      break;
    }

    gRes = GFileReadLine(gf, cmdBuf, sizeof(cmdBuf));
    if (gRes != GF_RES_OK) {
      errCode = EC_ERROR;
      break;
    }

    if (strncmp(cmdBuf, "define", 6) == 0)
      errCode = getCmdDef(fileName, gf, cmdBuf + 7);
    else
      errCode = CipExecCmd(cmdBuf);

    if (errCode == EC_ERROR)
      break;
  }

  BreakActivate(FALSE);
  if (BreakInterrupted())
    errCode = EC_WARNING;

  MacroLevel--;
  if (MacroLevel == 0) {
    HistorySwitch(TRUE);
    callUpdateCB();
  }

  GFileClose(gf);

  return errCode;
}

void
CipCmdSeqBegin(void)
/* Start of command sequence that results from one user action.
   State for undo is not saved until next CipCmdSeqEnd. */
{
  if (SeqLevel == 0) {
    PuSetBusyCursor(TRUE);
    UndoSave(US_ALL);
  }

  SeqLevel++;
  GraphRedrawEnable(FALSE);
}

void
CipCmdSeqEnd(void)
{
  SeqLevel--;
  GraphRedrawEnable(TRUE);

  if (SeqLevel == 0) {
    PuSetBusyCursor(FALSE);
    callUpdateCB();
  }
}

static void
extCmdInp(int fd, void *clientData)
{
#ifndef NO_EXT_CMD
  char buf[100];
  int res, i;

  if (ExtCmd == NULL)
    ExtCmd = DStrNew();

  res = read(fd, buf, sizeof(buf));
  if (res == 0)
    PuRemoveExtInput(0 /* stdin */, extCmdInp, NULL);

  for (i = 0; i < res; i++) {
    if (buf[i] == '\n') {
#if BEEP
      PuBeep();
#endif
      CipExecCmd(DStrToStr(ExtCmd));
      DStrAssignStr(ExtCmd, "");
    } else {
      DStrAppChar(ExtCmd, buf[i]);
    }
  }
#endif
}

void
CipInit(char *helpDir)
{
  int len;

  initCmdTab();

  len = strlen(helpDir) + 1;
  if (len >= sizeof(HelpDir))
    len = sizeof(HelpDir) - 1;

  (void) strncpy(HelpDir, helpDir, len - 1);
  HelpDir[len - 1] = '/';
  HelpDir[len] = '\0';

  SelHistoryInit();
  TextCmdInit();
  DrawobjCmdInit();

  PuAddCmdCB(cmdCB, NULL, NULL);

  CurrState = CS_CMD_INPUT;

  PuAddExtInput(0 /* stdin */, extCmdInp, NULL, NULL);
}

void
CipSetError(char *errStr)
{
  if (ErrStr == NULL)
    ErrStr = DStrNew();
  
  DStrAssignStr(ErrStr, errStr);
}
