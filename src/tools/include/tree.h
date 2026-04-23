/*
************************************************************************
*
*   tree.h - tree manager
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
*   Pathname of SCCS file     : /sgiext/molmol/tools/include/SCCS/s.tree.h
*   SCCS identification       : 1.1
*
************************************************************************
*/
#ifndef _TREE_H_
#define _TREE_H_

#include <callback.h>

typedef enum {
  TIB_RETURN_NULL,
  TIB_RETURN_OLD,
  TIB_OVERWRITE
} TreeInsBehav_E;

typedef struct treeStruc *TREE;       /* Tree pointer */

typedef int (*TreeCompareFunc) (void *, void *);
typedef void (*TreeApplyFunc) (void *, void *);
typedef void (*TreePrintFunc) (void *);

extern TREE TreeOpen(unsigned, TreeCompareFunc);
extern void TreeClose(TREE);
extern void TreeApply(TREE, TreeApplyFunc, void *);
extern void TreeAddDestroyCB(TREE, DestroyCB, void *, FreeCldta);
extern void TreeRemoveDestroyCB(TREE, DestroyCB, void *);
extern void *TreeInsert(TREE, void *, TreeInsBehav_E);
extern void *TreeSearch(TREE, void *);
extern void TreeRemove(TREE, void *);
extern unsigned TreeSize(TREE);
extern void TreePrint(TREE, TreePrintFunc);

#endif  /* _TREE_H_ */
