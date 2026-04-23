/*
************************************************************************
*
*   ExDialColor.c - DialColor command
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
*   Date of last modification : 01/05/27
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmddial/SCCS/s.ExDialColor.c
*   SCCS identification       : 1.8
*
************************************************************************
*/

#include <cmd_dial.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pu.h>
#include <arg.h>
#include <par_names.h>
#include <setup_file.h>
#include <cip.h>
#include <data_hand.h>
#include <data_sel.h>
#include <prim_hand.h>
#include <prim_sel.h>
#include <attr_struc.h>

#define NAME_LEN 100
#define NUM_LEN 6
#define CMD_NO 7

#define EPS ((float) 1.0e-5)

typedef struct {
  char *name;
  char r[NUM_LEN];
  char g[NUM_LEN];
  char b[NUM_LEN];
} ColorDesc;

static ColorDesc *ColorList;
static int ColorNo = 0;
static BOOL DialOn = FALSE;
static PuGizmo DialGizmo = NULL, NameGizmo, RGizmo, GGizmo, BGizmo, ColGizmo;
static DSTR NameStr = NULL, RStr, GStr, BStr;

static char *LabelList[] = {
  "Back",
  "Atom",
  "Bond",
  "Dist",
  "Dist2",
  "Prim",
  "Prim2"
};

static char *CmdList[] = {
  "BackColor",
  "ColorAtom",
  "ColorBond",
  "ColorDist",
  "TintDist",
  "ColorPrim",
  "TintPrim"
};

static void
readColorList(void)
{
  GFile gf;
  GFileRes res;
  ColorDesc desc;
  char name[NAME_LEN];

  gf = SetupOpen(PN_COLOR_LIST, "ColorList", FALSE);
  if (gf == NULL)
    return;
  
  while (! GFileEOF(gf)) {
    res = GFileReadStr(gf, desc.r, sizeof(desc.r));
    if (res != GF_RES_OK)
      break;

    res = GFileReadStr(gf, desc.g, sizeof(desc.g));
    if (res != GF_RES_OK)
      break;

    res = GFileReadStr(gf, desc.b, sizeof(desc.b));
    if (res != GF_RES_OK)
      break;

    res = GFileReadLine(gf, name, sizeof(name));
    if (res != GF_RES_OK)
      break;

    desc.name = malloc(strlen(name) + 1);
    (void) strcpy(desc.name, name);

    ColorNo++;
    if (ColorNo == 1)
      ColorList = malloc(sizeof(*ColorList));
    else
      ColorList = realloc(ColorList, ColorNo * sizeof(*ColorList));
    
    ColorList[ColorNo - 1] = desc;
  }

  GFileClose(gf);
}

static void
showColor(void)
{
  PuSetColor(ColGizmo, (float) atof(DStrToStr(RStr)),
      (float) atof(DStrToStr(GStr)), (float) atof(DStrToStr(BStr)));
}

static void
editCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  char *field = clientData;
  PuTextCBStruc *callP = callData;

  if (field[0] == 'N')
    DStrAssignStr(NameStr, callP->newText);
  else if (field[0] == 'R')
    DStrAssignStr(RStr, callP->newText);
  else if (field[0] == 'G')
    DStrAssignStr(GStr, callP->newText);
  else
    DStrAssignStr(BStr, callP->newText);
}

static void
updateFields(int colorI)
{
  PuSetStr(RGizmo, PU_SC_TEXT, ColorList[colorI].r);
  PuSetStr(GGizmo, PU_SC_TEXT, ColorList[colorI].g);
  PuSetStr(BGizmo, PU_SC_TEXT, ColorList[colorI].b);
}

static void
showName(void)
{
  float r, g, b, d;
  int i;

  r = (float) atof(DStrToStr(RStr));
  g = (float) atof(DStrToStr(GStr));
  b = (float) atof(DStrToStr(BStr));

  for (i = 0; i < ColorNo; i++) {
    d = r - (float) atof(ColorList[i].r);
    if (d > EPS || d < - EPS)
      continue;

    d = g - (float) atof(ColorList[i].g);
    if (d > EPS || d < - EPS)
      continue;

    d = b - (float) atof(ColorList[i].b);
    if (d > EPS || d < - EPS)
      continue;
    
    PuSetStr(NameGizmo, PU_SC_TEXT, ColorList[i].name);
    break;
  }

  if (i == ColorNo)
    PuSetStr(NameGizmo, PU_SC_TEXT, "");
}

static void
activateCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  char *field = clientData;
  int i;

  if (field[0] == 'N') {
    for (i = 0; i < ColorNo; i++)
      if (strcmp(ColorList[i].name, DStrToStr(NameStr)) == 0) {
	updateFields(i);
	break;
      }
  } else {
    showName();
  }

  showColor();
}

static void
selectCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  PuSelectCBStruc *callP = callData;
  int i;

  if (! callP->onOff)
    return;

  i = 0;
  while (strcmp(ColorList[i].name, name) != 0)
    i++;
  
  PuSetStr(NameGizmo, PU_SC_TEXT, ColorList[i].name);
  updateFields(i);

  showColor();
}

static void
cmdCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  DSTR cmdStr = DStrNew();
  int i;

  i = 0;
  while (strcmp(LabelList[i], name) != 0)
    i++;

  DStrAssignStr(cmdStr, CmdList[i]);
  DStrAppChar(cmdStr, ' ');
  DStrAppDStr(cmdStr, RStr);
  DStrAppChar(cmdStr, ' ');
  DStrAppDStr(cmdStr, GStr);
  DStrAppChar(cmdStr, ' ');
  DStrAppDStr(cmdStr, BStr);

  (void) CipExecCmd(DStrToStr(cmdStr));

  DStrFree(cmdStr);
}

static void
popdownCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  PuSwitchGizmo(DialGizmo, FALSE);
  DialOn = FALSE;
}

static void
showAttr(AttrP attrP)
{
  char buf[10];

  (void) sprintf(buf, "%5.3f", attrP->colR);
  PuSetStr(RGizmo, PU_SC_TEXT, buf);
  (void) sprintf(buf, "%5.3f", attrP->colG);
  PuSetStr(GGizmo, PU_SC_TEXT, buf);
  (void) sprintf(buf, "%5.3f", attrP->colB);
  PuSetStr(BGizmo, PU_SC_TEXT, buf);

  showName();
  showColor();
}

static void
selCB(DataEntityType entType, void *entP, SelKind kind)
{
  AttrP attrP;

  if (kind == SK_REMOVE || ! DialOn)
    return;
  
  switch(entType) {
    case DE_ATOM:
      attrP = DhAtomGetAttr(entP);
      break;
    case DE_BOND:
      attrP = DhBondGetAttr(entP);
      break;
    case DE_DIST:
      attrP = DhDistGetAttr(entP);
      break;
    default:
      return;
  }

  showAttr(attrP);
}

static void
primSelCB(PrimObjP primP, SelKind kind)
{
  if (kind == SK_REMOVE || ! DialOn)
    return;
  
  showAttr(PrimGetAttr(primP));
}

static void
helpCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  CipShowHelpFile(DialGizmo, "DialColor");
}

static void
buildDial(void)
{
  PuGizmo g;
  PuConstraints con;
  int i;

  DialGizmo = PuCreateDialog("Color Dialog", 7, 5);
  PuSetBool(DialGizmo, PU_BC_PLACE_OUTSIDE, TRUE);

  con.x = 0;
  con.w = 1;
  con.h = 1;

  g = PuCreateLabel(DialGizmo, "Name");
  con.y = 0;
  PuSetConstraints(g, con);
  PuSwitchGizmo(g, TRUE);

  g = PuCreateLabel(DialGizmo, "Red");
  con.y = 1;
  PuSetConstraints(g, con);
  PuSwitchGizmo(g, TRUE);

  g = PuCreateLabel(DialGizmo, "Green");
  con.y = 2;
  PuSetConstraints(g, con);
  PuSwitchGizmo(g, TRUE);

  g = PuCreateLabel(DialGizmo, "Blue");
  con.y = 3;
  PuSetConstraints(g, con);
  PuSwitchGizmo(g, TRUE);

  con.x = 1;
  con.w = 2;
  con.h = 1;

  NameStr = DStrNew();
  RStr = DStrNew();
  GStr = DStrNew();
  BStr = DStrNew();
  DStrAssignStr(RStr, "0.0");
  DStrAssignStr(GStr, "0.0");
  DStrAssignStr(BStr, "0.0");

  NameGizmo = PuCreateTextField(DialGizmo, "Name", DStrToStr(NameStr));
  con.y = 0;
  PuSetConstraints(NameGizmo, con);
  PuSetInt(NameGizmo, PU_IC_TEXT_WIDTH, 13);
  PuAddGizmoCB(NameGizmo, PU_CT_MODIFY, editCB, "N", NULL);
  PuAddGizmoCB(NameGizmo, PU_CT_ACTIVATE, activateCB, "N", NULL);
  showName();
  PuSwitchGizmo(NameGizmo, TRUE);

  RGizmo = PuCreateTextField(DialGizmo, "Red", DStrToStr(RStr));
  con.y = 1;
  PuSetConstraints(RGizmo, con);
  PuSetInt(RGizmo, PU_IC_TEXT_WIDTH, NUM_LEN);
  PuAddGizmoCB(RGizmo, PU_CT_MODIFY, editCB, "R", NULL);
  PuAddGizmoCB(RGizmo, PU_CT_ACTIVATE, activateCB, "R", NULL);
  PuSwitchGizmo(RGizmo, TRUE);

  GGizmo = PuCreateTextField(DialGizmo, "Green", DStrToStr(GStr));
  con.y = 2;
  PuSetConstraints(GGizmo, con);
  PuSetInt(GGizmo, PU_IC_TEXT_WIDTH, NUM_LEN);
  PuAddGizmoCB(GGizmo, PU_CT_MODIFY, editCB, "G", NULL);
  PuAddGizmoCB(GGizmo, PU_CT_ACTIVATE, activateCB, "G", NULL);
  PuSwitchGizmo(GGizmo, TRUE);

  BGizmo = PuCreateTextField(DialGizmo, "Blue", DStrToStr(BStr));
  con.y = 3;
  PuSetConstraints(BGizmo, con);
  PuSetInt(BGizmo, PU_IC_TEXT_WIDTH, NUM_LEN);
  PuAddGizmoCB(BGizmo, PU_CT_MODIFY, editCB, "B", NULL);
  PuAddGizmoCB(BGizmo, PU_CT_ACTIVATE, activateCB, "B", NULL);
  PuSwitchGizmo(BGizmo, TRUE);

  ColGizmo = PuCreateColorField(DialGizmo, "Color");
  con.x = 3;
  con.y = 0;
  con.w = 2;
  con.h = 4;
  PuSetConstraints(ColGizmo, con);
  showColor();
  PuSwitchGizmo(ColGizmo, TRUE);

  g = PuCreateList(DialGizmo, "Color List");
  for (i = 0; i < ColorNo; i++)
    PuAddListEntry(g, ColorList[i].name, FALSE);
  con.x = 5;
  con.y = 0;
  con.w = 2;
  con.h = 4;
  PuSetConstraints(g, con);
  PuSetBool(g, PU_BC_AUTO_DESEL, TRUE);
  PuAddGizmoCB(g, PU_CT_SELECT, selectCB, NULL, NULL);
  PuSwitchGizmo(g, TRUE);

  con.y = 4;
  con.w = 1;
  con.h = 1;

  for (i = 0; i < CMD_NO; i++) {
    g = PuCreateButton(DialGizmo, LabelList[i]);
    con.x = i;
    PuSetConstraints(g, con);
    PuAddGizmoCB(g, PU_CT_ACTIVATE, cmdCB, NULL, NULL);
    PuSwitchGizmo(g, TRUE);
  }

  PuAddGizmoCB(DialGizmo, PU_CT_CLOSE, popdownCB, NULL, NULL);
  PuAddGizmoCB(DialGizmo, PU_CT_HELP, helpCB, NULL, NULL);
}

ErrCode
ExDialColor(char *cmd)
{
  ArgDescr arg;
  EnumEntryDescr enumEntry[2];
  ErrCode errCode;

  arg.type = AT_ENUM;

  ArgInit(&arg, 1);

  arg.prompt = "Color Dialog";
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
      readColorList();
      buildDial();
      SelAddCB(selCB);
      PrimSelAddCB(primSelCB);
    }
    PuSwitchGizmo(DialGizmo, TRUE);
  } else {
    if (DialGizmo != NULL)
      PuSwitchGizmo(DialGizmo, FALSE);
  }

  return EC_OK;
}
