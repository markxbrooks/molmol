/*
************************************************************************
*
*   unknown_err.h - utility functions for printing error messages (unknown ...)
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
*   Date of last modification : 96/12/04
*   Pathname of SCCS file     : /local/home/kor/molmol/src/cmdio/SCCS/s.unknown_err.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <dstr.h>

extern void UnknownErrInit(void);

extern void UnknownErrAddRes(DSTR name);

extern void UnknownErrAddAtom(DSTR resName, DSTR atomName);

extern DSTR UnknownErrGet(void);
