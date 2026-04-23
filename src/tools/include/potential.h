/*
************************************************************************
*
*   potential.h - calculation of electrostatic potential
*
*   Copyright (c) 1996
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
*   Date of last modification : 96/04/16
*   Pathname of SCCS file     : /sgiext/molmol/tools/include/SCCS/s.potential.h
*   SCCS identification       : 1.5
*
************************************************************************
*/

#ifndef _POTENTIAL_H_
#define _POTENTIAL_H_

#include <bool.h>
#include <mat_vec.h>

typedef struct {
  Vec3 cent;
  float rad;
  float charge;
} PotAtomDescr;

typedef enum {
  PBC_ZERO,
  PBC_DEBYE
} PotBoundaryCond;

typedef struct {
  float origin[3];
  int pointNo[3];
  float spacing;
  float *valueA;
} PotGridDescr;

extern BOOL PotentialCalc(PotAtomDescr[], int, float,
    float, float,
    float, float,
    float, PotBoundaryCond, int,
    PotGridDescr *);

#endif  /* _POTENTIAL_H_ */
