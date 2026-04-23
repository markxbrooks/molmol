/*
************************************************************************
*
*   X11Text.c - X11 text drawing
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
*   Date of last modification : 95/02/07
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/x11/SCCS/s.X11Text.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include "x11_text.h"

#include <string.h>
#include <X11/Xlib.h>

#include <sg_map.h>
#include "x11_access.h"

XFontStruct *CurrFontP = NULL;

void
SgX11DrawAnnot(float x[2], char *str)
{
  Display *dpy = X11GetDisplay();
  GC gc = X11GetGC();
  float xm[2];

  SgVpMapPoint(xm, x);

  if (CurrFontP == NULL) {
    CurrFontP = XLoadQueryFont(dpy, "fixed");
    if (CurrFontP == NULL)
      return;
    XSetFont(dpy, gc, CurrFontP->fid);
  }

  XDrawString(dpy, X11GetDrawable(), gc, (int) xm[0], (int) xm[1],
      str, strlen(str));
}
