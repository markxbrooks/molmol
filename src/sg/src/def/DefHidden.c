/*
************************************************************************
*
*   DefHidden.c - hidden surface elimination
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
*   Date of last modification : 01/05/27
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/def/SCCS/s.DefHidden.c
*   SCCS identification       : 1.22
*
************************************************************************
*/

#include <sg_def.h>
#include "def_hidden.h"

#include <stdio.h>
#include <stdlib.h>
#include <values.h>
#include <string.h>

#include <mat_vec.h>
#include <linlist.h>
#include <rect_set.h>
#include <sg.h>
#include <sg_get.h>
#include <sg_shade.h>
#include "def_shade.h"

#define EPS ((float) 1.0e-5)
#define EPS_S (0.99f * EPS)
#define UNKNOWN_ORIENT 999

typedef enum {
  EK_MARKER,
  EK_LINE,
  EK_POLYGON,
  EK_ANNOT,
  EK_TEXT
} ElemKind;

typedef struct {
  Rect rect;
  float zMin, zMax;
  ElemKind kind;
  Vec3 *points;
  int pointNo;
  float colR, colG, colB;
  union {
    struct {
      float width;
      SgLineStyle style;
    } line;
    struct {
      Vec3 nv;
      Vec3 *col;
      float d;
      int orient;
    } poly;
    struct {
      float width;
      SgLineStyle style;
      float fontSize;
      char *str;
    } text;
  } u;
} ElemType;

typedef struct {
  ElemType *elemP;
  ElemType elem;
  BOOL isFront;
} ListEntry;

typedef enum {
  LC_OLD,
  LC_NEW
} ListChoice;

typedef enum {
  SR_LEFT,
  SR_RIGHT,
  SR_SPLIT,
  SR_NONE
} SplitRes;

static RECTSET LeftRectSet = NULL, RightRectSet = NULL, CurrRectSet;
static LINLIST LeftDrawList = NULL, RightDrawList = NULL, DrawList;
static BOOL NeedDepthSort = FALSE;
float LeftVpX, LeftVpY, LeftVpW, LeftVpH;
float RightVpX, RightVpY, RightVpW, RightVpH;
static LINLIST NewElemList = NULL;
static LINLIST OldElemList = NULL;

static void
initElem(ElemType *elemP, ElemKind kind, int n)
{
  elemP->kind = kind;
  elemP->points = malloc(n * sizeof(Vec3));
  elemP->pointNo = n;
}

static void
calcBox(ElemType *elemP)
{
  Rect *rectP = &elemP->rect;
  int i;

  rectP->xMin = elemP->points[0][0];
  rectP->xMax = elemP->points[0][0];
  rectP->yMin = elemP->points[0][1];
  rectP->yMax = elemP->points[0][1];
  elemP->zMin = elemP->points[0][2];
  elemP->zMax = elemP->points[0][2];

  for (i = 1; i < elemP->pointNo; i++) {
    if (elemP->points[i][0] < rectP->xMin)
      rectP->xMin = elemP->points[i][0];
    if (elemP->points[i][0] > rectP->xMax)
      rectP->xMax = elemP->points[i][0];
    if (elemP->points[i][1] < rectP->yMin)
      rectP->yMin = elemP->points[i][1];
    if (elemP->points[i][1] > rectP->yMax)
      rectP->yMax = elemP->points[i][1];
    if (elemP->points[i][2] < elemP->zMin)
      elemP->zMin = elemP->points[i][2];
    if (elemP->points[i][2] > elemP->zMax)
      elemP->zMax = elemP->points[i][2];
  }

  if (rectP->xMin == rectP->xMax)
    rectP->xMax += EPS;

  if (rectP->yMin == rectP->yMax)
    rectP->yMax += EPS;
}

static void
freeElem(void *p, void *clientData)
{
  ElemType *elemP = p;

  free(elemP->points);

  if (elemP->kind == EK_POLYGON && elemP->u.poly.col != NULL)
    free(elemP->u.poly.col);

  if (elemP->kind == EK_ANNOT || elemP->kind == EK_TEXT)
    free(elemP->u.text.str);
}

static int
getSign(float a)
{
  if (a < - EPS)
    return -1;
  else if (a > EPS)
    return 1;
  else
    return 0;
}

static void
vecNorm(float v[2])
{
  float xa, ya;

  if (v[0] < 0.0f)
    xa = - v[0];
  else
    xa = v[0];

  if (v[1] < 0.0f)
    ya = - v[1];
  else
    ya = v[1];

  if (xa > ya) {
    v[0] /= xa;
    v[1] /= xa;
  } else {
    v[0] /= ya;
    v[1] /= ya;
  }
}

static void
lineVect(float v[2], Vec3 p1, Vec3 p2)
{
  v[0] = p2[0] - p1[0];
  v[1] = p2[1] - p1[1];
  vecNorm(v);
}

static int
lineSide(Vec3 p, Vec3 p1, float v[2])
/* Return 1 if p is left of line starting at p1 and with direction v,
   0 if it is on the line and -1 if it is rigt of the line. */
{
  return getSign((p[1] - p1[1]) * v[0] - (p[0] - p1[0]) * v[1]);
}

static int
lineSides(ElemType *elemP, Vec3 p1, float v[2])
/* Return 1 if elemP is entirely on the left side of the line
   starting at p1 and with direction v, -1 if it is entirely on the
   right side, 0 otherwise. */
{
  int resSide, side;
  Vec3 *pointP;
  int i;

  pointP = elemP->points;

  resSide = 0;
  for (i = 0; i < elemP->pointNo; i++) {
    side = lineSide(pointP[i], p1, v);
    if (resSide == 0)
      resSide = side;
    if (side * resSide == -1)
      return 0;
  }

  if (resSide == 0)
    /* Element is in the plane of the line. It doesn't matter whether
       we say that it's left or right, but don't return 0 because then
       in might be split. */
    return 1;

  return resSide;
}

static int
lineIntersect(Vec3 p11, Vec3 p12, float v1[2],
    Vec3 p21, Vec3 p22, float v2[2])
/* Return 1 if line from p11 to p12 intersects line from p21 to p22,
   0 in degenerate cases (end point of one line is on other line),
   -1 if they do not intersect. v1 and v2 are the direction vectors of
   the lines. */
{
  int sign11, sign12;
  int sign21, sign22;

  sign11 = lineSide(p11, p21, v2);
  sign12 = lineSide(p12, p21, v2);

  if (sign11 * sign12 == 1)
    return -1;

  sign21 = lineSide(p21, p11, v1);
  sign22 = lineSide(p22, p11, v1);

  if (sign21 * sign22 == 1)
    return -1;

  if (sign11 * sign12 == -1 && sign21 * sign22 == -1)
    return 1;

  return 0;
}

static int
pointInPolygon(Vec3 p, Vec3 *polyP, int n, int orient)
/* return 1 if p is inside polygon, 0 if it is on the border, -1 if
   it is outside */
{
  int resSide, side;
  float v[2];
  int i0, i1;

  resSide = 1;

  for (i0 = 0; i0 < n; i0++) {
    i1 = (i0 + 1) % n;

    lineVect(v, polyP[i0], polyP[i1]);
    side = lineSide(p, polyP[i0], v);
    if (side == - orient)
      return -1;
    if (side == 0)
      resSide = 0;
  }

  return resSide;
}

static int
getFront(ElemType *elem1P, ElemType *elem2P)
/* Return 1 if elem1P is entirely on the same side from the plane of
   elem2P as the viewpoint, -1 if it is entirely on the opposite side,
   0 otherwise. */
{
  int resSide, side;
  Vec3 *pointP;
  Vec3 *nvP;
  float d;
  int i;

  if (elem1P->zMax < elem2P->zMin)
    return 1;

  if (elem2P->zMax < elem1P->zMin)
    return -1;

  pointP = elem1P->points;
  nvP = &elem2P->u.poly.nv;
  d = elem2P->u.poly.d;

  resSide = 0;
  for (i = 0; i < elem1P->pointNo; i++) {
    side = getSign(Vec3Scalar(pointP[i], *nvP) - d);
    if (resSide == 0)
      resSide = side;
    if (side * resSide == -1)
      return 0;
  }

  if (resSide == 0)
    /* Elements are in the same plane. It doesn't matter which one
       is considered to be in front, but don't return 0 because then
       one of the elements would be split. */
    return 1;

  if ((*nvP)[2] > 0.0f)
    return (- resSide);
  else
    return resSide;
}

static int
getSignS(float a)
/* use a slightly smaller EPS for splitting to avoid failed
   splits due to numerical problems */
{
  if (a < - EPS_S)
    return -1;
  else if (a > EPS_S)
    return 1;
  else
    return 0;
}

static void
planeIntersect(int *indP, float *parP, int *dirP,
   Vec3 *pointP, int n, Vec3 nv, float d)
{
  float dist, prevDist;
  int side, prevSide;
  int i;

  dist = Vec3Scalar(pointP[*indP], nv) - d;
  side = getSignS(dist);
  for (i = *indP + 1; i < n; i++) {
    prevDist = dist;
    prevSide = side;
    dist = Vec3Scalar(pointP[i], nv) - d;
    side = getSignS(dist);
    if (side != prevSide && side != 0)
      break;
  }

  if (i == n) {
    prevDist = dist;
    dist = Vec3Scalar(pointP[0], nv) - d;
    side = getSignS(dist);
  }

  *indP = i - 1;
  *parP = prevDist / (prevDist - dist);

  if (*parP < 0.0f)
    *parP = 0.0f;
  else if (*parP > 1.0f)
    *parP = 1.0f;
  
  *dirP = side;
}

static void
planeSplit(ElemType *splitP, Vec3 nv, float d,
    ElemType *elem1P, ElemType *elem2P)
/* Split splitP at the plane given by nv and d, return the two pieces
   in elem1P and elem2P. */
{
  Vec3 *pointP, *colP;
  int n, nNew;
  int ind1, ind2;
  float par1, par2;
  int dir;
  Vec3 v, intersP1, intersP2;
  Vec3 intersCol1, intersCol2;
  BOOL parZero;
  ElemType *elemP;
  int i;

  pointP = splitP->points;
  n = splitP->pointNo;

  ind1 = 0;
  planeIntersect(&ind1, &par1, &dir, pointP, n, nv, d);

  Vec3Copy(v, pointP[ind1 + 1]);
  Vec3Sub(v, pointP[ind1]);
  Vec3Copy(intersP1, pointP[ind1]);
  Vec3ScaleAdd(intersP1, par1, v);

  *elem1P = *splitP;  /* copy attributes */
  *elem2P = *splitP;

  if (splitP->kind == EK_POLYGON) {
    colP = splitP->u.poly.col;
    if (colP != NULL) {
      Vec3Copy(v, colP[ind1 + 1]);
      Vec3Sub(v, colP[ind1]);
      Vec3Copy(intersCol1, colP[ind1]);
      Vec3ScaleAdd(intersCol1, par1, v);
    }

    /* Since the polygons are assumed to be convex, there must be exactly
       two edges intersecting the plane. */

    ind2 = ind1 + 1;
    planeIntersect(&ind2, &par2, &dir, pointP, n, nv, d);

    Vec3Copy(v, pointP[(ind2 + 1) % n]);
    Vec3Sub(v, pointP[ind2]);
    Vec3Copy(intersP2, pointP[ind2]);
    Vec3ScaleAdd(intersP2, par2, v);
    if (colP != NULL) {
      Vec3Copy(v, colP[(ind2 + 1) % n]);
      Vec3Sub(v, colP[ind2]);
      Vec3Copy(intersCol2, colP[ind2]);
      Vec3ScaleAdd(intersCol2, par2, v);
    }

    parZero = (getSign(par2) == 0);

    if (parZero)
      nNew = ind2 - ind1 + 1;
    else
      nNew = ind2 - ind1 + 2;

    if (dir == 1)
      elemP = elem1P;
    else
      elemP = elem2P;

    initElem(elemP, EK_POLYGON, nNew);
    if (colP != NULL)
      elemP->u.poly.col = malloc(nNew * sizeof(Vec3));

    Vec3Copy(elemP->points[0], intersP1);
    if (colP != NULL)
      Vec3Copy(elemP->u.poly.col[0], intersCol1);

    for (i = ind1 + 1; i <= ind2; i++) {
      Vec3Copy(elemP->points[i - ind1], pointP[i]);
      if (colP != NULL)
	Vec3Copy(elemP->u.poly.col[i - ind1], colP[i]);
    }
    
    if (! parZero) {
      Vec3Copy(elemP->points[nNew - 1], intersP2);
      if (colP != NULL)
	Vec3Copy(elemP->u.poly.col[nNew - 1], intersCol2);
    }

    parZero = (getSign(par1) == 0);

    if (parZero)
      nNew = ind1 + n - ind2 + 1;
    else
      nNew = ind1 + n - ind2 + 2;

    if (dir == 1)
      elemP = elem2P;
    else
      elemP = elem1P;

    initElem(elemP, EK_POLYGON, nNew);
    if (colP != NULL)
      elemP->u.poly.col = malloc(nNew * sizeof(Vec3));

    Vec3Copy(elemP->points[0], intersP2);
    if (colP != NULL)
      Vec3Copy(elemP->u.poly.col[0], intersCol2);

    for (i = ind2 + 1; i <= ind1 + n; i++) {
      Vec3Copy(elemP->points[i - ind2], pointP[i % n]);
      if (colP != NULL)
	Vec3Copy(elemP->u.poly.col[i - ind2], colP[i % n]);
    }
    
    if (! parZero) {
      Vec3Copy(elemP->points[nNew - 1], intersP1);
      if (colP != NULL)
	Vec3Copy(elemP->u.poly.col[nNew - 1], intersCol1);
    }
  } else {
    if (dir == 1)
      elemP = elem1P;
    else
      elemP = elem2P;

    initElem(elemP, EK_LINE, 2);
    Vec3Copy(elemP->points[0], pointP[0]);
    Vec3Copy(elemP->points[1], intersP1);

    if (dir == 1)
      elemP = elem2P;
    else
      elemP = elem1P;

    initElem(elemP, EK_LINE, 2);
    Vec3Copy(elemP->points[0], intersP1);
    Vec3Copy(elemP->points[1], pointP[1]);
  }
}

static void
lineSplit(ElemType *splitP, Vec3 p1, float v[2],
    ElemType *elem1P, ElemType *elem2P)
/* Split splitP at the line starting at p1 and with direction v,
   return the two pieces in elem1P and elem2P. */
{
  Vec3 nv;
  float d;

  nv[0] = v[1];
  nv[1] = - v[0];
  nv[2] = 0.0f;
  d = Vec3Scalar(nv, p1);
  planeSplit(splitP, nv, d, elem1P, elem2P);
}

static int
elemOrient(ElemType *elemP)
/* Return 1 if polygon is counter clockwise, -1 if it is clockwise,
   0 if it is degenerate. */
{
  Vec3 *pointP;
  int pointNo;
  int i, i1, i2;
  float v[2];
  int side;

  if (elemP->kind != EK_POLYGON)  /* should not be called */
    return 0;

  if (elemP->u.poly.orient != UNKNOWN_ORIENT)
    return elemP->u.poly.orient;

  pointP = elemP->points;
  pointNo = elemP->pointNo;

  for (i = 0; i < pointNo; i++) {
    i1 = (i + 1) % pointNo;
    i2 = (i + 2) % pointNo;

    lineVect(v, pointP[i], pointP[i1]);
    side = lineSide(pointP[i2], pointP[i], v);
    if (side != 0)
      break;
  }

  elemP->u.poly.orient = side;

  return side;
}

static SplitRes
splitEdge(ElemType *backP, int backOrient,
    Vec3 p1, Vec3 p2, float frontV[2])
{
  Vec3 *backPts;
  int backN;
  int backMaxI;
  float backV[2];
  int side, side1, side2;
  int inters;
  int backI, backI1, prevI;

  backPts = backP->points;
  backN = backP->pointNo;

  if (backP->kind == EK_POLYGON)
    backMaxI = backN;
  else
    backMaxI = backN - 1;

  side = lineSides(backP, p1, frontV);

  if (side == 1)
    return SR_LEFT;
  
  if (side == -1)
    return SR_RIGHT;

  if (backP->kind == EK_POLYGON &&
      pointInPolygon(p1, backPts, backN, backOrient) == 1)
    return SR_SPLIT;

  for (backI = 0; backI < backMaxI; backI++) {
    backI1 = (backI + 1) % backN;

    lineVect(backV, backPts[backI], backPts[backI1]);

    inters = lineIntersect(
	backPts[backI], backPts[backI1], backV,
	p1, p2, frontV);

    if (inters == -1)
      continue;

    if (inters == 1)
      return SR_SPLIT;

    if (lineSide(backPts[backI], p1, frontV) == 0) {
      if (backI == 0 && backP->kind != EK_POLYGON)
	continue;
      prevI = (backI + backN - 1) % backN;
    } else {
      prevI = backI;
    }

    if (lineSide(backPts[prevI], p1, frontV) *
	lineSide(backPts[backI1], p1, frontV) != -1)
      continue;
    
    side1 = lineSide(p1, backPts[backI], backV);
    side2 = lineSide(p2, backPts[backI], backV);
    
    if (side1 * side2 == -1)
      return SR_SPLIT;

    if (backP->kind == EK_POLYGON &&
	(side1 == backOrient || side2 == backOrient))
      return SR_SPLIT;
  }

  return SR_NONE;
}

static BOOL
calcPieces(ElemType *backP, ListChoice lc, ElemType *frontP)
/* Append the pieces of backP that are not obscured by frontP at
   the end of list. Return TRUE if backP was split, FALSE otherwise. */
{
  Vec3 *frontPts;
  int frontN;
  int frontOrient, backOrient;
  BOOL split, inside, degen;
  ElemType *restP;
  SplitRes splitRes;
  float frontV[2];
  int frontI, frontI1, restI;
  ElemType inElem, restElem;
  ListEntry outEntry;

  frontOrient = elemOrient(frontP);
  if (frontOrient == 0)
    return FALSE;

  frontPts = frontP->points;
  frontN = frontP->pointNo;

  if (backP->kind == EK_POLYGON) {
    backOrient = elemOrient(backP);
    if (backOrient == 0)
      /* discard degenerate polygons */
      return TRUE;
  } else {
    backOrient = 0;  /* not used */
  }

  split = FALSE;
  inside = TRUE;
  degen = FALSE;

  restP = backP;

  for (frontI = 0; frontI < frontN; frontI++) {
    frontI1 = (frontI + 1) % frontN;

    lineVect(frontV, frontPts[frontI], frontPts[frontI1]);

    splitRes = splitEdge(restP, backOrient,
	frontPts[frontI], frontPts[frontI1], frontV);

    if (splitRes == SR_LEFT) {
      if (frontOrient == -1) {
	if (! split)
	  return FALSE;
	degen = TRUE;
	break;
      } else {
	continue;
      }
    } else if (splitRes == SR_RIGHT) {
      if (frontOrient == 1) {
	if (! split)
	  return FALSE;
	degen = TRUE;
	break;
      } else {
	continue;
      }
    } else if (splitRes == SR_NONE) {
      inside = FALSE;
      continue;
    }

    if (split) {
      restElem = *restP;
      restP = &restElem;
    }

    if (frontOrient == 1)
      lineSplit(restP, frontPts[frontI], frontV, &inElem, &outEntry.elem);
    else
      lineSplit(restP, frontPts[frontI], frontV, &outEntry.elem, &inElem);

    if (split)
      freeElem(restP, NULL);

    restP = &inElem;

    if (lc == LC_OLD) {
      outEntry.elemP = NULL;
      outEntry.isFront = FALSE;
      (void) ListInsertLast(OldElemList, &outEntry);
    } else {
      (void) ListInsertFirst(NewElemList, &outEntry.elem);
    }

    split = TRUE;
  }

  if (! split)
    /* backP could not be split, must be either completely inside or
       completely outside of frontP. */
    return inside;

  if (! (inside || degen)) {  /* test for possible degeneracy */
    for (restI = 0; restI < restP->pointNo; restI++)
      if (pointInPolygon(restP->points[restI],
	  frontPts, frontN, frontOrient) == -1) {
	degen = TRUE;
	break;
      }
  }

  if (! degen) {  /* no degeneracy, throw away obscured part */
    freeElem(restP, NULL);
    return TRUE;
  }

  /* degenerated, keep rest to avoid possible holes */
  if (lc == LC_OLD) {
    outEntry.elemP = NULL;
    outEntry.elem = *restP;
    outEntry.isFront = FALSE;
    (void) ListInsertLast(OldElemList, &outEntry);
  } else {
    (void) ListInsertFirst(NewElemList, restP);
  }

  return TRUE;
}

static void
handlePoint(void *p1, void *p2)
{
  ElemType *oldP = p1;
  ElemType *newP = p2;
  int side;

  if (oldP->kind != EK_POLYGON)
    return;

  if (ListSize(NewElemList) == 0)
    return;

  side = getFront(newP, oldP);
  if (side == 1)
    return;

  if (pointInPolygon(newP->points[0],
      oldP->points, oldP->pointNo, elemOrient(oldP)) != -1) {
    freeElem(newP, NULL);
    ListRemove(NewElemList, newP);
  }
}

static void
handleLine(void *p1, void *p2)
{
  ElemType *oldP = p1;
  ElemType *newP, *nextP, backElem, frontElem;
  int side;
  BOOL split;

  if (oldP->kind != EK_POLYGON)
    return;

  newP = ListFirst(NewElemList);
  while (newP != NULL) {
    nextP = ListNext(NewElemList, newP);

    side = getFront(newP, oldP);

    if (side == -1) {
      split = calcPieces(newP, LC_NEW, oldP);
      if (split) {
	freeElem(newP, NULL);
	ListRemove(NewElemList, newP);
      }
    } else if (side == 0) {
      if (oldP->u.poly.nv[2] > 0.0f)
	planeSplit(newP, oldP->u.poly.nv, oldP->u.poly.d,
	    &frontElem, &backElem);
      else
	planeSplit(newP, oldP->u.poly.nv, oldP->u.poly.d,
	    &backElem, &frontElem);

      split = calcPieces(&backElem, LC_NEW, oldP);

      if (split) {
	freeElem(newP, NULL);
	ListRemove(NewElemList, newP);

	(void) ListInsertFirst(NewElemList, &frontElem);
      } else {
	freeElem(&frontElem, NULL);
      }

      freeElem(&backElem, NULL);
    }

    newP = nextP;
  }
}

static void
handlePoly(void *p1, void *p2)
{
  ElemType *oldP = p1;
  ElemType *newP = p2;
  int side;
  BOOL split;
  ListEntry frontEntry;
  ElemType backElem;

  if (oldP->kind != EK_POLYGON && oldP->kind != EK_LINE) {
    if (pointInPolygon(oldP->points[0],
	newP->points, newP->pointNo, elemOrient(newP)) != -1) {
      freeElem(oldP, NULL);
      RectSetRemove(CurrRectSet, oldP);
    }

    return;
  }

  side = getFront(oldP, newP);

  if (side == 0 && oldP->kind == EK_POLYGON)
    side = - getFront(newP, oldP);

  if (side == 1) {
    if (oldP->kind == EK_LINE)
      return;

    frontEntry.elemP = oldP;
    frontEntry.isFront = TRUE;
    (void) ListInsertLast(OldElemList, &frontEntry);
  } else if (side == -1) {
    split = calcPieces(oldP, LC_OLD, newP);
    if (split) {
      freeElem(oldP, NULL);
      RectSetRemove(CurrRectSet, oldP);
    }
  } else {
    if (newP->u.poly.nv[2] > 0.0f)
      planeSplit(oldP, newP->u.poly.nv, newP->u.poly.d,
	  &frontEntry.elem, &backElem);
    else
      planeSplit(oldP, newP->u.poly.nv, newP->u.poly.d,
	  &backElem, &frontEntry.elem);

    split = calcPieces(&backElem, LC_OLD, newP);
    frontEntry.isFront = (oldP->kind == EK_POLYGON);

    if (split) {
      freeElem(oldP, NULL);
      RectSetRemove(CurrRectSet, oldP);

      frontEntry.elemP = NULL;
      (void) ListInsertLast(OldElemList, &frontEntry);
    } else {
      freeElem(&frontEntry.elem, NULL);

      frontEntry.elemP = oldP;
      (void) ListInsertLast(OldElemList, &frontEntry);
    }

    freeElem(&backElem, NULL);
  }
}

static void
addElem(ElemType *elemP)
{
  ElemType *oldP, *newP, *nextNewP;
  ListEntry *entryP, *nextEntryP;
  BOOL split;

  calcBox(elemP);

  if (SgGetEye() == SG_EYE_LEFT) {
    if (SgGetFeature(SG_FEATURE_HIDDEN)) {
      if (LeftRectSet == NULL) {
	LeftRectSet = RectSetOpen(sizeof(ElemType));
	SgGetViewport(&LeftVpX, &LeftVpY, &LeftVpW, &LeftVpH);
      }
      CurrRectSet = LeftRectSet;
    } else {
      if (LeftDrawList == NULL) {
	LeftDrawList = ListOpen(sizeof(ElemType));
	SgGetViewport(&LeftVpX, &LeftVpY, &LeftVpW, &LeftVpH);
      }
      (void) ListInsertLast(LeftDrawList, elemP);
      return;
    }
  } else {
    if (SgGetFeature(SG_FEATURE_HIDDEN)) {
      if (RightRectSet == NULL) {
	RightRectSet = RectSetOpen(sizeof(ElemType));
	SgGetViewport(&RightVpX, &RightVpY, &RightVpW, &RightVpH);
      }
      CurrRectSet = RightRectSet;
    } else {
      if (RightDrawList == NULL) {
	RightDrawList = ListOpen(sizeof(ElemType));
	SgGetViewport(&RightVpX, &RightVpY, &RightVpW, &RightVpH);
      }
      (void) ListInsertLast(RightDrawList, elemP);
      return;
    }
  }

  if (elemP->kind == EK_ANNOT || elemP->kind == EK_TEXT) {
    /* Because we only know the lower left corner of texts, we cannot
       decide whether they're visible or not. So do not eliminate
       any texts here. Additonal depth sorting much be switched on
       to get correct output of texts. */
    RectSetInsert(CurrRectSet, elemP);
    NeedDepthSort = TRUE;
    return;
  }

  if (NewElemList == NULL) {
    NewElemList = ListOpen(sizeof(ElemType));
    OldElemList = ListOpen(sizeof(ListEntry));
  }

  if (elemP->kind != EK_POLYGON)
    elemP = ListInsertFirst(NewElemList, elemP);

  if (elemP->kind == EK_POLYGON)
    RectSetFindOverlap(CurrRectSet, &elemP->rect, handlePoly, elemP);
  else if (elemP->kind == EK_LINE)
    RectSetFindOverlap(CurrRectSet, &elemP->rect, handleLine, NULL);
  else
    RectSetFindOverlap(CurrRectSet, &elemP->rect, handlePoint, elemP);

  if (elemP->kind == EK_POLYGON) {
    split = FALSE;

    entryP = ListFirst(OldElemList);
    while (entryP != NULL) {
      nextEntryP = ListNext(OldElemList, entryP);

      if (entryP->elemP == NULL) {
	oldP = &entryP->elem;
	calcBox(oldP);
	RectSetInsert(CurrRectSet, oldP);
      } else {
	oldP = entryP->elemP;
      }

      if (entryP->isFront) {
	if (split) {
	  newP = ListFirst(NewElemList);
	  while (newP != NULL) {
	    nextNewP = ListNext(NewElemList, newP);
	    if (calcPieces(newP, LC_NEW, oldP)) {
	      freeElem(newP, NULL);
	      ListRemove(NewElemList, newP);
	    }
	    newP = nextNewP;
	  }
	} else {
	  split = calcPieces(elemP, LC_NEW, oldP);
	}
      }

      ListRemove(OldElemList, entryP);
      entryP = nextEntryP;
    }

    if (split)
      freeElem(elemP, NULL);
    else
      RectSetInsert(CurrRectSet, elemP);
  }

  newP = ListFirst(NewElemList);
  while (newP != NULL) {
    nextNewP = ListNext(NewElemList, newP);
    calcBox(newP);
    RectSetInsert(CurrRectSet, newP);
    ListRemove(NewElemList, newP);
    newP = nextNewP;
  }
}

void
DefHiddenMarker(float x[3], float r, float g, float b)
{
  ElemType elem;

  initElem(&elem, EK_MARKER, 1);

  elem.colR = r;
  elem.colG = g;
  elem.colB = b;

  Vec3Copy(elem.points[0], x);

  addElem(&elem);
}

void
DefHiddenLine(float x0[3], float x1[3], float r, float g, float b)
{
  ElemType elem;

  initElem(&elem, EK_LINE, 2);

  elem.colR = r;
  elem.colG = g;
  elem.colB = b;

  Vec3Copy(elem.points[0], x0);
  Vec3Copy(elem.points[1], x1);

  elem.u.line.width = SgGetLineWidth();
  elem.u.line.style = SgGetLineStyle();

  addElem(&elem);
}

void
DefHiddenPolyline(float x[][3], int n, float r, float g, float b)
{
  ElemType elem;
  int i;

  elem.u.line.width = SgGetLineWidth();
  elem.u.line.style = SgGetLineStyle();

  for (i = 0; i < n - 1; i++) {
    initElem(&elem, EK_LINE, 2);
    elem.colR = r;
    elem.colG = g;
    elem.colB = b;
    Vec3Copy(elem.points[0], x[i]);
    Vec3Copy(elem.points[1], x[i + 1]);
    addElem(&elem);
  }
}

static void
initPoly(ElemType *elemP, float x[][3], int n)
{
  Vec3 v1, v2;
  int i;

  initElem(elemP, EK_POLYGON, n);

  for (i = 0; i < n; i++)
    Vec3Copy(elemP->points[i], x[i]);

  Vec3Copy(v1, x[1]);
  Vec3Sub(v1, x[0]);
  Vec3Copy(v2, x[2]);
  Vec3Sub(v2, x[0]);
  Vec3Cross(v1, v2);
  Vec3Norm(v1);
  Vec3Copy(elemP->u.poly.nv, v1);

  elemP->u.poly.d = Vec3Scalar(x[0], v1);
  elemP->u.poly.orient = UNKNOWN_ORIENT;
}

void
DefHiddenPolygon(float x[][3], int n, float r, float g, float b)
{
  ElemType elem;

  initPoly(&elem, x, n);

  elem.colR = r;
  elem.colG = g;
  elem.colB = b;

  elem.u.poly.col = NULL;

  addElem(&elem);
}

void
DefHiddenShadedPolygon(float x[][3], float col[][3], int n)
{
  ElemType elem;
  int i;

  initPoly(&elem, x, n);

  elem.u.poly.col = malloc(n * sizeof(Vec3));;
  for (i = 0; i < n; i++)
    Vec3Copy(elem.u.poly.col[i], col[i]);

  addElem(&elem);
}

static void
addText(ElemKind kind, float x[3], char *str, float r, float g, float b)
{
  ElemType elem;

  initElem(&elem, kind, 1);

  elem.colR = r;
  elem.colG = g;
  elem.colB = b;

  Vec3Copy(elem.points[0], x);

  elem.u.text.str = malloc(strlen(str) + 1);
  (void) strcpy(elem.u.text.str, str);

  elem.u.text.width = SgGetLineWidth();
  elem.u.text.style = SgGetLineStyle();
  elem.u.text.fontSize = SgGetFontSize();

  addElem(&elem);
}

void
DefHiddenAnnot(float x[3], char *str, float r, float g, float b)
{
  addText(EK_ANNOT, x, str, r, g, b);
}

void
DefHiddenText(float x[3], char *str, float r, float g, float b)
{
  addText(EK_TEXT, x, str, r, g, b);
}

static void
drawElem(void *p, void *clientData)
{
  ElemType *elemP = p;
  Vec3 *pointP;
  float x2[SG_POLY_SIZE][2];
  int i;

  if (elemP->kind != EK_POLYGON || elemP->u.poly.col == NULL)
    SgSetColor(elemP->colR, elemP->colG, elemP->colB);

  pointP = elemP->points;

  switch (elemP->kind) {
    case EK_MARKER:
      x2[0][0] = pointP[0][0];
      x2[0][1] = pointP[0][1];
      SgDrawMarkers2D(x2, 1);
      break;
    case EK_LINE:
      SgSetLineWidth(elemP->u.line.width);
      SgSetLineStyle(elemP->u.line.style);
      SgDrawLine2D(pointP[0], pointP[1]);
      break;
    case EK_POLYGON:
      for (i = 0; i < elemP->pointNo; i++) {
	x2[i][0] = pointP[i][0];
	x2[i][1] = pointP[i][1];
      }
      if (elemP->u.poly.col == NULL)
	SgDrawPolygon2D(x2, elemP->pointNo);
      else
	SgDrawShadedPolygon2D(x2, elemP->u.poly.col, elemP->pointNo);
      break;
    case EK_ANNOT:
      SgSetLineWidth(elemP->u.text.width);
      SgSetLineStyle(elemP->u.text.style);
      SgSetFontSize(elemP->u.text.fontSize);
      SgDrawAnnot2D(pointP[0], elemP->u.text.str);
      break;
    case EK_TEXT:
      SgSetLineWidth(elemP->u.text.width);
      SgSetLineStyle(elemP->u.text.style);
      SgSetFontSize(elemP->u.text.fontSize);
      SgDrawText2D(pointP[0], elemP->u.text.str);
      break;
  }
}

static int
compDepth(void *p1, void *p2)
{
  ElemType *elem1P = p1;
  ElemType *elem2P = p2;

  if (elem1P->zMin > elem2P->zMin)
    return -1;
  else
    return 1;
}

static void
listNonPoly(void *p, void *clientData)
{
  ElemType *elemP = p;

  if (elemP->kind == EK_POLYGON)
    drawElem(elemP, NULL);
  else
    (void) ListInsertFirst(DrawList, elemP);
}

static void
listElem(void *p, void *clientData)
{
  (void) ListInsertFirst(DrawList, p);
}

static void
drawAndFreeSet(RECTSET *setP)
{
  DrawList = ListOpen(sizeof(ElemType));

  if (NeedDepthSort)
    RectSetApply(*setP, listElem, NULL);
  else
    RectSetApply(*setP, listNonPoly, NULL);

  ListSort(DrawList, compDepth);
  ListApply(DrawList, drawElem, NULL);

  ListClose(DrawList);

  RectSetAddDestroyCB(*setP, freeElem, NULL, NULL);
  RectSetClose(*setP);
  *setP = NULL;
}

static void
drawAndFreeList(LINLIST *listP)
{
  ListSort(*listP, compDepth);
  ListApply(*listP, drawElem, NULL);

  ListAddDestroyCB(*listP, freeElem, NULL, NULL);
  ListClose(*listP);
  *listP = NULL;
}

void
SgDefCalcHidden(void)
{
  float x, y, w, h;
  BOOL vpChanged;

  SgGetViewport(&x, &y, &w, &h);
  vpChanged = FALSE;

  if (LeftDrawList != NULL || LeftRectSet != NULL) {
    SgSetEye(SG_EYE_LEFT);
    if (LeftVpX != x || LeftVpY != y || LeftVpW != w || LeftVpH != h) {
      SgSetViewport(LeftVpX, LeftVpY, LeftVpW, LeftVpH);
      vpChanged = TRUE;
    }

    if (LeftRectSet != NULL)
      drawAndFreeSet(&LeftRectSet);

    if (LeftDrawList != NULL)
      drawAndFreeList(&LeftDrawList);
  }

  if (RightDrawList != NULL || RightRectSet != NULL) {
    SgSetEye(SG_EYE_RIGHT);
    if (RightVpX != x || RightVpY != y || RightVpW != w || RightVpH != h) {
      SgSetViewport(RightVpX, RightVpY, RightVpW, RightVpH);
      vpChanged = TRUE;
    }

    if (RightRectSet != NULL)
      drawAndFreeSet(&RightRectSet);

    if (RightDrawList != NULL)
      drawAndFreeList(&RightDrawList);
  }

  if (vpChanged)
    SgSetViewport(x, y, w, h);

  NeedDepthSort = FALSE;
}
