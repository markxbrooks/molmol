/*
************************************************************************
*
*   Grid.c - implementation of GRID type
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
*   Date of last modification : 01/05/26
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/tools/src/SCCS/s.Grid.c
*   SCCS identification       : 1.7
*
************************************************************************
*/

#include <grid.h>

#include <stdio.h>
#include <stdlib.h>
#include <values.h>
#include <math.h>

#define MAX_CELL_NO 10000

typedef struct {
  int entryNo;
  int size;
  void **entryPA;
} Cell, *CellP;

struct gridStruc {
  float coordMin[3], coordMax[3];
  float cellSize, cellSizeInv;
  int cellNo[3], totCellNo;
  CellP cells;
};

GRID
GridNew(void)
{
  GRID gridP;
  int coordI;

  gridP = malloc(sizeof(*gridP));

  for (coordI = 0; coordI < 3; coordI++) {
    gridP->coordMin[coordI] = MAXFLOAT;
    gridP->coordMax[coordI] = - MAXFLOAT;
  }

  return gridP;
}

void
GridPrepareAddEntry(GRID gridP, float *x)
{
  int coordI;

  for (coordI = 0; coordI < 3; coordI++) {
    if (x[coordI] < gridP->coordMin[coordI])
      gridP->coordMin[coordI] = x[coordI];
    if (x[coordI] > gridP->coordMax[coordI])
      gridP->coordMax[coordI] = x[coordI];
  }
}

static void
setSizes(GRID gridP, float cellSize)
{
  int coordI;
  float intervSize, excess;

  gridP->cellSize = cellSize;
  gridP->cellSizeInv = 1.0f / gridP->cellSize;

  for (coordI = 0; coordI < 3; coordI++) {
    intervSize = gridP->coordMax[coordI] - gridP->coordMin[coordI];

    if (intervSize < 0.0f) {
      /* no entries, set size to 1 so that malloc works */
      gridP->cellNo[coordI] = 1;
    } else {
      gridP->cellNo[coordI] =
	  (int) ceilf(gridP->cellSizeInv * intervSize + (float) 1.0e-6);
      if (gridP->cellNo[coordI] == 0)
	gridP->cellNo[coordI] = 1;

      excess = gridP->cellNo[coordI] * gridP->cellSize - intervSize;
      gridP->coordMin[coordI] -= 0.5f * excess;
      gridP->coordMax[coordI] += 0.5f * excess;
    }
  }

  gridP->totCellNo = 1;
  for (coordI = 0; coordI < 3; coordI++)
    gridP->totCellNo *= gridP->cellNo[coordI];
}

void
GridInsertInit(GRID gridP, float cellSize)
{
  int cellI;

  for (;;) {
    setSizes(gridP, cellSize);
    if (gridP->totCellNo < MAX_CELL_NO)
      break;
    cellSize *= 1.5f;
  }

  gridP->cells = malloc(gridP->totCellNo * sizeof(*gridP->cells));
  for (cellI = 0; cellI < gridP->totCellNo; cellI++) {
    gridP->cells[cellI].entryNo = 0;
    gridP->cells[cellI].size = 5;
    gridP->cells[cellI].entryPA = malloc(
	gridP->cells[cellI].size * sizeof(void *));
  }
}

static void
getCellInd(GRID gridP, int cellI[3], float *x)
{
  int coordI;

  for (coordI = 0; coordI < 3; coordI++)
    cellI[coordI] = (int) (gridP->cellSizeInv *
	(x[coordI] - gridP->coordMin[coordI]));
}

static CellP
getCell(GRID gridP, int cellI[3])
{
  return gridP->cells +
      (cellI[2] * gridP->cellNo[1] + cellI[1]) * gridP->cellNo[0] + cellI[0];
}

void
GridInsertEntry(GRID gridP, float *x, void *entryP)
{
  int cellI[3];
  CellP cellP;

  getCellInd(gridP, cellI, x);
  cellP = getCell(gridP, cellI);

  if (cellP->entryNo == cellP->size) {
    cellP->size *= 2;
    cellP->entryPA = realloc(cellP->entryPA, cellP->size * sizeof(void *));
  }

  cellP->entryPA[cellP->entryNo] = entryP;
  cellP->entryNo++;
}

void
GridFind(GRID gridP, float *x, int range, GridFindCB findCB, void *clientData)
{
  int cellI[3], nCellI[3], entryI;
  CellP cellP;

  getCellInd(gridP, cellI, x);

  for (nCellI[0] = cellI[0] - range; nCellI[0] <= cellI[0] + range;
      nCellI[0]++) {
    if (nCellI[0] < 0)
      continue;
    if (nCellI[0] >= gridP->cellNo[0])
      continue;

    for (nCellI[1] = cellI[1] - range; nCellI[1] <= cellI[1] + range;
	nCellI[1]++) {
      if (nCellI[1] < 0)
	continue;
      if (nCellI[1] >= gridP->cellNo[1])
	continue;

      for (nCellI[2] = cellI[2] - range; nCellI[2] <= cellI[2] + range;
	  nCellI[2]++) {
	if (nCellI[2] < 0)
	  continue;
	if (nCellI[2] >= gridP->cellNo[2])
	  continue;

	cellP = getCell(gridP, nCellI);

	for (entryI = 0; entryI < cellP->entryNo; entryI++)
	  if (! findCB(cellP->entryPA[entryI], x, clientData))
	    return;
      }
    }
  }
}

void
GridDestroy(GRID gridP)
{
  int cellI;

  for (cellI = 0; cellI < gridP->totCellNo; cellI++)
    free(gridP->cells[cellI].entryPA);

  free(gridP->cells);
  free(gridP);
}
