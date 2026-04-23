/*
************************************************************************
*
*   data_ref.h - manage reference atom(s)
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
*   Date of last modification : 95/05/25
*   Pathname of SCCS file     : /sgiext/molmol/include/SCCS/s.data_ref.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#ifndef _DATA_REF_H_
#define _DATA_REF_H_

#include <data_hand.h>

extern void RefInit(void);

extern void RefAddAtom(DhAtomP);

extern float RefGetDist(Vec3);

#endif  /* _DATA_REF_H_ */
