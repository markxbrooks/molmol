/*
************************************************************************
*
*   prop_tab.h - management of property tables
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
*   Date of last modification : 96/02/13
*   Pathname of SCCS file     : /sgiext/molmol/include/SCCS/s.prop_tab.h
*   SCCS identification       : 1.4
*
************************************************************************
*/

#ifndef _PROP_TAB_H_
#define _PROP_TAB_H_

#include <bool.h>
#include <g_file.h>
#include <prop_def.h>

struct PropRefS {
  int index;
  unsigned mask;
};

typedef void (*PropListFunc) (char *, void *);

extern unsigned *PropNewTab(BOOL);

extern unsigned *PropGet(unsigned *);

extern unsigned *PropChange(unsigned *, PropRefP, BOOL);

extern void PropFreeTab(unsigned *);

extern void PropList(unsigned *, PropListFunc, void *);

extern BOOL PropDump(GFile gf);

extern int PropGetIndex(unsigned *);

extern BOOL PropUndumpStart(GFile gf, int);

extern unsigned *PropFindIndex(int);

extern void PropUndumpEnd(void);

#endif  /* _PROP_TAB_H_ */
