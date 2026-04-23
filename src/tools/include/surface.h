/*
************************************************************************
*
*   surface.h - molecule surface calculation
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
*   Pathname of SCCS file     : /sgiext/molmol/tools/include/SCCS/s.surface.h
*   SCCS identification       : 1.2
*
************************************************************************
*/

#ifndef _SURFACE_H_
#define _SURFACE_H_

#include <bool.h>
#include <mat_vec.h>
#include <iso_surface.h>

typedef struct {
  Vec3 cent;
  float rad;
  void *userData;
} SphereDescr;

typedef struct {
  Vec3 *pointA;
  int pointNo;
  void *userData;
} PatchDescr;

extern void SurfaceDots(SphereDescr[], int, float, int, BOOL,
    PatchDescr **, int *);

extern void SurfaceSolid(SphereDescr[], int, float, int,
    IsoSurface **, int *);

#endif  /* _SURFACE_H_ */
