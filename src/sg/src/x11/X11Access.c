/*
************************************************************************
*
*   X11Access.c - set/get Display, Window,...
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/x11/SCCS/s.X11Access.c
*   SCCS identification       : 1.7
*
************************************************************************
*/

#include <sg_x11_p.h>
#include "x11_access.h"

#include <stdio.h>
#include <X11/Xlib.h>

static Display *CurrDpyP;
/* static Colormap CurrColormap; */
static Window CurrWindow = 0;
static Pixmap CurrPixmap = 0;
static Drawable CurrDrawable = 0;
static GC CurrGC = NULL;

void
X11SetDisplay(Display *dpy)
{
  CurrDpyP = dpy;
}

void
X11SetColormap(Colormap map)
{
/*  CurrColormap = map; */
}

void
X11SetWindow(Window window)
{
  if (CurrWindow != 0 && CurrDrawable == CurrWindow)
    CurrDrawable = window;

  CurrWindow = window;
}

Display *
X11GetDisplay(void)
{
  return CurrDpyP;
}

Colormap
X11GetColormap(void)
{
/*  return CurrColormap; */
  return DefaultColormap(CurrDpyP, DefaultScreen(CurrDpyP));
}

Window
X11GetWindow(void)
{
  return CurrWindow;
}

void
X11SetPixmap(Pixmap pMap)
{
  if (CurrDrawable == 0 || CurrDrawable == CurrPixmap)
    CurrDrawable = pMap;

  CurrPixmap = pMap;
}

Pixmap
X11GetPixmap(void)
{
  return CurrPixmap;
}

void
X11SetDrawable(Drawable draw)
{
  CurrDrawable = draw;
}

Drawable
X11GetDrawable(void)
{
  return CurrDrawable;
}

GC
X11GetGC(void)
{
  XGCValues gcv;

  if (CurrGC == NULL) {
    gcv.cap_style = CapRound;
    gcv.join_style = JoinRound;
    CurrGC = XCreateGC(CurrDpyP, CurrWindow, GCCapStyle | GCJoinStyle, &gcv);
  }

  return CurrGC;
}
