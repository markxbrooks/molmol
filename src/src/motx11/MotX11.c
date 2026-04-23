/*
************************************************************************
*
*   MotX11.c - Motif/X11 device
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
*   Date of last modification : 01/05/27
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/motx11/SCCS/s.MotX11.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include "motx11.h"

#include <stdio.h>
#include <X11/Intrinsic.h>
#include <Xm/DrawingA.h>

#include <sg.h>
#include <sg_x11_p.h>
#include <pu_cb.h>
#include <motif_p.h>

static Dimension CurrWidth = 0, CurrHeight = 0;
static BOOL ResizeCalled = FALSE;

static void
callResize(Widget w)
{
  Arg args[20];
  int n;
  Dimension width, height;

  ResizeCalled = TRUE;

  n = 0;
  XtSetArg(args[n], XmNwidth, &width); n++;
  XtSetArg(args[n], XmNheight, &height); n++;
  XtGetValues(w, args, n);
  PuCallResizeCB((PuWindow) w, width, height);
  if (width <= CurrWidth && height <= CurrHeight)
    PuCallExposeCB((PuWindow) w, 0, 0, width, height);
  CurrWidth = width;
  CurrHeight = height;
}

static void
exposeCB(Widget w, XtPointer clientData, XtPointer callData)
{
  XmDrawingAreaCallbackStruct *callP =
      (XmDrawingAreaCallbackStruct *) callData;
  XExposeEvent *evtP = &callP->event->xexpose;
  Position totXMin, totXMax, totYMin, totYMax;
  Position xMin, xMax, yMin, yMax;

  /* make sure that resize callbacks are called before the
     first expose callback */
  if (! ResizeCalled)
    callResize(w);

  totXMin = evtP->x;
  totXMax = evtP->x + evtP->width;
  totYMin = evtP->y;
  totYMax = evtP->y + evtP->height;
  while (XCheckTypedWindowEvent(evtP->display, evtP->window,
      Expose, (XEvent *) evtP)) {
    xMin = evtP->x;
    xMax = evtP->x + evtP->width;
    yMin = evtP->y;
    yMax = evtP->y + evtP->height;
    if (xMin < totXMin)
      totXMin = xMin;
    if (xMax > totXMax)
      totXMax = xMax;
    if (yMin < totYMin)
      totYMin = yMin;
    if (yMax > totYMax)
      totYMax = yMax;
  }

  PuCallExposeCB((PuWindow) w, totXMin, totYMin,
      totXMax - totXMin, totYMax - totYMin);
}

static void
resizeCB(Widget w, XtPointer clientData, XtPointer callData)
{
  callResize(w);
}

IORes
IOMotifX11Init(char *appName, int *argcP, char *argv[])
{
  Arg args[20];
  int n;
  Widget drawParentW, drawW;

  (void) PuInit(appName, argcP, argv);

  drawParentW = MotifGetDrawParentW();
  n = 0;
  drawW = XtCreateManagedWidget("drawing area",
      xmDrawingAreaWidgetClass, drawParentW, args, n);
  if (drawW == 0)
    return IO_RES_ERR;
  MotifSetDrawW(drawW);
  MotifSetPopupParentW(drawW);

  XtAddCallback(drawW, XmNexposeCallback, exposeCB, NULL);
  XtAddCallback(drawW, XmNresizeCallback, resizeCB, NULL);
  IOSetWindow(drawW);

  (void) SgInit(argcP, argv);

  return IO_RES_OK;
}

IORes
IOMotifX11Cleanup(void)
{
  (void) SgCleanup();
  (void) PuCleanup();
  return IO_RES_OK;
}

IORes
IOMotifX11SetWindow(PuWindow win)
{
  Widget w = (Widget) win;

  X11SetDisplay(XtDisplay(w));
  X11SetWindow(XtWindow(w));
  return IO_RES_OK;
}

void
IOMotifX11EndFrame(void)
{
  /* completely handled by SgX11EndFrame() */
}
