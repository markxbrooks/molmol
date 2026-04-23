/*
************************************************************************
*
*   sg_plot.h - plotting, used internally in Sg
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
*   Date of last modification : 96/03/21
*   Pathname of SCCS file     : /sgiext/molmol/sg/include/SCCS/s.sg_plot.h
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <sg_types.h>

extern SgPlotOrient SgPlotGetOrient(void);

extern float SgPlotGetGamma(void);

extern void SgPlotDoGamma(float *, float *, float *);

extern void SgPlotOutputStr(char *);

extern void SgPlotOutputInt(int);

extern void SgPlotOutputFloat(float);
