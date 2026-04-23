/*
************************************************************************
*
*   def_hidden.h - hidden line elimination, used internally in Sg
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
*   Date of last modification : 96/02/25
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/def/SCCS/s.def_hidden.h
*   SCCS identification       : 1.5
*
************************************************************************
*/

extern void DefHiddenMarker(float[3], float, float, float);

extern void DefHiddenLine(float[3], float[3], float, float, float);

extern void DefHiddenPolyline(float[][3], int, float, float, float);

extern void DefHiddenPolygon(float[][3], int, float, float, float);

extern void DefHiddenShadedPolygon(float[][3], float[][3], int);

extern void DefHiddenAnnot(float[3], char *, float, float, float);

extern void DefHiddenText(float[3], char *, float, float, float);
