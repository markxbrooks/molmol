/*
************************************************************************
*
*   UiCmdField.c - creation of command fields
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
*   Date of last modification : 96/03/19
*   Pathname of SCCS file     : /sgiext/molmol/src/ui/SCCS/s.UiCmdField.c
*   SCCS identification       : 1.9
*
************************************************************************
*/

#include <ui_cmd_field.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <dstr.h>
#include <g_file.h>
#include <pu.h>
#include <par_names.h>
#include <setup_file.h>
#include <cip.h>

#define KEY_WORD "button"
#define HELP_KEY_WORD "help"
#define LINE_LEN 200

typedef struct {
  int cmdNo;
  char **cmdList;
  char *helpLine;
} CmdFieldData;

static void
freeCmdFieldData(void *p)
{
  CmdFieldData *dataP = p;
  int i;

  for (i = 0; i < dataP->cmdNo; i++)
    free(dataP->cmdList[i]);

  if (dataP->cmdNo > 0)
    free(dataP->cmdList);

  free(dataP);
}

static void
cmdFieldCB(PuCmdField cmdField, void *clientData, PuCmdFieldCBStruc *callP)
{
  CmdFieldData *dataP = clientData;
  int i;

  if (callP->act == PU_MA_ACTIVATE) {
    if (dataP->cmdNo == 0)
      return;

    if (dataP->cmdNo == 1) {
      CipExecCmd(dataP->cmdList[0]);
    } else {
      CipCmdSeqBegin();
      for (i = 0; i < dataP->cmdNo; i++)
	CipExecCmd(dataP->cmdList[i]);
      CipCmdSeqEnd();
    }
  } else if (callP->act == PU_MA_ENTER) {
    PuSetTextField(PU_TF_STATUS, dataP->helpLine);
  } else {
    PuSetTextField(PU_TF_STATUS, " ");
  }
}

static char *
skipWhite(char *s)
{
  while (isspace(*s))
    s++;

  return s;
}

void
UiCmdFieldsCreate(void)
{
  GFile gf;
  GFileRes res;
  char lineBuf[LINE_LEN];
  char *posP;
  CmdFieldData *dataP = NULL;

  gf = SetupOpen(PN_MENU_DIR, "Buttons", TRUE);
  if (gf == NULL)
    return;

  while (! GFileEOF(gf)) {
    res = GFileReadLine(gf, lineBuf, sizeof(lineBuf));
    if (res != GF_RES_OK)
      return;

    if (lineBuf[0] == '#')
      continue;
    
    posP = skipWhite(lineBuf);

    if (*posP == '\0')
      continue;

    if (strncmp(posP, KEY_WORD, strlen(KEY_WORD)) == 0) {
      posP += strlen(KEY_WORD);
      posP = skipWhite(posP);

      if (*posP == '\0') {
	(void) PuCreateCmdField("", NULL, NULL, NULL);
      } else {
	dataP = malloc(sizeof(*dataP));
	dataP->cmdNo = 0;
	dataP->helpLine = " ";
	(void) PuCreateCmdField(posP, cmdFieldCB, dataP, freeCmdFieldData);
      }
    } else if (dataP != NULL) {
      posP = skipWhite(posP);

      if (strncmp(posP, HELP_KEY_WORD, strlen(HELP_KEY_WORD)) == 0) {
	posP += strlen(HELP_KEY_WORD);
	posP = skipWhite(posP);

	dataP->helpLine = malloc(strlen(posP) + 1);
	(void) strcpy(dataP->helpLine, posP);
      } else {
	dataP->cmdNo++;
	if (dataP->cmdNo == 1)
	  dataP->cmdList = malloc(sizeof(*dataP->cmdList));
	else
	  dataP->cmdList = realloc(dataP->cmdList,
	      dataP->cmdNo * sizeof(*dataP->cmdList));

	dataP->cmdList[dataP->cmdNo - 1] = malloc(strlen(posP) + 1);
	(void) strcpy(dataP->cmdList[dataP->cmdNo - 1], posP);
      }
    }
  }

  GFileClose(gf);
}
