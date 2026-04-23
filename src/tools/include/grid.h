/*
************************************************************************
*
*   grid.h - GRID type
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
*   Date of last modification : 95/02/06
*   Pathname of SCCS file     : /sgiext/molmol/tools/include/SCCS/s.grid.h
*   SCCS identification       : 1.1
*
************************************************************************
*/

#ifndef _GRID_H_
#define _GRID_H_

#include <bool.h>

typedef BOOL (* GridFindCB) (void *, float *, void *);

typedef struct gridStruc *GRID;

extern GRID GridNew(void);
extern void GridPrepareAddEntry(GRID, float *);
extern void GridInsertInit(GRID, float);
extern void GridInsertEntry(GRID, float *, void *);
extern void GridFind(GRID, float *, int, GridFindCB, void *);
extern void GridDestroy(GRID);

#endif  /* _GRID_H_ */
