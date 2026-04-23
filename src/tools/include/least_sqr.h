/*
************************************************************************
*
*   least_sqr.h - solve least squares problems
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
*   Date of last modification : 94/06/02
*   Pathname of SCCS file     : /sgiext/molmol/tools/include/SCCS/s.least_sqr.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#ifndef _LEAST_SQR_H_
#define _LEAST_SQR_H_

extern void LeastSqrMatTransf(float **, int, int, float *);

extern void LeastSqrCalcSol(float **, float *, float*, int, int,
    float *, float *);

#endif  /* _LEAST_SQR_H_ */
