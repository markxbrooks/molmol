/*
************************************************************************
*
*   ExDialMol.c - DialMol command
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmddial/SCCS/s.ExDialMol.c
*   SCCS identification       : 1.10
*
************************************************************************
*/

#include <cmd_dial.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <break.h>
#include <pu.h>
#include <arg.h>
#include <cip.h>
#include <data_hand.h>

#define PROP_NO 3

typedef struct {
  char *label;
  char *propName;
  char *cmd;
} PropDesc;

static BOOL DialOn = FALSE;
static PuGizmo DialGizmo = NULL, ListGizmo;
static int CurrProp;
static PropRefP CurrRefP;
static int MolNo, MolI;
static BOOL *MolStateA;
static BOOL SuppressUpdate = FALSE;

static PropDesc PropTab[] = {
  {"sel", PROP_SELECTED, "SelectMol"},
  {"disp", PROP_DISPLAYED, "DefPropMol 'displayed'"},
  {"move", PROP_MOVABLE, "DefPropMol 'movable'"}
};

static void
countMol(DhMolP molP, void *clientData)
{
  MolNo++;
}

static void
addMol(DhMolP molP, void *clientData)
{
  DSTR name;
  char buf[10];
  BOOL propVal;

  name = DStrNew();

  if (MolNo > 1000)
    (void) sprintf(buf, "%4d", MolI + 1);
  else if (MolNo > 100)
    (void) sprintf(buf, "%3d", MolI + 1);
  else if (MolNo > 10)
    (void) sprintf(buf, "%2d", MolI + 1);
  else
    (void) sprintf(buf, "%1d", MolI + 1);
  
  DStrAssignStr(name, buf);
  DStrAppStr(name, " ");
  DStrAppDStr(name, DhMolGetName(molP));

  propVal = DhMolGetProp(CurrRefP, molP);
  if (! BreakCheck(10))
    PuAddListEntry(ListGizmo, DStrToStr(name), propVal);
  MolStateA[MolI] = propVal;

  DStrFree(name);
  MolI++;
}

static void
selectMolCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  PuSelectCBStruc *callP = callData;
  DSTR cmd;
  BOOL isFirst;
  char buf[10];
  int num, startI, i;

  (void) sscanf(name, "%d", &num);
  MolStateA[num - 1] = callP->onOff;

  if (! callP->last)
    return;

  cmd = DStrNew();
  DStrAssignStr(cmd, PropTab[CurrProp].cmd);
  isFirst = TRUE;

  for (i = 0; i < MolNo; i++) {
    if (i == 0 || ! MolStateA[i - 1])
      startI = i;
    if (MolStateA[i] && (i == MolNo - 1 || ! MolStateA[i + 1])) {
      if (isFirst)
        DStrAppStr(cmd, " 'num = ");
      else
        DStrAppStr(cmd, ",");
      (void) sprintf(buf, "%d", startI + 1);
      DStrAppStr(cmd, buf);
      if (i > startI) {
        DStrAppStr(cmd, "..");
        (void) sprintf(buf, "%d", i + 1);
        DStrAppStr(cmd, buf);
      }

      isFirst = FALSE;
    }
  }

  if (isFirst)
    DStrAppStr(cmd, " '0'");
  else
    DStrAppStr(cmd, "'");

  SuppressUpdate = TRUE;
  CipExecCmd(DStrToStr(cmd));
  SuppressUpdate = FALSE;

  DStrFree(cmd);
}

static void
buildList(void)
{
  int lineNo;

  MolNo = 0;
  DhApplyMol(PropGetRef(PROP_ALL, FALSE), countMol, NULL);
  if (MolNo > 0) {
    if (MolStateA == NULL)
      MolStateA = malloc(MolNo * sizeof(*MolStateA));
    else
      MolStateA = realloc(MolStateA, MolNo * sizeof(*MolStateA));
    lineNo = MolNo;
    if (lineNo > 40)
      lineNo = 40;
  } else {
    lineNo = 1;
  }

  PuSetInt(ListGizmo, PU_IC_ENTRY_NO, lineNo);

  MolI = 0;
  BreakActivate(TRUE);
  DhApplyMol(PropGetRef(PROP_ALL, FALSE), addMol, NULL);
  BreakActivate(FALSE);

  PuSwitchGizmo(ListGizmo, TRUE);
}

static void
updateList(void)
{
  if (SuppressUpdate)
    return;

  PuSwitchGizmo(ListGizmo, FALSE);
  PuRemoveListEntries(ListGizmo, 0, MolNo);
  buildList();
}

static void
selectPropCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  PuSelectCBStruc *callP = callData;
  PropRefP newPropP;
  int i;

  if (! callP->onOff)
    return;

  for (i = 0; i < PROP_NO; i++)
    if (strcmp(name, PropTab[i].label) == 0)
      break;

  newPropP = PropGetRef(PropTab[i].propName, FALSE);

  if (newPropP != CurrRefP) {
    CurrRefP = newPropP;
    CurrProp = i;
    updateList();
  }
}

static void
popdownCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  PuSwitchGizmo(DialGizmo, FALSE);
  DialOn = FALSE;
}

static void
listCB(void *clientData)
{
  updateList();
}

static void
helpCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  CipShowHelpFile(DialGizmo, "DialMol");
}

static void
buildDial(void)
{
  PuGizmo g;
  int i;

  CurrProp = 0;
  CurrRefP = PropGetRef(PropTab[CurrProp].propName, FALSE);

  DialGizmo = PuCreateDialog("Molecule Dialog", 0, 0);
  PuSetBool(DialGizmo, PU_BC_PLACE_OUTSIDE, TRUE);

  g = PuCreateRadioBox(DialGizmo, "Property");
  for (i = 0; i < PROP_NO; i++)
    PuAddToggle(g, PropTab[i].label, i == 0);
  PuAddGizmoCB(g, PU_CT_SELECT, selectPropCB, NULL, NULL);
  PuSwitchGizmo(g, TRUE);

  ListGizmo = PuCreateList(DialGizmo, "Molecules");
  PuAddGizmoCB(ListGizmo, PU_CT_SELECT, selectMolCB, NULL, NULL);
  PuSetBool(ListGizmo, PU_BC_MULT_SEL, TRUE);
  buildList();

  PuAddGizmoCB(DialGizmo, PU_CT_CLOSE, popdownCB, NULL, NULL);
  PuAddGizmoCB(DialGizmo, PU_CT_HELP, helpCB, NULL, NULL);
}

ErrCode
ExDialMol(char *cmd)
{
  ArgDescr arg;
  EnumEntryDescr enumEntry[2];
  ErrCode errCode;

  arg.type = AT_ENUM;

  ArgInit(&arg, 1);

  arg.prompt = "Molecule Dialog";
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
    if (DialGizmo == NULL) {
      buildDial();
      DhAddMolListCB(listCB, NULL);
    }
    PuSwitchGizmo(DialGizmo, TRUE);
  } else {
    if (DialGizmo != NULL)
      PuSwitchGizmo(DialGizmo, FALSE);
  }

  return EC_OK;
}
