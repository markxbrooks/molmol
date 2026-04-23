/*
************************************************************************
*
*   MotifPlace.c - Motif dialog placement
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
*   Date of last modification : 95/12/12
*   Pathname of SCCS file     : /sgiext/molmol/src/motif/SCCS/s.MotifPlace.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include "motif_place.h"

#include <X11/StringDefs.h>
#include <Xm/DialogS.h>

#include <bool.h>
#include "motif_access.h"

/* Take hard-coded sizes for WM decorations. This is not nice, but
   much easier than doing it right, and normally not a problem. */
#define WM_BORDER 8
#define WM_TITLE_HEIGHT 32

typedef enum {
  DP_LEFT,
  DP_RIGHT
} DialogPlace;

static BOOL FirstPlaced = FALSE;
static int ScreenW, ScreenH;
static DialogPlace Place;
static Position PlaceX, PlaceY;

static void
moveEH(Widget w, XtPointer clientData, XEvent *evtP, Boolean *ctd)
{
  if (evtP->type != ConfigureNotify)
    return;

  if (Place == DP_LEFT)
    PlaceX = evtP->xconfigure.x + evtP->xconfigure.width;
  else
    PlaceX = evtP->xconfigure.x;

  PlaceY = evtP->xconfigure.y + evtP->xconfigure.height +
      WM_BORDER + WM_TITLE_HEIGHT;
}

static void
mapCB(Widget w, XtPointer clientData, XtPointer callData)
{
  Widget mainW;
  Position mx, my, dx, dy;
  Dimension mw, dw, dh;

  XtRemoveCallback(w, XtNpopupCallback, mapCB, NULL);

  if (! FirstPlaced) {
    ScreenW = WidthOfScreen(XtScreen(w));
    ScreenH = HeightOfScreen(XtScreen(w));

    mainW = MotifGetMainW();

    XtVaGetValues(mainW,
        XmNx, &mx,
        XmNy, &my,
        XmNwidth, &mw,
	NULL);
    XtTranslateCoords(mainW, mx, my, &mx, &my);

    if (mx > ScreenW - (mx + mw)) {
      Place = DP_LEFT;
      PlaceX = mx - 2 * WM_BORDER;
    } else {
      Place = DP_RIGHT;
      PlaceX = mx + mw + 2 * WM_BORDER;
    }

    PlaceY = my;

    FirstPlaced = TRUE;
  }

  XtVaGetValues(w,
      XmNwidth, &dw,
      XmNheight, &dh,
      NULL);

  if (Place == DP_LEFT) {
    dx = PlaceX - dw;
    if (dx < 0)
      dx = 0;
  } else {
    dx = PlaceX;
    if (dx + dw > ScreenW)
      dx = ScreenW - dw;
  }

  dy = PlaceY;
  if (dy + dh > ScreenH)
    dy = ScreenH - dh;
  
  XtVaSetValues(w,
      XmNx, dx,
      XmNy, dy,
      NULL);

  PlaceY = dy + dh + WM_BORDER + WM_TITLE_HEIGHT;
  
  XtAddEventHandler(w, StructureNotifyMask, FALSE, moveEH, NULL);
}

void
MotifPlace(Widget w)
{
  XtVaSetValues(w,
      XmNdefaultPosition, False,
      NULL);
  XtAddCallback(w, XtNpopupCallback, mapCB, NULL);
}
