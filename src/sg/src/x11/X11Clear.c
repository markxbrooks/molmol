/*
************************************************************************
*
*   X11Clear.c - X11 clear window
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
*   Date of last modification : 01/06/02
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/x11/SCCS/s.X11Clear.c
*   SCCS identification       : 1.6
*
************************************************************************
*/

#include "x11_clear.h"

#include <X11/Xlib.h>

#include "x11_access.h"

static int WinWidth = -1, WinHeight = -1;
static BOOL WindowModified = FALSE;

void
SgX11Clear(void)
{
  Display *dpy = X11GetDisplay();
  Window win = X11GetWindow();
  Pixmap pMap = X11GetPixmap();
  GC gc = X11GetGC();
  XWindowAttributes attr;
  XGCValues gcv;

  (void) XGetWindowAttributes(dpy, win, &attr);

  if (attr.width != WinWidth || attr.height != WinHeight) {
    if (pMap != 0) {
      XFreePixmap(dpy, pMap);
      pMap = 0;
    }

    WinWidth = attr.width;
    WinHeight = attr.height;
  }

  if (pMap == 0) {
    pMap = XCreatePixmap(dpy, win, WinWidth, WinHeight, attr.depth);
    X11SetPixmap(pMap);
  }

  XGetGCValues(dpy, gc, GCForeground | GCBackground, &gcv);
  XSetForeground(dpy, gc, gcv.background);
  XFillRectangle(dpy, pMap, gc, 0, 0, WinWidth, WinHeight);
  XChangeGC(dpy, gc, GCForeground, &gcv);
}

void
SgX11SetBuffer(SgBuffer buf)
{
  if (buf == SG_BUF_FRONT) {
    X11SetDrawable(X11GetWindow());
    WindowModified = TRUE;
  } else {
    X11SetDrawable(X11GetPixmap());
  }
}

void
SgX11FlushFrame(void)
{
  Display *dpy = X11GetDisplay();

  XFlush(dpy);
}

void
SgX11EndFrame(void)
{
  Display *dpy = X11GetDisplay();
  Window win = X11GetWindow();
  Pixmap pMap = X11GetPixmap();
  GC gc = X11GetGC();

  XCopyArea(dpy, pMap, win, gc, 0, 0, WinWidth, WinHeight, 0, 0);
  XFlush(dpy);

  WindowModified = FALSE;
}

BOOL
SgX11Refresh(void)
{
  Display *dpy = X11GetDisplay();
  Window win = X11GetWindow();
  Pixmap pMap = X11GetPixmap();
  GC gc = X11GetGC();
  XWindowAttributes attr;

  if (pMap == 0)
    return FALSE;

  if (WindowModified)
    return FALSE;

  (void) XGetWindowAttributes(dpy, win, &attr);
  if (attr.width != WinWidth || attr.height != WinHeight)
    return FALSE;

  XCopyArea(dpy, pMap, win, gc, 0, 0, WinWidth, WinHeight, 0, 0);
  XFlush(dpy);

  return TRUE;
}
