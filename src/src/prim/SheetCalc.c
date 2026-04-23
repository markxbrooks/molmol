/*
************************************************************************
*
*   SheetCalc.c - calculate sheet
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
*   Date of last modification : 01/05/27
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/prim/SCCS/s.SheetCalc.c
*   SCCS identification       : 1.7
*
************************************************************************
*/

#include "sheet_calc.h"

#include <stdio.h>
#include <stdlib.h>

#include <mat_vec.h>
#include <par_hand.h>
#include <par_names.h>
#include <prop_def.h>
#include <prop_second.h>
#include <data_hand.h>
#include "prim_struc.h"

#define SHEET_THICK 0.01f
#define OVERLAP 2

typedef struct {
  DhAtomP atomP;
  DhResP resP;
  BOOL isSheet;
  BOOL posSet;
  int strandI, strandPos;
} ResData;

typedef struct {
  BOOL coordSet;
  Vec3 coord;
} GridData;

static void
setPos(ResData *dataA, int resI, int minResI, int maxResI,
    int strandI, int strandPos)
{
  int dataI;
  int sInc, rI, sPos;
  DhResP nResP;

  if (resI < minResI || resI > maxResI)
    return;

  dataI = resI - minResI;
  if (dataA[dataI].posSet || ! dataA[dataI].isSheet)
    return;

  dataA[dataI].posSet = TRUE;
  dataA[dataI].strandI = strandI;
  dataA[dataI].strandPos = strandPos;

  if (strandI == 0 && strandPos == 0) {
    sInc = 1;
  } else if (dataI > 0 && dataA[dataI - 1].posSet) {
    if (dataA[dataI - 1].strandPos < dataA[dataI].strandPos)
      sInc = 1;
    else
      sInc = -1;
  } else if (dataI < maxResI - minResI && dataA[dataI + 1].posSet) {
    if (dataA[dataI].strandPos < dataA[dataI + 1].strandPos)
      sInc = 1;
    else
      sInc = -1;
  } else {
    sInc = 0;
  }

  if (sInc != 0) {
    rI = resI;
    sPos = strandPos;
    while (rI <= maxResI && dataA[rI - minResI].isSheet) {
      setPos(dataA, rI, minResI, maxResI, strandI, sPos);
      rI++;
      sPos += sInc;
    }

    rI = resI;
    sPos = strandPos;
    while (rI >= minResI && dataA[rI - minResI].isSheet) {
      setPos(dataA, rI, minResI, maxResI, strandI, sPos);
      rI--;
      sPos -= sInc;
    }
  }

  nResP = DhResGetNeigh(dataA[dataI].resP, NC_LEFT);
  if (nResP != NULL)
    setPos(dataA, DhResGetNumber(nResP), minResI, maxResI,
	strandI - 1, strandPos);

  nResP = DhResGetNeigh(dataA[dataI].resP, NC_RIGHT);
  if (nResP != NULL)
    setPos(dataA, DhResGetNumber(nResP), minResI, maxResI,
	strandI + 1, strandPos);
}

static void
getSplinePoint(Vec3 x, Vec3 dx, CubicSplineCoeff *coeffP, float par)
{
  int i;

  for (i = 0; i < 3; i++) {
    x[i] = ((coeffP->c3[i] * par + coeffP->c2[i]) * par +
	coeffP->c1[i]) * par + coeffP->c0[i];
    if (dx != NULL)
      dx[i] = (3.0f * coeffP->c3[i] * par +
	  2.0f * coeffP->c2[i]) * par + coeffP->c1[i];
  }
}

static void
fillGrid(GridData *grid, int strandNo, int strandLen,
    ResData *dataA, int dataNo)
{
  int gridI, dataI, strandI;
  CubicSplineCoeff *coeffA;
  Vec3 coord, *xsA, v1;
  int splineLen, splineI;
  int i1, i2, i;

  for (gridI = 0; gridI < strandNo * strandLen; gridI++)
    grid[gridI].coordSet = FALSE;
  
  for (dataI = 0; dataI < dataNo; dataI++) {
    if (! dataA[dataI].posSet)
      continue;
    
    gridI = dataA[dataI].strandI * strandLen + dataA[dataI].strandPos;

    DhAtomGetCoord(dataA[dataI].atomP, grid[gridI].coord);
    if (dataI > 0 && dataA[dataI - 1].atomP != NULL &&
	dataI < dataNo - 1 && dataA[dataI + 1].atomP != NULL) {
      /* smooth */
      Vec3Scale(grid[gridI].coord, 0.5f);
      DhAtomGetCoord(dataA[dataI - 1].atomP, coord);
      Vec3ScaleAdd(grid[gridI].coord, 0.25f, coord);
      DhAtomGetCoord(dataA[dataI + 1].atomP, coord);
      Vec3ScaleAdd(grid[gridI].coord, 0.25f, coord);
    }
    grid[gridI].coordSet = TRUE;
  }

  coeffA = malloc(strandLen * sizeof(*coeffA));
  xsA = malloc(strandLen * sizeof(*xsA));

  for (strandI = 0; strandI < strandNo; strandI++) {
    splineLen = 0;
    i1 = -1;
    for (i = 0; i < strandLen; i++) {
      gridI = strandI * strandLen + i;
      if (grid[gridI].coordSet) {
	Vec3Copy(xsA[splineLen], grid[gridI].coord);
	splineLen++;
	if (i1 < 0) {
	  i1 = i;
	  Vec3Copy(v1, grid[gridI].coord);
	}
	i2 = i;
      }
    }
    i2 = strandLen - 1 - i2;

    CubicSplineCalc(xsA, splineLen, coeffA);

    /* fill missing coordinates at start of strand */
    for (i = 0; i <= i1; i++) {
      gridI = strandI * strandLen + i;
      if (splineLen > 1) {
	getSplinePoint(grid[gridI].coord, NULL,
	    coeffA, (float) i / (i1 + 1));
      } else {
	/* can't happen if each strand has at least 2 residues,
	   take short line in arbitrary direction */
	Vec3Copy(grid[gridI].coord, v1);
	grid[gridI].coord[0] -= (i1 - i) * 0.1f;
      }
      grid[gridI].coordSet = TRUE;
    }

    /* fill missing coordinates at end of strand */
    for (i = 0; i <= i2; i++) {
      gridI = strandI * strandLen + strandLen - 1 - i;
      if (splineLen > 1) {
	getSplinePoint(grid[gridI].coord, NULL,
	    coeffA + splineLen - 2, 1.0f - (float) i / (i2 + 1));
      } else {
	/* can't happen if each strand has at least 2 residues,
	   take short line in arbitrary direction */
	Vec3Copy(grid[gridI].coord, v1);
	grid[gridI].coord[0] += (i2 - i) * 0.1f;
      }
      grid[gridI].coordSet = TRUE;
    }

    /* fill gaps of strand */
    i1 = 0;
    splineI = 0;
    for (i2 = 0; i2 < strandLen; i2++)
      if (grid[strandI * strandLen + i2].coordSet) {
	for (i = i1 + 1; i < i2; i++) {
	  gridI = strandI * strandLen + i;
	  getSplinePoint(grid[gridI].coord, NULL,
	      coeffA + splineI, (float) (i - i1) / (i2 - i1));
	}
	i1 = i;
	splineI++;
      }
  }

  free(coeffA);
  free(xsA);
}

static void
fillPoints(Vec3 *xA, Vec3 *nvA,
    GridData *grid, int strandNo, int strandLen, BOOL closed, int subDiv)
{
  int strandInc, maxStrandI, splineLen;
  int strandI, strandPos, gridI;
  CubicSplineCoeff *coeff1A, *coeff2A, *coeffP;
  Vec3 *xsA, *dxA, dx;
  float par1, par2, parInc;
  int i1, i2, maxI, xI;

  if (closed) {
    strandInc = OVERLAP;
    maxStrandI = strandNo;
  } else {
    strandInc = 0;
    maxStrandI = strandNo - 1;
  }

  splineLen = strandNo + 2 * strandInc;

  coeff1A = malloc(strandLen * (splineLen - 1) * sizeof(*coeff1A));
  xsA = malloc(splineLen * sizeof(*xsA));

  for (strandPos = 0; strandPos < strandLen; strandPos++) {
    for (strandI = 0; strandI < splineLen; strandI++) {
      gridI = (strandI - strandInc + strandNo) % strandNo;
      gridI = gridI * strandLen + strandPos;
      Vec3Copy(xsA[strandI], grid[gridI].coord);
    }
    
    CubicSplineCalc(xsA, splineLen, coeff1A + strandPos * (splineLen - 1));
  }

  free(xsA);

  coeff2A = malloc((strandLen - 1) * sizeof(*coeff2A));
  xsA = malloc(strandLen * sizeof(*xsA));
  dxA = malloc(strandLen * sizeof(*dxA));

  parInc = 1.0f / subDiv;

  /* this could be optimized by using a more efficient spline
     evaluation algorithm (forward differences) */
  for (strandI = 0; strandI < maxStrandI; strandI++) {
    par1 = 0.0f;
    if (strandI == maxStrandI - 1 && ! closed)
      maxI = subDiv;
    else
      maxI = subDiv - 1;

    for (i1 = 0; i1 <= maxI; i1++) {
      for (strandPos = 0; strandPos < strandLen; strandPos++)
	getSplinePoint(xsA[strandPos], dxA[strandPos],
	    coeff1A + strandPos * (splineLen - 1) + strandI + strandInc,
	    par1);

      CubicSplineCalc(xsA, strandLen, coeff2A);

      for (strandPos = 0; strandPos < strandLen - 1; strandPos++) {
	coeffP = coeff2A + strandPos;
	par2 = 0.0f;
	for (i2 = 0; i2 <= subDiv; i2++) {
	  xI = (subDiv * (strandLen - 1) + 1) * (strandI * subDiv + i1) +
	      strandPos * subDiv + i2;
	  getSplinePoint(xA[xI], nvA[xI], coeffP, par2);

	  /* linear interpolation of gradient vector */
	  Vec3Zero(dx);
	  Vec3ScaleAdd(dx, 1.0f - par2, dxA[strandPos]);
	  Vec3ScaleAdd(dx, par2, dxA[strandPos + 1]);
	  Vec3Cross(dx, nvA[xI]);
	  Vec3Norm(dx);
	  Vec3Copy(nvA[xI], dx);

	  par2 += parInc;
	}
      }
      par1 += parInc;
    }
  }
  
  free(coeff1A);
  free(coeff2A);
  free(xsA);
  free(dxA);
}

void
SheetCalc(PrimObjP primP)
{
  PrimSheet *sheetP;
  int prec;
  PropRefP refP;
  int resNo;
  int resI, minResI, maxResI;
  ResData *resDataA;
  DhResP resP, nResP;
  int minStrandI, minStrandPos, strandNo, strandLen;
  GridData *grid;
  int subDiv;
  int traceI, ind1, ind2;
  int i;

  sheetP = &primP->u.sheet;

  prec = ParGetIntVal(PN_DRAW_PREC);
  if (sheetP->pointValid && prec == sheetP->prec)
    return;
  
  sheetP->prec = prec;

  if (sheetP->xA1 != NULL) {
    free(sheetP->xA1);
    free(sheetP->xA2);
    free(sheetP->nvA1);
    free(sheetP->nvA2);
  }

  refP = PropGetRef(PROP_SHEET, FALSE);

  resNo = 0;
  for (i = 0; i < sheetP->atomNo; i++) {
    resP = DhAtomGetRes(sheetP->atomA[i]);
    if (DhResGetProp(refP, resP))
      resNo++;

    resNo++;
    resI = DhResGetNumber(resP);
    if (i == 0 || resI < minResI)
      minResI = resI;
    if (i == 0 || resI > maxResI)
      maxResI = resI;
  }

  if (resNo < 4)
    return;

  resDataA = malloc((maxResI - minResI + 1) * sizeof(*resDataA));
  for (i = minResI; i <= maxResI; i++) {
    resDataA[i - minResI].atomP = NULL;
    resDataA[i - minResI].resP = NULL;
    resDataA[i - minResI].isSheet = FALSE;
    resDataA[i - minResI].posSet = FALSE;
  }

  for (i = 0; i < sheetP->atomNo; i++) {
    resP = DhAtomGetRes(sheetP->atomA[i]);
    resI = DhResGetNumber(resP);
    if (DhResGetProp(refP, resP))
      resDataA[resI - minResI].isSheet = TRUE;

    resDataA[resI - minResI].atomP = sheetP->atomA[i];
    resDataA[resI - minResI].resP = resP;
  }

  minStrandI = 0;
  minStrandPos = 0;
  for (i = minResI; i <= maxResI; i++)
    if (resDataA[i - minResI].isSheet) {
      setPos(resDataA, i, minResI, maxResI, minStrandI, minStrandPos);
      break;
    }

  strandNo = 0;
  strandLen = 0;
  for (i = minResI; i <= maxResI; i++) {
    if (! resDataA[i - minResI].posSet)
      continue;
    if (resDataA[i - minResI].strandI < minStrandI)
      minStrandI = resDataA[i - minResI].strandI;
    if (resDataA[i - minResI].strandPos < minStrandPos)
      minStrandPos = resDataA[i - minResI].strandPos;
    if (resDataA[i - minResI].strandI > strandNo)
      strandNo = resDataA[i - minResI].strandI;
    if (resDataA[i - minResI].strandPos > strandLen)
      strandLen = resDataA[i - minResI].strandPos;
  }

  sheetP->closed = FALSE;
  for (i = minResI; i <= maxResI; i++) {
    if (! resDataA[i - minResI].posSet)
      continue;

    resDataA[i - minResI].strandI -= minStrandI;
    resDataA[i - minResI].strandPos -= minStrandPos;

    if (resDataA[i - minResI].strandI == 0) {
      nResP = DhResGetNeigh(resDataA[i - minResI].resP, NC_LEFT);
      if (nResP != NULL) {
	resI = DhResGetNumber(nResP);
	if (resI >= minResI && resI <= maxResI &&
	    resDataA[resI - minResI].posSet)
	  sheetP->closed = TRUE;
      }
    }
  }

  strandNo = strandNo - minStrandI + 1;
  strandLen = strandLen - minStrandPos + 1;

  if (strandNo < 2 || strandLen < 2) {
    free(resDataA);
    return;
  }

  grid = malloc(strandNo * strandLen * sizeof(*grid));
  fillGrid(grid, strandNo, strandLen, resDataA, maxResI - minResI + 1);

  free(resDataA);

  subDiv = 1 << sheetP->prec;

  if (sheetP->closed)
    sheetP->traceNo = subDiv * strandNo;
  else
    sheetP->traceNo = subDiv * (strandNo - 1) + 1;

  sheetP->traceLen = subDiv * (strandLen - 1) + 1;

  sheetP->xA1 = malloc(sheetP->traceNo * sheetP->traceLen * sizeof(Vec3));
  sheetP->nvA1 = malloc(sheetP->traceNo * sheetP->traceLen * sizeof(Vec3));

  fillPoints(sheetP->xA1, sheetP->nvA1, grid, strandNo, strandLen,
      sheetP->closed, subDiv);

  free(grid);

  sheetP->xA2 = malloc(sheetP->traceNo * sheetP->traceLen * sizeof(Vec3));
  sheetP->nvA2 = malloc(sheetP->traceNo * sheetP->traceLen * sizeof(Vec3));

  for (traceI = 0; traceI < sheetP->traceNo; traceI++)
    for (i = 0; i < sheetP->traceLen; i++) {
      ind1 = traceI * sheetP->traceLen + i;
      ind2 = (sheetP->traceNo - 1 - traceI) * sheetP->traceLen + i;

      Vec3Copy(sheetP->xA2[ind2], sheetP->xA1[ind1]);

      Vec3Copy(sheetP->nvA2[ind2], sheetP->nvA1[ind1]);
      Vec3Scale(sheetP->nvA2[ind2], -1.0f);

      if (traceI > 0 && traceI < sheetP->traceNo - 1 &&
	  i > 0 && i < sheetP->traceLen - 1) {
	Vec3ScaleAdd(sheetP->xA1[ind1], SHEET_THICK, sheetP->nvA1[ind1]);
	Vec3ScaleAdd(sheetP->xA2[ind2], SHEET_THICK, sheetP->nvA2[ind2]);
      }
    }

  sheetP->pointValid = TRUE;
}
