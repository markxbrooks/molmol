/*
************************************************************************
*
*   iso_surface.h - iso-surface calculation
*
*   Copyright (c) 1995
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
*   Date of last modification : 95/06/05
*   Pathname of SCCS file     : /sgiext/molmol/tools/include/SCCS/s.iso_surface.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#ifndef _ISO_SURFACE_H_
#define _ISO_SURFACE_H_

#include <bool.h>
#include <mat_vec.h>

typedef struct {
  int *indA;
  int pointNo;
} IsoSurfaceMesh;

typedef struct {
  Vec3 *xA;
  Vec3 *nvA;
  int pointNo;
  IsoSurfaceMesh *meshA;
  int meshNo;
} IsoSurface;

extern void IsoSurfaceCalc(float *, int, int, int,
    float, float, float, float, float, float,
    float, BOOL,
    IsoSurface **, int *);

#endif  /* _ISO_SURFACE_H_ */
