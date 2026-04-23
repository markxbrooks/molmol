/*
************************************************************************
*
*   ExRecordMac.c - RecordMac command
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmddial/SCCS/s.ExRecordMac.c
*   SCCS identification       : 1.5
*
************************************************************************
*/

#include <cmd_dial.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dstr.h>
#include <pu.h>
#include <cip.h>
#include <history.h>

#define DIAL_H 12
#define DEFAULT_NAME "new.mac"

typedef struct {
  PuGizmo dialG, textG;
  char *str;
  int len;
  int addNo;
  BOOL addAuto;
  BOOL testMode;
  char *fileName;
} RecordData;

static void
editCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  RecordData *dataP = clientData;
  PuTextCBStruc *callP = callData;

  dataP->len = strlen(callP->newText);
  dataP->str = realloc(dataP->str, dataP->len + 1);
  (void) strcpy(dataP->str, callP->newText);
}

static void
editAddNoCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  RecordData *dataP = clientData;
  PuTextCBStruc *callP = callData;

  dataP->addNo = atoi(callP->newText);
}

static void
addCmd(RecordData *dataP, char *cmd)
{
  int newLen;

  newLen = dataP->len + strlen(cmd) + 1;
  dataP->str = realloc(dataP->str, newLen + 1);
  (void) strcpy(dataP->str + dataP->len, cmd);
  dataP->str[newLen - 1] = '\n';
  dataP->str[newLen] = '\0';

  dataP->len = newLen;
}

static void
addCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  RecordData *dataP = clientData;
  char *cmd;
  int i;

  for (i = 0; i < dataP->addNo; i++) {
    cmd = HistoryGetCmd(dataP->addNo - i - 1);
    if (cmd != NULL)
      addCmd(dataP, cmd);
  }

  PuSetStr(dataP->textG, PU_SC_TEXT, dataP->str);
}

static void
selectCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  RecordData *dataP = clientData;
  PuSelectCBStruc *callP = callData;

  dataP->addAuto = callP->onOff;
}

static void
testCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  RecordData *dataP = clientData;
  DSTR cmdStr;
  ErrCode errCode;
  char *s;

  cmdStr = DStrNew();
  CipCmdSeqBegin();
  dataP->testMode = TRUE;

  s = dataP->str;
  while (*s != '\0') {
    if (*s == '\n') {
      errCode = CipExecCmd(DStrToStr(cmdStr));
      if (errCode == EC_ERROR)
	break;
      DStrAssignStr(cmdStr, "");
    } else {
      DStrAppChar(cmdStr, *s);
    }

    s++;
  }

  dataP->testMode = FALSE;
  CipCmdSeqEnd();
  DStrFree(cmdStr);
}

static void
saveCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  RecordData *dataP = clientData;
  FILE *fp;

  fp = fopen(dataP->fileName, "w");
  if (fp == NULL)
    return;
  
  (void) fwrite(dataP->str, 1, dataP->len, fp);
  (void) fclose(fp);
}

static void
editFileNameCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  RecordData *dataP = clientData;
  PuTextCBStruc *callP = callData;

  free(dataP->fileName);
  dataP->fileName = malloc(strlen(callP->newText) + 1);
  (void) strcpy(dataP->fileName, callP->newText);
}

static void
historyCB(char *cmd, void *clientData)
{
  RecordData *dataP = clientData;

  if (dataP->addAuto && ! dataP->testMode) {
    addCmd(dataP, cmd);
    PuSetStr(dataP->textG, PU_SC_TEXT, dataP->str);
  }
}

static void
popdownCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  RecordData *dataP = clientData;

  PuDestroyGizmo(dataP->dialG);

  HistoryRemoveCB(historyCB, dataP);

  free(dataP->str);
  free(dataP->fileName);
  free(dataP);
}

static void
helpCB(PuGizmo g, char *name, void *clientData, void *callData)
{
  RecordData *dataP = clientData;

  CipShowHelpFile(dataP->dialG, "RecordMac");
}

ErrCode
ExRecordMac(char *cmd)
{
  RecordData *dataP;
  PuConstraints con;
  PuGizmo g;

  dataP = malloc(sizeof(*dataP));
  dataP->str = malloc(1);
  dataP->len = 0;
  dataP->addNo = 1;
  dataP->addAuto = FALSE;
  dataP->testMode = FALSE;
  dataP->fileName = malloc(strlen(DEFAULT_NAME) + 1);
  (void) strcpy(dataP->fileName, DEFAULT_NAME);

  dataP->dialG = PuCreateDialog("Macro Recorder", 3, DIAL_H);
  PuSetBool(dataP->dialG, PU_BC_PLACE_OUTSIDE, TRUE);

  dataP->textG = PuCreateText(dataP->dialG, "Text", "");
  con.x = 0;
  con.w = 3;
  con.y = 0;
  con.h = DIAL_H - 2;
  PuSetConstraints(dataP->textG, con);
  PuSetInt(dataP->textG, PU_IC_TEXT_WIDTH, 40);
  PuAddGizmoCB(dataP->textG, PU_CT_MODIFY, editCB, dataP, NULL);
  PuSwitchGizmo(dataP->textG, TRUE);

  g = PuCreateButton(dataP->dialG, "Add");
  con.x = 0;
  con.w = 1;
  con.y = DIAL_H - 2;
  con.h = 1;
  PuSetConstraints(g, con);
  PuAddGizmoCB(g, PU_CT_ACTIVATE, addCB, dataP, NULL);
  PuSwitchGizmo(g, TRUE);

  g = PuCreateTextField(dataP->dialG, "AddNo", "1");
  con.x = 1;
  con.w = 1;
  con.y = DIAL_H - 2;
  con.h = 1;
  PuSetConstraints(g, con);
  PuSetInt(g, PU_IC_TEXT_WIDTH, 5);
  PuAddGizmoCB(g, PU_CT_MODIFY, editAddNoCB, dataP, NULL);
  PuAddGizmoCB(g, PU_CT_ACTIVATE, addCB, dataP, NULL);
  PuSwitchGizmo(g, TRUE);

  g = PuCreateCheckBox(dataP->dialG, "");
  PuAddToggle(g, "automatic record", FALSE);
  con.x = 2;
  con.w = 1;
  con.y = DIAL_H - 2;
  con.h = 1;
  PuSetConstraints(g, con);
  PuAddGizmoCB(g, PU_CT_SELECT, selectCB, dataP, NULL);
  PuSwitchGizmo(g, TRUE);

  g = PuCreateButton(dataP->dialG, "Test");
  con.x = 0;
  con.w = 1;
  con.y = DIAL_H - 1;
  con.h = 1;
  PuSetConstraints(g, con);
  PuAddGizmoCB(g, PU_CT_ACTIVATE, testCB, dataP, NULL);
  PuSwitchGizmo(g, TRUE);

  g = PuCreateButton(dataP->dialG, "Save");
  con.x = 1;
  con.w = 1;
  con.y = DIAL_H - 1;
  con.h = 1;
  PuSetConstraints(g, con);
  PuAddGizmoCB(g, PU_CT_ACTIVATE, saveCB, dataP, NULL);
  PuSwitchGizmo(g, TRUE);

  g = PuCreateTextField(dataP->dialG, "FileName", DEFAULT_NAME);
  con.x = 2;
  con.w = 1;
  con.y = DIAL_H - 1;
  con.h = 1;
  PuSetConstraints(g, con);
  PuSetInt(g, PU_IC_TEXT_WIDTH, 15);
  PuAddGizmoCB(g, PU_CT_MODIFY, editFileNameCB, dataP, NULL);
  PuAddGizmoCB(g, PU_CT_ACTIVATE, saveCB, dataP, NULL);
  PuSwitchGizmo(g, TRUE);

  PuAddGizmoCB(dataP->dialG, PU_CT_CLOSE, popdownCB, dataP, NULL);
  PuAddGizmoCB(dataP->dialG, PU_CT_HELP, helpCB, dataP, NULL);

  HistoryAddCB(historyCB, dataP);

  PuSwitchGizmo(dataP->dialG, TRUE);

  return EC_OK;
}
