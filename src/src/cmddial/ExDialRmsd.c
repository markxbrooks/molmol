/*
************************************************************************
*
*   ExDialRmsd.c - DialRmsd command
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmddial/SCCS/s.ExDialRmsd.c
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include <cmd_dial.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <values.h>

#include <pu.h>
#include <arg.h>
#include <cip.h>
#include <data_hand.h>
#include <data_sel.h>

#define MAX_GROUP_NO 10
#define OPT_NO 4

typedef struct {
  int minNum, maxNum;
} ResRangeData;

static BOOL DialOn = FALSE;
static int GroupNo = 1;
static PuGizmo DialGizmo = NULL;
static int RMSDSelA[2] = {0, 1};
static DSTR RMSDStrA[2] = {NULL, NULL};
static DSTR MolRangeA[MAX_GROUP_NO] = {NULL};
static DSTR ResRangeA[MAX_GROUP_NO] = {NULL};
static BOOL LocalSelA[3][3] = {
    {TRUE, TRUE, TRUE}, {TRUE, FALSE, FALSE}, {TRUE, TRUE, TRUE}};
static char *OptStrA[OPT_NO] = {"out of order fit", "fit pairs",
   "relative to mean", "group RMSDs"};
static BOOL OptA[4] = {FALSE, TRUE, TRUE, FALSE};

static void
selectCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  int *valP = clientData;
  PuSelectCBStruc *callP = callData;

  if (! callP->onOff)
    return;
  
  if (strcmp(name, "bb") == 0)
    *valP = 0;
  else if (strcmp(name, "heavy") == 0)
    *valP = 1;
  else
    *valP = 2;
}

static void
localOptCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  BOOL *valA = clientData;
  PuSelectCBStruc *callP = callData;

  if (strcmp(name, "bb") == 0)
    valA[0] = callP->onOff;
  else if (strcmp(name, "heavy") == 0)
    valA[1] = callP->onOff;
  else
    valA[2] = callP->onOff;
}

static void
optCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  BOOL *valA = clientData;
  PuSelectCBStruc *callP = callData;
  int i;

  for (i = 0; i < OPT_NO; i++)
    if (strcmp(name, OptStrA[i]) == 0) {
      valA[i] = callP->onOff;
      break;
    }
}

static void
editCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  DSTR str = clientData;
  PuTextCBStruc *callP = callData;

  DStrAssignStr(str, callP->newText);
}

static int
usedGroupNo(void)
{
  int groupNo, groupI;

  groupNo = 0;
  for (groupI = 0; groupI < GroupNo; groupI++) {
    if (DStrLen(MolRangeA[groupI]) == 0)
      break;
    groupNo++;
  }

  return groupNo;
}

static void
getPropStr(int ind, DSTR propStr)
{
  if (RMSDSelA[ind] == 0)
    DStrAssignStr(propStr, "bb");
  else if (RMSDSelA[ind] == 1)
    DStrAssignStr(propStr, "heavy");
  else
    DStrAssignDStr(propStr, RMSDStrA[ind]);
}

static void
getResStr(int groupNo, DSTR resStr)
{
  int groupI;

  for (groupI = 0; groupI < groupNo; groupI++) {
    if (groupI > 0)
      DStrAppChar(resStr, ' ');

    DStrAppChar(resStr, '#');
    DStrAppDStr(resStr, MolRangeA[groupI]);

    DStrAppChar(resStr, ':');
    DStrAppDStr(resStr, ResRangeA[groupI]);
  }
}

static void
fitCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  int groupNo;
  DSTR resStr, propStr, cmd;
  char *rangeStrA[MAX_GROUP_NO];
  char buf[20], ch;
  int fragmNo, fragmI, endNo, groupI;

  groupNo = usedGroupNo();

  propStr = DStrNew();
  getPropStr(0, propStr);
  cmd = DStrNew();

  CipCmdSeqBegin();

  if (OptA[0]) {
    for (groupI = 0; groupI < groupNo; groupI++)
      rangeStrA[groupI] = DStrToStr(ResRangeA[groupI]);

    fragmNo = 0;
    for (;;) {
      endNo = 0;
      for (groupI = 0; groupI < groupNo; groupI++) {
	while (isspace(*rangeStrA[groupI]))
	  rangeStrA[groupI]++;

	if (*rangeStrA[groupI] == '\0')
	  endNo++;
      }

      if (endNo > 0) {
	if (endNo < groupNo)
	  PuShowMessage(PU_MT_WARNING, "Fit", "different number of fragments");
	break;
      }

      fragmNo++;

      DStrAssignStr(cmd, "DefPropAtom '");
      (void) sprintf(buf, "f%d", fragmNo);
      DStrAppStr(cmd, buf);
      DStrAppStr(cmd, "' '");
      DStrAppDStr(cmd, propStr);
      DStrAppStr(cmd, " & (");

      for (groupI = 0; groupI < groupNo; groupI++) {
	if (groupI > 0)
	  DStrAppChar(cmd, ' ');
	DStrAppChar(cmd, '#');
	DStrAppDStr(cmd, MolRangeA[groupI]);
	DStrAppChar(cmd, ':');

	for (;;) {
	  ch = *rangeStrA[groupI];
	  if (ch == '\0' || isspace(ch) || ch == ',')
	    break;
	  DStrAppChar(cmd, ch);
	  rangeStrA[groupI]++;
	}
	if (ch == ',')
	  rangeStrA[groupI]++;
      }

      DStrAppStr(cmd, ")'");
      CipExecCmd(DStrToStr(cmd));
    }

    CipExecCmd("SelectMol ''");

    DStrAssignStr(cmd, "Fit to_first '");
    for (fragmI = 1; fragmI <= fragmNo; fragmI++) {
      if (fragmI > 1)
	DStrAppChar(cmd, ' ');
      (void) sprintf(buf, "f%d", fragmI);
      DStrAppStr(cmd, buf);
    }
    DStrAppStr(cmd, "'");
    CipExecCmd(DStrToStr(cmd));
  } else {
    resStr = DStrNew();
    getResStr(groupNo, resStr);

    DStrAssignStr(cmd, "SelectAtom '");
    DStrAppDStr(cmd, propStr);
    DStrAppStr(cmd, " & (");
    DStrAppDStr(cmd, resStr);
    DStrAppStr(cmd, ")'");
    CipExecCmd(DStrToStr(cmd));

    CipExecCmd("Fit to_first");

    DStrFree(resStr);
  }

  CipCmdSeqEnd();

  DStrFree(propStr);
  DStrFree(cmd);
}

static void
rmsdCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  int groupNo;
  DSTR cmd, propStr, resStr;
  char buf[20], *str;
  BOOL isFirst;
  BOOL bbNeeded, heavyNeeded, heavyscNeeded;
  int groupI, propI, localI;

  groupNo = usedGroupNo();

  if (OptA[2] && groupNo > 1)
    PuShowMessage(PU_MT_WARNING, "RMSD",
	"relative to mean only supported for one group");

  cmd = DStrNew();
  propStr = DStrNew();

  CipCmdSeqBegin();

  if (OptA[2] && groupNo == 1) {
    CipExecCmd("MeanMol 'mean_r' avg_disp mean_only");
    CipExecCmd("DefPropMol 'mean_r' '#mean_r'");
  }

  if (OptA[3]) {
    for (groupI = 0; groupI < groupNo; groupI++) {
      DStrAssignStr(cmd, "DefPropMol '");
      (void) sprintf(buf, "g%d", groupI + 1);
      DStrAppStr(cmd, buf);
      DStrAppStr(cmd, "' '#");
      DStrAppDStr(cmd, MolRangeA[groupI]);
      DStrAppStr(cmd, "'");

      CipExecCmd(DStrToStr(cmd));
    }
  }

  if (groupNo > 1) {
    resStr = DStrNew();
    getResStr(groupNo, resStr);

    bbNeeded = FALSE;
    heavyNeeded = FALSE;
    heavyscNeeded = FALSE;

    for (propI = 0; propI < 1; propI++) {
      getPropStr(propI, propStr);
      str = DStrToStr(propStr);
      if (strcmp(str, "bb") == 0) {
	bbNeeded = TRUE;
      } else if (strcmp(str, "heavy") == 0) {
	heavyNeeded = TRUE;
      } else if (strcmp(str, "heavysc") == 0) {
	heavyscNeeded = TRUE;
      } else {
	DStrAssignStr(cmd, "DefPropAtom '");
	DStrAppStr(cmd, str);
	DStrAppStr(cmd, "_r' '");
	DStrAppStr(cmd, str);
	DStrAppStr(cmd,  " & (");
	DStrAppDStr(cmd, resStr);
	DStrAppStr(cmd, ")'");
	CipExecCmd(DStrToStr(cmd));
      }
    }

    bbNeeded |= LocalSelA[0][0] || LocalSelA[1][0] || LocalSelA[2][0];
    if (bbNeeded) {
      DStrAssignStr(cmd, "DefPropAtom 'bb_r' 'bb & (");
      DStrAppDStr(cmd, resStr);
      DStrAppStr(cmd, ")'");
      CipExecCmd(DStrToStr(cmd));
    }

    heavyNeeded |= LocalSelA[0][1] || LocalSelA[1][1] || LocalSelA[2][1];
    if (heavyNeeded) {
      DStrAssignStr(cmd, "DefPropAtom 'heavy_r' 'heavy & (");
      DStrAppDStr(cmd, resStr);
      DStrAppStr(cmd, ")'");
      CipExecCmd(DStrToStr(cmd));
    }

    heavyscNeeded |= LocalSelA[0][2] || LocalSelA[1][2] || LocalSelA[2][2];
    if (heavyscNeeded) {
      DStrAssignStr(cmd, "DefPropAtom 'heavysc_r' 'heavysc & (");
      DStrAppDStr(cmd, resStr);
      DStrAppStr(cmd, ")'");
      CipExecCmd(DStrToStr(cmd));
    }

    DStrFree(resStr);
  }

  DStrAssignStr(cmd, "CalcRmsd ");

  if (OptA[1])
    DStrAppStr(cmd, "fit_pairs");
  else
    DStrAppStr(cmd, "as_is");

  DStrAppStr(cmd, " '");
  if (groupNo == 1)
    DStrAppDStr(cmd, ResRangeA[0]);
  DStrAppStr(cmd, "'");

  DStrAppStr(cmd, " '");
  getPropStr(0, propStr);
  DStrAppDStr(cmd, propStr);
  if (groupNo > 1)
    DStrAppStr(cmd, "_r");
  DStrAppStr(cmd, "' '");
  getPropStr(1, propStr);
  DStrAppDStr(cmd, propStr);
  if (groupNo > 1)
    DStrAppStr(cmd, "_r");
  DStrAppStr(cmd, "'");

  DStrAppStr(cmd, " '");
  if (OptA[3]) {
    for (groupI = 0; groupI < groupNo; groupI++) {
      if (groupI > 0)
	DStrAppChar(cmd, ' ');
      (void) sprintf(buf, "g%d", groupI + 1);
      DStrAppStr(cmd, buf);
    }
  }
  DStrAppStr(cmd, "'");

  DStrAppStr(cmd, " '");
  if (OptA[2] && groupNo == 1)
    DStrAppStr(cmd, "mean_r");
  DStrAppStr(cmd, "'");

  for (localI = 0; localI < 3; localI++) {
    DStrAppStr(cmd, " '");
    isFirst = TRUE;

    if (LocalSelA[localI][0]) {
      if (groupNo > 1)
	DStrAppStr(cmd, "bb_r");
      else
	DStrAppStr(cmd, "bb");
      isFirst = FALSE;
    }

    if (LocalSelA[localI][1]) {
      if (! isFirst)
	DStrAppChar(cmd, ' ');
      if (groupNo > 1)
	DStrAppStr(cmd, "heavy_r");
      else
	DStrAppStr(cmd, "heavy");
    }

    if (LocalSelA[localI][2]) {
      if (! isFirst)
	DStrAppChar(cmd, ' ');
      if (groupNo > 1)
	DStrAppStr(cmd, "heavysc_r");
      else
	DStrAppStr(cmd, "heavysc");
      isFirst = FALSE;
    }

    DStrAppStr(cmd, "'");
  }

  CipExecCmd(DStrToStr(cmd));

  if (OptA[2] && groupNo == 1) {
    CipExecCmd("DefPropMol 'prev_sel' 'selected'");
    CipExecCmd("SelectMol 'mean_r'");
    CipExecCmd("RemoveMol");
    CipExecCmd("SelectMol 'prev_sel'");
  }

  CipCmdSeqEnd();

  DStrFree(cmd);
  DStrFree(propStr);
}

static void
popdownCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  PuSwitchGizmo(DialGizmo, FALSE);
  DialOn = FALSE;
}

static int
countGroups(void)
{
  DhMolP *molPA;
  int molNo;
  int groupNo, molI;

  molNo = SelMolGet(NULL, 0);
  molPA = malloc(molNo * sizeof(*molPA));
  (void) SelMolGet(molPA, molNo);

  groupNo = 1;
  for (molI = 1; molI < molNo; molI++)
    if (! DhMolEqualStruc(molPA[molI - 1], molPA[molI]))
      groupNo++;

  free(molPA);

  return groupNo;
}

static void
fillResRange(DhResP resP, void *clientData)
{
  ResRangeData *dataP = clientData;
  int num;

  num = DhResGetNumber(resP);
  if (num < dataP->minNum)
    dataP->minNum = num;
  if (num > dataP->maxNum)
    dataP->maxNum = num;
}

static void
getResRange(DhMolP molP, DSTR str)
{
  ResRangeData data;
  char buf[20];

  data.minNum = MAXINT;
  data.maxNum = - MAXINT;
  DhMolApplyRes(PropGetRef(PROP_SELECTED, FALSE), molP, fillResRange, &data);

  if (data.minNum <= data.maxNum) {
    (void) sprintf(buf, "%d", data.minNum);
    DStrAssignStr(str, buf);
    DStrAppChar(str, '-');
    (void) sprintf(buf, "%d", data.maxNum);
    DStrAppStr(str, buf);
  }
}

static void
fillRange(void)
{
  DhMolP *molPA;
  int molNo, molI;
  int groupI;
  char buf[10];

  for (groupI = 0; groupI < GroupNo; groupI++) {
    if (MolRangeA[groupI] == NULL) {
      MolRangeA[groupI] = DStrNew();
      ResRangeA[groupI] = DStrNew();
    }
  }

  molNo = SelMolGet(NULL, 0);
  if (molNo == 0)
    return;

  molPA = malloc(molNo * sizeof(*molPA));
  (void) SelMolGet(molPA, molNo);

  (void) sprintf(buf, "%d", DhMolGetNumber(molPA[0]) + 1);
  DStrAssignStr(MolRangeA[0], buf);
  getResRange(molPA[0], ResRangeA[0]);

  groupI = 0;
  for (molI = 1; molI < molNo; molI++) {
    if (DhMolEqualStruc(molPA[molI - 1], molPA[molI]))
      continue;

    DStrAppChar(MolRangeA[groupI], '-');
    (void) sprintf(buf, "%d", DhMolGetNumber(molPA[molI - 1]) + 1);
    DStrAppStr(MolRangeA[groupI], buf);

    groupI++;

    (void) sprintf(buf, "%d", DhMolGetNumber(molPA[molI]) + 1);
    DStrAssignStr(MolRangeA[groupI], buf);
    getResRange(molPA[molI], ResRangeA[groupI]);
  }

  DStrAppChar(MolRangeA[groupI], '-');
  (void) sprintf(buf, "%d", DhMolGetNumber(molPA[molNo - 1]) + 1);
  DStrAppStr(MolRangeA[groupI], buf);

  free(molPA);
}

static void
helpCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  CipShowHelpFile(DialGizmo, "DialRmsd");
}

static void
buildDial(void)
{
  PuGizmo g;
  PuConstraints con;
  int i;

  DialGizmo = PuCreateDialog("RMSD Dialog", 4, 14 + GroupNo);
  PuSetBool(DialGizmo, PU_BC_PLACE_OUTSIDE, TRUE);

  if (RMSDStrA[0] == NULL)
    for (i = 0; i < 2; i++)
      RMSDStrA[i] = DStrNew();

  for (i = 0; i < 2; i++) {
    if (i == 0)
      g = PuCreateRadioBox(DialGizmo, "Fit/Global RMSD 1");
    else
      g = PuCreateRadioBox(DialGizmo, "Global RMSD 2");

    PuAddToggle(g, "bb", RMSDSelA[i] == 0);
    PuAddToggle(g, "heavy", RMSDSelA[i] == 1);
    PuAddToggle(g, "other", RMSDSelA[i] == 2);

    con.x = 0;
    con.y = 2 * i;
    con.w = 3;
    con.h = 2;
    PuSetConstraints(g, con);
    PuAddGizmoCB(g, PU_CT_SELECT, selectCB, RMSDSelA + i, NULL);
    PuSwitchGizmo(g, TRUE);

    g = PuCreateTextField(DialGizmo, "other", "");
    PuSetInt(g, PU_IC_TEXT_WIDTH, 8);
    con.x = 3;
    con.y = 2 * i + 1;
    con.w = 1;
    con.h = 1;
    PuSetConstraints(g, con);
    PuAddGizmoCB(g, PU_CT_MODIFY, editCB, RMSDStrA[i], NULL);
    PuSwitchGizmo(g, TRUE);
  }

  con.y = 4;
  con.h = 1;

  g = PuCreateLabel(DialGizmo, "Mol Range");
  con.x = 0;
  con.w = 1;
  PuSetConstraints(g, con);
  PuSwitchGizmo(g, TRUE);

  g = PuCreateLabel(DialGizmo, "Res Range");
  con.x = 1;
  con.w = 3;
  PuSetConstraints(g, con);
  PuSwitchGizmo(g, TRUE);

  for (i = 0; i < GroupNo; i++) {
    con.y = 5 + i;
    con.h = 1;

    g = PuCreateTextField(DialGizmo, "Mol", DStrToStr(MolRangeA[i]));
    PuSetInt(g, PU_IC_TEXT_WIDTH, 8);
    con.x = 0;
    con.w = 1;
    PuSetConstraints(g, con);
    PuAddGizmoCB(g, PU_CT_MODIFY, editCB, MolRangeA[i], NULL);
    PuSwitchGizmo(g, TRUE);

    g = PuCreateTextField(DialGizmo, "Fit", DStrToStr(ResRangeA[i]));
    PuSetInt(g, PU_IC_TEXT_WIDTH, 16);
    con.x = 1;
    con.w = 3;
    PuSetConstraints(g, con);
    PuAddGizmoCB(g, PU_CT_MODIFY, editCB, ResRangeA[i], NULL);
    PuSwitchGizmo(g, TRUE);
  }

  con.w = 1;
  con.y = 5 + GroupNo;
  con.h = 4;

  for (i = 0; i < 3; i++) {
    if (i == 0)
      g = PuCreateCheckBox(DialGizmo, "Global Displ.");
    else if (i == 1)
      g = PuCreateCheckBox(DialGizmo, "Local RMSD");
    else
      g = PuCreateCheckBox(DialGizmo, "Local Displ.");

    PuAddToggle(g, "bb", LocalSelA[i][0]);
    PuAddToggle(g, "heavy", LocalSelA[i][1]);
    PuAddToggle(g, "heavysc", LocalSelA[i][2]);

    con.x = i;
    PuSetConstraints(g, con);
    PuAddGizmoCB(g, PU_CT_SELECT, localOptCB, LocalSelA[i], NULL);
    PuSwitchGizmo(g, TRUE);
  }

  g = PuCreateCheckBox(DialGizmo, "Options");
  for (i = 0; i < OPT_NO; i++)
    PuAddToggle(g, OptStrA[i], OptA[i]);
  con.x = 0;
  con.y = 5 + GroupNo + 4;
  con.w = 2;
  con.h = 5;
  PuSetConstraints(g, con);
  PuAddGizmoCB(g, PU_CT_SELECT, optCB, OptA, NULL);
  PuSwitchGizmo(g, TRUE);

  g = PuCreateButton(DialGizmo, "Fit");
  con.x = 2;
  con.y = 5 + GroupNo + 5;
  con.w = 2;
  con.h = 1;
  PuSetConstraints(g, con);
  PuAddGizmoCB(g, PU_CT_ACTIVATE, fitCB, NULL, NULL);
  PuSwitchGizmo(g, TRUE);

  g = PuCreateButton(DialGizmo, "RMSD");
  con.x = 2;
  con.y = 5 + GroupNo + 6;
  con.w = 2;
  con.h = 1;
  PuSetConstraints(g, con);
  PuAddGizmoCB(g, PU_CT_ACTIVATE, rmsdCB, NULL, NULL);
  PuSwitchGizmo(g, TRUE);

  PuAddGizmoCB(DialGizmo, PU_CT_CLOSE, popdownCB, NULL, NULL);
  PuAddGizmoCB(DialGizmo, PU_CT_HELP, helpCB, NULL, NULL);
}

#define ARG_NUM 2

ErrCode
ExDialRmsd(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  EnumEntryDescr enumEntry[2];
  ErrCode errCode;
  int newGroupNo;

  arg[0].type = AT_ENUM;
  arg[1].type = AT_INT;

  ArgInit(arg, ARG_NUM);

  enumEntry[0].str = "off";
  enumEntry[1].str = "on";

  enumEntry[0].onOff = DialOn;
  enumEntry[1].onOff = ! DialOn;

  arg[0].prompt = "Style Dialog";
  arg[0].u.enumD.entryP = enumEntry;
  arg[0].u.enumD.n = 2;

  if (DialOn)
    arg[0].v.intVal = 0;
  else
    arg[0].v.intVal = 1;

  arg[1].prompt = "Group #";
  arg[1].optional = TRUE;
  arg[1].v.intVal = countGroups();

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  DialOn = (arg[0].v.intVal == 1);
  newGroupNo = arg[1].v.intVal;

  ArgCleanup(arg, ARG_NUM);

  if (newGroupNo > MAX_GROUP_NO)
    newGroupNo = MAX_GROUP_NO;
  else if (newGroupNo < 1)
    newGroupNo = 1;

  if (DialOn) {
    if (DialGizmo != NULL && newGroupNo != GroupNo) {
      PuDestroyGizmo(DialGizmo);
      DialGizmo = NULL;
    }
    GroupNo = newGroupNo;
    fillRange();
    if (DialGizmo == NULL)
      buildDial();
    PuSwitchGizmo(DialGizmo, TRUE);
  } else {
    if (DialGizmo != NULL)
      PuSwitchGizmo(DialGizmo, FALSE);
  }

  return EC_OK;
}
