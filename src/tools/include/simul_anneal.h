/*
************************************************************************
*
*   simul_anneal.h - minimization by simulated annealing
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
*   Date of last modification : 95/04/18
*   Pathname of SCCS file     : /sgiext/molmol/tools/include/SCCS/s.simul_anneal.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#ifndef _SIMUL_ANNEAL_H_
#define _SIMUL_ANNEAL_H_

#include <bool.h>

typedef BOOL (* MinFunc) (float *, float *, int, void *);

extern void SimulAnnealGenStart(float *, int, float *, float *);

extern BOOL SimulAnnealMin(MinFunc, int, float *, void *,
    float *, float *, float *, int);

#endif  /* _SIMUL_ANNEAL_H_ */
