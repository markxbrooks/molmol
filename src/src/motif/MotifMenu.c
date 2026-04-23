/*
************************************************************************
*
*   MotifMenu.c - Motif menus
*
*   Copyright (c) 1994-95
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
*   Date of last modification : 95/07/20
*   Pathname of SCCS file     : /sgiext/molmol/src/motif/SCCS/s.MotifMenu.c
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include "motif_menu.h"

#include <string.h>
#include <stdlib.h>

#include <Xm/RowColumn.h>
#include <Xm/CascadeB.h>
#include <Xm/PushB.h>
#include <Xm/MainW.h>

#include "motif_access.h"

typedef struct {
  char *name;
  PuMenuCB menuCB;
  void *clientData;
  PuFreeCB freeCB;
} MenuCBInfo;

static void
menuEnterEH(Widget w, XtPointer clientData, XEvent *evtP, Boolean *ctd)
{
  MenuCBInfo *infoP = clientData;
  PuMenuCBStruc menuStruc;

  menuStruc.act = PU_MA_ENTER;
  menuStruc.name = infoP->name;
  infoP->menuCB(w, infoP->clientData, &menuStruc);
}

static void
menuLeaveEH(Widget w, XtPointer clientData, XEvent *evtP, Boolean *ctd)
{
  MenuCBInfo *infoP = clientData;
  PuMenuCBStruc menuStruc;

  menuStruc.act = PU_MA_LEAVE;
  menuStruc.name = infoP->name;
  infoP->menuCB(w, infoP->clientData, &menuStruc);
}

static void
menuActCB(Widget w, XtPointer clientData, XtPointer callData)
{
  MenuCBInfo *infoP = clientData;
  PuMenuCBStruc menuStruc;

  menuStruc.act = PU_MA_ACTIVATE;
  menuStruc.name = infoP->name;
  infoP->menuCB(w, infoP->clientData, &menuStruc);
}

static void
destroyCB(Widget w, XtPointer clientData, XtPointer callData)
{
  MenuCBInfo *infoP = clientData;

  if (infoP->freeCB != NULL)
    infoP->freeCB(infoP->clientData);
  free(infoP->name);
  free(infoP);
}

PuMenubar
PuMotifCreateMenubar(void)
{
  return XmCreateMenuBar(MotifGetMainW(), "Menubar", NULL, 0);
}

PuMenu
PuMotifCreatePulldown(PuMenubar menubar, char *name, char mnemonic)
{
  Widget pullDownW, cascButtW;
  XmString labelStr;

  pullDownW = XmCreatePulldownMenu(menubar, "PullDown", NULL, 0);

  cascButtW = XmCreateCascadeButton(menubar, "Cascade", NULL, 0);
  labelStr = XmStringCreateSimple(name);
  XtVaSetValues(cascButtW,
      XmNlabelString, labelStr,
      XmNsubMenuId, pullDownW,
      NULL);
  XmStringFree(labelStr);

  if (mnemonic != '\0')
    XtVaSetValues(cascButtW, XmNmnemonic, mnemonic, NULL);

  if (strcmp(name, "Help") == 0)
    XtVaSetValues(menubar, XmNmenuHelpWidget, cascButtW, NULL);

  XtManageChild(cascButtW);

  return pullDownW;
}

PuMenu
PuMotifCreatePopup(char *name)
{
  Widget popupW;

  popupW = XmCreatePopupMenu(MotifGetPopupParentW(), "Popup", NULL, 0);

  return popupW;
}

PuMenu
PuMotifCreatePullright(PuMenu menu, char *name, char mnemonic)
{
  Widget pullDownW, cascButtW;
  XmString labelStr;

  pullDownW = XmCreatePulldownMenu(menu, "PullDown", NULL, 0);

  labelStr = XmStringCreateSimple(name);
  cascButtW = XmCreateCascadeButton(menu, "Cascade", NULL, 0);
  XtVaSetValues(cascButtW,
      XmNlabelString, labelStr,
      XmNsubMenuId, pullDownW,
      NULL);
  XmStringFree(labelStr);

  if (mnemonic != '\0')
    XtVaSetValues(cascButtW, XmNmnemonic, mnemonic, NULL);

  XtManageChild(cascButtW);

  return pullDownW;
}

PuMenuEntry
PuMotifCreateMenuEntry(PuMenu menu, char *name,
    char mnemonic, char *acc, char *accText,
    PuMenuCB menuCB, void *clientData, PuFreeCB freeCB)
{
  Widget entryW;
  XmString labelStr, accStr;
  MenuCBInfo *infoP;

  labelStr = XmStringCreateSimple(name);
  entryW = XmCreatePushButton(menu, "Entry", NULL, 0);
  XtVaSetValues(entryW, XmNlabelString, labelStr, NULL);
  XmStringFree(labelStr);

  if (mnemonic != '\0')
    XtVaSetValues(entryW, XmNmnemonic, mnemonic, NULL);

  if (acc != NULL && acc[0] != '\0') {
    accStr = XmStringCreateSimple(accText);
    XtVaSetValues(entryW,
	XmNaccelerator, acc,
	XmNacceleratorText, accStr,
	NULL);
    XmStringFree(accStr);
  }

  if (menuCB != NULL) {
    infoP = malloc(sizeof(*infoP));
    infoP->name = malloc(strlen(name) + 1);
    (void) strcpy(infoP->name, name);
    infoP->menuCB = menuCB;
    infoP->clientData = clientData;
    infoP->freeCB = freeCB;
    XtAddEventHandler(entryW, EnterWindowMask, False, menuEnterEH, infoP);
    XtAddEventHandler(entryW, LeaveWindowMask, False, menuLeaveEH, infoP);
    XtAddCallback(entryW, XmNactivateCallback, menuActCB, infoP);
    XtAddCallback(entryW, XmNdestroyCallback, destroyCB, infoP);
  }

  XtManageChild(entryW);

  return entryW;
}

void
PuMotifSwitchMenubar(PuMenubar menubar, BOOL onOff)
{
  if (onOff) {
    XtManageChild(menubar);
    MotifSetMenubarW(menubar);
  } else {
    XtUnmanageChild(menubar);
    MotifSetMenubarW(NULL);
  }
}

void
PuMotifSwitchPopup(PuMenu menu, BOOL onOff)
{
  if (onOff)
    MotifSetPopupW(menu);
  else
    MotifSetPopupW(NULL);
}
