/*
************************************************************************
*
*   WinEvent.c - handle events
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
*   Date of last modification : 01/05/19
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/win/SCCS/s.WinEvent.c
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include "win_event.h"

#include <stdlib.h>
#include <windows.h>

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#endif
#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
#endif

#include <linlist.h>
#include <pu_types.h>
#include <pu_cb.h>
#include "win_main_w.h"
#include "win_menu.h"

typedef struct {
  PuTimeOut timeOutF;
  void *clientData;
  UINT timerId;
} TimeOutDescr;

static LINLIST DialList = NULL;
static PuMouseButton CurrButton = PU_MB_NONE;
static BOOL DidDrag = FALSE;
static LINLIST TimeOutList = NULL;

void
WinAddDialog(HWND w)
{
  if (DialList == NULL)
    DialList = ListOpen(sizeof(HWND));

  (void) ListInsertLast(DialList, &w);
}

void
WinRemoveDialog(HWND w)
{
  HWND *wP;

  wP = ListFirst(DialList);
  while (*wP != w)
    wP = ListNext(DialList, wP);

  ListRemove(DialList, wP);
}

static void
destroyDialogs(void)
{
  HWND *wP;

  wP = ListFirst(DialList);
  while (wP != NULL) {
    DestroyWindow(*wP);
    wP = ListNext(DialList, wP);
  }
}

static void
processOneEvent(void)
{
  HACCEL accTab;
  MSG msg;
  HWND *wP;

  accTab = WinGetAccelTable();

  if (GetMessage(&msg, NULL, 0, 0) == 0) {
    PuCallQuitCB();
    destroyDialogs();
    if (accTab != NULL)
      DestroyAcceleratorTable(accTab);
    exit(0);
  }

  wP = ListFirst(DialList);
  while (wP != NULL) {
    if (IsDialogMessage(*wP, &msg))
      return;
    wP = ListNext(DialList, wP);
  }

  if (accTab != NULL &&
      TranslateAccelerator(WinGetMainW(), accTab, &msg))
    return;

  TranslateMessage(&msg);
  DispatchMessage(&msg);
}

void
PuWinEventLoop(void)
{
  for (;;)
    processOneEvent();
}

void
PuWinProcessEvent(void)
{
  processOneEvent();
}

static VOID CALLBACK
timerProc(HWND w, UINT msgI, UINT timerId, DWORD sysTime)
{
  TimeOutDescr *descrP;

  descrP = ListFirst(TimeOutList);
  while (descrP->timerId != timerId)
    descrP = ListNext(TimeOutList, descrP);

  descrP->timeOutF(descrP->clientData);

  ListRemove(TimeOutList, descrP);
  KillTimer(NULL, timerId);
}

void
PuWinAddTimeOut(int interv, PuTimeOut timeOutF, void *clientData)
{
  TimeOutDescr descr;

  descr.timeOutF = timeOutF;
  descr.clientData = clientData;
  descr.timerId = SetTimer(NULL, 0, interv, (TIMERPROC) timerProc);

  if (TimeOutList == NULL)
    TimeOutList = ListOpen(sizeof(TimeOutDescr));

  (void) ListInsertLast(TimeOutList, &descr);
}

static PuMouseButton
puButton(int state)
{
  int buttonNo;
  PuMouseButton button = 0;

  buttonNo = GetSystemMetrics(SM_CMOUSEBUTTONS);

  if ((state & MK_LBUTTON) != 0)
    button += 1;

  if (buttonNo > 2) {
    if ((state & MK_MBUTTON) != 0)
      button += 2;
    if ((state & MK_RBUTTON) != 0)
      button += 4;
  } else {
    if ((state & MK_RBUTTON) != 0)
      button += 2;
  }

  if (button > PU_MOUSE_BUTTON_NO)
    button = PU_MOUSE_BUTTON_NO;

  if (button == 0)
    return PU_MB_NONE;
  else
    return button - 1;
}

static LONG
winHeight(HWND w)
{
  RECT rect;

  GetClientRect(w, &rect);
  return rect.bottom;
}

LRESULT CALLBACK
WinDrawProc(HWND w, UINT msgKind, WPARAM wParam, LPARAM lParam)
{
  static int lastEnterId = 0;
  int notifyCode, childId;
  UINT flags;
  HMENU popup;
  HDC dc;
  PAINTSTRUCT ps;
  POINT point;
  int x, y;
  PuMouseButton newButton;

  switch (msgKind) {
    case WM_PAINT:
      dc = BeginPaint(w, &ps);
      PuCallExposeCB((PuWindow) w,
	  ps.rcPaint.left, ps.rcPaint.top,
	  ps.rcPaint.right - ps.rcPaint.left,
	  ps.rcPaint.bottom - ps.rcPaint.top);
      EndPaint(w, &ps);
      return 0;
    case WM_SIZE:
      PuCallResizeCB((PuWindow) w,
	  LOWORD(lParam), HIWORD(lParam));
      return 0;
    case WM_MENUSELECT:
      flags = HIWORD(wParam);
      childId = LOWORD(wParam);
      if (lastEnterId > 0) {
        WinMenuActivate(lastEnterId, PU_MA_LEAVE);
	lastEnterId = 0;
      }
      if (! (flags & MF_POPUP)) {
        WinMenuActivate(childId, PU_MA_ENTER);
	lastEnterId = childId;
	return 0;
      }
      break;
    case WM_COMMAND:
      notifyCode = HIWORD(wParam);
      childId = LOWORD(wParam);
      if (notifyCode == BN_CLICKED) {
        WinMenuActivate(childId, PU_MA_ACTIVATE);
	return 0;
      }
      break;
    case WM_CHAR:
      PuCallKeyCB((PuWindow) w, (char) wParam);
      return 0;
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
      popup = WinGetPopup();
      if (msgKind == WM_RBUTTONDOWN && popup != NULL) {
	point.x = GET_X_LPARAM(lParam);
	point.y = GET_Y_LPARAM(lParam);
	ClientToScreen(w, &point);
	TrackPopupMenu(popup, TPM_RIGHTBUTTON,
	    point.x, point.y, 0, w, NULL);
	return 0;
      }

      x = GET_X_LPARAM(lParam);
      y = winHeight(w) - GET_Y_LPARAM(lParam);

      newButton = puButton(wParam);
      if (newButton == CurrButton)
	return 0;

      if (CurrButton == PU_MB_NONE) {
	SetCapture(w);
        DidDrag = FALSE;
      } else {
        PuCallButtonReleaseCB((PuWindow) w, CurrButton, FALSE, x, y);
      }

      PuCallButtonPressCB((PuWindow) w, newButton,
           (wParam & (MK_SHIFT | MK_CONTROL)) != 0, x, y);

      CurrButton = newButton;
      return 0;
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
      popup = WinGetPopup();
      if (msgKind == WM_RBUTTONUP && popup != NULL)
	break;

      x = GET_X_LPARAM(lParam);
      y = winHeight(w) - GET_Y_LPARAM(lParam);

      newButton = puButton(wParam);

      if (newButton == CurrButton)
	return 0;

      PuCallButtonReleaseCB((PuWindow) w, CurrButton,
	  newButton == PU_MB_NONE && ! DidDrag, x, y);

      if (newButton == PU_MB_NONE)
	ReleaseCapture();
      else
        PuCallButtonPressCB((PuWindow) w, newButton,
	    (wParam & (MK_SHIFT | MK_CONTROL)) != 0, x, y);
      
      CurrButton = newButton;
      return 0;
    case WM_MOUSEMOVE:
      x = GET_X_LPARAM(lParam);
      y = winHeight(w) - GET_Y_LPARAM(lParam);

      if (CurrButton == PU_MB_NONE) {
        PuCallMoveCB((PuWindow) w, x, y);
      } else {
        PuCallDragCB((PuWindow) w, CurrButton, x, y);
        DidDrag = TRUE;
      }

      return 0;
  }

  return DefWindowProc(w, msgKind, wParam, lParam);
}
