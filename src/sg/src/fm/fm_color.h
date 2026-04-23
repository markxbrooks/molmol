/*
************************************************************************
*
*   fm_color.h - FrameMaker color setting
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
*   Date of last modification : 95/05/21
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/fm/SCCS/s.fm_color.h
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <sg_types.h>

extern void SgFM3SetColor(float, float, float, float, BOOL);

extern void SgFM4SetColor(float, float, float, float, BOOL);

extern void FM4WriteColorCatalog(void);
