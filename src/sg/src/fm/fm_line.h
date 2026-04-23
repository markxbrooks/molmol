/*
************************************************************************
*
*   fm_line.h - FrameMaker lines
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
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/fm/SCCS/s.fm_line.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <sg_types.h>

extern void SgFMDrawLine(float[2], float[2]);

extern void SgFMDrawPolyline(float[][2], int);

extern void SgFMSetLineWidth(float);

extern void SgFMSetLineStyle(SgLineStyle);
