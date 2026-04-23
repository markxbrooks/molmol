/*
************************************************************************
*
*   TTY.c - TTY functions
*
*   Copyright (c) 1994-97
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/tty/SCCS/s.TTY.c
*   SCCS identification       : 1.12
*
************************************************************************
*/

#include "tty.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <pu_cb.h>

#define MAX_LINE_LEN 1024

typedef struct {
  PuGizmoCB okCB;
  void *clientData;
  PuFreeCB freeCB;
  BOOL fileSel;
} DialData;

static BOOL PromptOnOff, StatusOnOff;
static char *PromptStr = NULL;

PuRes
PuTTYInit(char *appName, int *argcP, char *argv[])
{
  return PU_RES_OK;
}

PuRes
PuTTYCleanup(void)
{
  return PU_RES_OK;
}

static void
printPrompt(void)
{
  if (PromptOnOff)
    (void) fprintf(stderr, "%s", PromptStr);
  (void) fprintf(stderr, "> ");
}

static void
processStr(char *s)
{
  PuTextCBStruc textStruc;
  int x, y;
  int len;

  len = strlen(s);

  if (s[len - 1] == '\n') {
    len--;
    s[len] = '\0';
  }

  if (isdigit(s[0]) && sscanf(s, "%d %d", &x, &y) == 2) {
    /* coordinates, send mouse event */
    PuCallButtonPressCB(NULL, PU_MB_1, FALSE, x, y);
    PuCallButtonReleaseCB(NULL, PU_MB_1, TRUE, x, y);
  } else {
    textStruc.action = PU_CA_APPEND;
    textStruc.oldText = "";
    textStruc.oldLen = 0;
    textStruc.newText = s;
    textStruc.newLen = len;
    /* newPart must by malloced, callback can do realloc when making
       it longer. */
    textStruc.newPart = malloc(len + 1);
    (void) strcpy(textStruc.newPart, s);
    textStruc.newPartStart = 0;
    textStruc.newPartEnd = len;
    textStruc.doIt = TRUE;

    PuCallCmdCB(NULL, &textStruc);

    if (textStruc.doIt) {
      textStruc.action = PU_CA_CR;
      PuCallCmdCB(NULL, &textStruc);
    }

    free(textStruc.newPart);
  }
}

void
PuTTYEventLoop(void)
{
  char line[MAX_LINE_LEN], *s;

  for (;;) {
    printPrompt();
    s = fgets(line, sizeof(line), stdin);
    if (s == NULL)
      break;
    processStr(s);
  }

  PuCallQuitCB();
}

void
PuTTYProcessEvent(void)
{
  char line[MAX_LINE_LEN], *s;

  printPrompt();
  s = fgets(line, sizeof(line), stdin);
  if (s != NULL)
    processStr(s);
}

void
PuTTYAddTimeOut(int interv, PuTimeOut timeOutF, void *clientData)
{
}

void
PuTTYAddExtInput(int fd,
    PuExtInput extInpF, void *clientData, PuFreeCB freeCB)
{
}

void
PuTTYRemoveExtInput(int fd, PuExtInput extInpF, void *clientData)
{
}

void
PuTTYSetDrawSize(int w, int h)
{
  PuCallResizeCB(NULL, w, h);
}

void
PuTTYSwitchFullscreen(BOOL onOff)
{
}

void
PuTTYSetTextField(PuTextFieldChoice fieldChoice, char *text)
{
  if (text[0] == '\0' || (text[0] == ' ' && text[1] == '\0'))
    return;

  switch (fieldChoice) {
    case PU_TF_PROMPT:
      if (PromptStr != NULL)
	free(PromptStr);
      PromptStr = malloc(strlen(text) + 1);
      (void) strcpy(PromptStr, text);
      break;
    case PU_TF_STATUS:
      if (StatusOnOff)
	(void) fprintf(stderr, "STATUS: %s\n", text);
      break;
    default:
      break;
  }
}

void
PuTTYSwitchTextField(PuTextFieldChoice fieldChoice, BOOL onOff)
{
  switch (fieldChoice) {
    case PU_TF_PROMPT:
      PromptOnOff = onOff;
      break;
    case PU_TF_STATUS:
      StatusOnOff = onOff;
      break;
    default:
      break;
  }
}

PuMenubar
PuTTYCreateMenubar(void)
{
  return NULL;
}

PuMenu
PuTTYCreatePulldown(PuMenubar menubar, char *name, char mnemonic)
{
  return NULL;
}

PuMenu
PuTTYCreatePopup(char *name)
{
  return NULL;
}

PuMenu
PuTTYCreatePullright(PuMenu menu, char *name, char mnemonic)
{
  return NULL;
}

PuMenuEntry
PuTTYCreateMenuEntry(PuMenu menu, char *name,
    char mnemonic, char *acc, char *accText,
    PuMenuCB menuCB, void *clientData, PuFreeCB freeCB)
{
  return NULL;
}

void
PuTTYSwitchMenubar(PuMenubar menubar, BOOL onOff)
{
}

void
PuTTYSwitchPopup(PuMenu menu, BOOL onOff)
{
}

PuCmdField
PuTTYCreateCmdField(char *name,
    PuCmdFieldCB cmdFieldCB, void *clientData, PuFreeCB freeCB)
{
  return NULL;
}

void
PuTTYSwitchCmdFields(BOOL onOff)
{
}

PuValuatorBox
PuTTYCreateValuatorBox(char *title)
{
  return NULL;
}

PuValuator
PuTTYCreateValuator(PuValuatorBox box, char *name,
    float minVal, float maxVal, int digits, float initVal,
    PuValuatorCB valuatorCB, void *clientData, PuFreeCB freeCB)
{
  return NULL;
}

void
PuTTYSwitchValuatorBox(PuValuatorBox box, BOOL onOff)
{
}

void
PuTTYSetBusyCursor(BOOL onOff)
{
}

void
PuTTYShowMessage(PuMessageType type, char *title, char *msg)
{
  switch (type) {
    case PU_MT_BUSY:
      (void) fprintf(stderr, "BUSY");
      break;
    case PU_MT_INFO:
      (void) fprintf(stderr, "INFO");
      break;
    case PU_MT_QUESTION:
      (void) fprintf(stderr, "QUESTION");
      break;
    case PU_MT_WARNING:
      (void) fprintf(stderr, "WARNING");
      break;
    case PU_MT_ERROR:
      (void) fprintf(stderr, "ERROR");
      break;
  }

  (void) fprintf(stderr, " (%s): %s\n", title, msg);
}

void
PuTTYShowHelp(PuGizmo parent, char *title, char *fileName)
{
  FILE *fp;
  char ch;
  int ret;

  (void) fprintf(stderr, "HELP: %s\n", title);

  fp = fopen(fileName, "r");
  if (fp == NULL)
    return;
  
  for (;;) {
    ret = fread(&ch, 1, 1, fp);
    if (ret == 0)
      break;
    (void) fprintf(stderr, "%c", ch);
  }

  (void) fclose(fp);
}

void
PuTTYBeep(void)
{
  (void) fprintf(stderr, "\007");
}

PuTextWindow
PuTTYCreateTextWindow(char *title)
{
  return NULL;
}

void
PuTTYWriteStr(PuTextWindow textW, char *str)
{
  (void) fprintf(stdout, "%s", str);
}

static DialData *
createDial(BOOL fileSel)
{
  DialData *dialP;

  dialP = malloc(sizeof(*dialP));
  dialP->okCB = NULL;
  dialP->clientData = NULL;
  dialP->freeCB = NULL;
  dialP->fileSel = fileSel;

  return dialP;
}

PuGizmo
PuTTYCreateFileSelectionBox(char *name,
    char *dir, char *pattern, PuFileAccess acc)
{
  return (PuGizmo) createDial(TRUE);
}

PuGizmo
PuTTYCreateDialog(char *name, int xBase, int yBase)
{
  return (PuGizmo) createDial(FALSE);
}

PuGizmo
PuTTYCreateLabel(PuGizmo dial, char *name)
{
  return NULL;
}

PuGizmo
PuTTYCreateButton(PuGizmo dial, char *name)
{
  return NULL;
}

PuGizmo
PuTTYCreateColorField(PuGizmo dial, char *name)
{
  return NULL;
}

PuGizmo
PuTTYCreateSlider(PuGizmo dial, char *name,
    float minVal, float maxVal, int digits, float initVal)
{
  return NULL;
}

PuGizmo
PuTTYCreateTextField(PuGizmo dial, char *name, char *text)
{
  return NULL;
}

PuGizmo
PuTTYCreateText(PuGizmo dial, char *name, char *text)
{
  return NULL;
}

PuGizmo
PuTTYCreateFileViewer(PuGizmo dial, char *fileName, BOOL readOnly)
{
  return NULL;
}

PuGizmo
PuTTYCreateList(PuGizmo dial, char *name)
{
  return NULL;
}

void
PuTTYAddListEntry(PuGizmo list, char *name, BOOL onOff)
{
}

void
PuTTYRemoveListEntries(PuGizmo list, int pos, int no)
{
}

PuGizmo
PuTTYCreateRadioBox(PuGizmo dial, char *name)
{
  return NULL;
}

PuGizmo
PuTTYCreateCheckBox(PuGizmo dial, char *name)
{
  return NULL;
}

void
PuTTYAddToggle(PuGizmo box, char *name, BOOL onOff)
{
}

void
PuTTYSetBool(PuGizmo gizmo, PuBoolChoice choice, BOOL val)
{
}

void
PuTTYSetInt(PuGizmo gizmo, PuIntChoice choice, int val)
{
}

void
PuTTYSetStr(PuGizmo gizmo, PuStrChoice choice, char *val)
{
}

void
PuTTYSetColor(PuGizmo gizmo, float r, float g, float b)
{
}

void
PuTTYSetConstraints(PuGizmo gizmo, PuConstraints constr)
{
}

void
PuTTYSwitchGizmo(PuGizmo gizmo, BOOL onOff)
{
  DialData *dialP;
  PuFileSelCBStruc fileSelStruc;

  if (onOff && gizmo != NULL) {
    /* immediately call OK callback when a dialog box is opened */
    dialP = (DialData *) gizmo;
    if (dialP->okCB != NULL) {
      if (dialP->fileSel) {
	fileSelStruc.fileName = "";
	dialP->okCB(gizmo, "OK", dialP->clientData, &fileSelStruc);
      } else {
	dialP->okCB(gizmo, "OK", dialP->clientData, NULL);
      }
    }
  }
}

void
PuTTYDestroyGizmo(PuGizmo gizmo)
{
  DialData *dialP;

  if (gizmo != NULL) {
    dialP = (DialData *) gizmo;
    if (dialP->freeCB != NULL)
      dialP->freeCB(dialP->clientData);
    free(dialP);
  }
}

void
PuTTYAddGizmoCB(PuGizmo gizmo, PuGizmoCBType type,
    PuGizmoCB gizmoCB, void *clientData, PuFreeCB freeCB)
{
  DialData *dialP;

  if (gizmo != NULL && type == PU_CT_OK) {
    dialP = (DialData *) gizmo;
    dialP->okCB = gizmoCB;
    dialP->clientData = clientData;
    dialP->freeCB = freeCB;
  }
}
