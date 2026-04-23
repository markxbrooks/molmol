/*
************************************************************************
*
*   sg_shade.h - functions for handling shading of 2D polygons
*
*   Copyright (c) 1994-96
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
*   Date of last modification : 96/02/25
*   Pathname of SCCS file     : /sgiext/molmol/sg/include/SCCS/s.sg_shade.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <bool.h>

extern void SgDrawShadedPolygon2D(float[][2], float[][3], int);

extern BOOL SgHas2DShading(void);
