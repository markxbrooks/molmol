/*
************************************************************************
*
*   MotifHelp.c - Motif functions for online help
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
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/src/motif/SCCS/s.MotifHelp.c
*   SCCS identification       : 1.5
*
************************************************************************
*/

#include "motif_help.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/PanedW.h>
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

void
PuMotifShowHelp(PuGizmo parent, char *title, char *fileName)
{
  FILE *fp;
  struct stat st;
  char *buf;
  Widget dialogW, dummyW, paneW, textW, formW, okW;
  Arg args[10];

  fp = fopen(fileName, "r");
  if (fp == NULL)
    return;

  if (fstat(fileno(fp), &st) == -1)
    return;

  buf = malloc(st.st_size + 1);
  (void) fread(buf, 1, st.st_size, fp);
  buf[st.st_size] = '\0';
  (void) fclose(fp);

  if (parent == NULL)
    parent = MotifGetTopW();

  dialogW = XmCreateDialogShell(parent, "HelpWin", NULL, 0);
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

  XtSetArg(args[0], XmNscrollVertical, True);
  XtSetArg(args[1], XmNscrollHorizontal, False);
  XtSetArg(args[2], XmNeditMode, XmMULTI_LINE_EDIT);
  XtSetArg(args[3], XmNeditable, False);
  XtSetArg(args[4], XmNcursorPositionVisible, False);
  XtSetArg(args[5], XmNcolumns, 60);
  XtSetArg(args[6], XmNrows, 30);
  XtSetArg(args[7], XmNvalue, buf);
  textW = XmCreateScrolledText(paneW, "Text", args, 8);

  free(buf);

  formW = XmCreateForm(paneW, "Form", NULL, 0);
  XtVaSetValues(formW,
      XmNfractionBase, 5,
      XmNskipAdjust, True,
      NULL);
  
  okW = XmCreatePushButton(formW, "Close", NULL, 0);
  XtVaSetValues(okW,
      XmNleftAttachment, XmATTACH_POSITION,
      XmNleftPosition, 2,
      XmNrightAttachment, XmATTACH_POSITION,
      XmNrightPosition, 3,
      XmNtopAttachment, XmATTACH_FORM,
      XmNbottomAttachment, XmATTACH_FORM,
      NULL);
  XtAddCallback(okW, XmNactivateCallback, destroyDialogCB, dialogW);

  XtManageChild(textW);
  XtManageChild(okW);
  XtManageChild(formW);
  XtManageChild(paneW);
  XtManageChild(dummyW);

  MotifAddDialog(dialogW);
}
