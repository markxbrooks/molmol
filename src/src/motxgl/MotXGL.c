/*
************************************************************************
*
*   MotXGL.c - Motif/XGL device
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/motxgl/SCCS/s.MotXGL.c
*   SCCS identification       : 1.7
*
************************************************************************
*/

#include "motxgl.h"
#include "motxgl_int.h"

#include <stdio.h>
#include <stdlib.h>
#include <X11/Intrinsic.h>
#include <Xm/DrawingA.h>
#include <xgl/xgl.h>

#include <sg.h>
#include <sg_xgl_p.h>
#include <pu_cb.h>
#include <motif_p.h>

#define R_SIZE 6
#define G_SIZE 9
#define B_SIZE 4
#define COLOR_NO (R_SIZE * G_SIZE * B_SIZE)
#define COLOR_OFFSET 32

static Dimension CurrWidth = 0, CurrHeight = 0;
static BOOL ResizeCalled = FALSE;
static Xgl_sys_state SysState;
static Xgl_win_ras CurrRas;

static void
setupColorCube(Widget drawW, Xgl_win_ras ras)
{
  Display *dpy = XtDisplay(drawW);
  int screen = XScreenNumberOfScreen(XtScreen(drawW));
  Colormap xCmap;
  unsigned long pmask[256];
  unsigned long pix[256];
  XColor col[256];
  unsigned long pixMapping[COLOR_NO];
  int r, g, b, rStep, gStep, bStep;
  Xgl_cmap cmap;
  Xgl_usgn32 cubeSize[3];
  Window windows[2];
  int i, j;

  xCmap = XCreateColormap(dpy, XtWindow(drawW),
      DefaultVisual(dpy, screen), AllocNone);
  if (XAllocColorCells(dpy, xCmap, True,
      pmask, 0, pix, COLOR_OFFSET + COLOR_NO) == 0)
    return;
  
  for (i = 0; i < COLOR_OFFSET; i++)
    col[i].pixel = i;
  XQueryColors(dpy, DefaultColormap(dpy, screen), col, COLOR_OFFSET);

  for (i = COLOR_OFFSET; i < COLOR_OFFSET + COLOR_NO; i++) {
    col[i].pixel = i;
    col[i].flags = DoRed | DoGreen | DoBlue;
    pixMapping[i - COLOR_OFFSET] = i;
  }

  rStep = 65535 / (R_SIZE - 1);
  gStep = 65535 / (G_SIZE - 1);
  bStep = 65535 / (B_SIZE - 1);

  for (b = 0; b < B_SIZE; b++)
    for (g = 0; g < G_SIZE; g++)
      for (r = 0; r < R_SIZE; r++) {
	i = COLOR_OFFSET + r + (R_SIZE * (g + G_SIZE * b));
	col[i].red = r * rStep;
	col[i].green = g * gStep;
	col[i].blue = b * bStep;
      }
  
  XStoreColors(dpy, xCmap, col, COLOR_OFFSET + COLOR_NO);

  xgl_object_get(ras, XGL_DEV_COLOR_MAP, &cmap);
  cubeSize[0] = R_SIZE;
  cubeSize[1] = G_SIZE;
  cubeSize[2] = B_SIZE;
  xgl_object_set(cmap,
      XGL_CMAP_COLOR_CUBE_SIZE, cubeSize,
      XGL_CMAP_NAME, xCmap,
      NULL);
  
  xgl_object_set(ras,
      XGL_WIN_RAS_PIXEL_MAPPING, pixMapping,
      NULL);
  
  windows[0] = XtWindow(drawW);
  windows[1] = XtWindow(MotifGetTopW());
  XSetWindowColormap(dpy, windows[0], xCmap);
  XSetWindowColormap(dpy, windows[1], xCmap);
  XSetWMColormapWindows(dpy, windows[1], windows, 2);
}

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
  XSync(XtDisplay(w), False);  /* raster doesn't get new size otherwise */
  callResize(w);
}

IORes
IOMotifXGLInit(char *appName, int *argcP, char *argv[])
{
  Arg args[20];
  int n;
  Widget drawParentW, drawW;
  Xgl_X_window xglWin;
  Xgl_obj_desc rasDesc;
  Xgl_inquire *inqP;
  Xgl_win_ras ras;
  Xgl_usgn32 bufNo;

  (void) PuInit(appName, argcP, argv);

  drawParentW = MotifGetDrawParentW();
  n = 0;
  drawW = XtCreateManagedWidget("drawing area",
      xmDrawingAreaWidgetClass, drawParentW, args, n);
  if (drawW == NULL)
    return IO_RES_ERR;

  MotifSetDrawW(drawW);
  MotifSetPopupParentW(drawW);

  xglWin.X_display = XtDisplay(drawW);
  xglWin.X_window = XtWindow(drawW);
  xglWin.X_screen = XScreenNumberOfScreen(XtScreen(drawW));

  SysState = xgl_open(NULL);
  if (SysState == NULL)
    return IO_RES_ERR;

  XGLSetSysState(SysState);

  rasDesc.win_ras.type = XGL_WIN_X;
  rasDesc.win_ras.desc = &xglWin;

  inqP = xgl_inquire(SysState, &rasDesc);
  if (inqP->dga_flag && inqP->maximum_buffer < 2)
    rasDesc.win_ras.type = XGL_WIN_X | XGL_WIN_X_PROTO_PEX;

  ras = xgl_object_create(SysState, XGL_WIN_RAS, &rasDesc,
      XGL_DEV_COLOR_TYPE, XGL_COLOR_RGB,
      NULL);
  if (ras == NULL) {
    rasDesc.win_ras.type = XGL_WIN_X | XGL_WIN_X_PROTO_XLIB;
    ras = xgl_object_create(SysState, XGL_WIN_RAS, &rasDesc,
        XGL_DEV_COLOR_TYPE, XGL_COLOR_RGB,
        NULL);
  }

  free(inqP);

  setupColorCube(drawW, ras);

  xgl_object_set(ras,
      XGL_WIN_RAS_BUFFERS_REQUESTED, 2,
      NULL);
  xgl_object_get(ras, XGL_WIN_RAS_BUFFERS_ALLOCATED, &bufNo);

  XtVaSetValues(drawW, XmNuserData, ras, NULL);

  XtAddCallback(drawW, XmNexposeCallback, exposeCB, NULL);
  XtAddCallback(drawW, XmNresizeCallback, resizeCB, NULL);

  IOSetWindow(drawW);
  if (bufNo == 2)
    xgl_object_set(CurrRas,
	XGL_WIN_RAS_BUF_DRAW, 0,
	XGL_WIN_RAS_BUF_DISPLAY, 1,
	NULL);

  (void) SgInit(argcP, argv);

  return IO_RES_OK;
}

IORes
IOMotifXGLCleanup(void)
{
  (void) SgCleanup();
  (void) PuCleanup();
  return IO_RES_OK;
}

IORes
IOMotifXGLSetWindow(PuWindow win)
{
  Widget w = (Widget) win;

  XtVaGetValues(w, XmNuserData, &CurrRas, NULL);
  XGLSetRaster(CurrRas);

  return IO_RES_OK;
}

Xgl_sys_state
MotifXGLGetSysState(void)
{
  return SysState;
}

Xgl_win_ras
MotifXGLGetRaster(void)
{
  return CurrRas;
}

void
IOMotifXGLEndFrame(void)
{
}
