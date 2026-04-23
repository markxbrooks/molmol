/*
************************************************************************
*
*   data_res_lib.h - management of residue library
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
*   Date of last modification : 97/04/01
*   Pathname of SCCS file     : /local/home/kor/molmol/src/data/SCCS/s.data_res_lib.h
*   SCCS identification       : 1.6
*
************************************************************************
*/

#include <data_hand.h>

extern DhResDefP DhResDefGetRef(DhResDefP);
extern void DhResDefReturn(DhResDefP);

extern BOOL DhResDefIsPseudo(DhResDefP, int, int);

extern int DhResDefAddAtom(DhResDefP *, int, DSTR);
extern int DhResDefAddBond(DhResDefP *, int, int);
extern void DhResDefRemoveBond(DhResDefP *, int);
extern void DhResDefRemoveAngle(DhResDefP *, int);
extern void DhResDefSetPseudo(DhResDefP *, int, int[], int);
extern void DhResDefSetCharge(DhResDefP *, int, float);

extern void DhResLibDestroyAll(void);

extern BOOL DhResLibDump(GFile);
extern BOOL DhResLibUndump(GFile, int);
extern DhResDefP DhResDefFindIndex(int);
