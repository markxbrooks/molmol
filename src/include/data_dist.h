/*
************************************************************************
*
*   data_dist.h - manage list of distances
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
*   Date of last modification : 95/02/28
*   Pathname of SCCS file     : /sgiext/molmol/include/SCCS/s.data_dist.h
*   SCCS identification       : 1.2
*
************************************************************************
*/

#ifndef _DATA_DIST_
#define _DATA_DIST_

#include <data_hand.h>

typedef struct DistListS *DistList;

typedef void (* DistListApplyF) (DhDistP, void *);

extern DistList DistListBuildIntra(PropRefP, DhMolP *, int, DhDistKind);

extern DistList DistListBuildInter(PropRefP, DhDistKind);

extern DhDistP DistListFind(DistList, DhAtomP, DhAtomP);

extern void DistListApply(DistList, DistListApplyF, void *);

extern void DistListFree(DistList);

#endif  /* _DATA_DIST_ */
