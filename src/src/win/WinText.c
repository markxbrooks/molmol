/*
************************************************************************
*
*   WinText.c - Windows text utility functions
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
*   Date of last modification : 01/05/27
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/win/SCCS/s.WinText.c
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include "win_text.h"

#include <stdio.h>
#include <stdlib.h>

void
WinTextStrucInit(WinTextStruc *textP)
{
  textP->currText = malloc(1);
  textP->currText[0] = '\0';
  textP->editing = FALSE;
}

static void
getEqNo(PuTextCBStruc *puStrucP, int *startNoP, int *endNoP)
{
  int startNo, endNo;

  startNo = 0;
  while (startNo < puStrucP->oldLen &&
      startNo < puStrucP->newLen &&
      puStrucP->oldText[startNo] ==
      puStrucP->newText[startNo])
    startNo++;

  endNo = 0;
  while (startNo + endNo < puStrucP->oldLen &&
      startNo + endNo < puStrucP->newLen &&
      puStrucP->oldText[puStrucP->oldLen - endNo - 1] ==
      puStrucP->newText[puStrucP->newLen - endNo - 1])
    endNo++;

  *startNoP = startNo;
  *endNoP = endNo;
}

BOOL
TextStrucWinToPu(HWND w, WinTextStruc *textP, PuTextCBStruc *puStrucP)
{
  int eqStartNo, eqEndNo;
  int partLen;

  if (textP->editing)
    /* suppress reentrant calls through SetWindowText() */
    return FALSE;

  textP->editing = TRUE;

  puStrucP->oldText = textP->currText;
  puStrucP->oldLen = strlen(puStrucP->oldText);

  puStrucP->newLen = GetWindowTextLength(w);
  puStrucP->newText = malloc(puStrucP->newLen + 1);
  (void) GetWindowText(w, puStrucP->newText, puStrucP->newLen + 1);

  getEqNo(puStrucP, &eqStartNo, &eqEndNo);

  if (eqStartNo == puStrucP->oldLen) {
    puStrucP->action = PU_CA_APPEND;
    puStrucP->oldPart = NULL;
    puStrucP->newPartStart = eqStartNo;
    puStrucP->newPartEnd = puStrucP->newLen;
    partLen = puStrucP->newLen - eqStartNo;
    puStrucP->newPart = malloc(partLen + 1);
    (void) strcpy(puStrucP->newPart, puStrucP->newText + eqStartNo);
  } else if (eqStartNo + eqEndNo == puStrucP->oldLen) {
    puStrucP->action = PU_CA_INSERT;
    puStrucP->oldPart = NULL;
    puStrucP->newPartStart = eqStartNo;
    puStrucP->newPartEnd = puStrucP->newLen - eqEndNo;
    partLen = puStrucP->newLen - puStrucP->oldLen;
    puStrucP->newPart = malloc(partLen + 1);
    (void) strncpy(puStrucP->newPart, puStrucP->newText + eqStartNo, partLen);
    puStrucP->newPart[partLen] = '\0';
  } else if (eqStartNo + eqEndNo == puStrucP->newLen) {
    puStrucP->action = PU_CA_DELETE;
    puStrucP->newPart = NULL;
    puStrucP->oldPartStart = eqStartNo;
    puStrucP->oldPartEnd = puStrucP->oldLen - eqEndNo;
    partLen = puStrucP->oldLen - puStrucP->newLen;
    puStrucP->oldPart = malloc(partLen + 1);
    (void) strncpy(puStrucP->oldPart,
        puStrucP->oldText + eqStartNo, partLen);
    puStrucP->oldPart[partLen] = '\0';
  } else {
    puStrucP->action = PU_CA_REPLACE;
    puStrucP->oldPartStart = eqStartNo;
    puStrucP->oldPartEnd = puStrucP->oldLen - eqEndNo;
    partLen = puStrucP->oldLen - eqStartNo - eqEndNo;
    puStrucP->oldPart = malloc(partLen + 1);
    (void) strncpy(puStrucP->oldPart,
        puStrucP->oldText + eqStartNo, partLen);
    puStrucP->oldPart[partLen] = '\0';
    puStrucP->newPartStart = eqStartNo;
    puStrucP->newPartEnd = puStrucP->newLen - eqEndNo;
    partLen = puStrucP->newLen - eqStartNo - eqEndNo;
    puStrucP->newPart = malloc(partLen + 1);
    (void) strncpy(puStrucP->newPart,
        puStrucP->newText + eqStartNo, partLen);
    puStrucP->newPart[partLen] = '\0';
  }

  puStrucP->doIt = TRUE;

  return TRUE;
}

void
TextStrucPuToWin(HWND w, PuTextCBStruc *puStrucP, WinTextStruc *textP)
{
  DWORD selStart, selEnd;
  int eqStartNo, eqEndNo, partLen;
  int newCaretPos, newLen;

  SendMessage(w, EM_GETSEL, (WPARAM) &selStart, (WPARAM) &selEnd);
  getEqNo(puStrucP, &eqStartNo, &eqEndNo);
  newCaretPos = selStart;

  if (puStrucP->doIt) {
    if (puStrucP->action == PU_CA_DELETE) {
      puStrucP->newText = realloc(puStrucP->newText,
	  puStrucP->oldLen -
	  (puStrucP->oldPartEnd - puStrucP->oldPartStart) + 1);
      (void) strncpy(puStrucP->newText,
	  puStrucP->oldText, puStrucP->oldPartStart);
      (void) strcpy(puStrucP->newText + puStrucP->oldPartStart,
	  puStrucP->oldText + puStrucP->oldPartEnd);
    } else {
      partLen = strlen(puStrucP->newPart);
      newLen = puStrucP->oldLen + partLen;
      if (puStrucP->oldPart != NULL)
	newLen -= strlen(puStrucP->oldPart);
      if (newLen > puStrucP->newLen)
        newCaretPos = eqStartNo + partLen;
      puStrucP->newText = realloc(puStrucP->newText,
	  eqStartNo + partLen + eqEndNo + 1);
      strcpy(puStrucP->newText + eqStartNo, puStrucP->newPart);
      (void) strcpy(puStrucP->newText + eqStartNo + partLen,
	  puStrucP->oldText + puStrucP->oldLen - eqEndNo);
    }
  } else {
    puStrucP->newText = realloc(puStrucP->newText, puStrucP->oldLen + 1);
    (void) strcpy(puStrucP->newText, puStrucP->oldText);
  }

  SetWindowText(w, puStrucP->newText);
  SendMessage(w, EM_SETSEL, (WPARAM) newCaretPos, (LPARAM) newCaretPos);

  free(puStrucP->oldText);

  textP->currText = puStrucP->newText;

  if (puStrucP->oldPart != NULL)
    free(puStrucP->oldPart);

  if (puStrucP->newPart != NULL)
    free(puStrucP->newPart);

  textP->editing = FALSE;
}
