/*
************************************************************************
*
*   UiValuator.c - build valuator box
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
*   Date of last modification : 01/05/27
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/ui/SCCS/s.UiValuator.c
*   SCCS identification       : 1.8
*
************************************************************************
*/

#include <ui_valuator.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <dstr.h>
#include <prog_vers.h>
#include <g_file.h>
#include <pu.h>
#include <par_names.h>
#include <setup_file.h>
#include <cip.h>

#define LINE_LEN 200
#define LABEL_STR_LEN 50
#define CMD_STR_LEN 100
#define NUM_LEN 30

typedef struct {
  BOOL rel;
  float val;
  char *cmd;
} ValuatorData;

PuValuatorBox ValBox = NULL;

static void
freeValuatorData(void *p)
{
  ValuatorData *dataP = p;

  free(dataP->cmd);
  free(dataP);
}

static void
valuatorCB(PuValuator valuator, void *clientData, PuValuatorCBStruc *callP)
{
  ValuatorData *dataP = clientData;
  float cmdVal;
  DSTR cmdStr;
  char numBuf[NUM_LEN];

  if (callP->act == PU_MA_ENTER) {
    CipShowHelpLine(dataP->cmd);
    return;
  }

  if (callP->act == PU_MA_LEAVE) {
    PuSetTextField(PU_TF_STATUS, " ");
    return;
  }

  cmdStr = DStrNew();

  if (dataP->rel)
    cmdVal = callP->val - dataP->val;
  else
    cmdVal = callP->val;

  DStrAssignStr(cmdStr, dataP->cmd);
  DStrAppChar(cmdStr, ' ');
  (void) sprintf(numBuf, "%.5g", cmdVal);
  DStrAppStr(cmdStr, numBuf);

  (void) CipExecCmd(DStrToStr(cmdStr));

  DStrFree(cmdStr);
  dataP->val = callP->val;
}

static char *
skipWhite(char *s)
{
  while (isspace(*s))
    s++;

  return s;
}

static char *
getString(char *resStr, int resSize, char *line)
{
  char quoteCh;
  int i = 0;

  if (line[0] == '"' || line[0] == '\'') {
    quoteCh = line[0];
    line++;
    while (line[i] != '\0' && line[i] != quoteCh && i < resSize - 1) {
      resStr[i] = line[i];
      i++;
    }
    resStr[i] = '\0';
    line += i;
    if (line[0] == quoteCh)
      line++;
  } else {
    while (line[i] != '\0' && !isspace(line[i]) && i < resSize - 1) {
      resStr[i] = line[i];
      i++;
    }
    resStr[i] = '\0';
    line += i;
  }

  return line;
}

static char *
getInt(int *iP, char *line)
{
  DSTR numStr = DStrNew();

  while (isdigit(line[0])) {
    DStrAppChar(numStr, line[0]);
    line++;
  }

  *iP = atoi(DStrToStr(numStr));
  DStrFree(numStr);

  return line;
}

static char *
getFloat(float *fP, char *line)
{
  DSTR numStr = DStrNew();

  while (isdigit(line[0]) || line[0] == '.' ||
      line[0] == '+' || line[0] == '-' ||
      line[0] == 'e' || line[0] == 'E') {
    DStrAppChar(numStr, line[0]);
    line++;
  }

  *fP = (float) atof(DStrToStr(numStr));
  DStrFree(numStr);

  return line;
}

void
UiValuatorCreate(void)
{
  GFile gf;
  GFileRes res;
  char lineBuf[LINE_LEN];
  char *posP;
  char labelStr[LABEL_STR_LEN];
  char cmdStr[CMD_STR_LEN];
  BOOL rel;
  float minVal, maxVal, defVal;
  int digits;
  ValuatorData *dataP;

  gf = SetupOpen(PN_MENU_DIR, "Valuator", TRUE);
  if (gf == NULL)
    return;

  ValBox = PuCreateValuatorBox(PROG_NAME);
  PuSetBool(ValBox, PU_BC_PLACE_OUTSIDE, TRUE);

  while (! GFileEOF(gf)) {
    res = GFileReadLine(gf, lineBuf, sizeof(lineBuf));
    if (res != GF_RES_OK)
      return;

    if (lineBuf[0] == '#')
      continue;
    
    posP = skipWhite(lineBuf);

    if (*posP == '\0')
      continue;

    posP = getString(labelStr, sizeof(labelStr), posP);
    posP = skipWhite(posP);
    posP = getString(cmdStr, sizeof(cmdStr), posP);
    posP = skipWhite(posP);
    rel = (*posP == 'r' || *posP == 'R');
    posP++;
    posP = skipWhite(posP);
    posP = getFloat(&minVal, posP);
    posP = skipWhite(posP);
    posP = getFloat(&maxVal, posP);
    posP = skipWhite(posP);
    posP = getInt(&digits, posP);
    posP = skipWhite(posP);
    posP = getFloat(&defVal, posP);

    dataP = malloc(sizeof(*dataP));
    dataP->rel = rel;
    dataP->cmd = malloc(strlen(cmdStr) + 1);
    (void) strcpy(dataP->cmd, cmdStr);
    dataP->val = defVal;

    (void) PuCreateValuator(ValBox, labelStr,
	minVal, maxVal, digits, defVal,
	valuatorCB, dataP, freeValuatorData);
  }

  GFileClose(gf);
}

void
UiValuatorSwitch(BOOL onOff)
{
  if (ValBox != NULL)
    PuSwitchValuatorBox(ValBox, onOff);
}
