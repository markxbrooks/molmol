/*
************************************************************************
*
*   data_sel.h - utility functions for selection
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
*   Pathname of SCCS file     : /sgiext/molmol/include/SCCS/s.data_sel.h
*   SCCS identification       : 1.3
*
************************************************************************
*/

#ifndef _DATA_SEL_H_
#define _DATA_SEL_H_

#include <data_ent.h>
#include <data_hand.h>

typedef enum {
  SK_ADD,
  SK_REMOVE,
  SK_REPLACE
} SelKind;

typedef void (* SelCB) (DataEntityType, void *, SelKind);

extern void SelAddCB(SelCB);

extern int SelMolGet(DhMolP[], int);

extern int SelResGet(DhResP[], int);

extern int SelAtomGet(DhAtomP[], int);

extern int SelBondGet(DhBondP[], int);

extern int SelAngleGet(DhAngleP[], int);

extern int SelDistGet(DhDistP[], int);

extern void SelMol(DhMolP, SelKind);

extern void SelRes(DhResP, SelKind);

extern void SelAtom(DhAtomP, SelKind);

extern void SelBond(DhBondP, SelKind);

extern void SelAngle(DhAngleP, SelKind);

extern void SelDist(DhDistP, SelKind);

extern void DeselAllMol(void);

extern void DeselAllRes(void);

extern void DeselAllAtom(void);

extern void DeselAllBond(void);

extern void DeselAllAngle(void);

extern void DeselAllDist(void);

#endif  /* _DATA_SEL_H_ */
