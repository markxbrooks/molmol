/*
************************************************************************
*
*   MotOGL.c - Motif/OpenGL device
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
*   Date of last modification : 00/05/29
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/src/motogl/SCCS/s.MotOGL.c
*   SCCS identification       : 1.10
*
************************************************************************
*/

#include "motogl.h"
#include "motogl_int.h"

#include <stdio.h>
#include <X11/Intrinsic.h>
#include <Xm/XmStrDefs.h>

#define BOOL MOTIF_BOOL  /* hack to avoid naming conflict */
#include "/sw/include/GL/GLwMDrawA.h"
#undef BOOL

#include <sg.h>
#include <pu_cb.h>
#include <motif_p.h>

static Widget CurrW;

static void
exposeCB(Widget w, XtPointer clientData, XtPointer callData)
{
  GLwDrawingAreaCallbackStruct *callP =
      (GLwDrawingAreaCallbackStruct *) callData;
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
  GLwDrawingAreaCallbackStruct *callP =
      (GLwDrawingAreaCallbackStruct *) callData;

  PuCallResizeCB((PuWindow) w, callP->width, callP->height);
}

IORes
MotifOGLPrepareFont(Display *dpy)
{
  XFontStruct *fontP;
  int first, last, no, base;

  fontP = XLoadQueryFont(dpy, "fixed");
  if (fontP == NULL)
    return IO_RES_ERR;

  first = fontP->min_char_or_byte2;
  last = fontP->max_char_or_byte2;
  no = last - first + 1;

  base = glGenLists(no);
  if (base == 0)
    return IO_RES_ERR;
  
  glXUseXFont(fontP->fid, first, no, base);
  XFreeFontInfo(NULL, fontP, 0);

  glListBase(base - first);

  return IO_RES_OK;
}

static XVisualInfo *
findVisual(Display *dpy, BOOL stereo)
{
  int attrList[20];
  int n;

  n = 0;
  attrList[n++] = GLX_RGBA;
  attrList[n++] = GLX_DOUBLEBUFFER;
  if (stereo)
    attrList[n++] = GLX_STEREO;
  attrList[n++] = GLX_RED_SIZE; attrList[n++] = 1;
  attrList[n++] = GLX_GREEN_SIZE; attrList[n++] = 1;
  attrList[n++] = GLX_BLUE_SIZE; attrList[n++] = 1;
  attrList[n++] = GLX_DEPTH_SIZE; attrList[n++] = 1;
  attrList[n++] = None;

  return glXChooseVisual(dpy, DefaultScreen(dpy), attrList);
}

IORes
IOMotifOGLInit(char *appName, int *argcP, char *argv[])
{
  BOOL stereo;
  Arg args[20];
  int n;
  Widget topW, drawParentW, glW;
  Display *dpy;
  XVisualInfo *visP;
  Window windows[2];
  int i;

  (void) PuInit(appName, argcP, argv);
  stereo = FALSE;
  for (i = 1; i < *argcP; i++)
    if (strcmp(argv[i], "-stereo") == 0) {
      stereo = TRUE;
      break;
    }

  drawParentW = MotifGetDrawParentW();
  dpy = XtDisplay(drawParentW);

  visP = NULL;
  if (stereo)
    visP = findVisual(dpy, TRUE);
  if (visP == NULL)
    visP = findVisual(dpy, FALSE);
  if (visP == NULL)
    return IO_RES_ERR;

  n = 0;
  XtSetArg(args[n], GLwNvisualInfo, visP); n++;

  glW = GLwCreateMDrawingArea(drawParentW, "ogl", args, n);

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

  XtAddCallback(glW, GLwNexposeCallback, exposeCB, NULL);
  XtAddCallback(glW, GLwNresizeCallback, resizeCB, NULL);
  IOSetWindow(glW);

  (void) SgInit(argcP, argv);

  return MotifOGLPrepareFont(dpy);
}

IORes
IOMotifOGLCleanup(void)
{
  (void) SgCleanup();
  (void) PuCleanup();
  return IO_RES_OK;
}

IORes
IOMotifOGLSetWindow(PuWindow win)
{
  Widget w = (Widget) win;
  Display *dpy = XtDisplay(w);
  GLXContext glxContext;
  XVisualInfo *visP;

  XtVaGetValues(w, XmNuserData, &glxContext, NULL);
  if (glxContext == NULL) {
    XtVaGetValues(w, GLwNvisualInfo, &visP, NULL);
    glxContext = glXCreateContext(dpy, visP, NULL, True);
    XtVaSetValues(w, XmNuserData, glxContext, NULL);
  }

  glXMakeCurrent(dpy, XtWindow(w), glxContext);

  CurrW = w;

  return IO_RES_OK;
}

void
IOMotifOGLEndFrame(void)
{
  glXSwapBuffers(XtDisplay(CurrW), XtWindow(CurrW));
}

Widget
MotifOGLGetDrawW(void)
{
  return CurrW;
}
