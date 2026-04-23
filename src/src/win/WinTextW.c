/*
************************************************************************
*
*   WinTextW.c - Windows text windows
*
*   Copyright (c) 1996-99
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
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/src/win/SCCS/s.WinTextW.c
*   SCCS identification       : 1.6
*
************************************************************************
*/

#include "win_text_w.h"

#include <stdio.h>
#include <string.h>
#include <windows.h>

#include "win_main_w.h"
#include "win_place.h"
#include "win_event.h"

#define FIELD_H 12
#define TEXT_W 60
#define TEXT_H 30
#define BUTTON_H 14

static void
checkSize(char **dPP, int *dSizeP, int dLen)
{
  if (dLen <= *dSizeP)
    return;

  while (dLen > *dSizeP)
    *dSizeP *= 2;

  *dPP = realloc(*dPP, *dSizeP);
}

static void
alignLen(int *dLenP)
{
  *dLenP = ((*dLenP + 3) / 4) * 4;
}

static void
addButton(char **dPP, int *dSizeP, int *dLenP,
    char *label, int ind, int no, int width, int height)
{
  char *dP;
  int dLen, dSize, sLen;
  LPDLGITEMTEMPLATE itP;
  LPWORD wP;
  LPWSTR sP;

  dP = *dPP;
  dLen = *dLenP;
  dSize = *dSizeP;

  alignLen(&dLen);
  checkSize(&dP, &dSize, dLen + sizeof(*itP));
  itP = (LPDLGITEMTEMPLATE) (dP + dLen);
  itP->x = 2 * (ind - 1) * width / (2 * no - 1);
  itP->y = height - BUTTON_H;
  itP->cx = width / (2 * no - 1);
  itP->cy = BUTTON_H;
  itP->id = ind + 2;
  itP->style = WS_CHILD | WS_VISIBLE;
  itP->dwExtendedStyle = 0;
  dLen += sizeof(*itP);

  checkSize(&dP, &dSize, dLen + 2 * sizeof(*wP));
  wP = (LPWORD) (dP + dLen);
  wP[0] = 0xFFFF;
  wP[1] = 0x0080;  /* button */
  dLen += 2 * sizeof(*wP);

  sLen = strlen(label) + 1;
  checkSize(&dP, &dSize, dLen + sLen * sizeof(*sP));
  sP = (LPWSTR) (dP + dLen);
  (void) MultiByteToWideChar(CP_ACP, 0, label, sLen, sP, sLen);
  dLen += sLen * sizeof(*sP);

  checkSize(&dP, &dSize, dLen + sizeof(*wP));
  wP = (LPWORD) (dP + dLen);
  wP[0] = 0;  /* no creation data */
  dLen += 1 * sizeof(*wP);

  *dPP = dP;
  *dLenP = dLen;
  *dSizeP = dSize;
}

static void
saveText(char *fileName, char *text, char *mode)
{
  FILE *fp;

  fp = fopen(fileName, mode);
  if (fp == NULL)
    return;

  if (mode[0] == 'a')
    (void) fprintf(fp, "\n\n");

  (void) fwrite(text, 1, strlen(text), fp);

  (void) fclose(fp);
}

static BOOL CALLBACK
textProc(HWND w, UINT msgKind, WPARAM wParam, LPARAM lParam)
{
  int notifyCode, childId;
  HWND childW, nameW, textW;
  int len;
  char *fileName, *buf;

  switch (msgKind) {
    case WM_COMMAND:
      notifyCode = HIWORD(wParam);
      childId = LOWORD(wParam);
      childW = (HWND) lParam;

      switch (notifyCode) {
	case BN_CLICKED:
	  if (childId == 4) {
	    textW = GetDlgItem(w, 2);
	    SendMessage(textW, EM_SETREADONLY, FALSE, 0);
	    return TRUE;
	  }

	  if (childId == 5 || childId == 6) {
	    nameW = GetDlgItem(w, 1);
	    len = GetWindowTextLength(nameW);
	    fileName = malloc(len + 1);
	    (void) GetWindowText(nameW, fileName, len + 1);
	    textW = GetDlgItem(w, 2);
	    len = GetWindowTextLength(textW);
	    buf = malloc(len + 1);
	    (void) GetWindowText(textW, buf, len + 1);
	    if (childId == 5)
	      saveText(fileName, buf, "w");
	    else
	      saveText(fileName, buf, "a");
	    free(fileName);
	    free(buf);
	    return TRUE;
	  }

          DestroyWindow(w);
          WinRemoveDialog(w);
          return TRUE;
      }

      break;
    case WM_CLOSE:
      DestroyWindow(w);
      WinRemoveDialog(w);
      return TRUE;
  }

  return FALSE;
}

PuTextWindow
PuWinCreateTextWindow(char *title)
{
  HWND mainW, dialW, nameW;
  HINSTANCE instance;
  int dLen, dSize, sLen;
  char *dP;
  LPDLGTEMPLATE dtP;
  LPDLGITEMTEMPLATE itP;
  LPWORD wP;
  LPWSTR sP;

  mainW = WinGetMainW();
  instance = (HINSTANCE) GetWindowLong(mainW, GWL_HINSTANCE);

  dSize = 1024;
  dP = malloc(dSize);
  dLen = 0;

  checkSize(&dP, &dSize, dLen + sizeof(*dtP));
  dtP = (LPDLGTEMPLATE) (dP + dLen);
  dtP->cdit = 6;
  dtP->x = 10;
  dtP->y = 10;
  dtP->cx = (TEXT_W + 3) * 4;
  dtP->cy = FIELD_H + TEXT_H * 8 + BUTTON_H;
  dtP->style = WS_POPUP | WS_CAPTION | WS_SYSMENU |
      DS_FIXEDSYS;
  dtP->dwExtendedStyle = 0;
  dLen += sizeof(*dtP);

  checkSize(&dP, &dSize, dLen + 2 * sizeof(*wP));
  wP = (LPWORD) (dP + dLen);
  wP[0] = 0;  /* no menu */
  wP[1] = 0;  /* default class */
  dLen += 2 * sizeof(*wP);

  sLen = strlen(title) + 1;
  checkSize(&dP, &dSize, dLen + sLen * sizeof(*sP));
  sP = (LPWSTR) (dP + dLen);
  (void) MultiByteToWideChar(CP_ACP, 0, title, sLen, sP, sLen);
  dLen += sLen * sizeof(*sP);

  alignLen(&dLen);
  checkSize(&dP, &dSize, dLen + sizeof(*itP));
  dtP = (LPDLGTEMPLATE) dP;
  itP = (LPDLGITEMTEMPLATE) (dP + dLen);
  itP->x = 0;
  itP->y = 0;
  itP->cx = dtP->cx;
  itP->cy = FIELD_H;
  itP->id = 1;
  itP->style = WS_CHILD | WS_VISIBLE;
  itP->dwExtendedStyle = 0;
  dLen += sizeof(*itP);

  checkSize(&dP, &dSize, dLen + 3 * sizeof(*wP));
  wP = (LPWORD) (dP + dLen);
  wP[0] = 0xFFFF;
  wP[1] = 0x0081;  /* edit */
  wP[2] = 0;  /* set text later */
  dLen += 3 * sizeof(*wP);

  checkSize(&dP, &dSize, dLen + sizeof(*wP));
  wP = (LPWORD) (dP + dLen);
  wP[0] = 0;  /* no creation data */
  dLen += 1 * sizeof(*wP);

  alignLen(&dLen);
  checkSize(&dP, &dSize, dLen + sizeof(*itP));
  dtP = (LPDLGTEMPLATE) dP;
  itP = (LPDLGITEMTEMPLATE) (dP + dLen);
  itP->x = 0;
  itP->y = FIELD_H;
  itP->cx = dtP->cx;
  itP->cy = dtP->cy - FIELD_H - BUTTON_H;
  itP->id = 2;
  itP->style = WS_CHILD | WS_HSCROLL | WS_VSCROLL | WS_VISIBLE |
      ES_MULTILINE | ES_WANTRETURN | ES_READONLY;
  itP->dwExtendedStyle = 0;
  dLen += sizeof(*itP);

  checkSize(&dP, &dSize, dLen + 3 * sizeof(*wP));
  wP = (LPWORD) (dP + dLen);
  wP[0] = 0xFFFF;
  wP[1] = 0x0081;  /* edit */
  wP[2] = 0;  /* set text later */
  dLen += 3 * sizeof(*wP);

  checkSize(&dP, &dSize, dLen + sizeof(*wP));
  wP = (LPWORD) (dP + dLen);
  wP[0] = 0;  /* no creation data */
  dLen += 1 * sizeof(*wP);

  dtP = (LPDLGTEMPLATE) dP;
  addButton(&dP, &dSize, &dLen, "Close", 1, 4, dtP->cx, dtP->cy);
  addButton(&dP, &dSize, &dLen, "Edit", 2, 4, dtP->cx, dtP->cy);
  addButton(&dP, &dSize, &dLen, "Save", 3, 4, dtP->cx, dtP->cy);
  addButton(&dP, &dSize, &dLen, "Append", 4, 4, dtP->cx, dtP->cy);

  dtP = (LPDLGTEMPLATE) dP;
  dialW = CreateDialogIndirect(instance, dtP, mainW, textProc);
  WinAddDialog(dialW);

  nameW = GetDlgItem(dialW, 1);
  SetWindowText(nameW, title);

  WinPlace(dialW, FALSE);
  ShowWindow(dialW, SW_SHOW);
  UpdateWindow(dialW);

  free(dP);

  return dialW;
}

void
PuWinWriteStr(PuTextWindow dialW, char *str)
{
  HWND textW;
  int len, i;
  char *buf, *bP;

  textW = GetDlgItem(dialW, 2);

  len = strlen(str);
  buf = malloc(2 * len + 1);

  /* replace CR and LF by CR-LF */
  bP = buf;
  for (i = 0; i < len; i++) {
    if (str[i] == 13 && str[i + 1] != 10) {
      *bP = 13;
      bP++;
      *bP = 10;
    } else if (str[i] == 10) {
      *bP = 13;
      bP++;
      *bP = 10;
    } else {
      *bP = str[i];
    }
    bP++;
  }
  *bP = '\0';

  len = GetWindowTextLength(textW) + strlen(buf);
  SendMessage(textW, EM_LIMITTEXT, (WPARAM) len, 0);
  SendMessage(textW, EM_REPLACESEL, 0, (LPARAM) buf);

  free(buf);
}
