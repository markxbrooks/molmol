/*
************************************************************************
*
*   ExCalcRmsd.c - CalcRmsd command
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
*   Date of last modification : 96/04/23
*   Pathname of SCCS file     : /sgiext/molmol/src/cmdcalc/SCCS/s.ExCalcRmsd.c
*   SCCS identification       : 1.7
*
************************************************************************
*/

#include <cmd_calc.h>

#include <stdio.h>

#include <break.h>
#include <pu.h>
#include <arg.h>
#include <data_hand.h>
#include <calc_rmsd.h>

typedef struct {
  PropRefP refP;
  BOOL set;
} CheckData;

static BOOL FitPairs = TRUE;
static PuTextWindow TextW;
static char *ErrorMsg;

static void
writeStr(char *str)
{
  PuWriteStr(TextW, str);
}

static void
setError(char *msg)
{
  ErrorMsg = msg;
}

static void
checkRef(DhMolP molP, void *clientData)
{
  CheckData *dataP = clientData;

  if (DhMolGetProp(dataP->refP, molP))
    dataP->set = TRUE;
}

#define ARG_NUM 9
#define ENUM_SIZE 2

ErrCode
ExCalcRmsd(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  EnumEntryDescr enumEntry[ENUM_SIZE];
  ErrCode errCode;
  DSTR rangeStr;
  char *s;
  CheckData checkData;
  DSTR warnMsg;

  arg[0].type = AT_ENUM;
  arg[1].type = AT_STR;
  arg[2].type = AT_STR;
  arg[3].type = AT_STR;
  arg[4].type = AT_STR;
  arg[5].type = AT_STR;
  arg[6].type = AT_STR;
  arg[7].type = AT_STR;
  arg[8].type = AT_STR;

  ArgInit(arg, ARG_NUM);

  enumEntry[0].str = "fit_pairs";
  enumEntry[0].onOff = FitPairs;
  enumEntry[1].str = "as_is";
  enumEntry[1].onOff = ! FitPairs;

  arg[0].prompt = "Fitting";
  arg[0].u.enumD.entryP = enumEntry;
  arg[0].u.enumD.n = ENUM_SIZE;
  if (FitPairs)
    arg[0].v.intVal = 0;
  else
    arg[0].v.intVal = 1;

  arg[1].prompt = "region";

  arg[2].prompt = "global RMSD 1";
  DStrAssignStr(arg[2].v.strVal, "bb");

  arg[3].prompt = "global RMSD 2";
  DStrAssignStr(arg[3].v.strVal, "heavy");

  arg[4].prompt = "groups";

  arg[5].prompt = "reference";
  DStrAssignStr(arg[5].v.strVal, "mean");

  arg[6].prompt = "global displacement";
  DStrAssignStr(arg[6].v.strVal, "bb heavy heavysc");

  arg[7].prompt = "local RMSD";
  DStrAssignStr(arg[7].v.strVal, "bb");

  arg[8].prompt = "local displacement";
  DStrAssignStr(arg[8].v.strVal, "bb heavy heavysc");

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  FitPairs = (arg[0].v.intVal == 0);

  BreakActivate(TRUE);

  ErrorMsg = NULL;
  TextW = PuCreateTextWindow(cmd);

  /* CalcRmsd function evaluates "res.num = str". Preprocess
     the string to make input more flexible, e. g. to allow
     for using short notation. */
  rangeStr = DStrNew();
  s = DStrToStr(arg[1].v.strVal);
  if (s[0] == ':')
    s++;
  while (*s != '\0') {
    if (*s == '-')
      DStrAppStr(rangeStr, "..");
    else
      DStrAppChar(rangeStr, *s);
    s++;
  }

  CalcRmsd(FitPairs, DStrToStr(rangeStr),
      DStrToStr(arg[2].v.strVal),
      DStrToStr(arg[3].v.strVal),
      DStrToStr(arg[4].v.strVal),
      DStrToStr(arg[5].v.strVal),
      DStrToStr(arg[6].v.strVal),
      DStrToStr(arg[7].v.strVal),
      DStrToStr(arg[8].v.strVal),
      writeStr, setError);

  DStrFree(rangeStr);

  BreakActivate(FALSE);

  warnMsg = NULL;
  if (DStrLen(arg[5].v.strVal) > 0 && ErrorMsg == NULL) {
    checkData.refP = PropGetRef(DStrToStr(arg[5].v.strVal), FALSE);
    checkData.set = FALSE;
    DhApplyMol(PropGetRef(PROP_SELECTED, FALSE), checkRef, &checkData);
    if (! checkData.set) {
      warnMsg = DStrNew();
      DStrAppStr(warnMsg, "no reference (");
      DStrAppDStr(warnMsg, arg[5].v.strVal);
      DStrAppStr(warnMsg, ") molecule");
    }
  }

  ArgCleanup(arg, ARG_NUM);

  if (ErrorMsg != NULL) {
    CipSetError(ErrorMsg);
    return EC_ERROR;
  }

  if (warnMsg != NULL) {
    CipSetError(DStrToStr(warnMsg));
    DStrFree(warnMsg);
    return EC_WARNING;
  }

  return EC_OK;
}
