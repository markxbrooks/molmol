/*
************************************************************************
*
*   MotifMainW.c - Motif main window
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/motif/SCCS/s.MotifMainW.c
*   SCCS identification       : 1.29
*
************************************************************************
*/

#include "motif_main_w.h"

#include <stdio.h>
#include <stdlib.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <Xm/Xm.h>
#include <Xm/MainW.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/TextF.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>

#include <break.h>
#include <linlist.h>
#include <pu_cb.h>
#include "motif_access.h"
#include "motif_text.h"

#include "icon.xbm"
#include "mask.xbm"

#define HISTORY_SIZE 100

typedef struct {
  char *name;
  PuCmdFieldCB cmdFieldCB;
  void *clientData;
  PuFreeCB freeCB;
} CmdFieldCBInfo;

static Widget TopW, MainW;
static Widget FormW, FrameW, DrawW, PopupParentW, MenubarW, PopupW;
static Widget CmdFieldsW, PromptW, CmdW, StatusW, BreakW;
static BOOL PromptOnOff, CmdOnOff, StatusOnOff, CmdFieldsOnOff;
static int CmdFieldNo;

static BOOL CursorSet = FALSE;
static BOOL SuppressVerify = FALSE;

static LINLIST HistoryList = NULL;
static char **HistoryCurr;

static Bool
checkExpose(Display *dpy, XEvent *eventP, XPointer arg)
{
  Window *winP = (Window *) arg;

  return eventP->type == Expose && eventP->xexpose.window == *winP;
}

static Bool
checkPressOrExpose(Display *dpy, XEvent *eventP, XPointer arg)
{
  Window *winP = (Window *) arg;

  return (eventP->type == ButtonPress && eventP->xbutton.window == *winP) ||
      (eventP->type == Expose && eventP->xexpose.window == *winP);
}

static void
activateBreak(BOOL onOff)
{
  Display *dpy;
  Window win;
  Cursor curs;
  XEvent event;

  if (onOff)
    XtVaSetValues(BreakW,
	XmNsensitive, True,
	NULL);
  else
    XtVaSetValues(BreakW,
	XmNsensitive, False,
	NULL);

  dpy = XtDisplay(BreakW);
  win = XtWindow(BreakW);
  if (win == 0)  /* should not normally happen */
    return;

  if (! CursorSet) {
    /* no busy cursor in break button! */
    curs = XCreateFontCursor(dpy, XC_top_left_arrow);
    XDefineCursor(dpy, win, curs);
    CursorSet = TRUE;
  }

  XSync(dpy, False);
  if (XCheckIfEvent(dpy, &event, checkExpose, (XPointer) &win)) {
    XtDispatchEvent(&event);
    XFlush(dpy);
  }
}

static BOOL
checkBreak(void)
{
  Display *dpy;
  Window win;
  XEvent event;

  dpy = XtDisplay(BreakW);
  win = XtWindow(BreakW);
  /* this should normally not be called before the widget is
     mapped, but make sure that it doesn't crash */
  if (win == 0)
    return FALSE;

  if (XCheckIfEvent(dpy, &event, checkPressOrExpose, (XPointer) &win)) {
    if (event.type == Expose) {
      XtDispatchEvent(&event);
    } else {
      MotifSetFocus();
      return TRUE;
    }
  }

  return FALSE;
}

static void
registerBreak(void)
{
  XtVaSetValues(BreakW,
      XmNsensitive, False,
      NULL);

  BreakSetActivate(activateBreak);
  BreakSetCheck(checkBreak);
}

static void
setBottom(Widget w, Widget wAtt)
{
  if (wAtt == NULL)
    XtVaSetValues(w,
	XmNbottomAttachment, XmATTACH_FORM,
	NULL);
  else
    XtVaSetValues(w,
	XmNbottomAttachment, XmATTACH_WIDGET,
	XmNbottomWidget, wAtt,
	NULL);
}

static void
makeLayout(void)
{
  Widget bottomW = NULL;
  Widget rightW = NULL;

  XtUnmanageChild(FormW);
  XtUnmanageChild(FrameW);
  if (StatusW != NULL) {
    XtUnmanageChild(StatusW);
    XtUnmanageChild(BreakW);
  }
  if (CmdW != NULL)
    XtUnmanageChild(CmdW);
  if (PromptW != NULL)
    XtUnmanageChild(PromptW);
  if (CmdFieldsW != NULL)
    XtUnmanageChild(CmdFieldsW);

  if (StatusOnOff) {
    XtVaSetValues(BreakW,
	XmNleftAttachment, XmATTACH_NONE,
	XmNrightAttachment, XmATTACH_FORM,
	XmNtopAttachment, XmATTACH_NONE,
	XmNbottomAttachment, XmATTACH_FORM,
	NULL);
    XtVaSetValues(StatusW,
	XmNleftAttachment, XmATTACH_FORM,
	XmNrightAttachment, XmATTACH_WIDGET,
	XmNrightWidget, BreakW,
	XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET,
	XmNtopWidget, BreakW,
	XmNbottomAttachment, XmATTACH_FORM,
	NULL);
    bottomW = BreakW;
    XtManageChild(BreakW);
    XtManageChild(StatusW);
  }

  if (CmdOnOff) {
    if (PromptOnOff) {
      XtVaSetValues(PromptW,
	  XmNleftAttachment, XmATTACH_FORM,
	  XmNrightAttachment, XmATTACH_NONE,
	  XmNtopAttachment, XmATTACH_NONE,
	  NULL);
      setBottom(PromptW, bottomW);
      XtVaSetValues(CmdW,
	  XmNleftAttachment, XmATTACH_WIDGET,
	  XmNleftWidget, PromptW,
	  NULL);
      XtManageChild(PromptW);
    } else {
      XtVaSetValues(CmdW,
	  XmNleftAttachment, XmATTACH_FORM,
	  NULL);
    }

    XtVaSetValues(CmdW,
	XmNrightAttachment, XmATTACH_FORM,
	XmNtopAttachment, XmATTACH_NONE,
	NULL);
    setBottom(CmdW, bottomW);
    bottomW = CmdW;
    XtManageChild(CmdW);
  }

  if (CmdFieldsOnOff && CmdFieldsW != NULL) {
    XtVaSetValues(CmdFieldsW,
	XmNleftAttachment, XmATTACH_NONE,
	XmNrightAttachment, XmATTACH_FORM,
	XmNtopAttachment, XmATTACH_FORM,
	NULL);
    setBottom(CmdFieldsW, bottomW);
    rightW = CmdFieldsW;
    XtManageChild(CmdFieldsW);
  }

  XtVaSetValues(FrameW,
      XmNleftAttachment, XmATTACH_FORM,
      XmNtopAttachment, XmATTACH_FORM,
      NULL);

  if (rightW == NULL)
    XtVaSetValues(FrameW,
	XmNrightAttachment, XmATTACH_FORM,
	NULL);
  else
    XtVaSetValues(FrameW,
	XmNrightAttachment, XmATTACH_WIDGET,
	XmNrightWidget, rightW,
	NULL);

  setBottom(FrameW, bottomW);

  XtManageChild(FrameW);
  XtManageChild(FormW);

  MotifSetFocus();
}

static void
addIcon(void)
{
  Display *dpy = XtDisplay(TopW);
  int screen = XScreenNumberOfScreen(XtScreen(TopW));
  Pixmap pix;

  pix = XCreateBitmapFromData(dpy, RootWindow(dpy, screen),
      icon_bits, icon_width, icon_height);
  XtVaSetValues(TopW,
      XmNiconPixmap, pix,
      NULL);

  pix = XCreateBitmapFromData(dpy, RootWindow(dpy, screen),
      mask_bits, mask_width, mask_height);
  XtVaSetValues(TopW,
      XmNiconMask, pix,
      NULL);
}

void
MotifSetTopW(Widget w)
{
  TopW = w;
  addIcon();
}

Widget
MotifGetTopW(void)
{
  return TopW;
}

Widget
MotifGetMainW(void)
{
  if (MainW == NULL) {
    MainW = XmCreateMainWindow(TopW, "main", NULL, 0);
    FormW = XmCreateForm(MainW, "form", NULL, 0);
    FrameW = XmCreateFrame(FormW, "frame", NULL, 0);
    XtVaSetValues(FrameW, XmNtraversalOn, FALSE, NULL);
    XmMainWindowSetAreas(MainW, NULL, NULL, NULL, NULL, FormW);
    makeLayout();
    XtManageChild(FrameW);
    XtManageChild(FormW);
    XtManageChild(MainW);
  }
  return MainW;
}

void
MotifSetDrawW(Widget w)
{
  DrawW = w;
}

Widget
MotifGetDrawW(void)
{
  return DrawW;
}

void
MotifSetPopupParentW(Widget w)
{
  PopupParentW = w;
}
 
Widget
MotifGetPopupParentW(void)
{
  return PopupParentW;
}

Widget
MotifGetDrawParentW(void)
{
  return FrameW;
}

void
MotifSetMenubarW(Widget w)
{
  MenubarW = w;
  XtUnmanageChild(MainW);
  XmMainWindowSetAreas(MainW, MenubarW, NULL, NULL, NULL, FormW);
  XtManageChild(MainW);
}

void
MotifSetPopupW(Widget w)
{
  PopupW = w;
}

Widget
MotifGetPopupW(void)
{
  return PopupW;
}

typedef struct {
  Window win;
  int w, h;
} WinInfo;

void
PuMotifSwitchFullscreen(BOOL onOff)
{
  Display *dpy = XtDisplay(TopW);
  static int winNo;
  static WinInfo winInfo[7];
  static int xTop, yTop;
  int wScreen, hScreen;
  Window rootWin, topWin, drawWin;
  Window parent, child, *children;
  unsigned int nchildren;
  int xDraw, yDraw;
  int wInc, hInc;
  XWindowAttributes getAttr;
  XSetWindowAttributes setAttr;
  int i;

  if (onOff) {
    wScreen = WidthOfScreen(XtScreen(TopW));
    hScreen = HeightOfScreen(XtScreen(TopW));

    drawWin = XtWindow(DrawW);

    topWin = XtWindow(TopW);
    winNo = 0;
    setAttr.override_redirect = True;
    for (;;) {
      XChangeWindowAttributes(dpy, topWin, CWOverrideRedirect, &setAttr);

      XGetWindowAttributes(dpy, topWin, &getAttr);
      winInfo[winNo].win = topWin;
      winInfo[winNo].w = getAttr.width;
      winInfo[winNo].h = getAttr.height;

      winNo++;

      XQueryTree(dpy, topWin, &rootWin, &parent, &children, &nchildren);
      if (children != NULL)
	XFree((void *) children);
      if (parent == rootWin)
	break;
      topWin = parent;  /* reparented */
    }

    XGetWindowAttributes(dpy, drawWin, &getAttr);
    wInc = wScreen - getAttr.width;
    hInc = hScreen - getAttr.height;

    /* use Xlib functions to get around window manager */
    XGetWindowAttributes(dpy, topWin, &getAttr);
    xTop = getAttr.x;
    yTop = getAttr.y;
    XTranslateCoordinates(dpy, drawWin, rootWin, 0, 0,
	&xDraw, &yDraw, &child);
    setAttr.override_redirect = True;
    XChangeWindowAttributes(dpy, topWin, CWOverrideRedirect, &setAttr);
    XMoveWindow(dpy, topWin, xTop - xDraw, yTop - yDraw);

    for (i = winNo - 1; i >= 0; i--)
      XResizeWindow(dpy, winInfo[i].win,
	  winInfo[i].w + wInc, winInfo[i].h + hInc);

    XRaiseWindow(dpy, topWin);
  } else {
    topWin = winInfo[winNo - 1].win;

    for (i = 0; i < winNo; i++)
      XResizeWindow(dpy, winInfo[i].win,
	  winInfo[i].w, winInfo[i].h);

    XMoveWindow(dpy, topWin, xTop, yTop);

    setAttr.override_redirect = False;
    for (i = 0; i < winNo; i++)
      XChangeWindowAttributes(dpy, winInfo[i].win,
	  CWOverrideRedirect, &setAttr);

    XLowerWindow(dpy, topWin);
  }
}

void
PuMotifSetDrawSize(int w, int h)
{
  Dimension wCurr, hCurr;

  XtVaGetValues(DrawW,
      XmNwidth, &wCurr,
      XmNheight, &hCurr,
      NULL);
  if (w == wCurr && h == hCurr)
    /* resize unnecessary, and causes weird problems */
    return;

  XtVaSetValues(TopW,
      XmNallowShellResize, True,
      NULL);
  XtVaSetValues(DrawW,
      XmNwidth, (Dimension) w,
      XmNheight, (Dimension) h,
      NULL);
  if (CmdFieldsOnOff)
    /* these guys take the chance to get too high if we don't prevent it */
    XtVaSetValues(CmdFieldsW,
      XmNheight, (Dimension) h,
      NULL);
  XtVaSetValues(TopW,
      XmNallowShellResize, False,
      NULL);

  MotifSetFocus();
}

void
PuMotifSetTextField(PuTextFieldChoice fieldChoice, char *text)
{
  XmString str;

  switch (fieldChoice) {
    case PU_TF_TITLE:
      XtVaSetValues(MotifGetTopW(), XmNtitle, text, NULL);
      break;
    case PU_TF_PROMPT:
      if (PromptW != NULL) {
	str = XmStringCreateSimple(text);
	XtVaSetValues(PromptW, XmNlabelString, str, NULL);
	XmStringFree(str);
      }
      break;
    case PU_TF_CMD:
      /* cannot be set */
      break;
    case PU_TF_STATUS:
      if (StatusW != NULL) {
	str = XmStringCreateSimple(text);
	XtVaSetValues(StatusW, XmNlabelString, str, NULL);
	XmStringFree(str);
      }
      break;
  }
}

static void
cmdVerifyCB(Widget w, XtPointer clientData, XtPointer callData)
{
  PuTextCBStruc textStruc;

  if (SuppressVerify) {
    SuppressVerify = FALSE;
    return;
  }

  textStruc.oldText = XmTextGetString(w);

  if (TextStrucMotifToPu(callData, &textStruc)) {
    PuCallCmdCB((PuWindow) MotifGetDrawW(), &textStruc);
    TextStrucPuToMotif(&textStruc, callData);
  }

  XtFree(textStruc.oldText);
}

static void
cmdActivateCB(Widget w, XtPointer clientData, XtPointer callData)
{
  PuTextCBStruc textStruc;
  char *str, **strP;

  textStruc.action = PU_CA_CR;
  textStruc.oldText = XmTextGetString(w);
  textStruc.oldLen = strlen(textStruc.oldText);
  textStruc.oldPart = NULL;
  textStruc.oldPartStart = 0;
  textStruc.oldPartEnd = 0;
  textStruc.newText = XmTextGetString(w);
  textStruc.newLen = textStruc.oldLen;
  textStruc.newPart = NULL;
  textStruc.newPartStart = 0;
  textStruc.newPartEnd = 0;
  textStruc.doIt = TRUE;

  PuCallCmdCB((PuWindow) MotifGetDrawW(), &textStruc);

  if (textStruc.doIt && textStruc.newLen > 0) {
    SuppressVerify = TRUE;
    XtVaSetValues(w, XmNvalue, "", NULL);

    if (HistoryList == NULL)
      HistoryList = ListOpen(sizeof(char *));

    str = malloc(textStruc.newLen + 1);
    (void) strcpy(str, textStruc.newText);
    (void) ListInsertLast(HistoryList, &str);
    if (ListSize(HistoryList) > HISTORY_SIZE) {
      strP = ListFirst(HistoryList);
      free(*strP);
      ListRemove(HistoryList, strP);
    }

    HistoryCurr = NULL;
  }

  XtFree(textStruc.oldText);
  XtFree(textStruc.newText);
}

static void
setCmd(char *newCmd)
{
  if (newCmd[0] != '\0')
    XtVaSetValues(CmdW, XmNvalue, "", NULL);

  XtVaSetValues(CmdW, XmNvalue, newCmd, NULL);
  XmTextSetInsertionPosition(CmdW, (int) strlen(newCmd));
}

static void
keyEH(Widget w, XtPointer clientData, XEvent *evtP, Boolean *ctd)
{
  char buf[10];
  KeySym sym;

  (void) XLookupString(&evtP->xkey, buf, sizeof(buf), &sym, NULL);
  if (sym == XK_Up) {
    if (HistoryCurr == NULL)
      HistoryCurr = ListLast(HistoryList);
    else
      HistoryCurr = ListPrev(HistoryList, HistoryCurr);

    if (HistoryCurr == NULL)
      setCmd("");
    else
      setCmd(*HistoryCurr);
  } else if (sym == XK_Down) {
    if (HistoryCurr == NULL)
      HistoryCurr = ListFirst(HistoryList);
    else
      HistoryCurr = ListNext(HistoryList, HistoryCurr);

    if (HistoryCurr == NULL)
      setCmd("");
    else
      setCmd(*HistoryCurr);
  }
}

void
PuMotifSwitchTextField(PuTextFieldChoice fieldChoice, BOOL onOff)
{
  XmString str;

  switch (fieldChoice) {
    case PU_TF_TITLE:
      /* always on */
      break;
    case PU_TF_PROMPT:
      PromptOnOff = onOff;
      if (onOff && PromptW == NULL) {
        PromptW = XmCreateLabel(FormW, "Prompt", NULL, 0);
	XtVaSetValues(PromptW,
	    XmNmarginHeight, 10,
	    NULL);
      }
      makeLayout();
      break;
    case PU_TF_CMD:
      CmdOnOff = onOff;
      if (onOff && CmdW == NULL) {
        CmdW = XmCreateText(FormW, "CmdLine", NULL, 0);
	XtAddCallback(CmdW, XmNmodifyVerifyCallback, cmdVerifyCB, NULL);
	XtAddCallback(CmdW, XmNvalueChangedCallback, TextValueChangedCB, NULL);
	XtAddCallback(CmdW, XmNactivateCallback, cmdActivateCB, NULL);
	XtAddEventHandler(CmdW, KeyReleaseMask, False, keyEH, NULL);
      }
      makeLayout();
      break;
    case PU_TF_STATUS:
      StatusOnOff = onOff;
      if (onOff && StatusW == NULL) {
        StatusW = XmCreateLabel(FormW, "StatusLine", NULL, 0);
	str = XmStringCreateSimple(" ");
	XtVaSetValues(StatusW,
	    XmNalignment, XmALIGNMENT_BEGINNING,
	    XmNborderWidth, 2,
            XmNlabelString, str,
	    NULL);
	XmStringFree(str);
	BreakW = XmCreatePushButton(FormW, "BREAK", NULL, 0);
	registerBreak();
      }
      makeLayout();
      break;
  }
}

static void
cmdFieldEnterEH(Widget w, XtPointer clientData, XEvent *evtP, Boolean *ctd)
{
  CmdFieldCBInfo *infoP = clientData;
  PuCmdFieldCBStruc cmdFieldStruc;

  cmdFieldStruc.act = PU_MA_ENTER;
  cmdFieldStruc.name = infoP->name;
  infoP->cmdFieldCB(w, infoP->clientData, &cmdFieldStruc);
}

static void
cmdFieldLeaveEH(Widget w, XtPointer clientData, XEvent *evtP, Boolean *ctd)
{
  CmdFieldCBInfo *infoP = clientData;
  PuCmdFieldCBStruc cmdFieldStruc;

  cmdFieldStruc.act = PU_MA_LEAVE;
  cmdFieldStruc.name = infoP->name;
  infoP->cmdFieldCB(w, infoP->clientData, &cmdFieldStruc);
}

static void
cmdFieldActCB(Widget w, XtPointer clientData, XtPointer callData)
{
  CmdFieldCBInfo *infoP = clientData;
  PuCmdFieldCBStruc cmdFieldStruc;

  cmdFieldStruc.act = PU_MA_ACTIVATE;
  cmdFieldStruc.name = infoP->name;
  infoP->cmdFieldCB(w, infoP->clientData, &cmdFieldStruc);
  MotifSetFocus();
}

static void
cmdFieldDestroyCB(Widget w, XtPointer clientData, XtPointer callData)
{
  CmdFieldCBInfo *infoP = clientData;

  if (infoP->freeCB != NULL)
    infoP->freeCB(infoP->clientData);
  free(infoP->name);
  free(infoP);
}

PuCmdField
PuMotifCreateCmdField(char *name,
    PuCmdFieldCB cmdFieldCB, void *clientData, PuFreeCB freeCB)
{
  Widget buttonW;
  XmString str;
  CmdFieldCBInfo *infoP;

  if (CmdFieldsW == NULL) {
    CmdFieldsW = XmCreateForm(FormW, "CmdFieldsForm", NULL, 0);
    /* must be managed here, otherwise it doesn't work for Motif 1.2 !?! */
    XtManageChild(CmdFieldsW);
    CmdFieldNo = 0;
  }

  CmdFieldNo++;

  XtVaSetValues(CmdFieldsW,
      XmNfractionBase, CmdFieldNo,
      NULL);

  if (cmdFieldCB != NULL) {
    buttonW = XmCreatePushButton(CmdFieldsW, "CmdField", NULL, 0);
    str = XmStringCreateSimple(name);
    XtVaSetValues(buttonW,
	XmNlabelString, str,
	XmNleftAttachment, XmATTACH_FORM,
	XmNrightAttachment, XmATTACH_FORM,
	XmNtopAttachment, XmATTACH_POSITION,
	XmNtopPosition, CmdFieldNo - 1,
	XmNbottomAttachment, XmATTACH_POSITION,
	XmNbottomPosition, CmdFieldNo,
	XmNhighlightThickness, 1,
	NULL);
    XmStringFree(str);

    infoP = malloc(sizeof(*infoP));
    infoP->name = malloc(strlen(name) + 1);
    (void) strcpy(infoP->name, name);
    infoP->cmdFieldCB = cmdFieldCB;
    infoP->clientData = clientData;
    infoP->freeCB = freeCB;
    XtAddEventHandler(buttonW, EnterWindowMask, False, cmdFieldEnterEH, infoP);
    XtAddEventHandler(buttonW, LeaveWindowMask, False, cmdFieldLeaveEH, infoP);
    XtAddCallback(buttonW, XmNactivateCallback, cmdFieldActCB, infoP);
    XtAddCallback(buttonW, XmNdestroyCallback, cmdFieldDestroyCB, infoP);

    XtManageChild(buttonW);

    return buttonW;
  } else {
    return NULL;
  }
}

void
PuMotifSwitchCmdFields(BOOL onOff)
{
  CmdFieldsOnOff = onOff;
  makeLayout();
}

static Boolean
setFocus(XtPointer clientData)
{
  Widget w = clientData;

  XmProcessTraversal(w, XmTRAVERSE_CURRENT);

  return True;
}

void
MotifSetFocus(void)
{
  if (CmdW != NULL)
    (void) XtAppAddWorkProc(MotifGetAppContext(), setFocus, CmdW);
}
