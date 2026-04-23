/*
************************************************************************
*
*   CubicSpline.c - cubic spline calculation
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
*   Date of last modification : 01/05/26
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/tools/src/SCCS/s.CubicSpline.c
*   SCCS identification       : 1.6
*
************************************************************************
*/

#include <cubic_spline.h>

#include <stdlib.h>
#include <math.h>

/* Calculate cubic spline interpolation in 3D.
   Algorithm from H. R. Schwarz, Numerische Mathematik, P. 129-130. */

void
CubicSplineCalc(Vec3 inP[], int n, CubicSplineCoeff coeffP[])
/* coeffP must have the size (n - 1) */
{
  int intNo, equNo;
  float *h, *ih;  /* interval sizes */
  float *a;  /* diagonal of tridiagonal matrix */
  float *d;  /* constant part of linear equations */
  float *m, *l;  /* LR decomposition of tridiagonal matrix */
  float *y, *x;
  float d0, d1;
  int coordI, i;

  if (n == 1) {
    /* cannot calculate spline, return constant polynom */
    for (coordI = 0; coordI < 3; coordI++) {
      coeffP[0].c3[coordI] = 0.0f;
      coeffP[0].c2[coordI] = 0.0f;
      coeffP[0].c1[coordI] = 0.0f;
      coeffP[0].c0[coordI] = inP[0][coordI];
    }
    return;
  }
  
  if (n == 2) {
    /* cannot calculate spline, do linear interpolation */
    for (coordI = 0; coordI < 3; coordI++) {
      coeffP[0].c3[coordI] = 0.0f;
      coeffP[0].c2[coordI] = 0.0f;
      coeffP[0].c1[coordI] = inP[1][coordI] - inP[0][coordI];
      coeffP[0].c0[coordI] = inP[0][coordI];
    }
    return;
  }

  intNo = n - 1;  /* number of intervals */
  equNo = intNo - 1;  /* number of equations */

  h = malloc(intNo * sizeof(*h));
  ih = malloc(intNo * sizeof(*ih));

  a = malloc(equNo * sizeof(*a));
  d = malloc(equNo * sizeof(*d));

  m = malloc(equNo * sizeof(*m));
  l = malloc((equNo - 1) * sizeof(*l));
  y = malloc(equNo * sizeof(*y));
  x = malloc(equNo * sizeof(*x));

  /* calculate interval sizes as distance between points */
  for (i = 0; i < intNo; i++) {
    h[i] = Vec3DiffAbs(inP[i], inP[i + 1]);
    ih[i] = 1.0f / h[i];
  }

  /* calculate diagonal of tridiagonal matrix */
  for (i = 0; i < equNo; i++)
    a[i] = 2.0f * (h[i] + h[i + 1]);

  /* calculate LR decomposition of tridiagonal matrix */
  m[0] = a[0];
  for (i = 0; i < equNo - 1; i++) {
    l[i] = h[i + 1] / m[i];
    m[i + 1] = a[i + 1] - l[i] * h[i + 1];
  }

  for (coordI = 0; coordI < 3; coordI++) {
    /* interpolation is done separately for all 3 coordinates */

    for (i = 0; i < equNo; i++)
      d[i] = 6.0f * (ih[i] * (inP[i + 1][coordI] - inP[i][coordI]) -
          ih[i + 1] * (inP[i + 2][coordI] - inP[i + 1][coordI]));

    /* forward elimination */
    y[0] = d[0];
    for (i = 1; i < equNo; i++)
      y[i] = d[i] - l[i - 1] * y[i - 1];

    /* back substitution */
    x[equNo - 1] = - y[equNo - 1] / m[equNo - 1];
    for (i = equNo - 2; i >= 0; i--)
      x[i] = - (y[i] + h[i + 1] * x[i + 1]) / m[i];

    /* calculate spline points */
    for (i = 0; i < intNo; i++) {
      /* calculate polynom coefficients */
      if (i == 0)
	d0 = 0.0f;
      else
	d0 = x[i - 1];
      
      if (i == intNo - 1)
	d1 = 0.0f;
      else
	d1 = x[i];

      coeffP[i].c3[coordI] = h[i] * h[i] / 6.0f * (d1 - d0);
      coeffP[i].c2[coordI] = 0.5f * h[i] * h[i] * d0;
      coeffP[i].c1[coordI] = inP[i + 1][coordI] - inP[i][coordI] -
	   h[i] * h[i] / 6.0f * (d1 + 2.0f * d0);
      coeffP[i].c0[coordI] = inP[i][coordI];
    }
  }

  free(h);
  free(ih);
  free(a);
  free(d);
  free(m);
  free(l);
  free(y);
  free(x);
}
