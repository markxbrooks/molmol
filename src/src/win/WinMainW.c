/*
************************************************************************
*
*   WinMainW.c - Windows main window
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
*   Date of last modification : 01/05/27
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/win/SCCS/s.WinMainW.c
*   SCCS identification       : 1.10
*
************************************************************************
*/

#include "win_main_w.h"

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#endif
#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
#endif

#include <break.h>
#include <linlist.h>
#include <pu_cb.h>
#include "win_menu.h"
#include "win_text.h"

#define DRAW_ID         1
#define CMD_FIELDS_ID   2
#define PROMPT_ID       3
#define CMD_ID          4
#define STATUS_ID       5
#define BREAK_ID        6
#define CMD_FIELD_BASE 50

#define HISTORY_SIZE 100

typedef struct {
  char *name;
  PuCmdFieldCB cmdFieldCB;
  void *clientData;
} CmdFieldInfo;

static HINSTANCE Instance;
static WNDPROC OldCmdProc;
static WNDPROC OldFieldProc;
static HWND MainW;
static HWND DrawW, PromptW, CmdW, StatusW, BreakW;
static BOOL PromptOnOff, CmdOnOff, StatusOnOff, CmdFieldsOnOff;
static int CmdFieldNo = 0, CmdFieldLen = 0;

static BOOL BusyOn = FALSE;
static HCURSOR CursorNormal = NULL, CursorBusy = NULL;

static LINLIST HistoryList = NULL;
static char **HistoryCurr;

static void
activateBreak(BOOL onOff)
{
  if (BreakW == NULL)
    return;

  EnableWindow(BreakW, onOff);
}

static BOOL
checkBreak(void)
{
  MSG msg;

  if (BreakW == NULL)
    return FALSE;

  if (PeekMessage(&msg, BreakW, 0, 0, PM_REMOVE)) {
    if (msg.message == WM_LBUTTONUP)
      return TRUE;

    DispatchMessage(&msg);
  }

  return FALSE;
}

static void
registerBreak(void)
{
  BreakSetActivate(activateBreak);
  BreakSetCheck(checkBreak);
}

void
PuWinSetBusyCursor(BOOL onOff)
{
  BusyOn = onOff;
  (void) SetCursor(WinGetCursor());
}

HCURSOR
WinGetCursor(void)
{
  if (BusyOn)
    return CursorBusy;
  else
    return CursorNormal;
}

static void
makeLayout(void)
{
  RECT rect;
  LONG mainW, mainH, w, h, y0, y1;
  int charW, charH;
  HDWP wp;
  UINT showFlags, hideFlags, flags;
  int i;

  GetClientRect(MainW, &rect);
  mainW = rect.right;
  mainH = rect.bottom;

  charW = GET_X_LPARAM(GetDialogBaseUnits());
  charH = GET_Y_LPARAM(GetDialogBaseUnits());

  showFlags = SWP_SHOWWINDOW;
  hideFlags = SWP_HIDEWINDOW | SWP_NOMOVE | SWP_NOSIZE;

  wp = BeginDeferWindowPos(50);

  if (StatusOnOff) {
    flags = showFlags;
    h = charH * 3 / 2;
  } else if (StatusW != NULL) {
    flags = hideFlags;
    h = 0;
  }

  if (StatusW != NULL) {
    w = 7 * charW;
    wp = DeferWindowPos(wp, StatusW, HWND_TOP,
        0, mainH - h, mainW - w, h, flags);
    wp = DeferWindowPos(wp, BreakW, HWND_TOP,
        mainW - w, mainH - h, w, h, flags);
    mainH -= h;
  }

  if (CmdOnOff) {
    h = charH * 3 / 2;

    if (PromptOnOff) {
      flags = showFlags;
      w = charW * (GetWindowTextLength(PromptW) + 1);
    } else if (PromptW != NULL) {
      flags = hideFlags;
      w = 0;
    }

    if (PromptW != NULL)
      wp = DeferWindowPos(wp, PromptW, HWND_TOP,
          0, mainH - h, w, h, flags);
    else
      w = 0;

    flags = showFlags;
  } else if (CmdW != NULL) {
    flags = hideFlags;
    w = 0;
    h = 0;
  }
    
  if (CmdW != NULL) {
    wp = DeferWindowPos(wp, CmdW, HWND_TOP,
        w, mainH - h, mainW - w, h, flags);
    mainH -= h;
  }

  if (CmdFieldsOnOff) {
    flags = showFlags;
    w = charW * CmdFieldLen;
  } else {
    flags = hideFlags;
    w = 0;
  }

  y0 = 0;
  for (i = 0; i < CmdFieldNo; i++) {
    y1 = (i + 1) * mainH / CmdFieldNo;
    wp = DeferWindowPos(wp,
        GetDlgItem(MainW, CMD_FIELD_BASE + i), HWND_TOP,
        mainW - w, y0, w, y1 - y0, flags);
    y0 = y1;
  }

  wp = DeferWindowPos(wp, DrawW, HWND_TOP,
      0, 0, mainW - w, mainH, SWP_SHOWWINDOW);
 
  (void) EndDeferWindowPos(wp);
}

void
WinCreateMainW(char *title, HINSTANCE hInstance, int nCmdShow)
{
  CursorNormal = LoadCursor(NULL, IDC_ARROW);
  CursorBusy = LoadCursor(NULL, IDC_WAIT);

  MainW = CreateWindow("main", title,
      WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
      NULL, NULL, hInstance, NULL);

  DrawW = CreateWindow("draw", NULL,
      WS_CHILDWINDOW | WS_VISIBLE | WS_BORDER, 0, 0, 0, 0,
      MainW, (HMENU) DRAW_ID, hInstance, NULL);

  ShowWindow(MainW, nCmdShow);
  UpdateWindow(MainW);

  Instance = hInstance;

  DragAcceptFiles(MainW, TRUE);
}

HWND
WinGetMainW(void)
{
  return MainW;
}

HWND
WinGetDrawW(void)
{
  return DrawW;
}

void
PuWinSwitchFullscreen(BOOL onOff)
{
}

void
PuWinSetDrawSize(int w, int h)
{
  RECT rect;
  int dw, dh;

  GetClientRect(DrawW, &rect);

  dw = w - rect.right;
  dh = h - rect.bottom;

  GetWindowRect(MainW, &rect);
  MoveWindow(MainW, rect.left, rect.top,
      (rect.right - rect.left) + dw,
      (rect.bottom - rect.top) + dh, TRUE);
  makeLayout();
}

void
PuWinSetTextField(PuTextFieldChoice fieldChoice, char *text)
{
  switch (fieldChoice) {
    case PU_TF_TITLE:
      SetWindowText(MainW, text);
      break;
    case PU_TF_PROMPT:
      if (PromptW != NULL) {
	SetWindowText(PromptW, text);
	makeLayout();
      }
      break;
    case PU_TF_CMD:
      /* cannot be set */
      break;
    case PU_TF_STATUS:
      if (StatusW != NULL)
	SetWindowText(StatusW, text);
      break;
  }
}

static void
addHistory(char *cmd)
{
  char *str, **strP;

  if (HistoryList == NULL)
    HistoryList = ListOpen(sizeof(char *));

  str = malloc(strlen(cmd) + 1);
  (void) strcpy(str, cmd);
  (void) ListInsertLast(HistoryList, &str);
  if (ListSize(HistoryList) > HISTORY_SIZE) {
    strP = ListFirst(HistoryList);
    free(*strP);
    ListRemove(HistoryList, strP);
  }

  HistoryCurr = NULL;
}

static void
setCmd(char *newCmd)
{
  int len;
  if (newCmd[0] != '\0')
    SetWindowText(CmdW, "");

  SetWindowText(CmdW, newCmd);
  len = strlen(newCmd);
  SendMessage(CmdW, EM_SETSEL, (WPARAM) len, (LPARAM) len);
}

static void
upHistory(void)
{
  if (HistoryCurr == NULL)
    HistoryCurr = ListLast(HistoryList);
  else
    HistoryCurr = ListPrev(HistoryList, HistoryCurr);

  if (HistoryCurr == NULL)
    setCmd("");
  else
    setCmd(*HistoryCurr);
}

static void
downHistory(void)
{
  if (HistoryCurr == NULL)
    HistoryCurr = ListFirst(HistoryList);
  else
    HistoryCurr = ListNext(HistoryList, HistoryCurr);

  if (HistoryCurr == NULL)
    setCmd("");
  else
    setCmd(*HistoryCurr);
}

static LRESULT CALLBACK
cmdProc(HWND w, UINT msgKind, WPARAM wParam, LPARAM lParam)
{
  PuTextCBStruc textStruc;

  switch (msgKind) {
    case WM_KEYUP:
      if (wParam == VK_RETURN) {
	textStruc.action = PU_CA_CR;
	textStruc.oldLen = GetWindowTextLength(CmdW);
	textStruc.oldText = malloc(textStruc.oldLen + 1);
	(void) GetWindowText(CmdW, textStruc.oldText, textStruc.oldLen + 1);
        textStruc.oldPart = NULL;
        textStruc.oldPartStart = 0;
        textStruc.oldPartEnd = 0;
        textStruc.newLen = textStruc.oldLen;
        textStruc.newText = malloc(textStruc.newLen + 1);
	(void) strcpy(textStruc.newText, textStruc.oldText);
        textStruc.newPart = NULL;
        textStruc.newPartStart = 0;
        textStruc.newPartEnd = 0;
        textStruc.doIt = TRUE;
        SetWindowText(CmdW, "");
	PuCallCmdCB((PuWindow) DrawW, &textStruc);
        if (textStruc.doIt && textStruc.newLen > 0)
	  addHistory(textStruc.newText);
	free(textStruc.oldText);
        free(textStruc.newText);
      } else if (wParam == VK_UP) {
	upHistory();
      } else if (wParam == VK_DOWN) {
	downHistory();
      }
      break;
  }

  return CallWindowProc(OldCmdProc, w, msgKind, wParam, lParam);
}

void
PuWinSwitchTextField(PuTextFieldChoice fieldChoice, BOOL onOff)
{
  switch (fieldChoice) {
    case PU_TF_TITLE:
      /* always on */
      break;
    case PU_TF_PROMPT:
      PromptOnOff = onOff;
      if (onOff && PromptW == NULL)
	PromptW = CreateWindow("static", NULL,
	    WS_CHILDWINDOW, 0, 0, 0, 0,
	    MainW, (HMENU) PROMPT_ID, Instance, NULL);
      makeLayout();
      break;
    case PU_TF_CMD:
      CmdOnOff = onOff;
      if (onOff && CmdW == NULL) {
	CmdW = CreateWindow("edit", NULL,
	    WS_CHILDWINDOW | WS_BORDER | ES_AUTOHSCROLL, 0, 0, 0, 0,
	    MainW, (HMENU) CMD_ID, Instance, NULL);
	OldCmdProc = (WNDPROC) SetWindowLong(CmdW, GWL_WNDPROC, (LONG) cmdProc);
	WinSetFocus();
      }
      makeLayout();
      break;
    case PU_TF_STATUS:
      StatusOnOff = onOff;
      if (onOff && StatusW == NULL) {
	StatusW = CreateWindow("static", NULL,
	    WS_CHILDWINDOW, 0, 0, 0, 0,
	    MainW, (HMENU) STATUS_ID, Instance, NULL);
        BreakW = CreateWindow("button", "BREAK",
	    WS_CHILDWINDOW, 0, 0, 0, 0,
	    MainW, (HMENU) BREAK_ID, Instance, NULL);
	registerBreak();
      }
      makeLayout();
      break;
  }
}

static void
cmdFieldActivate(HWND w, PuMenuAction act)
{
  CmdFieldInfo *infoP;
  PuCmdFieldCBStruc cmdFieldStruc;

  infoP = (CmdFieldInfo *) GetWindowLong(w, GWL_USERDATA);
  if (infoP == NULL)
    return;

  cmdFieldStruc.act = act;
  cmdFieldStruc.name = infoP->name;
  infoP->cmdFieldCB(w, infoP->clientData, &cmdFieldStruc);
}

static LRESULT CALLBACK
fieldProc(HWND w, UINT msgKind, WPARAM wParam, LPARAM lParam)
{
  static BOOL inside = FALSE;
  POINT point;

  switch (msgKind) {
    case WM_MOUSEMOVE:
      if (inside) {
	point.x = LOWORD(lParam);
	point.y = HIWORD(lParam);
	if (ChildWindowFromPoint(w, point) != w) {
	  inside = FALSE;
	  ReleaseCapture();
	  cmdFieldActivate(w, PU_MA_LEAVE);
	}
      } else {
        cmdFieldActivate(w, PU_MA_ENTER);
        inside = TRUE;
	SetCapture(w);
      }
      break;
  }

  return CallWindowProc(OldFieldProc, w, msgKind, wParam, lParam);
}

PuCmdField
PuWinCreateCmdField(char *name,
    PuCmdFieldCB cmdFieldCB, void *clientData, PuFreeCB freeCB)
{
  HWND buttonW;
  char *control;
  int len;
  CmdFieldInfo *infoP;

  if (cmdFieldCB == NULL) {
    control = "static";
  } else {
    control = "button";
    len = strlen(name);
    if (len > CmdFieldLen)
      CmdFieldLen = len;
  }

  buttonW = CreateWindow(control, name,
      WS_CHILDWINDOW, 0, 0, 0, 0,
      MainW, (HMENU) (CMD_FIELD_BASE + CmdFieldNo), Instance, NULL);

  CmdFieldNo++;

  if (cmdFieldCB != NULL) {
    infoP = malloc(sizeof(*infoP));
    infoP->name = malloc(strlen(name) + 1);
    (void) strcpy(infoP->name, name);
    infoP->cmdFieldCB = cmdFieldCB;
    infoP->clientData = clientData;

    (void) SetWindowLong(buttonW, GWL_USERDATA, (LONG) infoP);
    OldFieldProc = (WNDPROC) SetWindowLong(buttonW,
        GWL_WNDPROC, (LONG) fieldProc);
  }

  return buttonW;
}

void
PuWinSwitchCmdFields(BOOL onOff)
{
  CmdFieldsOnOff = onOff;
  makeLayout();
}

void
WinSetFocus(void)
{
  (void) SetFocus(CmdW);
}

static void
textEdit(HWND w)
{
  WinTextStruc *textP;
  PuTextCBStruc puTextStruc;

  textP = (WinTextStruc *) GetWindowLong(w, GWL_USERDATA);
  if (textP == NULL) {
    textP = malloc(sizeof(*textP));
    WinTextStrucInit(textP);
    (void) SetWindowLong(w, GWL_USERDATA, (LONG) textP);
  }

  if (! TextStrucWinToPu(w, textP, &puTextStruc))
    return;

  PuCallCmdCB((PuWindow) DrawW, &puTextStruc);

  TextStrucPuToWin(w, &puTextStruc, textP);
}

static void
dropFiles(HANDLE drop)
{
  int fileNo, fileI, len, i;
  char **fileNameA, *fileName;

  fileNo = DragQueryFile(drop, 0xFFFFFFFF, NULL, 0);
  fileNameA = malloc(fileNo * sizeof(*fileNameA));

  for (fileI = 0; fileI < fileNo; fileI++) {
    len = DragQueryFile(drop, fileI, NULL, 0);
    fileName = malloc(len + 1);
    (void) DragQueryFile(drop, fileI, fileName, len + 1);
    for (i = 0; i < len; i++)
      if (fileName[i] == '\\')
        fileName[i] = '/';
     fileNameA[fileI] = fileName;
  }

  PuCallDropCB((PuWindow) DrawW, fileNameA, fileNo);

  for (fileI = 0; fileI < fileNo; fileI++)
    free(fileNameA[fileI]);

  free(fileNameA);
}

LRESULT CALLBACK
WinMainProc(HWND w, UINT msgKind, WPARAM wParam, LPARAM lParam)
{
  static int lastMenuId = 0;
  static int lastFieldId = 0;
  int notifyCode, childId;
  UINT flags;
  HWND childW;
  HANDLE drop;

  switch (msgKind) {
    case WM_ACTIVATE:
      if (LOWORD(wParam) != WA_INACTIVE)
	WinSetFocus();
      return 0;
    case WM_SIZE:
      makeLayout();
      return 0;
    case WM_MENUSELECT:
      flags = HIWORD(wParam);
      childId = LOWORD(wParam);
      if (lastMenuId > 0) {
        WinMenuActivate(lastMenuId, PU_MA_LEAVE);
	lastMenuId = 0;
      }
      if (! (flags & (MF_SYSMENU | MF_POPUP))) {
        WinMenuActivate(childId, PU_MA_ENTER);
	lastMenuId = childId;
	return 0;
      }
      break;
    case WM_COMMAND:
      notifyCode = HIWORD(wParam);
      childId = LOWORD(wParam);
      childW = (HWND) lParam;

      if (childW == NULL) {
        WinMenuActivate(childId, PU_MA_ACTIVATE);
	return 0;
      }

      switch (notifyCode) {
	case BN_CLICKED:
	  cmdFieldActivate(childW, PU_MA_ACTIVATE);
	  WinSetFocus();
	  return 0;
	case EN_UPDATE:
	  textEdit(CmdW);
	  return 0;
      }

      break;
    case WM_DROPFILES:
      drop = (HANDLE) wParam;
      dropFiles(drop);
      DragFinish(drop);
      return 0;
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
  }

  return DefWindowProc(w, msgKind, wParam, lParam);
}
