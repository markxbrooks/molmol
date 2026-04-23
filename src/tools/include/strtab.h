/*
************************************************************************
*
*   strtab.h - string table manager
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
*   Pathname of SCCS file     : /sgiext/molmol/tools/include/SCCS/s.strtab.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#ifndef _STR_TAB_H_
#define _STR_TAB_H_

#include <bool.h>

typedef struct strtab *STRTAB;

typedef void (*StrTabApplyFunc) (int, char *, BOOL, void *);

extern STRTAB StrTabOpen(int);

extern void StrTabApply(STRTAB, StrTabApplyFunc, void *);

extern int StrTabDefine(STRTAB, char *, BOOL);

extern int StrTabIndex(STRTAB, char *);

extern void StrTabClose(STRTAB);

#endif  /* _STR_TAB_H_ */
