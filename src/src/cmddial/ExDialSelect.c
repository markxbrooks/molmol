/*
************************************************************************
*
*   ExDialSelect.c - DialSelect command
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmddial/SCCS/s.ExDialSelect.c
*   SCCS identification       : 1.9
*
************************************************************************
*/

#include <cmd_dial.h>

#include <stdio.h>
#include <string.h>

#include <pu.h>
#include <arg.h>
#include <cip.h>

#define ENT_NO 8

static BOOL DialOn = FALSE;
static PuGizmo DialGizmo = NULL;

static char *SelectNames[] = {
  "Mol",
  "Res",
  "Atom",
  "Bond",
  "Angle",
  "R+A+B+A",
  "Dist",
  "Prim"
};

static DSTR ExprTab[ENT_NO];

static int
getCmdInd(char *cmd)
{
  int i;

  for (i = 0; i < ENT_NO; i++)
    if (strcmp(cmd, SelectNames[i]) == 0)
      return i;

  return 0;
}

static void
execCmd(char *name)
{
  int cmdInd = getCmdInd(name);
  DSTR cmdStr, argStr;

  cmdStr = DStrNew();
  argStr = DStrNew();

  DStrAssignStr(argStr, " '");
  if (ExprTab[cmdInd] != NULL)
    DStrAppDStr(argStr, ExprTab[cmdInd]);
  DStrAppStr(argStr, "'");

  if (name[1] == '+') {
    CipCmdSeqBegin();
    DStrAssignStr(cmdStr, "SelectAtom");
    DStrAppDStr(cmdStr, argStr);
    (void) CipExecCmd(DStrToStr(cmdStr));
    DStrAssignStr(cmdStr, "SelectBond");
    DStrAppDStr(cmdStr, argStr);
    (void) CipExecCmd(DStrToStr(cmdStr));
    DStrAssignStr(cmdStr, "SelectAngle");
    DStrAppDStr(cmdStr, argStr);
    (void) CipExecCmd(DStrToStr(cmdStr));
    DStrAssignStr(cmdStr, "SelectRes");
    DStrAppDStr(cmdStr, argStr);
    (void) CipExecCmd(DStrToStr(cmdStr));
    CipCmdSeqEnd();
  } else {
    DStrAssignStr(cmdStr, "Select");
    DStrAppStr(cmdStr, name);
    DStrAppDStr(cmdStr, argStr);
    (void) CipExecCmd(DStrToStr(cmdStr));
  }

  DStrFree(cmdStr);
  DStrFree(argStr);
}

static void
cmdCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  execCmd(name);
}

static void
editCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  char *cmd = clientData;
  PuTextCBStruc *callP = callData;
  int cmdInd = getCmdInd(cmd);

  if (ExprTab[cmdInd] == NULL)
    ExprTab[cmdInd] = DStrNew();

  DStrAssignStr(ExprTab[cmdInd], callP->newText);
}

static void
activateCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  execCmd(clientData);
}

static void
popdownCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  PuSwitchGizmo(DialGizmo, FALSE);
  DialOn = FALSE;
}

static void
helpCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  CipShowHelpFile(DialGizmo, "DialSelect");
}

static void
buildDial(void)
{
  PuGizmo g;
  PuConstraints con;
  int i;

  DialGizmo = PuCreateDialog("Selection Dialog", 6, ENT_NO);
  PuSetBool(DialGizmo, PU_BC_PLACE_OUTSIDE, TRUE);

  for (i = 0; i < ENT_NO; i++) {
    con.y = i;
    con.h = 1;

    g = PuCreateButton(DialGizmo, SelectNames[i]);
    con.x = 0;
    con.w = 1;
    PuSetConstraints(g, con);
    PuAddGizmoCB(g, PU_CT_ACTIVATE, cmdCB, NULL, NULL);
    PuSwitchGizmo(g, TRUE);

    g = PuCreateTextField(DialGizmo, "", "");
    con.x = 1;
    con.w = 5;
    PuSetConstraints(g, con);
    PuSetInt(g, PU_IC_HISTORY_SIZE, 20);
    PuAddGizmoCB(g, PU_CT_MODIFY, editCB, SelectNames[i], NULL);
    PuAddGizmoCB(g, PU_CT_ACTIVATE, activateCB, SelectNames[i], NULL);
    PuSwitchGizmo(g, TRUE);
  }

  PuAddGizmoCB(DialGizmo, PU_CT_CLOSE, popdownCB, NULL, NULL);
  PuAddGizmoCB(DialGizmo, PU_CT_HELP, helpCB, NULL, NULL);
}

ErrCode
ExDialSelect(char *cmd)
{
  ArgDescr arg;
  EnumEntryDescr enumEntry[2];
  ErrCode errCode;

  arg.type = AT_ENUM;

  ArgInit(&arg, 1);

  arg.prompt = "Selection Dialog";
  arg.u.enumD.entryP = enumEntry;
  arg.u.enumD.n = 2;

  enumEntry[0].str = "off";
  enumEntry[1].str = "on";

  enumEntry[0].onOff = DialOn;
  enumEntry[1].onOff = ! DialOn;
  if (DialOn)
    arg.v.intVal = 0;
  else
    arg.v.intVal = 1;

  errCode = ArgGet(&arg, 1);
  if (errCode != EC_OK) {
    ArgCleanup(&arg, 1);
    return errCode;
  }

  DialOn = (arg.v.intVal == 1);

  ArgCleanup(&arg, 1);

  if (DialOn) {
    if (DialGizmo == NULL)
      buildDial();
    PuSwitchGizmo(DialGizmo, TRUE);
  } else {
    if (DialGizmo != NULL)
      PuSwitchGizmo(DialGizmo, FALSE);
  }

  return EC_OK;
}
