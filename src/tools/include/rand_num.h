/*
************************************************************************
*
*   rand_num.h - random number generation
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
*   Date of last modification : 96/08/02
*   Pathname of SCCS file     : /sgiext/molmol/tools/include/SCCS/s.rand_num.h
*   SCCS identification       : 1.2
*
************************************************************************
*/

#ifndef _RAND_NUM_H_
#define _RAND_NUM_H_

extern void RandSetSeed(int);

extern int RandInt32(void);

extern float RandFloat(void);

extern float RandGauss(void);

#endif  /* _RAND_NUM_H_ */
