/*
************************************************************************
*
*   prim_sel.h - utility functions for selection
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
*   Pathname of SCCS file     : /sgiext/molmol/include/SCCS/s.prim_sel.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#ifndef _PRIM_SEL_H_
#define _PRIM_SEL_H_

#include <prim_hand.h>
#include <data_sel.h>

typedef void (* PrimSelCB) (PrimObjP, SelKind);

extern void PrimSelAddCB(PrimSelCB);

extern int PrimSelGet(PrimObjP[], int);

extern void PrimSel(PrimObjP, SelKind);

extern void PrimDeselAll(void);

#endif  /* _PRIM_SEL_H_ */
