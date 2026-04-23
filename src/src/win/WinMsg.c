/*
************************************************************************
*
*   WinMsg.c - Windows message dialogs
*
*   Copyright (c) 1996-97
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
*   Date of last modification : 97/02/17
*   Pathname of SCCS file     : /local/home/kor/molmol/src/win/SCCS/s.WinMsg.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include "win_msg.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "win_main_w.h"

void
PuWinShowMessage(PuMessageType type, char *title, char *msg)
{
  HWND mainW;
  UINT dialogType;
  char *msgType, *buf;

  mainW = WinGetMainW();

  switch (type) {
    case PU_MT_BUSY:
      dialogType = 0;
      msgType = "WORKING";
      break;
    case PU_MT_INFO:
      dialogType = MB_ICONINFORMATION;
      msgType = "INFO";
      break;
    case PU_MT_QUESTION:
      dialogType = MB_ICONQUESTION;
      msgType = "QUESTION";
      break;
    case PU_MT_WARNING:
      dialogType = MB_ICONWARNING;
      msgType = "WARNING";
      break;
    case PU_MT_ERROR:
      dialogType = MB_ICONERROR;
      msgType = "ERROR";
      break;
  }

  buf = malloc(strlen(msgType) + strlen(title) + 3);
  (void) sprintf(buf, "%s: %s", msgType, title);

  (void) MessageBox(mainW, msg, buf, MB_OK | dialogType);

  free(buf);

  WinSetFocus();
}

void
PuWinBeep(void)
{
  (void) MessageBeep(0xFFFFFFFF);
}
