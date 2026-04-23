/*
************************************************************************
*
*   calc_rmsd.h - calculate RMSD
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
*   Date of last modification : 96/03/14
*   Pathname of SCCS file     : /sgiext/molmol/include/SCCS/s.calc_rmsd.h
*   SCCS identification       : 1.3
*
************************************************************************
*/

#ifndef _CALC_RMSD_
#define _CALC_RMSD_

#include <bool.h>

typedef void (* CalcReportF) (char *);
typedef void (* CalcErrorF) (char *);

extern void CalcRmsd(BOOL, char *, char *, char *, char *, char *,
    char *, char *, char *,
    CalcReportF, CalcErrorF);

#endif  /* _CALC_RMSD_ */
