/*
************************************************************************
*
*   UiTip.c - display "Tip of the Day"
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
*   Date of last modification : 99/10/24
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/src/ui/SCCS/s.UiTip.c
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include <ui_tip.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <bool.h>
#include <rand_num.h>
#include <dstr.h>
#include <pu.h>
#include <os_seed.h>
#include <os_dir.h>
#include <prog_dir.h>
#include <par_names.h>
#include <par_hand.h>
#include <cip.h>

#define DIAL_H 6

typedef struct {
  PuGizmo dialG, textG;
  char **nameA;
  int nameNo;
} TipData;

static BOOL FirstTime = TRUE;

static void
switchCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  PuSelectCBStruc *callP = callData;

  ParSetIntVal(PN_SWITCH_TIP, callP->onOff);
}

static void
showOne(PuGizmo g, char **nameA, int nameNo)
{
  int nameI;
  char *name, *buf;
  FILE *fp;
  struct stat st;

  nameI = RandInt32() % nameNo;
  name = nameA[nameI];

  fp = fopen(name, "rb");
  if (fp == NULL)
    return;

  if (fstat(fileno(fp), &st) == -1)
    return;

  buf = malloc(st.st_size + 1);
  (void) fread(buf, 1, st.st_size, fp);
  buf[st.st_size] = '\0';
  (void) fclose(fp);

  PuSetStr(g, PU_SC_TEXT, buf);

  free(buf);
}

static void
nextCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  TipData *dataP = clientData;

  showOne(dataP->textG, dataP->nameA, dataP->nameNo);
}

static void
popdownCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  TipData *dataP = clientData;

  PuDestroyGizmo(dataP->dialG);

  OsDirFree(dataP->nameA, dataP->nameNo);
  free(dataP);
}

static void
helpCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  TipData *dataP = clientData;

  CipShowHelpFile(dataP->dialG, "HelpTip");
}

void
UiTipDisplay(void)
{
  DSTR tipDir;
  TipData *dataP;
  PuGizmo g;
  PuConstraints con;

  if (FirstTime) {
    RandSetSeed(OsSeed());
    FirstTime = FALSE;
  }

  dataP = malloc(sizeof(*dataP));

  tipDir = DStrNew();
  DStrAssignStr(tipDir, ProgDirGet());
  DStrAppStr(tipDir, "/tips");
  OsDirGet(DStrToStr(tipDir), "*", &dataP->nameA, &dataP->nameNo);
  DStrFree(tipDir);

  if (dataP->nameNo == 0)
    return;

  dataP->dialG = PuCreateDialog("Tip of the Day", 3, DIAL_H);

  dataP->textG = PuCreateText(dataP->dialG, "Text", "");
  con.x = 0;
  con.w = 3;
  con.y = 0;
  con.h = DIAL_H - 1;
  PuSetConstraints(dataP->textG, con);
  PuSetInt(dataP->textG, PU_IC_TEXT_WIDTH, 40);
  PuSetInt(dataP->textG, PU_IC_TEXT_HEIGHT, 7);
  PuSwitchGizmo(dataP->textG, TRUE);

  g = PuCreateButton(dataP->dialG, "Next Tip");
  con.x = 2;
  con.w = 1;
  con.y = DIAL_H - 1;
  con.h = 1;
  PuSetConstraints(g, con);
  PuAddGizmoCB(g, PU_CT_ACTIVATE, nextCB, dataP, NULL);
  PuSwitchGizmo(g, TRUE);

  g = PuCreateCheckBox(dataP->dialG, "");
  PuAddToggle(g, "show tip on startup", ParGetIntVal(PN_SWITCH_TIP));
  con.x = 0;
  con.w = 2;
  con.y = DIAL_H - 1;
  con.h = 1;
  PuSetConstraints(g, con);
  PuAddGizmoCB(g, PU_CT_SELECT, switchCB, NULL, NULL);
  PuSwitchGizmo(g, TRUE);

  PuAddGizmoCB(dataP->dialG, PU_CT_CLOSE, popdownCB, dataP, NULL);
  PuAddGizmoCB(dataP->dialG, PU_CT_HELP, helpCB, dataP, NULL);

  showOne(dataP->textG, dataP->nameA, dataP->nameNo);

  PuSwitchGizmo(dataP->dialG, TRUE);
}
