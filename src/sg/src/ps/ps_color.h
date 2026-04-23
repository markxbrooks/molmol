/*
************************************************************************
*
*   ps_color.h - PostScript color setting
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
*   Date of last modification : 96/07/23
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/ps/SCCS/s.ps_color.h
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include <sg_types.h>

extern void SgPSSetColor(float, float, float, float, BOOL);

extern void PSInvalidateColor(void);

extern void PSUpdateColor(void);
