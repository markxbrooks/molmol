/*
************************************************************************
*
*   MotGL.c - Motif/GL device
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
*   Date of last modification : 94/08/16
*   Pathname of SCCS file     : /sgiext/molmol/src/motgl/SCCS/s.MotGL.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include "motgl.h"

#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/Xirisw/GlxMDraw.h>

#include <sg.h>
#include <pu_cb.h>
#include <motif_p.h>

static void
exposeCB(Widget w, XtPointer clientData, XtPointer callData)
{
  GlxDrawCallbackStruct *callP = (GlxDrawCallbackStruct *) callData;
  XExposeEvent *evtP = &callP->event->xexpose;
  Position totXMin, totXMax, totYMin, totYMax;
  Position xMin, xMax, yMin, yMax;

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
  GlxDrawCallbackStruct *callP = (GlxDrawCallbackStruct *) callData;

  PuCallResizeCB((PuWindow) w, callP->width, callP->height);
}

IORes
IOMotifGLInit(char *appName, int *argcP, char *argv[])
{
  Arg args[20];
  int n;
  Widget topW, drawParentW, glW;
  GLXconfig config[] = {
    {GLX_NORMAL, GLX_DOUBLE, TRUE},
    {GLX_NORMAL, GLX_RGB, TRUE},
    {GLX_NORMAL, GLX_ZSIZE, GLX_NOCONFIG},
    {0, 0, 0}
  };
  Window windows[2];

  (void) PuInit(appName, argcP, argv);

  drawParentW = MotifGetDrawParentW();
  n = 0;
  config[0].arg = SgGetDoubleBuffer();
  XtSetArg(args[n], GlxNglxConfig, config); n++;
  glW = GlxCreateMDraw(drawParentW, "gl", args, n);
  if (glW == 0)
    return IO_RES_ERR;
  XtManageChild(glW);
  MotifSetDrawW(glW);

  /* popups must be children of the drawing area parent in
     mixed mode */
  MotifSetPopupParentW(drawParentW);

  topW = MotifGetTopW();
  windows[0] = XtWindow(glW);
  windows[1] = XtWindow(topW);
  XSetWMColormapWindows(XtDisplay(topW), XtWindow(topW), windows, 2);

  XtAddCallback(glW, GlxNexposeCallback, exposeCB, NULL);
  XtAddCallback(glW, GlxNresizeCallback, resizeCB, NULL);
  IOSetWindow(glW);

  (void) SgInit(argcP, argv);

  return IO_RES_OK;
}

IORes
IOMotifGLCleanup(void)
{
  (void) SgCleanup();
  (void) PuCleanup();
  return IO_RES_OK;
}

IORes
IOMotifGLSetWindow(PuWindow win)
{
  Widget w = (Widget) win;

  GLXwinset(XtDisplay(w), XtWindow(w));
  return IO_RES_OK;
}

void
IOMotifGLEndFrame(void)
{
  /* completely handled by SgGLEndFrame() */
}
