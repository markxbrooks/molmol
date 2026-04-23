/*
************************************************************************
*
*   princip_axis.h - principal axis transformation
*
*   Copyright (c) 1994-95
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
*   Date of last modification : 95/08/01
*   Pathname of SCCS file     : /sgiext/molmol/tools/include/SCCS/s.princip_axis.h
*   SCCS identification       : 1.3
*
************************************************************************
*/

#ifndef _PRINCIP_AXIS_H_
#define _PRINCIP_AXIS_H_

#include <bool.h>
#include <mat_vec.h>

extern void PrincipAxis(Vec3[], int, BOOL, Mat4, float[3]);

#endif  /* _PRINCIP_AXIS_H_ */
