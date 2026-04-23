/*
************************************************************************
*
*   hashtab.h - hash table manager
*
*   Copyright (c) 1994-99
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
*   Date of last modification : 99/10/30
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/tools/include/SCCS/s.hashtab.h
*   SCCS identification       : 1.2
*
************************************************************************
*/
#ifndef _HASHTAB_H_
#define _ASHTAB_H_

#include <bool.h>
#include <callback.h>

typedef struct hashTabStruc *HASHTABLE;       /* Hash Table pointer */

/* function pointer types */
typedef unsigned (*HashFunc) (void *, unsigned);
typedef int (*HashCompareFunc) (void *, void *);
typedef void (*HashApplyCB) (void *, void *);

extern HASHTABLE HashtabOpen(unsigned, unsigned, HashFunc, HashCompareFunc);
extern void HashtabClose(HASHTABLE);
extern void HashtabApply(HASHTABLE, HashApplyCB, void *);
extern void HashtabAddDestroyCB(HASHTABLE, DestroyCB, void *, FreeCldta);
extern void HashtabRemoveDestroyCB(HASHTABLE, DestroyCB, void *);
extern void *HashtabInsert(HASHTABLE, void *, BOOL);
extern void *HashtabSearch(HASHTABLE, void *);
extern void HashtabRemove(HASHTABLE, void *);
extern unsigned HashtabSize(HASHTABLE);

#endif  /* _HASHTAB_H_ */
