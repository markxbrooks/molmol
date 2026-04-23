/*
************************************************************************
*
*   x11_access.h - get Display, Window,...
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
*   Date of last modification : 95/02/05
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/x11/SCCS/s.x11_access.h
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <X11/Xlib.h>

extern Display *X11GetDisplay(void);

extern Colormap X11GetColormap(void);

extern Window X11GetWindow(void);

extern void X11SetPixmap(Pixmap);

extern Pixmap X11GetPixmap(void);

extern void X11SetDrawable(Drawable);

extern Drawable X11GetDrawable(void);

extern GC X11GetGC(void);
