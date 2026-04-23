/*
************************************************************************
*
*   pov_line.h - POV line drawing
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
*   Date of last modification : 94/11/15
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/pov/SCCS/s.pov_line.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <sg_types.h>

extern void SgPOVDrawLine2D(float[2], float[2]);

extern void SgPOVDrawPolyline2D(float[][2], int);

extern void SgPOVDrawLine(float[3], float[3]);

extern void SgPOVDrawPolyline(float[][3], int);

extern void SgPOVSetLineWidth(float);

extern void SgPOVSetLineStyle(SgLineStyle);
