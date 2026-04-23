/*
************************************************************************
*
*   def_shade.h - shading, used internally in Sg
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
*   Date of last modification : 96/03/05
*   Pathname of SCCS file     : /local/home/kor/molmol/sg/src/def/SCCS/s.def_shade.h
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include <sg_types.h>

extern void DefShade(float *, float *, float *, float[3], float[3]);

extern void DefFog(float *, float *, float *, float);

extern SgShadeModel DefGetShadeModel(void);

extern BOOL DefFogEnabled(void);
