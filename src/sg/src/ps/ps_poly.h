/*
************************************************************************
*
*   ps_poly.h - PostScript polygon drawing
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
*   Date of last modification : 96/02/27
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/ps/SCCS/s.ps_poly.h
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <sg_types.h>

extern void SgPSDrawPolygon(float[][2], int);

extern void SgPSDrawShadedPolygon(float[][2], float [][3], int);
