/*
************************************************************************
*
*   fit_cylinder.h - fit cylinder
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
*   Date of last modification : 96/03/07
*   Pathname of SCCS file     : /sgiext/molmol/tools/include/SCCS/s.fit_cylinder.h
*   SCCS identification       : 1.2
*
************************************************************************
*/

#ifndef _FIT_CYLINDER_H_
#define _FIT_CYLINDER_H_

#include <mat_vec.h>

extern void FitCylinderLeastSq(Vec3[], int, Vec3, Vec3, float *);

extern void FitCylinderSpacing(Vec3[], int, Vec3, Vec3, float *);

#endif  /* _FIT_CYLINDER_H_ */
