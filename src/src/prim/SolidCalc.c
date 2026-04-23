/*
************************************************************************
*
*   SolidCalc.c - calculate solids
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/prim/SCCS/s.SolidCalc.c
*   SCCS identification       : 1.7
*
************************************************************************
*/

#include "solid_calc.h"

#include <stdio.h>
#include <stdlib.h>
#include <values.h>
#include <math.h>

#include <break.h>
#include <mat_vec.h>
#include <simul_anneal.h>
#include <par_hand.h>
#include <par_names.h>
#include "prim_struc.h"

#define MAX_PAR_NO 8
#define MAX_POINT_NO 128
/* not MAXFLOAT to avoid overflow when calculating */
#define BIG_SIZE ((float) 1.0e7)

typedef struct {
  int ind;
  float val;
} ValDescr;

typedef struct {
  PrimSolidKind kind;
  Vec3 *p;
  int n;
  int nOut;
  ValDescr *val1A, *val2A;
  BOOL *out1, *out2;
} PointList;

static void
fillMat(Mat3 m, float xAng, float yAng, float zAng, BOOL inv)
{
  float cx, sx, cy, sy, cz, sz;

  cx = cosf(xAng);
  sx = sinf(xAng);
  cy = cosf(yAng);
  sy = sinf(yAng);

  if (zAng == 0.0f) {
    m[0][0] = cy;
    m[0][1] = - sx * sy;
    m[0][2] = - cx * sy;
    m[1][0] = 0.0f;
    m[1][1] = cx;
    m[1][2] = - sx;
    m[2][0] = sy;
    m[2][1] = sx * cy;
    m[2][2] = cx * cy;
  } else {
    cz = cosf(zAng);
    sz = sinf(zAng);

    m[0][0] = cy * cz;
    m[0][1] = - sx * sy * cz - cx * sz;
    m[0][2] = - cx * sy * cz + sx * sz;
    m[1][0] = cy * sz;
    m[1][1] = - sx * sy * sz + cx * cz;
    m[1][2] = - cx * sy * sz - sx * cz;
    m[2][0] = sy;
    m[2][1] = sx * cy;
    m[2][2] = cx * cy;
  }

  if (inv)
    Mat3Transp(m);
}

static void
sortVal(ValDescr *valA, int leftI, int rightI, int usedI)
{
  int i, j;
  float x;
  ValDescr w;

  if (leftI > usedI)
    return;  /* only sort as much as is used */
  
  /* quicksort (Wirth) */
  i = leftI;
  j = rightI;
  x = valA[(leftI + rightI) / 2].val;
  for (;;) {
    while (valA[i].val > x)
      i++;
    while (x > valA[j].val)
      j--;

    if (i <= j) {
      w = valA[i];
      valA[i] = valA[j];
      valA[j] = w;
      i++;
      j--;
    }

    if (i > j)
      break;
  }

  if (leftI < j)
    sortVal(valA, leftI, j, usedI);
  if (i < rightI)
    sortVal(valA, i, rightI, usedI);
}

static void
testTetr(Vec3 *p, int n, int i1, int i2, int i3, int i4, BOOL *keepA)
{
  Mat3 m;
  Vec3 x;
  int i;

  if (i1 == i2 || i1 == i3 || i1 == i4 || i2 == i3 || i2 == i4 || i3 == i4)
    return;
  
  Vec3Copy(m[0], p[i2]);
  Vec3Sub(m[0], p[i1]);

  Vec3Copy(m[1], p[i3]);
  Vec3Sub(m[1], p[i1]);

  Vec3Copy(m[2], p[i4]);
  Vec3Sub(m[2], p[i1]);

  Mat3Inv(m);

  for (i = 0; i < n; i++) {
    if (! keepA[i])
      continue;

    if (i == i1 || i == i2 || i == i3 || i == i4)
      continue;
    
    Vec3Copy(x, p[i]);
    Vec3Sub(x, p[i1]);
    Mat3VecMult(x, m);

    if (x[0] >= 0.0f && x[1] >= 0.0f && x[2] >= 0.0f &&
	x[0] + x[1] + x[2] <= 1.0f)
      keepA[i] = FALSE;
  }
}

static void
reduceList(PointList *listP)
{
  ValDescr *valA;
  BOOL *keepA;
  int pi[14];
  int i0, i1, i2, i, k;

  if (2 * listP->nOut >= listP->n)
    return;

  valA = malloc(listP->n * sizeof(*valA));
  keepA = malloc(listP->n * sizeof(*keepA));

  for (i = 0; i < listP->n; i++)
    keepA[i] = TRUE;

  for (k = 0; k < 3; k++) {
    for (i = 0; i < listP->n; i++) {
      valA[i].val = listP->p[i][k];
      valA[i].ind = i;
    }

    sortVal(valA, 0, listP->n - 1, listP->n);

    pi[2 * k] = valA[listP->nOut].ind;
    pi[2 * k + 1] = valA[listP->n - 1 - listP->nOut].ind;
  }

  for (i0 = 0; i0 < 2; i0++)
    for (i1 = 0; i1 < 2; i1++)
      for (i2 = 0; i2 < 2; i2++) {
	for (i = 0; i < listP->n; i++) {
	  valA[i].val = 0.0f;

	  if (i0 == 0)
	    valA[i].val += listP->p[i][0];
	  else
	    valA[i].val -= listP->p[i][0];

	  if (i1 == 0)
	    valA[i].val += listP->p[i][1];
	  else
	    valA[i].val -= listP->p[i][1];

	  if (i2 == 0)
	    valA[i].val += listP->p[i][2];
	  else
	    valA[i].val -= listP->p[i][2];

	  valA[i].ind = i;
	}

	sortVal(valA, 0, listP->n - 1, listP->nOut);
	pi[6 + 4 * i2 + 2 * i1 + i0] = valA[listP->nOut].ind;
      }

  /* cube */
  testTetr(listP->p, listP->n, pi[6], pi[9], pi[11], pi[12], keepA);
  testTetr(listP->p, listP->n, pi[6], pi[10], pi[11], pi[12], keepA);
  testTetr(listP->p, listP->n, pi[6], pi[8], pi[9], pi[12], keepA);
  testTetr(listP->p, listP->n, pi[6], pi[7], pi[9], pi[11], keepA);
  testTetr(listP->p, listP->n, pi[9], pi[11], pi[12], pi[13], keepA);

  /* 4-sided pyramids on cube faces */
  testTetr(listP->p, listP->n, pi[0], pi[6], pi[8], pi[12], keepA);
  testTetr(listP->p, listP->n, pi[0], pi[6], pi[10], pi[12], keepA);
  testTetr(listP->p, listP->n, pi[1], pi[7], pi[9], pi[11], keepA);
  testTetr(listP->p, listP->n, pi[1], pi[9], pi[11], pi[13], keepA);
  testTetr(listP->p, listP->n, pi[2], pi[6], pi[7], pi[11], keepA);
  testTetr(listP->p, listP->n, pi[2], pi[6], pi[10], pi[11], keepA);
  testTetr(listP->p, listP->n, pi[3], pi[8], pi[9], pi[12], keepA);
  testTetr(listP->p, listP->n, pi[3], pi[9], pi[12], pi[13], keepA);
  testTetr(listP->p, listP->n, pi[4], pi[6], pi[8], pi[9], keepA);
  testTetr(listP->p, listP->n, pi[4], pi[6], pi[7], pi[9], keepA);
  testTetr(listP->p, listP->n, pi[5], pi[10], pi[11], pi[12], keepA);
  testTetr(listP->p, listP->n, pi[5], pi[11], pi[12], pi[13], keepA);

  /* octahedron */
  testTetr(listP->p, listP->n, pi[0], pi[1], pi[2], pi[4], keepA);
  testTetr(listP->p, listP->n, pi[0], pi[1], pi[3], pi[4], keepA);
  testTetr(listP->p, listP->n, pi[0], pi[1], pi[2], pi[5], keepA);
  testTetr(listP->p, listP->n, pi[0], pi[1], pi[3], pi[5], keepA);

  /* tetrahedrons on octahedron faces */
  testTetr(listP->p, listP->n, pi[0], pi[2], pi[4], pi[6], keepA);
  testTetr(listP->p, listP->n, pi[1], pi[2], pi[4], pi[7], keepA);
  testTetr(listP->p, listP->n, pi[0], pi[3], pi[4], pi[8], keepA);
  testTetr(listP->p, listP->n, pi[1], pi[3], pi[4], pi[9], keepA);
  testTetr(listP->p, listP->n, pi[0], pi[2], pi[5], pi[10], keepA);
  testTetr(listP->p, listP->n, pi[1], pi[2], pi[5], pi[11], keepA);
  testTetr(listP->p, listP->n, pi[0], pi[3], pi[5], pi[12], keepA);
  testTetr(listP->p, listP->n, pi[1], pi[3], pi[5], pi[13], keepA);

  k = 0;
  for (i = 0; i < listP->n; i++)
    if (keepA[i]) {
      Vec3Copy(listP->p[k], listP->p[i]);
      k++;
    }
  
  listP->n = k;

  free(valA);
  free(keepA);
}

static BOOL
getSizes(PointList *listP, float *sol,
    float *size1P, float *size2P, float *size3P)
{
  Mat3 rotMat;
  int nOut0;
  BOOL neg;
  Vec3 x;
  float f0, f1, f2;
  float size1, size2, size3;
  float vol, volMin;
  int i1, i2, i1old, i2old, i;

  if (listP->kind != PSK_SPHERE)
    fillMat(rotMat, sol[3], sol[4], sol[5], TRUE);

  nOut0 = 0;

  for (i = 0; i < listP->n; i++) {
    listP->val1A[i].ind = i;
    listP->val2A[i].ind = i;
    listP->out1[i] = FALSE;
    listP->out2[i] = FALSE;

    Vec3Copy(x, listP->p[i]);
    Vec3Sub(x, sol);

    if (listP->kind != PSK_SPHERE)
      Mat3VecMult(x, rotMat);

    if (listP->kind == PSK_SPHERE) {
      listP->val1A[i].val = x[0] * x[0] + x[1] * x[1] + x[2] * x[2];
    } else if (listP->kind == PSK_CYLINDER) {
      if (x[2] < 0.0f)
	x[2] = - x[2];

      listP->val1A[i].val = x[2];
      listP->val2A[i].val = x[0] * x[0] + x[1] * x[1];
    } else if (listP->kind == PSK_CONE) {
      if (x[2] < 0.0f)
	nOut0++;

      listP->val1A[i].val = x[2];
      listP->val2A[i].val = (x[0] * x[0] + x[1] * x[1]) / (x[2] * x[2]);
    } else if (listP->kind == PSK_ELLIPSOID) {
      if (x[2] < 0.0f)
	x[2] = - x[2];
      
      if (x[0] < 0.0f)
	x[0] = - x[0];
      
      f2 = x[2] / sol[6];
      f0 = x[0] / sol[7];
      f1 = 1.0f - f2 * f2 - f0 * f0;

      if (f1 < 0.0f) {
	nOut0++;
	if (nOut0 > listP->nOut)
	  return FALSE;
	listP->val1A[i].val = BIG_SIZE;
      } else {
	listP->val1A[i].val = x[1] * x[1] / f1;
      }
    } else {
      if (x[2] < 0.0f)
	x[2] = - x[2];
      
      if (x[2] > sol[6]) {
	nOut0++;
	if (nOut0 > listP->nOut)
	  return FALSE;
	listP->val1A[i].val = BIG_SIZE;
	listP->val2A[i].val = BIG_SIZE;
      } else {
	if (x[0] < 0.0f)
	  x[0] = - x[0];
	
	if (x[1] < 0.0f)
	  x[1] = - x[1];
	
	listP->val1A[i].val = x[0];
	listP->val2A[i].val = x[1];
      }
    }
  }

  if (listP->kind == PSK_CONE) {
    neg = (nOut0 > listP->n / 2);
    if (neg) {
      for (i = 0; i < listP->n; i++)
	listP->val1A[i].val = - listP->val1A[i].val;
      nOut0 = listP->n - nOut0;
    }

    if (nOut0 > listP->nOut)
      return FALSE;
  }

  size1 = 0.0f;
  size2 = 0.0f;
  size3 = 0.0f;

  if (listP->nOut == 0) {
    for (i = 0; i < listP->n; i++) {
      if (listP->val1A[i].val > size1)
	size1 = listP->val1A[i].val;

      if (listP->kind != PSK_SPHERE && listP->kind != PSK_ELLIPSOID) {
	if (listP->val2A[i].val > size2)
	  size2 = listP->val2A[i].val;
      }
    }
  } else {
    sortVal(listP->val1A, 0, listP->n - 1, listP->nOut);

    if (listP->kind == PSK_SPHERE || listP->kind == PSK_ELLIPSOID) {
      size1 = listP->val1A[listP->nOut].val;
    } else {
      sortVal(listP->val2A, 0, listP->n - 1, listP->nOut);

      for (i = 0; i < listP->nOut; i++) {
	i1old = listP->val1A[i].ind;
	listP->out1[i1old] = TRUE;
      }

      i1 = listP->nOut;
      i2 = 0;
      volMin = MAXFLOAT;
      for (;;) {
	if (listP->kind == PSK_CONE)
	  vol = listP->val1A[i1].val * listP->val1A[i1].val *
	      listP->val1A[i1].val * listP->val2A[i2].val;
	else
	  vol = listP->val1A[i1].val * listP->val2A[i2].val;

	if (vol < volMin) {
	  volMin = vol;
	  size1 = listP->val1A[i1].val;
	  size2 = listP->val2A[i2].val;
	}

	i1--;
	if (i1 < 0)
	  break;

	i1old = listP->val1A[i1].ind;
	listP->out1[i1old] = FALSE;

	if (listP->out2[i1old])
	  continue;

	for (;;) {
	  i2old = listP->val2A[i2].ind;
	  listP->out2[i2old] = TRUE;
	  i2++;
	  if (! listP->out1[i2old])
	    break;
	}
      }
    }
  }

  if (listP->kind == PSK_SPHERE) {
    size1 = sqrtf(size1);
  } else if (listP->kind == PSK_CYLINDER) {
    size2 = sqrtf(size2);
  } else if (listP->kind == PSK_CONE) {
    size2 = sqrtf(size2) * size1;
    if (neg)
      size1 = - size1;
  } else if (listP->kind == PSK_ELLIPSOID) {
    size3 = sqrtf(size1);
    size1 = sol[6];
    size2 = sol[7];
  } else {
    size3 = size2;
    size2 = size1;
    size1 = sol[6];
  }

  *size1P = size1;
  *size2P = size2;
  *size3P = size3;

  return TRUE;
}

static BOOL
minFunc(float *valP, float *sol, int dim, void *clientData)
{
  PointList *listP = clientData;
  float size1, size2, size3;

  if (BreakCheck(10000 / listP->n))
    return FALSE;

  if (! getSizes(listP, sol, &size1, &size2, &size3))
    return FALSE;

  if (listP->kind == PSK_SPHERE) {
    *valP = size1 * size1 * size1;
  } else if (listP->kind == PSK_CYLINDER) {
    *valP = size1 * size2 * size2;
  } else if (listP->kind == PSK_CONE ) {
    if (size1 < 0.0f)
      *valP = - size1 * size2 * size2;
    else
      *valP = size1 * size2 * size2;
  } else {
    *valP = size1 * size2 * size3;
  }

  return TRUE;
}

void
SolidCalcGeom(PrimObjP primP)
{
  PrimSolid *solidP;
  Vec3 vAxis, v;
  float dMax, d;
  float sol[MAX_PAR_NO];
  float solMin[MAX_PAR_NO], solMax[MAX_PAR_NO], solInc[MAX_PAR_NO];
  PointList list;
  Mat3 rotMat;
  int i, k;

  solidP = &primP->u.solid;

  if (solidP->geomValid)
    return;

  list.n = solidP->atomNo;
  list.p = malloc(list.n * sizeof(*list.p));
  list.nOut = list.n - (int) (solidP->cover * list.n + 0.5f);

  for (i = 0; i < list.n; i++)
    DhAtomGetCoord(solidP->atomA[i], list.p[i]);

  if (list.nOut == 0)
    reduceList(&list);

  for (k = 0; k < 3; k++) {
    solMin[k] = MAXFLOAT;
    solMax[k] = - MAXFLOAT;
  }

  for (i = 0; i < list.n; i++) {
    for (k = 0; k < 3; k++) {
      if (list.p[i][k] < solMin[k])
        solMin[k] = list.p[i][k];
      if (list.p[i][k] > solMax[k])
        solMax[k] = list.p[i][k];
    }
  }

  list.kind = solidP->kind;
  list.val1A = malloc(list.n * sizeof(*list.val1A));
  list.val2A = malloc(list.n * sizeof(*list.val2A));
  list.out1 = malloc(list.n * sizeof(*list.out1));
  list.out2 = malloc(list.n * sizeof(*list.out2));

  for (k = 0; k < 3; k++) {
    sol[k] = 0.5f * (solMin[k] + solMax[k]);
    solInc[k] = 0.3f * (solMax[k] - solMin[k]);
  }

  if (solidP->kind == PSK_SPHERE) {
    (void) SimulAnnealMin(minFunc, 3, sol, &list,
	solMin, solMax, solInc, 15);
  } else {
    dMax = 0.0f;
    for (i = 0; i < list.n; i++) {
      Vec3Copy(v, list.p[i]);
      Vec3Sub(v, sol);
      d = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
      if (d > dMax) {
	Vec3Copy(vAxis, v);
	dMax = d;
      }
    }

    if (solidP->kind == PSK_CONE) {
      Vec3ScaleAdd(sol, 1.5f, vAxis);
      Vec3Scale(vAxis, -1.0f);

      for (k = 0; k < 3; k++) {
	solMin[k] = - MAXFLOAT;
	solMax[k] = MAXFLOAT;
      }
    }

    sol[3] = atanf(vAxis[1] / vAxis[2]);
    sol[4] = atanf(vAxis[0] /
	(vAxis[1] * sinf(sol[0]) + vAxis[2] * cosf(sol[0])));

    solMin[3] = - 0.5f * (float) M_PI;
    solMax[3] = 0.5f * (float) M_PI;
    solInc[3] = 1.0f;

    solMin[4] = - 0.5f * (float) M_PI;
    solMax[4] = 0.5f * (float) M_PI;
    solInc[4] = 1.0f;

    if (solidP->kind == PSK_BOX || solidP->kind == PSK_ELLIPSOID) {
      sol[5] = 0.0;
      solMin[5] = - 0.5f * (float) M_PI;
      solMax[5] = 0.5f * (float) M_PI;
      solInc[5] = 1.0f;

      sol[6] = 1.5f * Vec3Abs(vAxis);
      solMin[6] = 0.0f;
      solMax[6] = MAXFLOAT;
      solInc[6] = 0.3f * sol[6];

      if (solidP->kind == PSK_ELLIPSOID) {
	sol[7] = 1.0f * Vec3Abs(vAxis);
	solMin[7] = 0.0f;
	solMax[7] = MAXFLOAT;
	solInc[7] = 0.3f * sol[7];

	(void) SimulAnnealMin(minFunc, 8, sol, &list,
	    solMin, solMax, solInc, 15);
      } else {
	(void) SimulAnnealMin(minFunc, 7, sol, &list,
	    solMin, solMax, solInc, 15);
      }
    } else {
      solMin[6] = 0.0f;

      (void) SimulAnnealMin(minFunc, 5, sol, &list,
	  solMin, solMax, solInc, 15);
    }
  }

  (void) getSizes(&list, sol, &solidP->size1, &solidP->size2, &solidP->size3);

  free(list.p);
  free(list.val1A);
  free(list.val2A);
  free(list.out1);
  free(list.out2);

  Vec3Copy(solidP->cent, sol);

  solidP->axis1[0] = 0.0f;
  solidP->axis1[1] = 0.0f;
  solidP->axis1[2] = 1.0f;

  if (solidP->kind == PSK_SPHERE) {
    solidP->axis2[0] = 1.0f;
    solidP->axis2[1] = 0.0f;
    solidP->axis2[2] = 0.0f;
  } else {
    if (solidP->kind == PSK_CONE && solidP->size1 < 0.0f) {
      solidP->size1 = - solidP->size1;
      solidP->axis1[2] = -1.0f;
    }

    fillMat(rotMat, sol[3], sol[4], sol[5], FALSE);
    Mat3VecMult(solidP->axis1, rotMat);

    if (solidP->kind == PSK_BOX || solidP->kind == PSK_ELLIPSOID) {
      solidP->axis2[0] = 1.0f;
      solidP->axis2[1] = 0.0f;
      solidP->axis2[2] = 0.0f;
      Mat3VecMult(solidP->axis2, rotMat);
    } else {
      solidP->axis2[0] = - solidP->axis1[1];
      solidP->axis2[1] = solidP->axis1[0];
      solidP->axis2[2] = 0.0f;
      Vec3Norm(solidP->axis2);
    }
  }

  Vec3Copy(solidP->axis3, solidP->axis1);
  Vec3Cross(solidP->axis3, solidP->axis2);

  solidP->geomValid = TRUE;
  solidP->pointValid = FALSE;
}

static int
getCircPointNo(PrimSolid *solidP)
{
  int n;

  n = 4 * (1 << solidP->prec);
  if (n > MAX_POINT_NO)
    n = MAX_POINT_NO;
  
  return n;
}

static void
getCosSin(float *cA, float *sA, int n)
{
  float angInc, ang;
  int i;

  angInc = 2.0f * (float) M_PI / n;

  for (i = 0; i < n; i++) {
    ang = i * angInc;
    cA[i] = cosf(ang);
    sA[i] = sinf(ang);
  }
}

static void
calcEllipsoidPoints(PrimSolid *solidP)
{
  int pointNo, pointI, meshI, traceI, nextI;
  float cA[MAX_POINT_NO], sA[MAX_POINT_NO];
  float s1, s2, s3, cp, sp, ct, st;
  Vec3 x, nv;

  pointNo = getCircPointNo(solidP);
  getCosSin(cA, sA, pointNo);

  solidP->meshA = malloc(pointNo * sizeof(*solidP->meshA));
  solidP->meshNo = pointNo;

  for (meshI = 0; meshI < pointNo; meshI++) {
    solidP->meshA[meshI].xA = malloc(pointNo * sizeof(Vec3));
    solidP->meshA[meshI].nvA = malloc(pointNo * sizeof(Vec3));
    solidP->meshA[meshI].pointNo = pointNo;
  }

  s1 = solidP->size1;
  s2 = solidP->size2;
  s3 = solidP->size3;

  Vec3Copy(x, solidP->cent);
  Vec3ScaleAdd(x, s1, solidP->axis1);
  Vec3Copy(nv, solidP->axis1);
  for (meshI = 0; meshI < pointNo; meshI++) {
    Vec3Copy(solidP->meshA[meshI].xA[0], x);
    Vec3Copy(solidP->meshA[meshI].nvA[0], nv);
  }

  Vec3Copy(x, solidP->cent);
  Vec3ScaleSub(x, s1, solidP->axis1);
  Vec3Scale(nv, -1.0f);
  for (meshI = 0; meshI < pointNo; meshI++) {
    Vec3Copy(solidP->meshA[meshI].xA[pointNo - 1], x);
    Vec3Copy(solidP->meshA[meshI].nvA[pointNo - 1], nv);
  }

  for (traceI = 0; traceI < pointNo; traceI++) {
    ct = cA[traceI];
    st = sA[traceI];
    nextI = (traceI + 1) % pointNo;

    for (pointI = 1; pointI < pointNo / 2; pointI++) {
      cp = cA[pointI];
      sp = sA[pointI];

      Vec3Copy(x, solidP->cent);
      Vec3ScaleAdd(x, s1 * cp, solidP->axis1);
      Vec3ScaleAdd(x, s2 * sp * ct, solidP->axis2);
      Vec3ScaleAdd(x, s3 * sp * st, solidP->axis3);

      Vec3Copy(solidP->meshA[traceI].xA[2 * pointI], x);
      Vec3Copy(solidP->meshA[nextI].xA[2 * pointI - 1], x);

      Vec3Zero(nv);
      Vec3ScaleAdd(nv, s2 * s3 * cp * sp, solidP->axis1);
      Vec3ScaleAdd(nv, s1 * s3 * sp * sp * ct, solidP->axis2);
      Vec3ScaleAdd(nv, s1 * s2 * sp * sp * st, solidP->axis3);
      Vec3Norm(nv);

      Vec3Copy(solidP->meshA[traceI].nvA[2 * pointI], nv);
      Vec3Copy(solidP->meshA[nextI].nvA[2 * pointI - 1], nv);
    }
  }
}

static void
calcConePoints(PrimSolid *solidP)
{
  int pointNo, pointI;
  int meshNo, meshI, prevI;
  float cA[MAX_POINT_NO], sA[MAX_POINT_NO];
  Vec3 x, nv;

  pointNo = getCircPointNo(solidP);
  getCosSin(cA, sA, pointNo);

  solidP->polyA = malloc(sizeof(*solidP->polyA));
  solidP->polyNo = 1;

  solidP->polyA[0].xA = malloc(pointNo * sizeof(Vec3));
  solidP->polyA[0].pointNo = pointNo;
  Vec3Copy(solidP->polyA[0].nv, solidP->axis1);

  meshNo = pointNo / 2;
  solidP->meshA = malloc(meshNo * sizeof(*solidP->meshA));
  solidP->meshNo = meshNo;

  for (meshI = 0; meshI < meshNo; meshI++) {
    solidP->meshA[meshI].xA = malloc(4 * sizeof(Vec3));
    solidP->meshA[meshI].nvA = malloc(4 * sizeof(Vec3));
    solidP->meshA[meshI].pointNo = 4;
  }

  for (pointI = 0; pointI < pointNo; pointI++) {
    Vec3Copy(x, solidP->cent);
    Vec3ScaleAdd(x, solidP->size1, solidP->axis1);
    Vec3ScaleAdd(x, solidP->size2 * cA[pointI], solidP->axis2);
    Vec3ScaleAdd(x, solidP->size2 * sA[pointI], solidP->axis3);

    Vec3Copy(solidP->polyA[0].xA[pointI], x);

    Vec3Zero(nv);
    Vec3ScaleAdd(nv, - solidP->size2, solidP->axis1);
    Vec3ScaleAdd(nv, solidP->size1 * cA[pointI], solidP->axis2);
    Vec3ScaleAdd(nv, solidP->size1 * sA[pointI], solidP->axis3);
    Vec3Norm(nv);

    meshI = pointI / 2;
    if (pointI % 2 == 0) {
      prevI = (meshI + meshNo - 1) % meshNo;

      Vec3Copy(solidP->meshA[meshI].xA[0], x);
      Vec3Copy(solidP->meshA[meshI].nvA[0], nv);
      Vec3Copy(solidP->meshA[prevI].xA[3], x);
      Vec3Copy(solidP->meshA[prevI].nvA[3], nv);
    } else {
      Vec3Copy(solidP->meshA[meshI].xA[1], solidP->cent);
      Vec3Copy(solidP->meshA[meshI].nvA[1], nv);
      Vec3Copy(solidP->meshA[meshI].xA[2], x);
      Vec3Copy(solidP->meshA[meshI].nvA[2], nv);
    }
  }
}

static void
calcBoxPoints(PrimSolid *solidP)
{
  Vec3 x, nv;
  int i;

  solidP->polyA = malloc(6 * sizeof(*solidP->polyA));
  solidP->polyNo = 6;

  for (i = 0; i < 6; i++) {
    solidP->polyA[i].xA = malloc(4 * sizeof(Vec3));
    solidP->polyA[i].pointNo = 4;
  }

  Vec3Copy(nv, solidP->axis1);
  Vec3Copy(solidP->polyA[0].nv, nv);
  Vec3Scale(nv, -1.0f);
  Vec3Copy(solidP->polyA[1].nv, nv);

  Vec3Copy(nv, solidP->axis2);
  Vec3Copy(solidP->polyA[2].nv, nv);
  Vec3Scale(nv, -1.0f);
  Vec3Copy(solidP->polyA[3].nv, nv);

  Vec3Copy(nv, solidP->axis3);
  Vec3Copy(solidP->polyA[4].nv, nv);
  Vec3Scale(nv, -1.0f);
  Vec3Copy(solidP->polyA[5].nv, nv);

  Vec3Copy(x, solidP->cent);
  Vec3ScaleAdd(x, solidP->size1, solidP->axis1);
  Vec3ScaleAdd(x, solidP->size2, solidP->axis2);
  Vec3ScaleAdd(x, solidP->size3, solidP->axis3);
  Vec3Copy(solidP->polyA[0].xA[0], x);
  Vec3Copy(solidP->polyA[2].xA[0], x);
  Vec3Copy(solidP->polyA[4].xA[0], x);

  Vec3Copy(x, solidP->cent);
  Vec3ScaleSub(x, solidP->size1, solidP->axis1);
  Vec3ScaleAdd(x, solidP->size2, solidP->axis2);
  Vec3ScaleAdd(x, solidP->size3, solidP->axis3);
  Vec3Copy(solidP->polyA[1].xA[0], x);
  Vec3Copy(solidP->polyA[2].xA[3], x);
  Vec3Copy(solidP->polyA[4].xA[1], x);

  Vec3Copy(x, solidP->cent);
  Vec3ScaleAdd(x, solidP->size1, solidP->axis1);
  Vec3ScaleSub(x, solidP->size2, solidP->axis2);
  Vec3ScaleAdd(x, solidP->size3, solidP->axis3);
  Vec3Copy(solidP->polyA[0].xA[1], x);
  Vec3Copy(solidP->polyA[3].xA[0], x);
  Vec3Copy(solidP->polyA[4].xA[3], x);

  Vec3Copy(x, solidP->cent);
  Vec3ScaleSub(x, solidP->size1, solidP->axis1);
  Vec3ScaleSub(x, solidP->size2, solidP->axis2);
  Vec3ScaleAdd(x, solidP->size3, solidP->axis3);
  Vec3Copy(solidP->polyA[1].xA[3], x);
  Vec3Copy(solidP->polyA[3].xA[1], x);
  Vec3Copy(solidP->polyA[4].xA[2], x);

  Vec3Copy(x, solidP->cent);
  Vec3ScaleAdd(x, solidP->size1, solidP->axis1);
  Vec3ScaleAdd(x, solidP->size2, solidP->axis2);
  Vec3ScaleSub(x, solidP->size3, solidP->axis3);
  Vec3Copy(solidP->polyA[0].xA[3], x);
  Vec3Copy(solidP->polyA[2].xA[1], x);
  Vec3Copy(solidP->polyA[5].xA[0], x);

  Vec3Copy(x, solidP->cent);
  Vec3ScaleSub(x, solidP->size1, solidP->axis1);
  Vec3ScaleAdd(x, solidP->size2, solidP->axis2);
  Vec3ScaleSub(x, solidP->size3, solidP->axis3);
  Vec3Copy(solidP->polyA[1].xA[1], x);
  Vec3Copy(solidP->polyA[2].xA[2], x);
  Vec3Copy(solidP->polyA[5].xA[3], x);

  Vec3Copy(x, solidP->cent);
  Vec3ScaleAdd(x, solidP->size1, solidP->axis1);
  Vec3ScaleSub(x, solidP->size2, solidP->axis2);
  Vec3ScaleSub(x, solidP->size3, solidP->axis3);
  Vec3Copy(solidP->polyA[0].xA[2], x);
  Vec3Copy(solidP->polyA[3].xA[3], x);
  Vec3Copy(solidP->polyA[5].xA[1], x);

  Vec3Copy(x, solidP->cent);
  Vec3ScaleSub(x, solidP->size1, solidP->axis1);
  Vec3ScaleSub(x, solidP->size2, solidP->axis2);
  Vec3ScaleSub(x, solidP->size3, solidP->axis3);
  Vec3Copy(solidP->polyA[1].xA[2], x);
  Vec3Copy(solidP->polyA[3].xA[2], x);
  Vec3Copy(solidP->polyA[5].xA[2], x);
}

void
SolidCalcPoints(PrimObjP primP)
{
  PrimSolid *solidP;
  int prec;

  solidP = &primP->u.solid;

  prec = ParGetIntVal(PN_DRAW_PREC);
  if (solidP->pointValid && prec == solidP->prec)
    return;

  solidP->prec = prec;
  SolidCalcGeom(primP);

  if (solidP->meshA != NULL) {
    free(solidP->meshA);
    solidP->meshA = NULL;
    solidP->meshNo = 0;
  }

  if (solidP->polyA != NULL) {
    free(solidP->polyA);
    solidP->polyA = NULL;
    solidP->polyNo = 0;
  }

  switch (solidP->kind) {
    case PSK_SPHERE:
      break;
    case PSK_ELLIPSOID:
      calcEllipsoidPoints(solidP);
      break;
    case PSK_CYLINDER:
      break;
    case PSK_CONE:
      calcConePoints(solidP);
      break;
    case PSK_BOX:
      calcBoxPoints(solidP);
      break;
  }

  solidP->pointValid = TRUE;
}
