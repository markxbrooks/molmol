/*
************************************************************************
*
*   PuCB.c - management of Pu callbacks
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/pudev/SCCS/s.PuCB.c
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include <stdio.h>
#include <stdlib.h>

#include <linlist.h>

#include <pu_cb.h>

typedef void (*CBFunc) ();

typedef struct {
  PuWindow win;
  CBFunc cb;
  void *clientData;
  PuFreeCB freeCB;
} ListEntry;

typedef struct {
  ListEntry *first;
} List;

typedef void (* ApplyFunc) (CBFunc, PuWindow, void *, void *);

static PuWindow CurrWindow;

static LINLIST ExposeList;
static LINLIST ResizeList;
static LINLIST KeyList;
static LINLIST PressList[PU_MOUSE_BUTTON_NO];
static LINLIST ReleaseList[PU_MOUSE_BUTTON_NO];
static LINLIST DragList[PU_MOUSE_BUTTON_NO];
static LINLIST MoveList;
static LINLIST CmdList;
static LINLIST DropList;
static LINLIST QuitList;

void
PuInitCallbacks(void)
{
  int i;

  ExposeList = ListOpen(sizeof(ListEntry));
  ResizeList = ListOpen(sizeof(ListEntry));
  KeyList = ListOpen(sizeof(ListEntry));
  for (i = 0; i < PU_MOUSE_BUTTON_NO; i++) {
    PressList[i] = ListOpen(sizeof(ListEntry));
    ReleaseList[i] = ListOpen(sizeof(ListEntry));
    DragList[i] = ListOpen(sizeof(ListEntry));
  }
  MoveList = ListOpen(sizeof(ListEntry));
  CmdList = ListOpen(sizeof(ListEntry));
  DropList = ListOpen(sizeof(ListEntry));
  QuitList = ListOpen(sizeof(ListEntry));
}

static void
insertCallback(LINLIST list, CBFunc cb, void *clientData, PuFreeCB freeCB)
{
  ListEntry entry;

  entry.win = CurrWindow;
  entry.cb = cb;
  entry.clientData = clientData;
  entry.freeCB = freeCB;

  (void) ListInsertLast(list, &entry);
}

static void
removeCallback(LINLIST list, CBFunc cb, void *clientData)
{
  ListEntry *entryP;

  entryP = ListFirst(list);
  while (entryP != NULL) {
    if (entryP->cb == cb && entryP->clientData == clientData && 
	entryP->win == CurrWindow) {
      if (entryP->freeCB != NULL)
	entryP->freeCB(entryP->clientData);
      ListRemove(list, entryP);
    }
    entryP = ListNext(list, entryP);
  }
}

static void
listApply(LINLIST list, PuWindow win, void *cbStrucP, ApplyFunc func)
{
  ListEntry *entryP;

  entryP = ListFirst(list);
  while (entryP != NULL) {
    if (entryP->win == win)
      func(entryP->cb, win, entryP->clientData, cbStrucP);
    entryP = ListNext(list, entryP);
  }
}

void
PuSetWindow(PuWindow win)
{
  CurrWindow = win;
}

void
PuAddExposeCB(PuExposeCB exposeCB, void *clientData, PuFreeCB freeCB)
{
  insertCallback(ExposeList, exposeCB, clientData, freeCB);
}

void
PuRemoveExposeCB(PuExposeCB exposeCB, void *clientData)
{
  removeCallback(ExposeList, exposeCB, clientData);
}

static void
callExpose(CBFunc cb, PuWindow win, void *clientData, void *cbStrucP)
{
  PuExposeCB exposeCB = (PuExposeCB) cb;

  exposeCB(win, clientData, cbStrucP);
}

void
PuCallExposeCB(PuWindow win, int x, int y, int w, int h)
{
  PuExposeCBStruc exposeStruc;

  exposeStruc.x = x;
  exposeStruc.y = y;
  exposeStruc.w = w;
  exposeStruc.h = h;

  listApply(ExposeList, win, &exposeStruc, callExpose);
}

void
PuAddResizeCB(PuResizeCB resizeCB, void *clientData, PuFreeCB freeCB)
{
  insertCallback(ResizeList, resizeCB, clientData, freeCB);
}

void
PuRemoveResizeCB(PuResizeCB resizeCB, void *clientData)
{
  removeCallback(ResizeList, resizeCB, clientData);
}

static void
callResize(CBFunc cb, PuWindow win, void *clientData, void *cbStrucP)
{
  PuResizeCB resizeCB = (PuResizeCB) cb;

  resizeCB(win, clientData, cbStrucP);
}

void
PuCallResizeCB(PuWindow win, int w, int h)
{
  PuResizeCBStruc resizeStruc;

  resizeStruc.w = w;
  resizeStruc.h = h;

  listApply(ResizeList, win, &resizeStruc, callResize);
}

void
PuAddKeyCB(PuKeyCB keyCB, void *clientData, PuFreeCB freeCB)
{
  insertCallback(KeyList, keyCB, clientData, freeCB);
}

void
PuRemoveKeyCB(PuKeyCB keyCB, void *clientData)
{
  removeCallback(KeyList, keyCB, clientData);
}

static void
callKey(CBFunc cb, PuWindow win, void *clientData, void *cbStrucP)
{
  PuKeyCB keyCB = (PuKeyCB) cb;

  keyCB(win, clientData, cbStrucP);
}

void
PuCallKeyCB(PuWindow win, char ch)
{
  PuKeyCBStruc keyStruc;

  keyStruc.ch = ch;

  listApply(KeyList, win, &keyStruc, callKey);
}

void
PuAddButtonPressCB(PuMouseButton button,
    PuMouseCB mouseCB, void *clientData, PuFreeCB freeCB)
{
  insertCallback(PressList[button], mouseCB, clientData, freeCB);
}

void
PuRemoveButtonPressCB(PuMouseButton button,
    PuMouseCB mouseCB, void *clientData)
{
  removeCallback(PressList[button], mouseCB, clientData);
}

static void
callMouse(CBFunc cb, PuWindow win, void *clientData, void *cbStrucP)
{
  PuMouseCB mouseCB = (PuMouseCB) cb;

  mouseCB(win, clientData, cbStrucP);
}

void
PuCallButtonPressCB(PuWindow win, PuMouseButton button,
    BOOL addSel, int x, int y)
{
  PuMouseCBStruc mouseStruc;

  mouseStruc.button = button;
  mouseStruc.pressed = TRUE;
  mouseStruc.addSel = addSel;
  mouseStruc.x = x;
  mouseStruc.y = y;

  listApply(PressList[button], win, &mouseStruc, callMouse);
}

void
PuAddButtonReleaseCB(PuMouseButton button,
    PuMouseCB mouseCB, void *clientData, PuFreeCB freeCB)
{
  insertCallback(ReleaseList[button], mouseCB, clientData, freeCB);
}

void
PuRemoveButtonReleaseCB(PuMouseButton button,
    PuMouseCB mouseCB, void *clientData)
{
  removeCallback(ReleaseList[button], mouseCB, clientData);
}

void
PuCallButtonReleaseCB(PuWindow win, PuMouseButton button,
    BOOL doSel, int x, int y)
{
  PuMouseCBStruc mouseStruc;

  mouseStruc.button = button;
  mouseStruc.pressed = FALSE;
  mouseStruc.doSel = doSel;
  mouseStruc.x = x;
  mouseStruc.y = y;

  listApply(ReleaseList[button], win, &mouseStruc, callMouse);
}

void
PuAddDragCB(PuMouseButton button,
    PuMouseCB mouseCB, void *clientData, PuFreeCB freeCB)
{
  insertCallback(DragList[button], mouseCB, clientData, freeCB);
}

void
PuRemoveDragCB(PuMouseButton button, PuMouseCB mouseCB, void *clientData)
{
  removeCallback(DragList[button], mouseCB, clientData);
}

void
PuCallDragCB(PuWindow win, PuMouseButton button, int x, int y)
{
  PuMouseCBStruc mouseStruc;

  mouseStruc.button = button;
  mouseStruc.pressed = TRUE;
  mouseStruc.x = x;
  mouseStruc.y = y;

  listApply(DragList[button], win, &mouseStruc, callMouse);
}

void
PuAddMoveCB(PuMouseCB mouseCB, void *clientData, PuFreeCB freeCB)
{
  insertCallback(MoveList, mouseCB, clientData, freeCB);
}

void
PuRemoveMoveCB(PuMouseCB mouseCB, void *clientData)
{
  removeCallback(MoveList, mouseCB, clientData);
}

void
PuCallMoveCB(PuWindow win, int x, int y)
{
  PuMouseCBStruc mouseStruc;

  mouseStruc.button = PU_MB_NONE;
  mouseStruc.pressed = FALSE;
  mouseStruc.x = x;
  mouseStruc.y = y;

  listApply(MoveList, win, &mouseStruc, callMouse);
}

void
PuAddCmdCB(PuCmdCB cmdCB, void *clientData, PuFreeCB freeCB)
{
  insertCallback(CmdList, cmdCB, clientData, freeCB);
}

void
PuRemoveCmdCB(PuCmdCB cmdCB, void *clientData)
{
  removeCallback(CmdList, cmdCB, clientData);
}

static void
callCmd(CBFunc cb, PuWindow win, void *clientData, void *cbStrucP)
{
  PuCmdCB cmdCB = (PuCmdCB) cb;

  cmdCB(win, clientData, cbStrucP);
}

void
PuCallCmdCB(PuWindow win, PuTextCBStruc *cmdStrucP)
{
  listApply(CmdList, win, cmdStrucP, callCmd);
}

void
PuAddDropCB(PuDropCB dropCB, void *clientData, PuFreeCB freeCB)
{
  insertCallback(DropList, dropCB, clientData, freeCB);
}

void
PuRemoveDropCB(PuDropCB dropCB, void *clientData)
{
  removeCallback(DropList, dropCB, clientData);
}

static void
callDrop(CBFunc cb, PuWindow win, void *clientData, void *cbStrucP)
{
  PuDropCB dropCB = (PuDropCB) cb;

  dropCB(win, clientData, cbStrucP);
}

void
PuCallDropCB(PuWindow win, char **fileNameA, int fileNo)
{
  PuDropCBStruc dropStruc;

  dropStruc.fileNameA = fileNameA;
  dropStruc.fileNo = fileNo;

  listApply(DropList, win, &dropStruc, callDrop);
}

void
PuAddQuitCB(PuQuitCB quitCB, void *clientData, PuFreeCB freeCB)
{
  insertCallback(QuitList, quitCB, clientData, freeCB);
}

void
PuRemoveQuitCB(PuQuitCB quitCB, void *clientData)
{
  removeCallback(QuitList, quitCB, clientData);
}

void
PuCallQuitCB(void)
{
  ListEntry *entryP;
  PuQuitCB quitCB;

  entryP = ListFirst(QuitList);
  while (entryP != NULL) {
    quitCB = (PuQuitCB) entryP->cb;
    quitCB(entryP->clientData);
    entryP = ListNext(QuitList, entryP);
  }
}
