/*
************************************************************************
*
*   MotifBusy.c - management of busy cursor
*
*   Copyright (c) 1994
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
*   Date of last modification : 94/10/27
*   Pathname of SCCS file     : /sgiext/molmol/src/motif/SCCS/s.MotifBusy.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include "motif_busy.h"

#include <X11/cursorfont.h>
#include <Xm/Xm.h>

#include <linlist.h>
#include "motif_access.h"

static LINLIST DialogList = NULL;
static Cursor BusyCursor = 0;
static BOOL BusyOn = FALSE;

static void
destroyCB(Widget w, XtPointer clientData, XtPointer callData)
{
  Widget *wP = clientData;

  ListRemove(DialogList, wP);
}

void
MotifAddDialog(Widget w)
{
  Widget *wP;

  if (DialogList == NULL)
    DialogList = ListOpen(sizeof(Widget));
  
  wP = ListInsertLast(DialogList, &w);

  XtAddCallback(w, XmNdestroyCallback, destroyCB, wP);

  if (BusyOn && XtWindow(w) != 0)
    XDefineCursor(XtDisplay(w), XtWindow(w), BusyCursor);
}

void
PuMotifSetBusyCursor(BOOL onOff)
{
  Display *dpy;
  Widget mainW;
  Cursor curs;
  Widget *wP;

  mainW = MotifGetMainW();
  dpy = XtDisplay(mainW);

  if (onOff) {
    if (BusyCursor == 0)
      BusyCursor = XCreateFontCursor(dpy, XC_watch);
    curs = BusyCursor;
  } else {
    curs = None;
  }

  XDefineCursor(dpy, XtWindow(mainW), curs);

  wP = ListFirst(DialogList);
  while (wP != NULL) {
    if (XtWindow(*wP) != 0)
      XDefineCursor(dpy, XtWindow(*wP), curs);
    wP = ListNext(DialogList, wP);
  }

  XFlush(dpy);

  BusyOn = onOff;
}
