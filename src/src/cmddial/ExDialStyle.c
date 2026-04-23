/*
************************************************************************
*
*   ExDialStyle.c - DialStyle command
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmddial/SCCS/s.ExDialStyle.c
*   SCCS identification       : 1.5
*
************************************************************************
*/

#include <cmd_dial.h>

#include <stdio.h>
#include <string.h>

#include <pu.h>
#include <arg.h>
#include <cip.h>

#define CMD_NO 3
#define MAX_ARG 9

static BOOL DialOn = FALSE;
static PuGizmo DialGizmo = NULL;

typedef struct {
  char *cmd;
  char *arg[MAX_ARG];
} CmdDescr;

static CmdDescr CmdTab[] = {
  {"Atom", {"invisible", "sphere", "tetrahedron"}},
  {"Bond", {"invisible", "line", "half_line",
      "cylinder", "half_cylinder", "neon", "half_neon", "cone", "half_cone"}},
  {"Dist", {"invisible", "line", "viol", "cylinder", "neon", "cone"}}
};

static void
activateCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  CipExecCmd(DStrToStr(clientData));
}

static void
popdownCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  PuSwitchGizmo(DialGizmo, FALSE);
  DialOn = FALSE;
}

static void
freeStr(void *p)
{
  DStrFree(p);
}

static void
helpCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  CipShowHelpFile(DialGizmo, "DialStyle");
}

static void
buildDial(void)
{
  PuGizmo g;
  PuConstraints lCon, bCon;
  DSTR cmd;
  int i, k;

  DialGizmo = PuCreateDialog("Style Dialog", CMD_NO, MAX_ARG + 1);
  PuSetBool(DialGizmo, PU_BC_PLACE_OUTSIDE, TRUE);

  lCon.y = 0;
  lCon.h = 1;

  for (i = 0; i < CMD_NO; i++) {
    g = PuCreateLabel(DialGizmo, CmdTab[i].cmd);
    lCon.x = i;
    lCon.w = 1;
    PuSetConstraints(g, lCon);
    PuSwitchGizmo(g, TRUE);

    bCon.x = i;
    bCon.w = 1;

    for (k = 0; k < MAX_ARG; k++) {
      if (CmdTab[i].arg[k] == NULL)
	break;

      cmd = DStrNew();
      DStrAssignStr(cmd, "Style");
      DStrAppStr(cmd, CmdTab[i].cmd);
      DStrAppStr(cmd, " ");
      DStrAppStr(cmd, CmdTab[i].arg[k]);

      g = PuCreateButton(DialGizmo, CmdTab[i].arg[k]);
      bCon.y = k + 1;
      bCon.h = 1;
      PuSetConstraints(g, bCon);
      PuAddGizmoCB(g, PU_CT_ACTIVATE, activateCB, cmd, freeStr);
      PuSwitchGizmo(g, TRUE);
    }
  }

  PuAddGizmoCB(DialGizmo, PU_CT_CLOSE, popdownCB, NULL, NULL);
  PuAddGizmoCB(DialGizmo, PU_CT_HELP, helpCB, NULL, NULL);
}

ErrCode
ExDialStyle(char *cmd)
{
  ArgDescr arg;
  EnumEntryDescr enumEntry[2];
  ErrCode errCode;

  arg.type = AT_ENUM;

  ArgInit(&arg, 1);

  arg.prompt = "Style Dialog";
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
