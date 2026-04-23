/*
************************************************************************
*
*   WinHelp.c - Windows functions for online help
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
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/src/win/SCCS/s.WinHelp.c
*   SCCS identification       : 1.7
*
************************************************************************
*/

#include "win_help.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <windows.h>

#include "win_main_w.h"
#include "win_place.h"
#include "win_event.h"

#define HELP_W 60
#define HELP_H 30
#define CLOSE_W 50
#define CLOSE_H 14

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

static BOOL CALLBACK
helpProc(HWND w, UINT msgKind, WPARAM wParam, LPARAM lParam)
{
  int notifyCode, childId;
  HWND childW;

  switch (msgKind) {
    case WM_COMMAND:
      notifyCode = HIWORD(wParam);
      childId = LOWORD(wParam);
      childW = (HWND) lParam;

      switch (notifyCode) {
	case BN_CLICKED:
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

void
PuWinShowHelp(PuGizmo parent, char *title, char *fileName)
{
  FILE *fp;
  struct stat st;
  char *buf;
  HWND mainW, dialW, textW;
  HINSTANCE instance;
  int dLen, dSize, sLen;
  char *dP;
  LPDLGTEMPLATE dtP;
  LPDLGITEMTEMPLATE itP;
  LPWORD wP;
  LPWSTR sP;

  fp = fopen(fileName, "rb");
  if (fp == NULL)
    return;

  if (fstat(fileno(fp), &st) == -1)
    return;

  buf = malloc(st.st_size + 1);
  (void) fread(buf, 1, st.st_size, fp);
  buf[st.st_size] = '\0';
  (void) fclose(fp);

  mainW = WinGetMainW();
  instance = (HINSTANCE) GetWindowLong(mainW, GWL_HINSTANCE);

  dSize = 1024;
  dP = malloc(dSize);
  dLen = 0;

  checkSize(&dP, &dSize, dLen + sizeof(*dtP));
  dtP = (LPDLGTEMPLATE) dP;
  dtP->style = WS_POPUP | WS_CAPTION | WS_SYSMENU |
      DS_FIXEDSYS;
  dtP->dwExtendedStyle = 0;
  dtP->cdit = 2;
  dtP->x = 10;
  dtP->y = 10;
  dtP->cx = (HELP_W + 3) * 4;
  dtP->cy = HELP_H * 8 + CLOSE_H;
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
  itP->cy = dtP->cy - CLOSE_H;
  itP->id = 1;
  itP->style = WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL |
      ES_MULTILINE | ES_READONLY;
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
  itP->x = (dtP->cx - CLOSE_W) / 2;
  itP->y = dtP->cy - CLOSE_H;
  itP->cx = CLOSE_W;
  itP->cy = CLOSE_H;
  itP->id = 2;
  itP->style = WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON;
  itP->dwExtendedStyle = 0;
  dLen += sizeof(*itP);

  checkSize(&dP, &dSize, dLen + 2 * sizeof(*wP));
  wP = (LPWORD) (dP + dLen);
  wP[0] = 0xFFFF;
  wP[1] = 0x0080;  /* button */
  dLen += 2 * sizeof(*wP);

  checkSize(&dP, &dSize, dLen + 6 * sizeof(*sP));
  sP = (LPWSTR) (dP + dLen);
  (void) MultiByteToWideChar(CP_ACP, 0, "Close", 6, sP, 6);
  dLen += 6 * sizeof(*sP);

  checkSize(&dP, &dSize, dLen + sizeof(*wP));
  wP = (LPWORD) (dP + dLen);
  wP[0] = 0;  /* no creation data */
  dLen += 1 * sizeof(*wP);

  dtP = (LPDLGTEMPLATE) dP;
  dialW = CreateDialogIndirect(instance, dtP, mainW, helpProc);
  WinAddDialog(dialW);

  textW = GetDlgItem(dialW, 1);
  SetWindowText(textW, buf);

  WinPlace(dialW, FALSE);
  ShowWindow(dialW, SW_SHOW);
  SendMessage(textW, EM_SETSEL, (WPARAM) -1, -1);

  free(dP);
  free(buf);
}
