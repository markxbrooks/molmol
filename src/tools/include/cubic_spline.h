/*
************************************************************************
*
*   cubic_spline.h - cubic spline calculation
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
*   Date of last modification : 95/04/07
*   Pathname of SCCS file     : /sgiext/molmol/tools/include/SCCS/s.cubic_spline.h
*   SCCS identification       : 1.2
*
************************************************************************
*/

#ifndef _CUBIC_SPLINE_H_
#define _CUBIC_SPLINE_H_

#include <mat_vec.h>

typedef struct {
  float c3[3], c2[3], c1[3], c0[3];
} CubicSplineCoeff;

extern void CubicSplineCalc(Vec3[], int, CubicSplineCoeff[]);

#endif  /* _CUBIC_SPLINE_H_ */
