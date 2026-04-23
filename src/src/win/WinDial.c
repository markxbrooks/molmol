/*
************************************************************************
*
*   WinDial.c - build Windows dialog boxes
*
*   Copyright (c) 1996-2000
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
*   Date of last modification : 01/06/02
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/win/SCCS/s.WinDial.c
*   SCCS identification       : 1.18
*
************************************************************************
*/

#include "win_dial.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>

#include <linlist.h>
#include "win_text.h"
#include "win_main_w.h"
#include "win_event.h"
#include "win_place.h"

#define ICOLOR(c) (int) (c * 255.9999f)

#define BUTTON_FIRST PU_CT_OK
#define BUTTON_LAST  PU_CT_HELP
#define BUTTON_NO    (BUTTON_LAST - BUTTON_FIRST + 1)
#define BUTTON_SPACE 4
#define BUTTON_H     14

#define LABEL_OK     "OK"
#define LABEL_CLOSE  "Close"
#define LABEL_APPLY  "Apply"
#define LABEL_RESET  "Reset"
#define LABEL_CANCEL "Cancel"
#define LABEL_HELP   "Help"

#define SCROLL_OFFSET 500

typedef enum {
  GIZMO_FILE_SELECTION_BOX,
  GIZMO_DIALOG,
  GIZMO_VALUATOR_BOX,
  GIZMO_LABEL,
  GIZMO_BUTTON,
  GIZMO_COLOR_FIELD,
  GIZMO_SLIDER,
  GIZMO_TEXT_FIELD,
  GIZMO_TEXT,
  GIZMO_SCROLL,
  GIZMO_LIST,
  GIZMO_RADIO_BOX,
  GIZMO_CHECK_BOX,
  GIZMO_TOGGLE
} GizmoType;

typedef struct {
  PuGizmoCBType type;
  PuGizmoCB gizmoCB;
  void *clientData;
  PuFreeCB freeCB;
} GizmoCBInfo;

typedef struct {
  char *str;
  BOOL onOff;
} ListEntry;

typedef struct GizmoInfoS *GizmoInfoP;

typedef struct GizmoInfoS {
  GizmoType type;
  char *name;
  PuConstraints constr;
  LINLIST cbList;
  int xSize, ySize;
  GizmoInfoP dialP;
  HWND w;
  union {
    struct {
      char *dir;
      char *pattern;
      PuFileAccess acc;
    } fileSel;
    struct {
      int xBase, yBase;
      int childNo;
      GizmoInfoP *childPA;
      BOOL buttonSwitchA[BUTTON_NO];
      BOOL outside;
    } dial;
    struct {
      float r, g, b;
    } color;
    struct {
      float minVal, maxVal, val;
      float factor;
      PuValuatorCB valuatorCB;
      void *clientData;
      PuFreeCB freeCB;
    } slider;
    struct {
      char *str;
      int historySize;
      LINLIST historyList;
      char **historyCurr;
      BOOL firstMod;
      WinTextStruc textS;
    } text;
    struct {
      GizmoInfoP textP;
    } scroll;
    struct {
      int entryNo;
      ListEntry *entryA;
      BOOL multSel, autoDesel, showBottom;
    } list;
    struct {
      int lineNo;
      int entryNo;
	  int entrySize;
    } box;
    struct {
      GizmoInfoP boxP;
      BOOL onOff;
      int entryI;
    } toggle;
  } u;
} GizmoInfo;

static char *ButtonLabelA[BUTTON_NO] = {
  LABEL_OK,
  LABEL_CLOSE,
  LABEL_APPLY,
  LABEL_RESET,
  LABEL_CANCEL,
  LABEL_HELP
};

static GizmoInfo *FileSelInfoP;

static GizmoInfo *
newInfo(GizmoType type, char *name)
{
  GizmoInfo *infoP;

  infoP = malloc(sizeof(*infoP));

  infoP->type = type;
  infoP->constr.x = 0;
  infoP->constr.y = 0;
  infoP->constr.w = 0;
  infoP->constr.h = 0;
  infoP->cbList = ListOpen(sizeof(GizmoCBInfo));
  infoP->xSize = 0;
  infoP->ySize = 0;
  infoP->w = NULL;
  infoP->dialP = NULL;

  infoP->name = malloc(strlen(name) + 1);
  (void) strcpy(infoP->name, name);

  return  infoP;
}

static GizmoInfo *
newChildInfo(GizmoInfoP dialInfoP, GizmoType type, char *name)
{
  GizmoInfo *infoP;

  infoP = newInfo(type, name);

  if (dialInfoP->u.dial.childNo == 0)
    dialInfoP->u.dial.childPA =
        malloc(sizeof(*dialInfoP->u.dial.childPA));
  else
    dialInfoP->u.dial.childPA = realloc(dialInfoP->u.dial.childPA,
        (dialInfoP->u.dial.childNo + 1) *
	sizeof(*dialInfoP->u.dial.childPA));

  dialInfoP->u.dial.childPA[dialInfoP->u.dial.childNo++] = infoP;
  infoP->dialP = dialInfoP;

  return infoP;
}

static void
freeInfo(GizmoInfo *infoP)
{
  GizmoCBInfo *cbP;
  int i;

  free(infoP->name);

  cbP = ListFirst(infoP->cbList);
  while (cbP != NULL) {
    if (cbP->freeCB != NULL)
      cbP->freeCB(cbP->clientData);
    cbP = ListNext(infoP->cbList, cbP);
  }
  ListClose(infoP->cbList);

  if (infoP->type == GIZMO_FILE_SELECTION_BOX) {
    free(infoP->u.fileSel.dir);
    free(infoP->u.fileSel.pattern);
  } else if (infoP->type == GIZMO_DIALOG || infoP->type == GIZMO_VALUATOR_BOX) {
    free(infoP->u.dial.childPA);
  } else if (infoP->type == GIZMO_SLIDER) {
    if (infoP->u.slider.valuatorCB != NULL && infoP->u.slider.freeCB != NULL)
      infoP->u.slider.freeCB(infoP->u.slider.clientData);
  } else if (infoP->type == GIZMO_TEXT_FIELD || infoP->type == GIZMO_TEXT) {
    free(infoP->u.text.str);
    if (infoP->u.text.historySize > 0)
      ListClose(infoP->u.text.historyList);
  } else if (infoP->type == GIZMO_LIST) {
    for (i = 0; i < infoP->u.list.entryNo; i++)
      free(infoP->u.list.entryA[i].str);
    free(infoP->u.list.entryA);
  }
}

static void
callCB(GizmoInfo *infoP, PuGizmoCBType type, char *name, void *callData)
{
  LINLIST cbMatchList;
  GizmoCBInfo *cbP;

  /* copy callbacks that match into a new list, a callback may destroy
     the Gizmo, including the GizmoInfo with its cbList */
  cbMatchList = ListOpen(sizeof(GizmoCBInfo));
  cbP = ListFirst(infoP->cbList);
  while (cbP != NULL) {
    if (cbP->type == type)
      (void) ListInsertLast(cbMatchList, cbP);
    cbP = ListNext(infoP->cbList, cbP);
  }

  cbP = ListFirst(cbMatchList);
  while (cbP != NULL) {
    cbP->gizmoCB((PuGizmo) infoP, name, cbP->clientData, callData);
    cbP = ListNext(cbMatchList, cbP);
  }

  ListClose(cbMatchList);
}

PuGizmo
PuWinCreateFileSelectionBox(char *name,
    char *dir, char *pattern, PuFileAccess acc)
{
  GizmoInfo *infoP;
  int len, i;

  infoP = newInfo(GIZMO_FILE_SELECTION_BOX, name);

  len = strlen(dir);
  infoP->u.fileSel.dir = malloc(len + 1);
  (void) strcpy(infoP->u.fileSel.dir, dir);
  for (i = 0; i < len; i++)
    if (infoP->u.fileSel.dir[i] == '/')
      infoP->u.fileSel.dir[i] = '\\';

  infoP->u.fileSel.pattern = malloc(strlen(pattern) + 1);
  (void) strcpy(infoP->u.fileSel.pattern, pattern);

  infoP->u.fileSel.acc = acc;

  return (PuGizmo) infoP;
}

PuGizmo
PuWinCreateDialog(char *name, int xBase, int yBase)
{
  GizmoInfo *infoP;
  int buttonI;

  infoP = newInfo(GIZMO_DIALOG, name);

  if (xBase == 0)
    xBase = 1;
  infoP->u.dial.xBase = xBase;
  infoP->u.dial.yBase = yBase;
  infoP->u.dial.childNo = 0;
  infoP->u.dial.outside = FALSE;

  for (buttonI = 0; buttonI < BUTTON_NO; buttonI++)
    infoP->u.dial.buttonSwitchA[buttonI] = FALSE;

  return (PuGizmo) infoP;
}

PuGizmo
PuWinCreateLabel(PuGizmo dial, char *name)
{
  GizmoInfo *dialInfoP = (GizmoInfo *) dial;
  GizmoInfo *infoP;

  infoP = newChildInfo(dialInfoP, GIZMO_LABEL, name);
  infoP->xSize = 4 * strlen(name);
  infoP->ySize = 8;

  return infoP;
}

PuGizmo
PuWinCreateButton(PuGizmo dial, char *name)
{
  GizmoInfo *dialInfoP = (GizmoInfo *) dial;
  GizmoInfo *infoP;

  infoP = newChildInfo(dialInfoP, GIZMO_BUTTON, name);
  infoP->xSize = 4 * (strlen(name) + 2);
  infoP->ySize = 12;

  return infoP;
}

PuGizmo
PuWinCreateColorField(PuGizmo dial, char *name)
{
  GizmoInfo *dialInfoP = (GizmoInfo *) dial;
  GizmoInfo *infoP;

  infoP = newChildInfo(dialInfoP, GIZMO_COLOR_FIELD, name);
  infoP->u.color.r = 0.0f;
  infoP->u.color.g = 0.0f;
  infoP->u.color.b = 0.0f;

  return infoP;
}

PuGizmo
PuWinCreateSlider(PuGizmo dial, char *name,
    float minVal, float maxVal, int digits, float initVal)
{
  GizmoInfo *dialInfoP = (GizmoInfo *) dial;
  GizmoInfo *infoP;

  infoP = newChildInfo(dialInfoP, GIZMO_SLIDER, name);
  infoP->xSize = 80;
  infoP->ySize = 12;
  infoP->u.slider.minVal = minVal;
  infoP->u.slider.maxVal = maxVal;
  infoP->u.slider.val = initVal;
  infoP->u.slider.factor = 1.0f;
  while (digits > 0) {
    infoP->u.slider.factor *= 10.0f;
    digits--;
  }
  infoP->u.slider.valuatorCB = NULL;

  return infoP;
}

PuGizmo
PuWinCreateTextField(PuGizmo dial, char *name, char *text)
{
  GizmoInfo *dialInfoP = (GizmoInfo *) dial;
  GizmoInfo *infoP;

  infoP = newChildInfo(dialInfoP, GIZMO_TEXT_FIELD, name);
  infoP->xSize = 8;
  infoP->ySize = 12;
  infoP->u.text.str = malloc(strlen(text) + 1);
  (void) strcpy(infoP->u.text.str, text);
  infoP->u.text.historySize = 0;
  WinTextStrucInit(&infoP->u.text.textS);

  return infoP;
}

PuGizmo
PuWinCreateText(PuGizmo dial, char *name, char *text)
{
  GizmoInfo *dialInfoP = (GizmoInfo *) dial;
  GizmoInfo *infoP;

  infoP = newChildInfo(dialInfoP, GIZMO_TEXT, name);
  infoP->xSize = 8;
  infoP->ySize = 12;
  infoP->u.text.str = malloc(strlen(text) + 1);
  (void) strcpy(infoP->u.text.str, text);
  infoP->u.text.historySize = 0;
  WinTextStrucInit(&infoP->u.text.textS);

  return infoP;
}

PuGizmo
PuWinCreateFileViewer(PuGizmo dial, char *fileName, BOOL readOnly)
{
  return NULL;
}

PuGizmo
PuWinCreateList(PuGizmo dial, char *name)
{
  GizmoInfo *dialInfoP = (GizmoInfo *) dial;
  GizmoInfo *infoP;

  infoP = newChildInfo(dialInfoP, GIZMO_LIST, name);
  infoP->u.list.entryA = NULL;
  infoP->u.list.entryNo = 0;
  infoP->u.list.multSel = FALSE;
  infoP->u.list.autoDesel = FALSE;
  infoP->u.list.showBottom = FALSE;

  return infoP;
}

void
PuWinAddListEntry(PuGizmo list, char *name, BOOL onOff)
{
  GizmoInfo *infoP = (GizmoInfo *) list;
  ListEntry *entryA;
  int entryNo;

  entryA = infoP->u.list.entryA;
  entryNo = infoP->u.list.entryNo;

  if (entryNo == 0)
    entryA = malloc(sizeof(*entryA));
  else
    entryA = realloc(entryA, (entryNo + 1) * sizeof(*entryA));

  entryA[entryNo].str = malloc(strlen(name) + 1);
  (void) strcpy(entryA[entryNo].str, name);
  entryA[entryNo].onOff = onOff;

  if (2 + 4 * (int) strlen(name) > infoP->xSize)
    infoP->xSize = 2 + 4 * strlen(name);

  if (infoP->w != NULL) {
    SendMessage(infoP->w, LB_ADDSTRING, 0, (LPARAM) name);
    if (onOff)
      SendMessage(infoP->w, LB_SETSEL, 1, entryNo);
    if (infoP->u.list.showBottom)
      SendMessage(infoP->w, LB_SETTOPINDEX, entryNo, 0);
  }

  entryNo++;

  infoP->u.list.entryA = entryA;
  infoP->u.list.entryNo = entryNo;
}

void
PuWinRemoveListEntries(PuGizmo list, int pos, int no)
{
  GizmoInfo *infoP = (GizmoInfo *) list;
  int entryI;

  if (infoP->w == NULL)
    return;

  SendMessage(infoP->w, WM_SETREDRAW, FALSE, 0);

  for (entryI = pos + no - 1; entryI >= pos; entryI--)
    SendMessage(infoP->w, LB_DELETESTRING, entryI, 0);

  SendMessage(infoP->w, WM_SETREDRAW, TRUE, 0);

  infoP->u.list.entryNo -= no;
}

PuGizmo
PuWinCreateRadioBox(PuGizmo dial, char *name)
{
  GizmoInfo *dialInfoP = (GizmoInfo *) dial;
  GizmoInfo *infoP;

  infoP = newChildInfo(dialInfoP, GIZMO_RADIO_BOX, name);
  infoP->xSize = 4 * (strlen(name) + 2);
  infoP->ySize = 24;
  infoP->u.box.lineNo = 1;
  infoP->u.box.entryNo = 0;
  infoP->u.box.entrySize = 0;

  return infoP;
}

PuGizmo
PuWinCreateCheckBox(PuGizmo dial, char *name)
{
  GizmoInfo *dialInfoP = (GizmoInfo *) dial;
  GizmoInfo *infoP;

  infoP = newChildInfo(dialInfoP, GIZMO_CHECK_BOX, name);
  infoP->xSize = 4 * (strlen(name) + 2);
  if (name[0] == '\0')
    infoP->ySize = 0;
  else
    infoP->ySize = 16;
  infoP->u.box.lineNo = 0;
  infoP->u.box.entryNo = 0;

  return infoP;
}

void
PuWinAddToggle(PuGizmo box, char *name, BOOL onOff)
{
  GizmoInfo *boxInfoP = (GizmoInfo *) box;
  GizmoInfo *dialInfoP, *infoP;
  int xSize;

  dialInfoP = boxInfoP->dialP;

  infoP = newChildInfo(dialInfoP, GIZMO_TOGGLE, name);
  infoP->u.toggle.boxP = boxInfoP;
  infoP->u.toggle.onOff = onOff;
  infoP->u.toggle.entryI = boxInfoP->u.box.entryNo;

  boxInfoP->u.box.entryNo++;

  xSize = 4 * (strlen(name) + 3);
  if (boxInfoP->type == GIZMO_CHECK_BOX) {
    if (xSize > boxInfoP->xSize)
      boxInfoP->xSize = xSize;

    boxInfoP->ySize += 8;
    boxInfoP->u.box.lineNo++;
  } else {
    if (xSize > boxInfoP->u.box.entrySize)
      boxInfoP->u.box.entrySize = xSize;
  }
}

PuValuatorBox
PuWinCreateValuatorBox(char *title)
{
  GizmoInfo *infoP;
  int buttonI;

  infoP = newInfo(GIZMO_VALUATOR_BOX, title);

  infoP->u.dial.xBase = 1;
  infoP->u.dial.yBase = 0;
  infoP->u.dial.childNo = 0;
  infoP->u.dial.outside = FALSE;

  for (buttonI = 0; buttonI < BUTTON_NO; buttonI++)
    infoP->u.dial.buttonSwitchA[buttonI] = FALSE;

  return (PuValuatorBox) infoP;
}

PuValuator
PuWinCreateValuator(PuValuatorBox box, char *name,
    float minVal, float maxVal, int digits, float initVal,
    PuValuatorCB valuatorCB, void *clientData, PuFreeCB freeCB)
{
  GizmoInfo *infoP;

  (void) PuWinCreateLabel((PuGizmo) box, name);
  infoP = (GizmoInfo *) PuWinCreateSlider((PuGizmo) box, name,
      minVal, maxVal, digits, initVal);
  infoP->u.slider.valuatorCB = valuatorCB;
  infoP->u.slider.clientData = clientData;
  infoP->u.slider.freeCB = freeCB;
  
  return (PuValuator) infoP;
}

void
PuWinSwitchValuatorBox(PuValuatorBox box, BOOL onOff)
{
  PuWinSwitchGizmo((PuGizmo) box, onOff);
}

void
PuWinSetBool(PuGizmo gizmo, PuBoolChoice choice, BOOL val)
{
  GizmoInfo *infoP = (GizmoInfo *) gizmo;

  switch (choice) {
    case PU_BC_PLACE_OUTSIDE:
      infoP->u.dial.outside = val;
      break;
    case PU_BC_MULT_SEL:
      infoP->u.list.multSel = val;
      break;
    case PU_BC_AUTO_DESEL:
      infoP->u.list.autoDesel = val;
      break;
    case PU_BC_SHOW_BOTTOM:
      infoP->u.list.showBottom = val;
      break;
  }
}

static void
freeStr(void *p, void *clientData)
{
  char **strP = p;

  free(*strP);
}

static void
addScroll(GizmoInfo *textInfoP)
{
  GizmoInfo *dialInfoP, *infoP;

  dialInfoP = textInfoP->dialP;

  infoP = newChildInfo(dialInfoP, GIZMO_SCROLL, "scroll");
  infoP->u.scroll.textP = textInfoP;
}

static void
getDialDim(HWND w, int *xP, int *yP, int *wP, int *hP)
{
  RECT rect;

  GetWindowRect(w, &rect);
  *xP = rect.left;
  *yP = rect.top;
  *wP = rect.right - *xP;
  *hP = rect.bottom - *yP;
}

static void
getChildDim(HWND dialW, HWND w, int *xP, int *yP, int *wP, int *hP)
{
  RECT rect;
  POINT point;

  GetWindowRect(w, &rect);
  point.x = rect.left;
  point.y = rect.top;
  ScreenToClient(dialW, &point);
  *xP = point.x;
  *yP = point.y;
  point.x = rect.right;
  point.y = rect.bottom;
  ScreenToClient(dialW, &point);
  *wP = point.x - *xP;
  *hP = point.y - *yP;
}

static void
resizeList(GizmoInfo *infoP, int hInc)
{
  int dx, dy, dw, dh;
  int cx, cy, cw, ch;
  int childNo, i;
  HDWP wp;
  GizmoInfo *childP;
  BOOL move;
  HWND buttonW;

  if (hInc == 0)
    return;

  hInc *= HIWORD(GetDialogBaseUnits()) / 8;

  childNo = infoP->dialP->u.dial.childNo;

  getDialDim(infoP->dialP->w, &dx, &dy, &dw, &dh);
  MoveWindow(infoP->dialP->w, dx, dy, dw, dh + hInc, TRUE);

  wp = BeginDeferWindowPos(childNo + BUTTON_NO);

  move = FALSE;
  for (i = 0; i < childNo; i++) {
    childP = infoP->dialP->u.dial.childPA[i];
    getChildDim(infoP->dialP->w, childP->w, &cx, &cy, &cw, &ch);
    if (childP == infoP) {
      wp = DeferWindowPos(wp, childP->w, HWND_TOP,
          cx, cy, cw, ch + hInc, SWP_SHOWWINDOW);
      move = TRUE;
    } else if (move) {
      wp = DeferWindowPos(wp, childP->w, HWND_TOP,
          cx, cy + hInc, cw, ch, SWP_SHOWWINDOW);
    } else {
      wp = DeferWindowPos(wp, childP->w, HWND_TOP,
          cx, cy, cw, ch, SWP_SHOWWINDOW);
    }
  }

  for (i = 0; i < BUTTON_NO; i++) {
    if (! infoP->dialP->u.dial.buttonSwitchA[i])
      continue;

    buttonW = GetDlgItem(infoP->dialP->w, childNo + i);
    getChildDim(infoP->dialP->w, buttonW, &cx, &cy, &cw, &ch);
    wp = DeferWindowPos(wp, buttonW, HWND_TOP,
        cx, cy + hInc, cw, ch, SWP_SHOWWINDOW);
  }

  EndDeferWindowPos(wp);
}

void
PuWinSetInt(PuGizmo gizmo, PuIntChoice choice, int val)
{
  GizmoInfo *infoP = (GizmoInfo *) gizmo;
  int oldSize;

  switch (choice) {
    case PU_IC_TEXT_WIDTH:
      infoP->xSize = 4 * val;
      break;
    case PU_IC_TEXT_HEIGHT:
      infoP->ySize = 8 * val;
      break;
    case PU_IC_HISTORY_SIZE:
      if (infoP->u.text.historySize == 0) {
	infoP->u.text.historyList = ListOpen(sizeof(char *));
	ListAddDestroyCB(infoP->u.text.historyList, freeStr, NULL, NULL);
	infoP->u.text.historyCurr = NULL;
	infoP->u.text.firstMod = FALSE;
	addScroll(infoP);
      }
      infoP->u.text.historySize = val;
      break;
    case PU_IC_ENTRY_NO:
      oldSize = infoP->ySize;
      infoP->ySize = 2 + 8 * val;
      if (infoP->w != NULL && infoP->dialP->ySize == 0)
	resizeList(infoP, infoP->ySize - oldSize);
      break;
    case PU_IC_LINE_NO:
      infoP->u.box.lineNo = val;
      infoP->ySize = 16 + 8 * val;
      break;
  }
}

void
PuWinSetStr(PuGizmo gizmo, PuStrChoice choice, char *val)
{
  GizmoInfo *infoP = (GizmoInfo *) gizmo;
  int len, i;
  char *buf, *bP;
  
  len = strlen(val);
  buf = malloc(2 * len + 1);

  /* replace CR and LF by CR-LF */
  bP = buf;
  for (i = 0; i < len; i++) {
    if (val[i] == 13 && val[i + 1] != 10) {
      *bP = 13;
      bP++;
      *bP = 10;
    } else if (val[i] == 10) {
      *bP = 13;
      bP++;
      *bP = 10;
    } else {
      *bP = val[i];
    }
    bP++;
  }
  *bP = '\0';

  switch (choice) {
    case PU_SC_TEXT:
      infoP->u.text.str = realloc(infoP->u.text.str, strlen(buf) + 1);
      (void) strcpy(infoP->u.text.str, buf);
      if (infoP->w != 0)
	SetWindowText(infoP->w, buf);
      break;
  }

  free(buf);
}

static void
paintColor(HWND w, float r, float g, float b)
{
  HBRUSH brush;
  HDC dc;
  RECT rect;

  InvalidateRect(w, NULL, TRUE);
  UpdateWindow(w);

  dc = GetDC(w);
  brush = CreateSolidBrush(RGB(ICOLOR(r), ICOLOR(g), ICOLOR(b)));
  SelectObject(dc, brush);
  GetClientRect(w, &rect);
  Rectangle(dc, rect.left, rect.top, rect.right, rect.bottom);
  DeleteObject(brush);
  ReleaseDC(w, dc);
}

void
PuWinSetColor(PuGizmo gizmo, float r, float g, float b)
{
  GizmoInfo *infoP = (GizmoInfo *) gizmo;

  infoP->u.color.r = r;
  infoP->u.color.g = g;
  infoP->u.color.b = b;

  if (infoP->w != NULL)
    paintColor(infoP->w, r, g, b);
}

void
PuWinSetConstraints(PuGizmo gizmo, PuConstraints constr)
{
  GizmoInfo *infoP = (GizmoInfo *) gizmo;

  infoP->constr = constr;
}

static void
getFilter(char *filter, char *pattern)
{
  int patLen, extStart, pos, i;
  char ch;

  patLen = strlen(pattern);
  extStart = 0;
  for (i = 0; i < patLen; i++)
    if (pattern[i] == '.') {
      extStart = i + 1;
      break;
    }

  for (i = extStart; i < patLen; i++) {
    ch = pattern[i];
    if (ch >= 'a' && ch <= 'z')
      ch = ch - 'a' + 'A';
    filter[i - extStart] = ch;
  }
  pos = patLen - extStart;

  (void) strcpy(filter + pos, " Files");
  pos += strlen(filter + pos) + 1;

  (void) strcpy(filter + pos, pattern);
  pos += strlen(pattern) + 1;

  (void) strcpy(filter + pos, "All Files");
  pos += strlen(filter + pos) + 1;

  (void) strcpy(filter + pos, "*.*");
  pos += strlen(filter + pos) + 1;

  filter[pos] = '\0';
}

static void
replaceSlash(char *fileName)
{
  int len, i;

  len = strlen(fileName);
  for (i = 0; i < len; i++)
    if (fileName[i] == '\\')
      fileName[i] = '/';
}

static UINT APIENTRY
fileSelHook(HWND w, UINT msgKind, WPARAM wParam,LPARAM lParam)
{
  LPOFNOTIFY notifyP;

  if (msgKind == WM_NOTIFY) {
    notifyP = (LPOFNOTIFY) lParam;
    if (notifyP->hdr.code == CDN_HELP)
      callCB(FileSelInfoP, PU_CT_HELP, LABEL_HELP, NULL);
  }

  return 0;
}

static void
doFileSel(GizmoInfo *infoP)
{  
  OPENFILENAME ofn;
  char filter[200], fileName[_MAX_PATH];
  BOOL res;
  PuFileSelCBStruc fileSelStruc;

  getFilter(filter, infoP->u.fileSel.pattern);
  fileName[0] = '\0';

  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = WinGetMainW();
  ofn.hInstance = NULL;
  ofn.lpstrFilter = filter;
  ofn.lpstrCustomFilter = NULL;
  ofn.nMaxCustFilter = 0;
  ofn.nFilterIndex = 1;
  ofn.lpstrFile = fileName;
  ofn.nMaxFile = sizeof(fileName);
  ofn.lpstrFileTitle = NULL;
  ofn.nMaxFileTitle = 0;
  ofn.lpstrInitialDir = infoP->u.fileSel.dir;
  ofn.lpstrTitle = infoP->name;
  ofn.Flags = OFN_EXPLORER | OFN_SHOWHELP | OFN_HIDEREADONLY |
      OFN_ENABLEHOOK;
  ofn.nFileOffset = 0;
  ofn.nFileExtension = 0;
  ofn.lpstrDefExt = NULL;
  ofn.lCustData = 0;
  ofn.lpfnHook = fileSelHook;
  ofn.lpTemplateName = NULL;

  FileSelInfoP = infoP;

  if (infoP->u.fileSel.acc == PU_FA_READ) {
    ofn.Flags |= OFN_FILEMUSTEXIST;
    res = GetOpenFileName(&ofn);
  } else {
    ofn.Flags |= OFN_PATHMUSTEXIST;
    res = GetSaveFileName(&ofn);
  }

  if (res) {
    fileSelStruc.fileName = ofn.lpstrFile;
    replaceSlash(fileSelStruc.fileName);
    callCB(infoP, PU_CT_OK, LABEL_OK, &fileSelStruc);
  } else {
    callCB(infoP, PU_CT_CANCEL, LABEL_CANCEL, NULL);
  }
}

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
addHistory(GizmoInfo *infoP, char *str)
{
  LINLIST list;
  char *strCpy;

  if (infoP->u.text.historySize == 0)
    return;

  list = infoP->u.text.historyList;
  strCpy = malloc(strlen(str) + 1);
  (void) strcpy(strCpy, str);
  (void) ListInsertLast(list, &strCpy);
  if ((int) ListSize(list) > infoP->u.text.historySize)
    ListRemove(list, ListFirst(list));

  infoP->u.text.historyCurr = NULL;
}

static void
setStr(GizmoInfo *infoP, char *str)
{
  int len;

  SetWindowText(infoP->w, str);
  len = strlen(str);
  SendMessage(infoP->w, EM_SETSEL, (WPARAM) len, (LPARAM) len);
}

static void
upHistory(GizmoInfo *infoP)
{
  LINLIST list;
  char **currP;

  if (infoP->u.text.historySize == 0)
    return;

  list = infoP->u.text.historyList;
  currP = infoP->u.text.historyCurr;
  if (currP == NULL)
    currP = ListLast(list);
  else
    currP = ListPrev(list, currP);

  if (currP == NULL)
    setStr(infoP, "");
  else
    setStr(infoP, *currP);

  infoP->u.text.historyCurr = currP;
}

static void
downHistory(GizmoInfo *infoP)
{
  LINLIST list;
  char **currP;

  if (infoP->u.text.historySize == 0)
    return;

  list = infoP->u.text.historyList;
  currP = infoP->u.text.historyCurr;
  if (currP != NULL)
    currP = ListNext(list, currP);

  if (currP == NULL)
    setStr(infoP, "");
  else
    setStr(infoP, *currP);

  infoP->u.text.historyCurr = currP;
}

static void
textEdit(GizmoInfo *infoP)
{
  PuTextCBStruc puTextStruc;

  if (! TextStrucWinToPu(infoP->w, &infoP->u.text.textS, &puTextStruc))
    return;

  callCB(infoP, PU_CT_MODIFY, infoP->name, &puTextStruc);

  if (puTextStruc.doIt &&
      infoP->u.text.historySize > 0 && infoP->u.text.firstMod) {
    addHistory(infoP, puTextStruc.oldText);
    infoP->u.text.firstMod = FALSE;
  }

  TextStrucPuToWin(infoP->w, &puTextStruc, &infoP->u.text.textS);
}

static BOOL CALLBACK
dialProc(HWND w, UINT msgKind, WPARAM wParam, LPARAM lParam)
{
  int notifyCode, scrollCode, childId;
  GizmoInfo *infoP, *childP, *boxP, *cP;
  HWND childW;
  PuSelectCBStruc selectStruc;
  PuFloatCBStruc floatStruc;
  PuValuatorCBStruc valStruc;
  BOOL onOff;
  int buttonNo, buttonI, scrollPos, lastI, i ;

  infoP = (GizmoInfo *) GetWindowLong(w, GWL_USERDATA);

  switch (msgKind) {
    case WM_COMMAND:
      notifyCode = HIWORD(wParam);
      childId = LOWORD(wParam);
      childW = (HWND) lParam;
      childP = (GizmoInfo *) GetWindowLong(childW, GWL_USERDATA);

      switch (notifyCode) {
	case BN_CLICKED:
	  if (childId < infoP->u.dial.childNo) {
            childP = (GizmoInfo *) GetWindowLong(GetFocus(), GWL_USERDATA);
	    if (childP->type == GIZMO_TOGGLE) {
	      boxP = childP->u.toggle.boxP;
	      if (boxP->type == GIZMO_RADIO_BOX) {
		for (i = 0; i < infoP->u.dial.childNo; i++) {
		  cP = infoP->u.dial.childPA[i];
		  if (cP->type != GIZMO_TOGGLE)
		    continue;
		  if (cP->u.toggle.boxP != boxP)
		    continue;
	          onOff = SendMessage(cP->w, BM_GETCHECK, 0, 0);
		  if (onOff && cP != childP) {
		    cP->u.toggle.onOff = FALSE;
		    SendMessage(cP->w, BM_SETCHECK, FALSE, 0);
	            selectStruc.onOff = FALSE;
	            callCB(boxP, PU_CT_SELECT, cP->name, &selectStruc);
		    break;
		  }
		}
		if (cP != childP) {
		  childP->u.toggle.onOff = TRUE;
		  SendMessage(childP->w, BM_SETCHECK, TRUE, 0);
	          selectStruc.onOff = TRUE;
	          callCB(boxP, PU_CT_SELECT, childP->name, &selectStruc);
		}
	      } else {
	        onOff = ! SendMessage(childP->w, BM_GETCHECK, 0, 0);
		childP->u.toggle.onOff = onOff;
		SendMessage(childP->w, BM_SETCHECK, onOff, 0);
	        selectStruc.onOff = onOff;
	        callCB(boxP, PU_CT_SELECT, childP->name, &selectStruc);
	      }
	    } else {
	      callCB(childP, PU_CT_ACTIVATE, childP->name, NULL);
	      if (childP->type == GIZMO_TEXT_FIELD &&
		  childP->u.text.historySize > 0)
		childP->u.text.firstMod = TRUE;
	    }
	  } else {
	    buttonI = childId - infoP->u.dial.childNo;
	    callCB(infoP, BUTTON_FIRST + buttonI, ButtonLabelA[buttonI], NULL);
	  }
          return TRUE;
	case EN_UPDATE:
	  textEdit(childP);
	  return TRUE;
	case LBN_SELCHANGE:
	  for (i = 0; i < childP->u.list.entryNo; i++)
	    if (SendMessage(childW, LB_GETSEL, i, 0) !=
	        childP->u.list.entryA[i].onOff)
	      lastI = i;

	  for (i = 0; i < childP->u.list.entryNo; i++) {
	    onOff = SendMessage(childW, LB_GETSEL, i, 0);
	    if (onOff != childP->u.list.entryA[i].onOff) {
	      if (! childP->u.list.autoDesel)
                childP->u.list.entryA[i].onOff = onOff;
	      selectStruc.onOff = onOff;
	      selectStruc.last = (i == lastI);
	      callCB(childP, PU_CT_SELECT,
		  childP->u.list.entryA[i].str, &selectStruc);
	    }
	  }

	  if (childP->u.list.autoDesel)
	    SendMessage(childW, LB_SETCURSEL, -1, 0);

	  return TRUE;
      }

      break;
    case WM_VSCROLL:
      childW = (HWND) lParam;
      childP = (GizmoInfo *) GetWindowLong(childW, GWL_USERDATA);
      scrollCode = LOWORD(wParam);

      if (scrollCode == SB_LINEUP) {
	upHistory(childP->u.scroll.textP);
      } else if (scrollCode == SB_LINEDOWN) {
	downHistory(childP->u.scroll.textP);
      } else {
	return FALSE;
      }
      return TRUE;
    case WM_HSCROLL:
      scrollCode = LOWORD(wParam);
      if (scrollCode != SB_THUMBPOSITION && scrollCode != SB_THUMBTRACK)
	return FALSE;

      scrollPos = (int)(short)HIWORD(wParam);
      childW = (HWND) lParam;
      childP = (GizmoInfo *) GetWindowLong(childW, GWL_USERDATA);

      floatStruc.oldVal = childP->u.slider.val;
      childP->u.slider.val = scrollPos / childP->u.slider.factor;
      SetScrollPos(childW, SB_CTL,
	  (int) (childP->u.slider.val * childP->u.slider.factor),
	  TRUE);

      if (childP->u.slider.valuatorCB == NULL) {
	floatStruc.newVal = childP->u.slider.val;
	callCB(childP, PU_CT_MODIFY, childP->name, &floatStruc);
      } else {
        valStruc.act = PU_MA_ACTIVATE;
        valStruc.name = childP->name;
        valStruc.val = childP->u.slider.val;
        childP->u.slider.valuatorCB(childP, childP->u.slider.clientData, &valStruc);
      }

      return TRUE;
    case WM_PAINT:
      for (childId = 0; childId < infoP->u.dial.childNo; childId++) {
        childP = infoP->u.dial.childPA[childId];
	if (childP->type == GIZMO_COLOR_FIELD)
	  paintColor(childP->w,
	      childP->u.color.r, childP->u.color.g, childP->u.color.b);
      }
      break;
    case WM_MOVE:
      if (infoP->u.dial.outside) {
	WinPlaceChanged(infoP->w);
	return TRUE;
      }
      break;
    case WM_CLOSE:
      buttonNo = 0;
      for (i = 0; i < BUTTON_NO; i++)
        if (infoP->u.dial.buttonSwitchA[i])
	  buttonNo++;
      if (buttonNo == 0)
	ShowWindow(infoP->w, SW_HIDE);
      return TRUE;
    case WM_DESTROY:
      WinRemoveDialog(w);
      for (childId = 0; childId < infoP->u.dial.childNo; childId++)
        freeInfo(infoP->u.dial.childPA[childId]);
      freeInfo(infoP);
      return TRUE;
  }

  return FALSE;
}

static void
buildDial(GizmoInfo *infoP)
{
  int childNo, childI;
  int buttonNo, buttonI, buttonOnI;
  int maxLen;
  int xScale, yScale, xSize, ySize, yPos;
  GizmoInfo *childP, *boxP, *textP;
  HWND mainW, dialW, childW;
  HINSTANCE instance;
  int dLen, dSize, sLen;
  char *dP;
  LPDLGTEMPLATE dtP;
  LPDLGITEMTEMPLATE itP;
  LPWORD wP;
  LPWSTR sP;
  WORD classId;
  char *text;
  int colNo, colI, rowI, entryI;

  childNo = infoP->u.dial.childNo;

  buttonNo = 0;
  maxLen = 0;
  for (buttonI = 0; buttonI < BUTTON_NO; buttonI++) {
    if (! infoP->u.dial.buttonSwitchA[buttonI])
      continue;

    if ((int) strlen(ButtonLabelA[buttonI]) > maxLen)
      maxLen = strlen(ButtonLabelA[buttonI]);

    buttonNo++;
  }

  for (childI = 0; childI < childNo; childI++) {
    childP = infoP->u.dial.childPA[childI];

    if (childP->type == GIZMO_RADIO_BOX) {
      colNo = (childP->u.box.entryNo + childP->u.box.lineNo - 1) /
          childP->u.box.lineNo;
	  if (colNo * childP->u.box.entrySize > childP->xSize)
        childP->xSize = colNo * childP->u.box.entrySize;
    }
  }

  if (infoP->u.dial.yBase == 0) {
    xSize = 0;
    ySize = 0;
    for (childI = 0; childI < childNo; childI++) {
      childP = infoP->u.dial.childPA[childI];

      if (childP->xSize > xSize)
	xSize = childP->xSize;

      ySize += childP->ySize;
    }

    if ((2 * buttonNo - 1) * 4 * (maxLen + 1) > xSize)
      xSize = (2 * buttonNo - 1) * 4 * (maxLen + 1);
  } else {
    xScale = 0;
    yScale = 0;
    for (childI = 0; childI < childNo; childI++) {
      childP = infoP->u.dial.childPA[childI];

      if (xScale * childP->constr.w < childP->xSize)
        xScale = childP->xSize / childP->constr.w;
      if (yScale * childP->constr.h < childP->ySize)
        yScale = childP->ySize / childP->constr.h;
    }

    if ((2 * buttonNo - 1) * 4 * (maxLen + 1) > xScale * infoP->u.dial.xBase)
      xScale = (2 * buttonNo - 1) * 4 * (maxLen + 1) / infoP->u.dial.xBase;

    xSize = xScale * infoP->u.dial.xBase;
    ySize = yScale * infoP->u.dial.yBase;
  }

  mainW = WinGetMainW();
  instance = (HINSTANCE) GetWindowLong(mainW, GWL_HINSTANCE);

  dSize = 1024;
  dP = malloc(dSize);
  dLen = 0;

  checkSize(&dP, &dSize, dLen + sizeof(*dtP));
  dtP = (LPDLGTEMPLATE) (dP + dLen);
  dtP->style = WS_POPUP | WS_CAPTION | WS_SYSMENU;
  dtP->dwExtendedStyle = 0;
  dtP->cdit = childNo + buttonNo;
  dtP->x = 10;
  dtP->y = 10;
  dtP->cx = xSize;
  dtP->cy = ySize;
  if (buttonNo > 0)
    dtP->cy += BUTTON_SPACE + BUTTON_H;
  dLen += sizeof(*dtP);

  checkSize(&dP, &dSize, dLen + 2 * sizeof(*wP));
  wP = (LPWORD) (dP + dLen);
  wP[0] = 0;  /* no menu */
  wP[1] = 0;  /* default class */
  dLen += 2 * sizeof(*wP);

  sLen = strlen(infoP->name) + 1;
  checkSize(&dP, &dSize, dLen + sLen * sizeof(*sP));
  sP = (LPWSTR) (dP + dLen);
  (void) MultiByteToWideChar(CP_ACP, 0, infoP->name, sLen, sP, sLen);
  dLen += sLen * sizeof(*sP);

  yPos = 0;
  for (childI = 0; childI < childNo; childI++) {
    childP = infoP->u.dial.childPA[childI];

    alignLen(&dLen);
    checkSize(&dP, &dSize, dLen + sizeof(*itP));
    itP = (LPDLGITEMTEMPLATE) (dP + dLen);
    if (childP->type == GIZMO_TOGGLE) {
      boxP = childP->u.toggle.boxP;
      colNo = (boxP->u.box.entryNo + boxP->u.box.lineNo - 1) /
	  boxP->u.box.lineNo;
      rowI = childP->u.toggle.entryI / colNo;
      colI = childP->u.toggle.entryI % colNo;
      if (infoP->u.dial.yBase == 0) {
        itP->cx = (xSize - 8) / colNo;
        itP->x = 4 + colI * itP->cx;
        itP->cy = (boxP->ySize - 16) / boxP->u.box.lineNo;
        itP->y = 12 + boxP->constr.y + rowI * itP->cy;
      } else if (boxP->name[0] == '\0') {
        itP->cx = (xScale * boxP->constr.w - 8) / colNo;
        itP->x = 4 + xScale * boxP->constr.x + colI * itP->cx;
        itP->cy = yScale * boxP->constr.h / boxP->u.box.lineNo;
        itP->y = yScale * boxP->constr.y + rowI * itP->cy;
      } else {
        itP->cx = (xScale * boxP->constr.w - 8) / colNo;
        itP->x = 4 + xScale * boxP->constr.x + colI * itP->cx;
        itP->cy = (yScale * boxP->constr.h - 16) / boxP->u.box.lineNo;
        itP->y = 12 + yScale * boxP->constr.y + rowI * itP->cy;
      }
    } else if (childP->type == GIZMO_SCROLL) {
      textP = childP->u.scroll.textP;
      itP->x = xScale * (textP->constr.x + textP->constr.w) - 8;
      itP->y = yScale * textP->constr.y;
      itP->cx = 8;
      itP->cy = yScale * textP->constr.h;
    } else if (infoP->u.dial.yBase == 0) {
      itP->x = 0;
      itP->y = yPos;
      itP->cx = xSize;
      itP->cy = childP->ySize;
      childP->constr.y = yPos;
      yPos += childP->ySize;
    } else {
      itP->x = xScale * childP->constr.x;
      itP->y = yScale * childP->constr.y;
      itP->cx = xScale * childP->constr.w;
      itP->cy = yScale * childP->constr.h;
    }
    itP->id = childI;
    itP->style = WS_CHILD | WS_VISIBLE;
    itP->dwExtendedStyle = 0;
    dLen += sizeof(*itP);

    switch (childP->type) {
      case GIZMO_LABEL:
        classId = 0x0082;
	text = childP->name;
	break;
      case GIZMO_BUTTON:
        classId = 0x0080;
	text = childP->name;
	itP->style |= WS_TABSTOP | WS_GROUP;
	break;
      case GIZMO_COLOR_FIELD:
	classId = 0x0082;
	text = NULL;
	break;
      case GIZMO_SLIDER:
	classId = 0x0084;
	itP->style |= WS_BORDER;
	text = NULL;
	break;
      case GIZMO_TEXT_FIELD:
        classId = 0x0081;
	if (childP->u.text.historySize > 0)
	  itP->cx -= 8;
	itP->style |= WS_BORDER | WS_TABSTOP | WS_GROUP | ES_AUTOHSCROLL;
	text = NULL;
	break;
      case GIZMO_TEXT:
        classId = 0x0081;
	itP->style |= WS_BORDER | ES_MULTILINE;
	text = NULL;
	break;
      case GIZMO_SCROLL:
        classId = 0x0084;
	itP->style |= SBS_VERT;
	text = NULL;
	break;
      case GIZMO_LIST:
	classId = 0x0083;
	itP->style |= WS_BORDER | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT;
	if (! childP->u.list.autoDesel)
	  itP->style |= WS_TABSTOP | WS_GROUP;
	if (childP->u.list.multSel)
	  itP->style |= LBS_MULTIPLESEL | LBS_EXTENDEDSEL;
	text = NULL;
	break;
      case GIZMO_RADIO_BOX:
	classId = 0x0080;
	itP->style |= BS_GROUPBOX;
	text = childP->name;
	break;
      case GIZMO_CHECK_BOX:
	classId = 0x0080;
	itP->style |= BS_GROUPBOX;
	text = childP->name;
	break;
      case GIZMO_TOGGLE:
        classId = 0x0080;
	if (childP->u.toggle.boxP->type == GIZMO_RADIO_BOX)
	  itP->style |= BS_RADIOBUTTON;
	else
	  itP->style |= BS_CHECKBOX;
	if (childP->u.toggle.entryI == 0)
	  itP->style |= WS_TABSTOP | WS_GROUP;
	text = childP->name;
	break;
    }

    checkSize(&dP, &dSize, dLen + 2 * sizeof(*wP));
    wP = (LPWORD) (dP + dLen);
    wP[0] = 0xFFFF;
    wP[1] = classId;
    dLen += 2 * sizeof(*wP);

    if (text == NULL) {
      checkSize(&dP, &dSize, dLen + sizeof(*wP));
      wP = (LPWORD) (dP + dLen);
      wP[0] = 0;
      dLen += sizeof(*wP);
    } else {
      sLen = strlen(text) + 1;
      checkSize(&dP, &dSize, dLen + sLen * sizeof(*sP));
      sP = (LPWSTR) (dP + dLen);
      (void) MultiByteToWideChar(CP_ACP, 0, text, sLen, sP, sLen);
      dLen += sLen * sizeof(*sP);
    }

    checkSize(&dP, &dSize, dLen + sizeof(*wP));
    wP = (LPWORD) (dP + dLen);
    wP[0] = 0;  /* no creation data */
    dLen += sizeof(*wP);
  }

  buttonOnI = 0;
  for (buttonI = 0; buttonI < BUTTON_NO; buttonI++) {
    if (! infoP->u.dial.buttonSwitchA[buttonI])
      continue;

    alignLen(&dLen);
    checkSize(&dP, &dSize, dLen + sizeof(*itP));
    itP = (LPDLGITEMTEMPLATE) (dP + dLen);
    itP->x = 2 * buttonOnI * xSize / (2 * buttonNo - 1);
    itP->y = ySize + BUTTON_SPACE;
    itP->cx = xSize / (2 * buttonNo - 1);
    itP->cy = BUTTON_H;
    itP->id = childNo + buttonI;
    itP->style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_GROUP;
    itP->dwExtendedStyle = 0;
    dLen += sizeof(*itP);

    checkSize(&dP, &dSize, dLen + 2 * sizeof(*wP));
    wP = (LPWORD) (dP + dLen);
    wP[0] = 0xFFFF;
    wP[1] = 0x0080;  /* button */
    dLen += 2 * sizeof(*wP);

    text = ButtonLabelA[buttonI];
    sLen = strlen(text) + 1;
    checkSize(&dP, &dSize, dLen + sLen * sizeof(*sP));
    sP = (LPWSTR) (dP + dLen);
    (void) MultiByteToWideChar(CP_ACP, 0, text, sLen, sP, sLen);
    dLen += sLen * sizeof(*sP);

    checkSize(&dP, &dSize, dLen + sizeof(*wP));
    wP = (LPWORD) (dP + dLen);
    wP[0] = 0;  /* no creation data */
    dLen += 1 * sizeof(*wP);

    buttonOnI++;
  }

  dialW = CreateDialogIndirect(instance,
      (LPDLGTEMPLATE) dP, mainW, dialProc);

  WinAddDialog(dialW);
  (void) SetWindowLong(dialW, GWL_USERDATA, (LONG) infoP);
  infoP->w = dialW;

  for (childI = 0; childI < childNo; childI++) {
    childP = infoP->u.dial.childPA[childI];
    childW = GetDlgItem(dialW, childI);

    childP->w = childW;
    (void) SetWindowLong(childW, GWL_USERDATA, (LONG) childP);

    if (childP->type == GIZMO_SLIDER) {
      SetScrollRange(childW, SB_CTL,
	  (int) (childP->u.slider.minVal * childP->u.slider.factor),
	  (int) (childP->u.slider.maxVal * childP->u.slider.factor),
	  FALSE);
      SetScrollPos(childW, SB_CTL,
	  (int) (childP->u.slider.val * childP->u.slider.factor),
	  FALSE);
    } else if (childP->type == GIZMO_TEXT_FIELD || childP->type == GIZMO_TEXT) {
      SetWindowText(childW, childP->u.text.str);
    } else if (childP->type == GIZMO_LIST) {
      for (entryI = 0; entryI < childP->u.list.entryNo; entryI++) {
        SendMessage(childW, LB_ADDSTRING, 0,
	    (LPARAM) childP->u.list.entryA[entryI].str);
        if (childP->u.list.entryA[entryI].onOff)
          SendMessage(childW, LB_SETSEL, 1, entryI);
      }
     if (childP->u.list.showBottom)
	SendMessage(infoP->w, LB_SETTOPINDEX, childP->u.list.entryNo - 1, 0);
    } else if (childP->type == GIZMO_TOGGLE) {
      SendMessage(childP->w, BM_SETCHECK, childP->u.toggle.onOff, 0);
    }
  }

  WinPlace(dialW, infoP->u.dial.outside);
  ShowWindow(dialW, SW_SHOW);
  UpdateWindow(dialW);

  free(dP);
}

void
PuWinSwitchGizmo(PuGizmo gizmo, BOOL onOff)
{
  GizmoInfo *infoP = (GizmoInfo *) gizmo;

  if (infoP->w != NULL) {
    if (onOff) {
      ShowWindow(infoP->w, SW_SHOW);
      BringWindowToTop(infoP->w);
    } else {
      ShowWindow(infoP->w, SW_HIDE);
    }
    return;
  }

  if (! onOff)
    return;

  if (infoP->type == GIZMO_FILE_SELECTION_BOX) {
    doFileSel(infoP);
    freeInfo(infoP);
  } else if (infoP->type == GIZMO_DIALOG || infoP->type == GIZMO_VALUATOR_BOX) {
    buildDial(infoP);
  }
}

void
PuWinDestroyGizmo(PuGizmo gizmo)
{
  GizmoInfo *infoP = (GizmoInfo *) gizmo;

  /* is NULL for file selection dialog */
  if (infoP->w != NULL)
    DestroyWindow(infoP->w);
}

void
PuWinAddGizmoCB(PuGizmo gizmo, PuGizmoCBType type,
    PuGizmoCB gizmoCB, void *clientData, PuFreeCB freeCB)
{
  GizmoInfo *infoP = (GizmoInfo *) gizmo;
  GizmoCBInfo cb;
  int buttonI;

  cb.type = type;
  cb.gizmoCB = gizmoCB;
  cb.clientData = clientData;
  cb.freeCB = freeCB;

  (void) ListInsertLast(infoP->cbList, &cb);

  buttonI = type - BUTTON_FIRST;
  if (buttonI >= 0 && buttonI < BUTTON_NO)
    infoP->u.dial.buttonSwitchA[buttonI] = TRUE;
}
