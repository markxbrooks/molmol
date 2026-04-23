/*
************************************************************************
*
*   linlist.h - linear list manager
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
*   Pathname of SCCS file     : /sgiext/molmol/tools/include/SCCS/s.linlist.h
*   SCCS identification       : 1.1
*
************************************************************************
*/
#ifndef _LINLIST_H_
#define _LINLIST_H_

#include <callback.h>

typedef struct listStruc *LINLIST;       /* List pointer */

typedef void (*ListApplyFunc) (void *, void *);
typedef int (*ListEntryCompareFunc) (void *, void *);

extern LINLIST ListOpen(unsigned);
extern void ListClose(LINLIST);
extern void ListApply(LINLIST, ListApplyFunc, void *);
extern void ListAddDestroyCB(LINLIST, DestroyCB, void *, FreeCldta);
extern void ListRemoveDestroyCB(LINLIST, DestroyCB, void *);
extern void *ListInsertFirst(LINLIST, void *);
extern void *ListInsertLast(LINLIST, void *);
extern void *ListInsertBefore(LINLIST, void *, void *);
extern void *ListInsertAfter(LINLIST, void *, void *);
extern void ListMoveFirst(LINLIST, void *);
extern void ListSwap(LINLIST, void *, void *);
extern void ListRemove(LINLIST, void *);
extern void ListSort(LINLIST, ListEntryCompareFunc);
extern void *ListFirst(LINLIST);
extern void *ListLast(LINLIST);
extern void *ListNext(LINLIST, void *);
extern void *ListPrev(LINLIST, void *);
extern void *ListPos(LINLIST, int);
extern unsigned ListSize(LINLIST);

#endif  /* _LINLIST_H_ */
