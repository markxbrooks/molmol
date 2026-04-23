/*
************************************************************************
*
*   x11_line.h - X11 line drawing
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
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/x11/SCCS/s.x11_line.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <sg_types.h>

extern void SgX11DrawLine(float[2], float[2]);

extern void SgX11DrawPolyline(float[][2], int);

extern void SgX11SetLineWidth(float);

extern void SgX11SetLineStyle(SgLineStyle);
