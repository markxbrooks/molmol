/*
************************************************************************
*
*   MotifCB.c - callback handling
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
*   Date of last modification : 94/06/02
*   Pathname of SCCS file     : /sgiext/molmol/src/motif/SCCS/s.MotifCB.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include "motif_cb.h"

#include <Xm/RowColumn.h>

#include <pu_cb.h>
#include "motif_access.h"

#define MOUSE_BUTTON_NO 3

static PuMouseButton CurrButton = PU_MB_NONE;
static BOOL DidDrag = FALSE;

static PuMouseButton
puButton(int state)
{
  PuMouseButton button = 0;

  if ((state & Button1Mask) != 0)
    button += 1;
  if ((state & Button2Mask) != 0)
    button += 2;
  if ((state & Button3Mask) != 0)
    button += 4;

  if (button > PU_MOUSE_BUTTON_NO)
    button = PU_MOUSE_BUTTON_NO;

  if (button == 0)
    return PU_MB_NONE;
  else
    return button - 1;
}

static Dimension
widgetHeight(Widget w)
{
  Dimension height;

  XtVaGetValues(w, XmNheight, &height, NULL);

  return height;
}

static void
keyEH(Widget w, XtPointer clientData, XEvent *evtP, Boolean *ctd)
{
  int len;
  char buf[10];

  len = XLookupString(&evtP->xkey, buf, sizeof(buf), NULL, NULL);
  if (len == 1)
    PuCallKeyCB((PuWindow) w, buf[0]);
}

static void
buttonPressEH(Widget w, XtPointer clientData, XEvent *evtP, Boolean *ctd)
{
  XButtonEvent *buttEvtP = &evtP->xbutton;
  PuMouseButton oldButton, newButton;
  Widget popupW = NULL;

  oldButton = puButton(buttEvtP->state);
  newButton = oldButton;

  if (buttEvtP->button == Button1) {
    newButton = puButton(buttEvtP->state | Button1Mask);
  } else if (buttEvtP->button == Button2) {
    newButton = puButton(buttEvtP->state | Button2Mask);
  } else if (buttEvtP->button == Button3) {
    popupW = MotifGetPopupW();
    if (popupW != NULL) {
      XmMenuPosition(popupW, buttEvtP);
      XtManageChild(popupW);
      return;
    }
    newButton = puButton(buttEvtP->state | Button3Mask);
  }

  if (newButton == oldButton)
    return;

  if (oldButton == PU_MB_NONE)
    DidDrag = FALSE;
  else
    PuCallButtonReleaseCB((PuWindow) w, oldButton, FALSE,
	buttEvtP->x, widgetHeight(w) - buttEvtP->y);

  PuCallButtonPressCB((PuWindow) w, newButton,
      (buttEvtP->state & (ShiftMask | ControlMask)) != 0,
      buttEvtP->x, widgetHeight(w) - buttEvtP->y);

  CurrButton = newButton;
}

static void
buttonReleaseEH(Widget w, XtPointer clientData, XEvent *evtP, Boolean *ctd)
{
  XButtonEvent *buttEvtP = &evtP->xbutton;
  PuMouseButton oldButton, newButton;
  Widget popupW = NULL;

  oldButton = puButton(buttEvtP->state);
  newButton = oldButton;

  if (buttEvtP->button == Button1) {
    newButton = puButton(buttEvtP->state & ~Button1Mask);
  } else if (buttEvtP->button == Button2) {
    newButton = puButton(buttEvtP->state & ~Button2Mask);
  } else if (buttEvtP->button == Button3) {
    popupW = MotifGetPopupW();
    if (popupW == NULL)
      newButton = puButton(buttEvtP->state & ~Button3Mask);
  }

  if (newButton == oldButton)
    return;

  if (popupW == NULL)
    PuCallButtonReleaseCB((PuWindow) w, oldButton,
	newButton == PU_MB_NONE && ! DidDrag,
	buttEvtP->x, widgetHeight(w) - buttEvtP->y);

  if (newButton != PU_MB_NONE)
    PuCallButtonPressCB((PuWindow) w, newButton,
	(buttEvtP->state & (ShiftMask | ControlMask)) != 0,
	buttEvtP->x, widgetHeight(w) - buttEvtP->y);

  CurrButton = newButton;
}

static void
moveEH(Widget w, XtPointer clientData, XEvent *evtP, Boolean *ctd)
{
  XMotionEvent *motionEvtP = &evtP->xmotion;

  if (CurrButton == PU_MB_NONE) {
    PuCallMoveCB((PuWindow) w,
        motionEvtP->x, widgetHeight(w) - motionEvtP->y);
  } else {
    PuCallDragCB((PuWindow) w, CurrButton,
	motionEvtP->x, widgetHeight(w) - motionEvtP->y);
    DidDrag = TRUE;
  }
}

void
MotifInitCallbacks(void)
{
  Widget dW = MotifGetDrawW();
  Widget pW = MotifGetPopupParentW();

  XtAddEventHandler(dW, KeyReleaseMask, False, keyEH, NULL);
  XtAddEventHandler(dW, ButtonPressMask, False, buttonPressEH, NULL);
  XtAddEventHandler(dW, ButtonReleaseMask, False, buttonReleaseEH, NULL);
  XtAddEventHandler(dW, PointerMotionMask, False, moveEH, NULL);

  if (pW != dW)
    XtAddEventHandler(pW, ButtonPressMask, False, buttonPressEH, NULL);
}
