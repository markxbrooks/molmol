/*
************************************************************************
*
*   MotifMsg.c - Motif message dialogs
*
*   Copyright (c) 1994-96
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
*   Date of last modification : 96/11/29
*   Pathname of SCCS file     : /local/home/kor/molmol/src/motif/SCCS/s.MotifMsg.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include "motif_msg.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Xm/MessageB.h>

#include "motif_access.h"

void
PuMotifShowMessage(PuMessageType type, char *title, char *msg)
{
  Widget dialogW;
  int dialogType;
  char *msgType, *buf;
  XmString titleStr, msgStr;

  switch (type) {
    case PU_MT_BUSY:
      dialogType = XmDIALOG_WORKING;
      msgType = "WORKING";
      break;
    case PU_MT_INFO:
      dialogType = XmDIALOG_INFORMATION;
      msgType = "INFO";
      break;
    case PU_MT_QUESTION:
      dialogType = XmDIALOG_QUESTION;
      msgType = "QUESTION";
      break;
    case PU_MT_WARNING:
      dialogType = XmDIALOG_WARNING;
      msgType = "WARNING";
      break;
    case PU_MT_ERROR:
      dialogType = XmDIALOG_ERROR;
      msgType = "ERROR";
      break;
  }


  dialogW = XmCreateMessageDialog(MotifGetTopW(), "message", NULL, 0);

  buf = malloc(strlen(msgType) + strlen(title) + 3);
  (void) sprintf(buf, "%s: %s", msgType, title);
  titleStr = XmStringCreateSimple(buf);

  msgStr = XmStringCreateLtoR(msg, XmSTRING_DEFAULT_CHARSET);

  XtVaSetValues(dialogW,
      XmNdialogType, dialogType,
      XmNdialogTitle, titleStr,
      XmNmessageString, msgStr,
      XmNdeleteResponse, XmDESTROY,
      NULL);

  XmStringFree(titleStr);
  XmStringFree(msgStr);

  XtUnmanageChild(
      XmMessageBoxGetChild(dialogW, XmDIALOG_CANCEL_BUTTON));
  XtSetSensitive(
      XmMessageBoxGetChild(dialogW, XmDIALOG_HELP_BUTTON), False);

  XtManageChild(dialogW);
}

void
PuMotifBeep(void)
{
  Display *dpy = XtDisplay(MotifGetTopW());

  XBell(dpy, 100);
}
