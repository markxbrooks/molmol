/*
************************************************************************
*
*   FitCylinder.c - fit cylinder
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/tools/src/SCCS/s.FitCylinder.c
*   SCCS identification       : 1.8
*
************************************************************************
*/

#include <fit_cylinder.h>

#include <stdlib.h>
#include <values.h>
#include <math.h>

#include <mat_vec.h>
#include <least_sqr.h>

#define MAX_STEPS 100
#define PAR_NO_LS 5
#define PAR_NO_SP 4
#define EPS ((float) 1.0e-6)
#define TOO_BIG ((float) 1.0e6)

void
FitCylinderLeastSq(Vec3 p[], int n, Vec3 x1, Vec3 x2, float *radiusP)
/* Fit cylinder with least squares fit. */
{
  float *m, **c, *d;
  int avgPointNo;
  float v0[3], v1[3], v2[3], s;
  float tmp[PAR_NO_LS], isol[PAR_NO_LS], sol[PAR_NO_LS], dsol[PAR_NO_LS];
  float *res;
  float cx, sx, cy, sy, tx, ty;
  float x, y, z, xt, yt, zt, r;
  float rmsd, a;
  Mat3 rotMat;
  float minZ, maxZ, z1, z2;
  int step, i;

  m = malloc(PAR_NO_LS * n * sizeof(float));
  c = malloc(n * sizeof(float *));
  for (i = 0; i < n; i++)
    c[i] = m + PAR_NO_LS * i;
  d = malloc(n * sizeof(float));
  res = malloc(n * sizeof(float));

  /* determine initial cylinder */
  avgPointNo = 2 * n / 3;
  Vec3Zero(v1);
  Vec3Zero(v2);
  for (i = 0; i < avgPointNo; i++) {
    Vec3Add(v1, p[i]);
    Vec3Add(v2, p[n - 1 - i]);
  }
  Vec3Scale(v1, 1.0f / (float) avgPointNo);
  Vec3Scale(v2, 1.0f / (float) avgPointNo);

  Vec3Sub(v2, v1);
  Vec3Norm(v2);

  isol[0] = atanf(v2[1] / v2[2]);
  cx = cosf(isol[0]);
  sx = sinf(isol[0]);
  isol[3] = v1[2] * sx - v1[1] * cx;
  isol[1] = atanf(v2[0] / (v2[1] * sx + v2[2] * cx));
  cy = cosf(isol[1]);
  sy = sinf(isol[1]);
  isol[2] = (v1[1] * sx + v1[2] * cx) * sy - v1[0] * cy;
  isol[4] = 0.0f;
  for (i = 0; i < n; i++) {
    Vec3Copy(v0, p[i]);
    Vec3Sub(v0, v1);
    s = Vec3Scalar(v0, v2);
    isol[4] += Vec3Scalar(v0, v0) - s * s;
  }
  isol[4] /= n;

  for (i = 0; i < PAR_NO_LS; i++)
    sol[i] = isol[i];

  /* iteration (non linear least squares) */
  for (step = 0; step < MAX_STEPS; step++) {
    cx = cosf(sol[0]);
    sx = sinf(sol[0]);
    cy = cosf(sol[1]);
    sy = sinf(sol[1]);
    tx = sol[2];
    ty = sol[3];
    r = sol[4];

    for (i = 0; i < n; i++) {
      x = p[i][0];
      y = p[i][1];
      z = p[i][2];

      xt = x * cy - (y * sx + z * cx) * sy + tx;
      yt = y * cx - z * sx + ty;

      c[i][0] = 2 * xt * (- y * cx + z * sx) * sy +
                2 * yt * (- y * sx - z * cx);
      c[i][1] = 2 * xt * (- x * sy - (y * sx + z * cx) * cy);
      c[i][2] = 2 * xt;
      c[i][3] = 2 * yt;
      c[i][4] = - 2 * r;

      d[i] = xt * xt + yt * yt - r * r;
    }

    LeastSqrMatTransf(c, PAR_NO_LS, n, tmp);
    LeastSqrCalcSol(c, tmp, d, PAR_NO_LS, n, dsol, res);

    for (i = 0; i < PAR_NO_LS; i++)
      sol[i] += dsol[i];

    rmsd = 0.0f;
    for (i = 0; i < n; i++)
      rmsd += res[i] * res[i];

    a = 0.0f;
    for (i = 0; i < PAR_NO_LS; i++)
      a += dsol[i] * dsol[i];
    a = sqrtf(a / PAR_NO_LS);

    if (rmsd > TOO_BIG || a > TOO_BIG) {
      /* iteration is running wild, take start values and stop */
      for (i = 0; i < PAR_NO_LS; i++)
	sol[i] = isol[i];
      break;
    }

    if (rmsd < EPS || a < EPS)
      break;
  }

  *radiusP = sol[4];
  if (*radiusP < 0.0f)
    *radiusP = - *radiusP;

  cx = cosf(sol[0]);
  sx = sinf(sol[0]);
  cy = cosf(sol[1]);
  sy = sinf(sol[1]);
  tx = sol[2];
  ty = sol[3];

  rotMat[0][0] =        cy; rotMat[1][0] = 0.0f; rotMat[2][0] =      sy;
  rotMat[0][1] = - sx * sy; rotMat[1][1] =   cx; rotMat[2][1] = sx * cy;
  rotMat[0][2] = - cx * sy; rotMat[1][2] = - sx; rotMat[2][2] = cx * cy;

  minZ = MAXFLOAT;
  maxZ = - MAXFLOAT;
  for (i = 0; i < n; i++) {
    zt = rotMat[2][0] * p[i][0] +
	 rotMat[2][1] * p[i][1] +
	 rotMat[2][2] * p[i][2];

    if (i == 0)
      z1 = zt;
    else if (i == n - 1)
      z2 = zt;

    if (zt > maxZ)
      maxZ = zt;
    if (zt < minZ)
      minZ = zt;
  }

  if (z1 < z2) {
    z1 = minZ;
    z2 = maxZ;
  } else {
    z1 = maxZ;
    z2 = minZ;
  }

  x1[0] = - tx;
  x1[1] = - ty;
  x1[2] = z1;
  Mat3VecMult(x1, rotMat);
  x2[0] = - tx;
  x2[1] = - ty;
  x2[2] = z2;
  Mat3VecMult(x2, rotMat);

  free(m);
  free(c);
  free(d);
  free(res);
}

void
FitCylinderSpacing(Vec3 p[], int n, Vec3 x1, Vec3 x2, float *radiusP)
/* Fit cylinder so that spacing of points in axis direction is as
   regular as possible. */
{
  float *m, **c, *d;
  float tmp[PAR_NO_SP], sol[PAR_NO_SP];
  float *res;
  Vec3 vx, vy, vz, v;
  float xAvg, yAvg, sol2, rAvg, s;
  int i;

  m = malloc(PAR_NO_SP * n * sizeof(float));
  c = malloc(n * sizeof(float *));
  for (i = 0; i < n; i++)
    c[i] = m + PAR_NO_SP * i;
  d = malloc(n * sizeof(float));
  res = malloc(n * sizeof(float));

  for (i = 0; i < n; i++) {
    c[i][0] = p[i][0];
    c[i][1] = p[i][1];
    c[i][2] = p[i][2];
    c[i][3] = 1.0f;

    d[i] = - (float) i;
  }

  LeastSqrMatTransf(c, PAR_NO_SP, n, tmp);
  LeastSqrCalcSol(c, tmp, d, PAR_NO_SP, n, sol, res);

  Vec3Copy(vz, sol);
  Vec3Norm(vz);

  if (vz[0] > 0.5f || vz[0] < -0.5f) {
    vx[0] = vz[1];
    vx[1] = - vz[0];
    vx[2] = 0.0f;
  } else {
    vx[0] = 0.0f;
    vx[1] = vz[2];
    vx[2] = - vz[1];
  }
  Vec3Norm(vx);

  Vec3Copy(vy, vz);
  Vec3Cross(vy, vx);

  xAvg = 0.0f;
  yAvg = 0.0f;
  for (i = 0; i < n; i++) {
    xAvg += Vec3Scalar(p[i], vx);
    yAvg += Vec3Scalar(p[i], vy);
  }
  xAvg /= n;
  yAvg /= n;

  sol2 = sol[0] * sol[0] + sol[1] * sol[1] + sol[2] * sol[2];
  Vec3Zero(x1);
  Vec3ScaleAdd(x1, xAvg, vx);
  Vec3ScaleAdd(x1, yAvg, vy);
  Vec3ScaleAdd(x1, - sol[3] / sol2, sol);

  Vec3Copy(x2, x1);
  Vec3ScaleAdd(x2, (n - 1) / sol2, sol);

  rAvg = 0.0f;
  for (i = 0; i < n; i++) {
    Vec3Copy(v, p[i]);
    Vec3Sub(v, x1);
    s = Vec3Scalar(v, vx);
    rAvg += s * s;
    s = Vec3Scalar(v, vy);
    rAvg += s * s;
  }
  *radiusP = sqrtf(rAvg / n);

  free(m);
  free(c);
  free(d);
  free(res);
}
