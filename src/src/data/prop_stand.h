/*
************************************************************************
*
*   prop_stand.h - handling of standard properties
*
*   Copyright (c) 1994-98
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
*   Date of last modification : 98/07/21
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homea/rkoradi/molmol-master/src/data/SCCS/s.prop_stand.h
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <bool.h>
#include <g_file.h>

extern void PropStandSetMol(DhMolP);

extern void PropStandSetRes(DhResP);

extern void PropStandSetAtom(DhAtomP);

extern void PropStandSetBond(DhBondP);

extern void PropStandSetDist(DhDistP);

extern BOOL PropStandDump(GFile);

extern BOOL PropStandUndump(GFile, int);

extern void PropStandDestroyAll(void);
