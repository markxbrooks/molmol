/*
************************************************************************
*
*   LeastSqr.c - solve least squares problems
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
*   Date of last modification : 01/05/27
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/tools/src/SCCS/s.LeastSqr.c
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include <least_sqr.h>

#include <stdio.h>
#include <math.h>

#include <break.h>

/* Solve least squares problem by Householder transformations.
   Algorithm from H. R. Schwarz, Numerische Mathematik, P. 302-303. */

void
LeastSqrMatTransf(float **c, int n, int m, float *t)
{
  int breakCount;
  int i, j, k, l;
  float s, w, h, p;

  breakCount = 500000 / (4 * m);

  for (l = 0; l < n; l++) {
    s = 0.0f;
    for (i = l; i < m; i++)
      s += c[i][l] * c[i][l];
    s = sqrtf(s);
    
    if (c[l][l] >= 0.0f) {
      w = sqrtf(0.5f + 0.5f * c[l][l] / s);
      h = 2.0f * s * w;
      t[l] = - s;
    } else {
      w = sqrtf(0.5f - 0.5f * c[l][l] / s);
      h = - 2.0f * s * w;
      t[l] = s;
    }
    c[l][l] = w;

    for (k = l + 1; k < m; k++)
      c[k][l] /= h;
    
    for (j = l + 1; j < n; j++) {
      if (BreakCheck(breakCount))
	break;

      p = 0.0f;
      for (k = l; k < m; k++)
	p += c[k][l] * c[k][j];
      p *= 2.0f;

      for (i = l; i < m; i++)
	c[i][j] -= p * c[i][l];
    }
  }
}

void
LeastSqrCalcSol(float **c, float *t, float *d, int n, int m,
    float *x, float *r)
{
  int breakCount;
  int i, k, l;
  float s;

  breakCount = 500000 / (4 * m);

  for (l = 0; l < n; l++) {
    s = 0.0f;
    for (k = l; k < m; k++)
      s += c[k][l] * d[k];
    s *= 2.0f;

    for (k = l; k < m; k++)
      d[k] -= s * c[k][l];
  }

  for (i = n - 1; i >= 0; i--) {
    s = d[i];
    for (k = i + 1; k < n; k++)
      s += c[i][k] * x[k];
    x[i] = - s / t[i];
  }

  if (r == NULL)
    return;

  for (i = 0; i < n; i++)
    r[i] = 0.0f;

  for (i = n; i < m; i++)
    r[i] = d[i];

  for (l = n - 1; l >= 0; l--) {
    if (BreakCheck(breakCount))
      break;

    s = 0.0f;
    for (k = l; k < m; k++)
      s += c[k][l] * r[k];
    s *= 2.0f;

    for (k = l; k < m; k++)
      r[k] -= s * c[k][l];
  }
}
