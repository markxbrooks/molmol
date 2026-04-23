/*
************************************************************************
*
*   data_lib.h - management of residue library
*
*   Copyright (c) 1994-96
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
*   Date of last modification : 96/12/06
*   Pathname of SCCS file     : /local/home/kor/molmol/include/SCCS/s.data_lib.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#ifndef _DATA_LIB_H_
#define _DATA_LIB_H_

typedef void (*DhResLibReportFunc) (char *);

extern void DhResLibRead(DhResLibReportFunc);

#endif  /* _DATA_LIB_H_ */
