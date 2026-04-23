/*
************************************************************************
*
*   Arg.c - argument input
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
*   Date of last modification : 97/02/19
*   Pathname of SCCS file     : /local/home/kor/molmol/src/cip/SCCS/s.Arg.c
*   SCCS identification       : 1.11
*
************************************************************************
*/

#include <arg.h>
#include "arg_p.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <file_name.h>
#include <linlist.h>
#include <pu.h>
#include <history.h>
#include <cip.h>

#define ARG_LEN 30  /* max lenth for numbers */
#define MAX_LIST_VISIBLE 7

typedef struct {
  DSTR cmd;
  PuGizmo dialGizmo;
  BOOL finished;
  ErrCode errCode;
  DSTR fileName;
} ArgDialData, *ArgDialDataP;

typedef struct {
  DSTR cmd;
  DSTR dir;
  DSTR pattern;
} DialListEntry;

static BOOL Enabled = TRUE;

static DSTR CurrCmd = NULL;
static DSTR CurrArg = NULL;

static LINLIST DialList = NULL;

void
ArgEnable(BOOL onOff)
{
  Enabled = onOff;
}

void
ArgSetCmd(DSTR cmd)
{
  if (CurrCmd == NULL)
    CurrCmd = DStrNew();

  DStrAssignDStr(CurrCmd, cmd);
}

void
ArgSet(DSTR arg)
{
  if (CurrArg == NULL)
    CurrArg = DStrNew();

  DStrAssignDStr(CurrArg, arg);
}

void
ArgInit(ArgDescrP argP, int argNo)
{
  int argInd;

  for (argInd = 0; argInd < argNo; argInd++) {
    argP[argInd].prompt = NULL;
    argP[argInd].useSlider = FALSE;
    argP[argInd].optional = FALSE;
    argP[argInd].entType = DE_NONE;
    argP[argInd].verifyFunc = NULL;
    argP[argInd].userData = NULL;

    argP[argInd].v.exprP = NULL;

    switch (argP[argInd].type) {
      case AT_INT:
	argP[argInd].v.intVal = 0;
	argP[argInd].v.strVal = DStrNew();
	break;
      case AT_DOUBLE:
	argP[argInd].v.doubleVal = 0.0;
	argP[argInd].v.strVal = DStrNew();
	break;
      case AT_STR:
	argP[argInd].v.strVal = DStrNew();
	break;
      case AT_ENUM:
      case AT_MULT_ENUM:
      case AT_LIST:
      case AT_MULT_LIST:
	argP[argInd].v.intVal = 0;
	argP[argInd].u.enumD.entryP = NULL;
	argP[argInd].u.enumD.n = 0;
	argP[argInd].u.enumD.lineNo = 1;
	break;
    }
  }
}

void
ArgCleanup(ArgDescrP argP, int argNo)
{
  int argInd;

  for (argInd = 0; argInd < argNo; argInd++) {
    switch (argP[argInd].type) {
      case AT_INT:
      case AT_DOUBLE:
      case AT_STR:
	DStrFree(argP[argInd].v.strVal);
	break;
    }

    if (argP[argInd].v.exprP != NULL)
      ExprFree(argP[argInd].v.exprP);
  }
}

static void
helpCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  ArgDialDataP dialDataP = clientData;

  CipShowHelpFile(dialDataP->dialGizmo, DStrToStr(dialDataP->cmd));
}

static char *
arg2Str(char *str, ArgDescrP argP)
{
  switch (argP->type) {
    case AT_INT:
      (void) sprintf(str, "%d", argP->v.intVal);
      return str;
    case AT_DOUBLE:
      (void) sprintf(str, "%.5g", argP->v.doubleVal);
      return str;
    case AT_STR:
      return DStrToStr(argP->v.strVal);
    default:
      return NULL;
  }
}

static void
sliderCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  ArgDescrP argP = clientData;
  PuFloatCBStruc *callP = callData;

  if (argP->type == AT_INT)
    argP->v.intVal = (int) callP->newVal;
  else
    argP->v.doubleVal = callP->newVal;

  if (argP->verifyFunc != NULL)
    (void) argP->verifyFunc(argP);
}

static void
textCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  ArgDescrP argP = clientData;
  PuTextCBStruc *callP = callData;

  DStrAssignStr(argP->v.strVal, callP->newText);
}

static void
selectCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  ArgDescrP argP = clientData;
  PuSelectCBStruc *callP = callData;
  int i;

  i = 0;
  while (strcmp(argP->u.enumD.entryP[i].str, name) != 0)
    i++;

  argP->u.enumD.entryP[i].onOff = callP->onOff;
  if (callP->onOff)
    argP->v.intVal = i;
}

static void
activateCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  ArgDialDataP dialDataP = clientData;

  dialDataP->finished = TRUE;
  dialDataP->errCode = EC_OK;
}

static void
popdownCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  ArgDialDataP dialDataP = clientData;
  PuFileSelCBStruc *callP = callData;

  dialDataP->finished = TRUE;

  if (strcmp(name, "OK") != 0) {
    dialDataP->errCode = EC_CANCEL;
    return;
  }

  dialDataP->errCode = EC_OK;

  if (dialDataP->fileName != NULL)
    /* file selection box, get filename */
    DStrAssignStr(dialDataP->fileName, callP->fileName);
}

static BOOL
simpleType(ArgType argType)
{
  return argType == AT_INT ||
      argType == AT_DOUBLE ||
      argType == AT_STR;
}

static int
vertSize(ArgDescrP argP)
{
  int lineNo;

  if (simpleType(argP->type))
    return 1;

  if (argP->type == AT_ENUM)
    lineNo = argP->u.enumD.lineNo;
  else if (argP->type == AT_MULT_ENUM)
    lineNo = argP->u.enumD.n;
  else if (argP->u.enumD.n > MAX_LIST_VISIBLE)
    lineNo = MAX_LIST_VISIBLE;
  else
    lineNo = argP->u.enumD.n;
  
  return 1 + lineNo;
}

PuGizmo
createDialog(ArgDescrP argP, int argNo, ArgDialData *dialDataP)
{
  int argInd;
  int dialSize, dialPos, i;
  PuGizmo gDial, g;
  ArgType argType;
  PuConstraints con;
  float initVal;
  EnumDescr *enumP;

  dialSize = 0;
  for (argInd = 0; argInd < argNo; argInd++)
    dialSize += vertSize(argP + argInd);

  gDial = PuCreateDialog(DStrToStr(CurrCmd), 3, dialSize);

  dialPos = 0;

  for (argInd = 0; argInd < argNo; argInd++) {
    argType = argP[argInd].type;

    if (simpleType(argType)) {
      g = PuCreateLabel(gDial, argP[argInd].prompt);
      con.x = 0;
      con.y = dialPos;
      con.w = 1;
      con.h = 1;
      PuSetConstraints(g, con);
      PuSwitchGizmo(g, TRUE);
      if (argP[argInd].useSlider) {
	if (argType == AT_INT)
	  initVal = (float) argP[argInd].v.intVal;
	else
	  initVal = (float) argP[argInd].v.doubleVal;
	g = PuCreateSlider(gDial, argP[argInd].prompt,
	    argP[argInd].u.sliderD.minVal, argP[argInd].u.sliderD.maxVal,
	    argP[argInd].u.sliderD.digits, initVal);
	PuAddGizmoCB(g, PU_CT_MODIFY, sliderCB, argP + argInd, NULL);
      } else {
	g = PuCreateTextField(gDial, argP[argInd].prompt,
	    DStrToStr(argP[argInd].v.strVal));
	PuAddGizmoCB(g, PU_CT_MODIFY, textCB, argP + argInd, NULL);
	PuAddGizmoCB(g, PU_CT_ACTIVATE, activateCB, dialDataP, NULL);
      }

      con.x = 1;
      con.y = dialPos;
      con.w = 2;
      con.h = 1;
    } else {
      enumP = &argP[argInd].u.enumD;

      if (argType == AT_ENUM || argType == AT_MULT_ENUM) {
	if (argType == AT_ENUM) {
	  g = PuCreateRadioBox(gDial, argP[argInd].prompt);
	  PuSetInt(g, PU_IC_LINE_NO, enumP->lineNo);
	} else {
	  g = PuCreateCheckBox(gDial, argP[argInd].prompt);
	}

	for (i = 0; i < enumP->n; i++)
	  PuAddToggle(g,
	      enumP->entryP[i].str, enumP->entryP[i].onOff);
      } else {
        g = PuCreateList(gDial, argP[argInd].prompt);
	PuSetBool(g, PU_BC_MULT_SEL, argType == AT_MULT_LIST);
	PuSetInt(g, PU_IC_ENTRY_NO, vertSize(argP + argInd) - 1);
	for (i = 0; i < enumP->n; i++)
	  PuAddListEntry(g,
	      enumP->entryP[i].str, enumP->entryP[i].onOff);
      }

      PuAddGizmoCB(g, PU_CT_SELECT, selectCB, argP + argInd, NULL);

      con.x = 0;
      con.y = dialPos;
      con.w = 3;
      con.h = vertSize(argP + argInd);
    }

    PuSetConstraints(g, con);
    dialPos += con.h;

    PuSwitchGizmo(g, TRUE);
  }

  return gDial;
}

void
writeHistory(ArgDescrP argP, int argNo)
{
  int argInd;
  char str[ARG_LEN];
  EnumDescr *enumP;
  int i;

  for (argInd = 0; argInd < argNo; argInd++) {
    if (argP[argInd].type == AT_STR) {
      HistoryArgStr(DStrToStr(argP[argInd].v.strVal));
    } else if (simpleType(argP[argInd].type)) {
      if (argP[argInd].useSlider)
	HistoryArgIdent(arg2Str(str, argP + argInd));
      else
	HistoryArgExpr(DStrToStr(argP[argInd].v.strVal));
    } else if (argP[argInd].type == AT_ENUM ||
	argP[argInd].type == AT_LIST) {
      enumP = &argP[argInd].u.enumD;
      HistoryArgIdent(enumP->entryP[argP[argInd].v.intVal].str);
    } else {
      enumP = &argP[argInd].u.enumD;
      for (i = 0; i < enumP->n; i++)
	if (enumP->entryP[i].onOff)
	  HistoryArgIdent("1");
	else
	  HistoryArgIdent("0");
    }
  }
}

static ErrCode
evalArg(ArgDescrP argP)
{
  ExprResType resType;
  ExprRes exprRes;

  argP->v.exprP = ExprCompile(argP->entType,
      DStrToStr(argP->v.strVal), &resType);
  if (argP->v.exprP == NULL) {
    CipSetError(ExprGetErrorMsg());
    return EC_ERROR;
  }
  if (resType != ER_INT &&
      (resType != ER_FLOAT || argP->type != AT_DOUBLE)) {
    CipSetError("wrong argument type");
    return EC_ERROR;
  }

  if (argP->entType == DE_NONE) {
    ExprEval(NULL, argP->v.exprP, &exprRes);
    if (argP->type == AT_INT)
      argP->v.intVal = exprRes.u.intVal;
    else if (resType == ER_INT)
      argP->v.doubleVal = (double) exprRes.u.intVal;
    /* this test avoids an optimizer bug on ALPHA machines! */
    else if (argP->v.doubleVal != exprRes.u.floatVal)
      argP->v.doubleVal = exprRes.u.floatVal;
    ExprFree(argP->v.exprP);
    argP->v.exprP = NULL;

    if (argP->verifyFunc != NULL &&
	! argP->verifyFunc(argP)) {
      CipSetError("invalid command argument");
      return EC_ERROR;
    }
  }

  return EC_OK;
}

static void
waitForArg(ArgDialData *dialDataP)
{
  dialDataP->finished = FALSE;

  PuAddGizmoCB(dialDataP->dialGizmo, PU_CT_OK,
      popdownCB, dialDataP, NULL);
  PuAddGizmoCB(dialDataP->dialGizmo, PU_CT_CANCEL,
      popdownCB, dialDataP, NULL);
  PuAddGizmoCB(dialDataP->dialGizmo, PU_CT_HELP,
      helpCB, dialDataP, NULL);
  PuSwitchGizmo(dialDataP->dialGizmo, TRUE);

  PuSetBusyCursor(FALSE);

  while (! dialDataP->finished)
    PuProcessEvent();

  PuSetBusyCursor(TRUE);

  PuDestroyGizmo(dialDataP->dialGizmo);
}

ErrCode
ArgGet(ArgDescrP argP, int argNo)
{
  int argInd;
  char *argStr;
  DSTR oneArg;
  BOOL ok;
  int parLevel;
  char quoteCh;
  EnumDescr *enumP;
  char str[ARG_LEN];
  ErrCode errCode;
  ArgDialData dialData;
  int i;

  if (! Enabled) {
    CipSetError("no license");
    return EC_ERROR;
  }

  argInd = 0;
  argStr = DStrToStr(CurrArg);

  while (argStr[0] != '\0' && argInd < argNo) {
    oneArg = DStrNew();
    ok = TRUE;

    switch (argP[argInd].type) {
      case AT_INT:
      case AT_DOUBLE:
	if (argStr[0] == '(') {
	  argStr++;
	  parLevel = 1;
	  while (argStr[0] != '\0') {
	    if (argStr[0] == '(') {
	      parLevel++;
	    } else if (argStr[0] == ')') {
	      parLevel--;
	      if (parLevel == 0)
		break;
	    }
	    DStrAppChar(oneArg, argStr[0]);
	    argStr++;
	  }
	  if (parLevel > 0) {
	    ok = FALSE;
	    CipSetError("unbalanced parantheses");
	  } else {
	    argStr++;
	  }
	} else {
	  while (argStr[0] != ' ' && argStr[0] != '\0') {
	    DStrAppChar(oneArg, argStr[0]);
	    argStr++;
	  }
	}
	if (ok) {
	  DStrAssignDStr(argP[argInd].v.strVal, oneArg);
	  if (argP[argInd].useSlider)
	    if (evalArg(argP + argInd) != EC_OK)
	      ok = FALSE;
	}
	break;
      case AT_STR:
	quoteCh = argStr[0];
	if (quoteCh != '"' && quoteCh != '\'')
	  break;
	argStr++;
	while (argStr[0] != quoteCh && argStr[0] != '\0') {
	  if (argStr[0] == '\\' && argStr[1] == quoteCh)
	    argStr++;
	  DStrAppChar(oneArg, argStr[0]);
	  argStr++;
	}
	if (argStr[0] == '\0') {
	  ok = FALSE;
	  CipSetError("unterminated string");
	} else {
	  argStr++;
	  DStrAssignDStr(argP[argInd].v.strVal, oneArg);
	}
	break;
      case AT_ENUM:
      case AT_LIST:
	enumP = &argP[argInd].u.enumD;
	while (isprint(argStr[0]) && ! isspace(argStr[0])) {
	  DStrAppChar(oneArg, argStr[0]);
	  argStr++;
	}
	for (i = 0; i < enumP->n; i++)
	  enumP->entryP[i].onOff = FALSE;
	i = 0;
	for (;;) {
	  if (i == enumP->n) {
	    ok = FALSE;
            CipSetError("unknown enumeration");
	    break;
	  }
	  if (strcmp(enumP->entryP[i].str, DStrToStr(oneArg)) == 0)
	    break;
	  i++;
	}
	if (ok) {
	  enumP->entryP[i].onOff = TRUE;
	  argP[argInd].v.intVal = i;
	}
	break;
      case AT_MULT_ENUM:
      case AT_MULT_LIST:
	enumP = &argP[argInd].u.enumD;
	i = 0;
	for (;;) {
	  if (argStr[0] == '0') {
	    enumP->entryP[i].onOff = FALSE;
	  } else if (argStr[0] == '1') {
	    enumP->entryP[i].onOff = TRUE;
	  } else {
            CipSetError("0 or 1 expected");
	    ok = FALSE;
	    break;
	  }
	  argStr++;
	  i++;
	  if (i == enumP->n)
	    break;
	  if (argStr[0] != ' ') {
            CipSetError("space expected");
	    ok = FALSE;
	    break;
	  }
	  argStr++;
	}
	argP[argInd].v.intVal = 0;
    }

    DStrFree(oneArg);

    if (! ok)
      return EC_ERROR;

    ok = (argStr[0] == ',' || argStr[0] == ' ' || argStr[0] == '\0');
    if (! ok) {
      CipSetError("syntax error in command arguments");
      return EC_ERROR;
    }

    while (argStr[0] == ' ')
      argStr++;
    
    if (argStr[0] == ',') {
      argStr++;
      while (argStr[0] == ' ')
	argStr++;
    }

    argInd++;
  }

  if (argStr[0] != '\0') {
    CipSetError("too many arguments");
    return EC_ERROR;
  }

  for (i = argInd; i < argNo; i++)
    if ((argP[i].type == AT_INT || argP[i].type == AT_DOUBLE) &&
	DStrLen(argP[i].v.strVal) == 0)
      DStrAssignStr(argP[i].v.strVal, arg2Str(str, argP + i));

  while (argInd < argNo && argP[argInd].optional)
    argInd++;
  
  if (argInd == argNo) {  /* all arguments entered on command line */
    errCode = EC_OK;
  } else {
    dialData.cmd = DStrNew();
    DStrAssignDStr(dialData.cmd, CurrCmd);
    dialData.dialGizmo = createDialog(argP, argNo, &dialData);
    dialData.fileName = NULL;

    waitForArg(&dialData);

    DStrFree(dialData.cmd);

    errCode = dialData.errCode;
  }

  if (errCode != EC_OK)
    return errCode;

  for (argInd = 0; argInd < argNo; argInd++) {
    if (argP[argInd].type != AT_INT && argP[argInd].type != AT_DOUBLE)
      continue;

    if (argP[argInd].useSlider)
      continue;

    errCode = evalArg(argP + argInd);
    if (errCode != EC_OK)
      return errCode;
  }

  writeHistory(argP, argNo);

  return EC_OK;
}

ErrCode
ArgGetFilename(DSTR fileName, char *dir, char *pattern, BOOL forRead)
{
  DialListEntry *entryP, entry;
  PuFileAccess acc;
  PuGizmo g;
  ArgDialData dialData;
  DSTR ext;

  if (! Enabled) {
    CipSetError("no license");
    return EC_ERROR;
  }

  if (DStrLen(CurrArg) > 0) {
    DStrAssignDStr(fileName, CurrArg);
    HistoryArgIdent(DStrToStr(fileName));
    return EC_OK;
  }

  if (DialList == NULL) {
    DialList = ListOpen(sizeof(DialListEntry));
    entryP = NULL;
  } else {
    entryP = ListFirst(DialList);
    while (entryP != NULL) {
      if (DStrCmp(CurrCmd, entryP->cmd) == 0)
	break;
      entryP = ListNext(DialList, entryP);
    }
  }

  if (entryP == NULL) {
    entryP = ListInsertFirst(DialList, &entry);
    entryP->cmd = DStrNew();
    DStrAssignDStr(entryP->cmd, CurrCmd);
    entryP->dir = DStrNew();
    DStrAssignStr(entryP->dir, dir);
    entryP->pattern = DStrNew();
    DStrAssignStr(entryP->pattern, pattern);
  }

  if (forRead)
    acc = PU_FA_READ;
  else
    acc = PU_FA_WRITE;

  g = PuCreateFileSelectionBox(DStrToStr(CurrCmd),
      DStrToStr(entryP->dir), DStrToStr(entryP->pattern), acc);

  dialData.cmd = DStrNew();
  DStrAssignDStr(dialData.cmd, CurrCmd);
  dialData.dialGizmo = g;
  dialData.fileName = fileName;

  waitForArg(&dialData);

  DStrFree(dialData.cmd);

  HistoryArgIdent(DStrToStr(dialData.fileName));

  if (dialData.errCode == EC_OK) {
    DStrAssignDStr(entryP->dir, dialData.fileName);
    FileNamePath(entryP->dir);
    DStrAssignStr(entryP->pattern, "*.");
    ext = DStrNew();
    DStrAssignDStr(ext, dialData.fileName);
    FileNameExt(ext);
    DStrAppDStr(entryP->pattern, ext);
    DStrFree(ext);
  }

  return dialData.errCode;
}
