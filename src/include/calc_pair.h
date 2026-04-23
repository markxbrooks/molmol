/*
************************************************************************
*
*   calc_pair.h - find close distances of atom pairs
*
*   Copyright (c) 1994-97
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
*   Date of last modification : 97/04/27
*   Pathname of SCCS file     : /local/home/kor/molmol/include/SCCS/s.calc_pair.h
*   SCCS identification       : 1.6
*
************************************************************************
*/

#ifndef _CALC_PAIR_
#define _CALC_PAIR_

#include <bool.h>
#include <data_hand.h>

typedef enum {
  PE_NONE,
  PE_BOND,
  PE_2_BOND
} PairExclude;

typedef void (* CalcPrepareF) (DhMolP[], int);
typedef BOOL (* CalcCheckAtomF) (DhAtomP);
typedef BOOL (* CalcCheckPairF) (DhAtomP, DhAtomP, float, float *);
typedef BOOL (* CalcGetLimitsF) (DhAtomP, DhAtomP, float *);
typedef void (* CalcListPairF) (DhAtomP, DhAtomP);
typedef void (* CalcReportF) (char *);

extern void CalcPair(float, BOOL, BOOL, PairExclude, int,
    CalcPrepareF,
    char *, char *,
    CalcCheckAtomF, CalcCheckAtomF,
    CalcCheckPairF, int,
    CalcGetLimitsF, int,
    CalcListPairF, CalcReportF);

#endif  /* _CALC_PAIR_ */
