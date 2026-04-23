/*
************************************************************************
*
*   MotifTextW.c - Motif text windows
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
*   Date of last modification : 99/10/17
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/src/motif/SCCS/s.MotifTextW.c
*   SCCS identification       : 1.8
*
************************************************************************
*/

#include "motif_text_w.h"

#include <stdio.h>
#include <string.h>

#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/PanedW.h>
#include <Xm/Label.h>
#include <Xm/Text.h>
#include <Xm/PushB.h>

#include "motif_access.h"
#include "motif_busy.h"
#include "motif_main_w.h"

static void
destroyDialogCB(Widget buttonW, XtPointer clientData, XtPointer callData)
{
  Widget dialogW = clientData;

  XtDestroyWidget(dialogW);
}

static void
editCB(Widget buttonW, XtPointer clientData, XtPointer callData)
{
  Widget textW = clientData;

  XtVaSetValues(textW,
      XmNeditable, True,
      XmNcursorPositionVisible, True,
      NULL);
}

static void
saveText(Widget buttonW, Widget fileNameW, char *mode)
{
  Widget textW;
  char *fileName, *text;
  FILE *fp;

  XtVaGetValues(buttonW,
      XmNuserData, &textW,
      NULL);

  fileName = XmTextGetString(fileNameW);
  fp = fopen(fileName, mode);
  XtFree(fileName);
  if (fp == NULL)
    return;

  if (mode[0] == 'a')
    (void) fprintf(fp, "\n\n");

  text = XmTextGetString(textW);
  (void) fwrite(text, 1, strlen(text), fp);
  XtFree(text);
  (void) fclose(fp);
}

static void
saveCB(Widget buttonW, XtPointer clientData, XtPointer callData)
{
  saveText(buttonW, clientData, "w");
}

static void
appendCB(Widget buttonW, XtPointer clientData, XtPointer callData)
{
  saveText(buttonW, clientData, "a");
}

PuTextWindow
PuMotifCreateTextWindow(char *title)
{
  Widget dialogW, dummyW, paneW;
  Widget nameFormW, labelW, fileNameW;
  Widget textW;
  Widget buttonFormW, okW, editW, saveW, appendW;
  Arg args[10];

  dialogW = XmCreateDialogShell(MotifGetTopW(), "TextWin", NULL, 0);
  XtVaSetValues(dialogW,
      XmNtitle, title,
      XmNdeleteResponse, XmDESTROY,
      NULL);

  /* Dialogs don't get properly placed when a PanedWindow is the
     child of the DialogShell. Put a Form in between. */
  dummyW = XmCreateForm(dialogW, "DialogForm", NULL, 0);

  paneW = XmCreatePanedWindow(dummyW, "PanedWindow", NULL, 0);
  XtVaSetValues(paneW,
      XmNsashWidth, 1,
      XmNsashHeight, 1,
      XmNleftAttachment, XmATTACH_FORM,
      XmNrightAttachment, XmATTACH_FORM,
      XmNtopAttachment, XmATTACH_FORM,
      XmNbottomAttachment, XmATTACH_FORM,
      NULL);

  nameFormW = XmCreateForm(paneW, "Form", NULL, 0);
  XtVaSetValues(nameFormW,
      XmNskipAdjust, True,
      NULL);
  
  labelW = XmCreateLabel(nameFormW, "File Name", NULL, 0);
  XtVaSetValues(labelW,
      XmNleftAttachment, XmATTACH_FORM,
      XmNtopAttachment, XmATTACH_FORM,
      XmNbottomAttachment, XmATTACH_FORM,
      NULL);

  fileNameW = XmCreateText(nameFormW, "FileName", NULL, 0);
  XtVaSetValues(fileNameW,
      XmNleftAttachment, XmATTACH_WIDGET,
      XmNleftWidget, labelW,
      XmNrightAttachment, XmATTACH_FORM,
      XmNtopAttachment, XmATTACH_FORM,
      XmNbottomAttachment, XmATTACH_FORM,
      NULL);
  XmTextSetString(fileNameW, title);

  XtSetArg(args[0], XmNscrollVertical, True);
  XtSetArg(args[1], XmNscrollHorizontal, True);
  XtSetArg(args[2], XmNeditMode, XmMULTI_LINE_EDIT);
  XtSetArg(args[3], XmNeditable, False);
  XtSetArg(args[4], XmNcursorPositionVisible, False);  /* Motif 1.1 */
  XtSetArg(args[5], XmNautoShowCursorPosition, False);  /* Motif 1.2 */
  XtSetArg(args[6], XmNcolumns, 60);
  XtSetArg(args[7], XmNrows, 30);
  XtSetArg(args[8], XmNvalue, "");
  textW = XmCreateScrolledText(paneW, "Text", args, 9);

  buttonFormW = XmCreateForm(paneW, "Form", NULL, 0);
  XtVaSetValues(buttonFormW,
      XmNfractionBase, 7,
      XmNskipAdjust, True,
      NULL);
  
  okW = XmCreatePushButton(buttonFormW, "Close", NULL, 0);
  XtVaSetValues(okW,
      XmNleftAttachment, XmATTACH_POSITION,
      XmNleftPosition, 0,
      XmNrightAttachment, XmATTACH_POSITION,
      XmNrightPosition, 1,
      XmNtopAttachment, XmATTACH_FORM,
      XmNbottomAttachment, XmATTACH_FORM,
      NULL);
  XtAddCallback(okW, XmNactivateCallback, destroyDialogCB, dialogW);

  editW = XmCreatePushButton(buttonFormW, "Edit", NULL, 0);
  XtVaSetValues(editW,
      XmNleftAttachment, XmATTACH_POSITION,
      XmNleftPosition, 2,
      XmNrightAttachment, XmATTACH_POSITION,
      XmNrightPosition, 3,
      XmNtopAttachment, XmATTACH_FORM,
      XmNbottomAttachment, XmATTACH_FORM,
      NULL);
  XtAddCallback(editW, XmNactivateCallback, editCB, textW);

  saveW = XmCreatePushButton(buttonFormW, "Save", NULL, 0);
  XtVaSetValues(saveW,
      XmNleftAttachment, XmATTACH_POSITION,
      XmNleftPosition, 4,
      XmNrightAttachment, XmATTACH_POSITION,
      XmNrightPosition, 5,
      XmNtopAttachment, XmATTACH_FORM,
      XmNbottomAttachment, XmATTACH_FORM,
      NULL);
  XtVaSetValues(saveW,
      XmNuserData, textW,  /* used for saveCB */
      NULL);
  XtAddCallback(saveW, XmNactivateCallback, saveCB, fileNameW);

  appendW = XmCreatePushButton(buttonFormW, "Append", NULL, 0);
  XtVaSetValues(appendW,
      XmNleftAttachment, XmATTACH_POSITION,
      XmNleftPosition, 6,
      XmNrightAttachment, XmATTACH_POSITION,
      XmNrightPosition, 7,
      XmNtopAttachment, XmATTACH_FORM,
      XmNbottomAttachment, XmATTACH_FORM,
      NULL);
  XtVaSetValues(appendW,
      XmNuserData, textW,  /* used for appendCB */
      NULL);
  XtAddCallback(appendW, XmNactivateCallback, appendCB, fileNameW);

  XtManageChild(labelW);
  XtManageChild(fileNameW);
  XtManageChild(nameFormW);
  XtManageChild(textW);
  XtManageChild(okW);
  XtManageChild(editW);
  XtManageChild(saveW);
  XtManageChild(appendW);
  XtManageChild(buttonFormW);
  XtManageChild(paneW);
  XtManageChild(dummyW);

  XmUpdateDisplay(dialogW);
  MotifAddDialog(dialogW);

  return textW;
}

void
PuMotifWriteStr(PuTextWindow textW, char *str)
{
  XmTextInsert(textW, XmTextGetLastPosition(textW), str);
}
