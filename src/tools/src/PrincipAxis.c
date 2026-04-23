/*
************************************************************************
*
*   PrincipAxis.c - principal axis transformation
*
*   Copyright (c) 1994-98
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/tools/src/SCCS/s.PrincipAxis.c
*   SCCS identification       : 1.7
*
************************************************************************
*/

#include <princip_axis.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <sing_val.h>

static float *LastSumA = NULL;
static int LastN;

void
PrincipAxis(Vec3 xA[], int n, BOOL likeLast, Mat4 mapM, float lenA[3])
{
  float am[3 * 3], *a[3], w[3], vm[3 * 3], *v[3];
  float sMax, s, t, det, ws, f;
  int iMax, pointI, i1, i2, i, k;

  if (LastSumA == NULL || LastN != n) {
    if (LastSumA != NULL) {
      free(LastSumA);
      LastSumA = NULL;
    }
    likeLast = FALSE;
  }

  if (LastSumA == NULL) {
    LastSumA = malloc(n * sizeof(*LastSumA));
    LastN = n;
  }

  for (i = 0; i < 3; i++) {
    a[i] = am + i * 3;
    v[i] = vm + i * 3;
  }

  for (i = 0; i < 3; i++)
    for (k = 0; k < 3; k++)
      a[i][k] = 0.0f;

  sMax = 0.0f;
  for (pointI = 0; pointI < n; pointI++) {
    s = xA[pointI][0] * xA[pointI][0] +
	xA[pointI][1] * xA[pointI][1] +
	xA[pointI][2] * xA[pointI][2];

    if (s > sMax) {
      sMax = s;
      iMax = pointI;
    }

    for (i = 0; i < 3; i++) {
      a[i][i] += s;
      for (k = 0; k < 3; k++)
	a[i][k] -= xA[pointI][i] * xA[pointI][k];
    }
  }

  (void) SingValDecomp(a, 3, 3, w, v);

  /* sort Eigenvalues */
  for (i1 = 0; i1 < 2; i1++)
    for (i2 = i1 + 1; i2 < 3; i2++)
      if (w[i1] > w[i2]) {
	t = w[i1];
	w[i1] = w[i2];
	w[i2] = t;

	for (k = 0; k < 3; k++) {
	  t = a[k][i1];
	  a[k][i1] = a[k][i2];
	  a[k][i2] = t;
	}
      }

  if (lenA != NULL) {
    ws = w[0] + w[1] + w[2];
    f = sqrtf(2.5f / n);
    for (i1 = 0; i1 < 3; i1++)
      lenA[i1] = f * sqrtf(ws - 2.0f * w[i1]);
  }

  if (likeLast) {
    /* orient with values from last call */
    for (i = 0; i < 2; i++) {
      s = 0.0f;
      for (pointI = 0; pointI < n; pointI++)
	for (k = 0; k < 3; k++)
	  s += LastSumA[pointI] * a[k][i] * xA[pointI][k];

      if (s < 0.0f)
	for (k = 0; k < 3; k++)
	  a[k][i] = - a[k][i];
    }
  } else {
    /* Change direction of vector if scalar product with largest input
       point is negative. This is used to make the solution unique. */
    for (i = 0; i < 2; i++) {
      s = 0.0f;
      for (k = 0; k < 3; k++)
	s += a[k][i] * xA[iMax][k];

      if (s < 0.0f)
	for (k = 0; k < 3; k++)
	  a[k][i] = - a[k][i];
    }
  }

  /* save sum of coordinates, using the principaly axes as basis */
  for (pointI = 0; pointI < n; pointI++) {
    LastSumA[pointI] = 0.0f;
    for (i = 0; i < 3; i++)
      for (k = 0; k < 3; k++)
	LastSumA[pointI] += a[k][i] * xA[pointI][k];
  }

  det = a[0][0] * a[1][1] * a[2][2] +
	a[0][1] * a[1][2] * a[2][0] +
	a[0][2] * a[1][0] * a[2][1] -
	a[2][0] * a[1][1] * a[0][2] -
	a[2][1] * a[1][2] * a[0][0] -
	a[2][2] * a[1][0] * a[0][1];

  if (det < 0.0f)
    for (k = 0; k < 3; k++)
      a[k][2] = - a[k][2];

  Mat4Ident(mapM);
  for (i = 0; i < 3; i++)
    for (k = 0; k < 3; k++)
      mapM[i][k] = a[i][k];
}
