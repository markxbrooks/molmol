/*
************************************************************************
*
*   sg_x11_p.h - set X11 Display and Window, used inside I/O device
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
*   Pathname of SCCS file     : /sgiext/molmol/sg/include/SCCS/s.sg_x11_p.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <X11/Xlib.h>

extern void X11SetDisplay(Display *);

extern void X11SetWindow(Window);
