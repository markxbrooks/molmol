/*
************************************************************************
*
*   MapCoord.c - find least squares superposition
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/tools/src/SCCS/s.MapCoord.c
*   SCCS identification       : 1.5
*
************************************************************************
*/

#include <map_coord.h>

#include <sing_val.h>

void
MapCoord(Vec3 x0A[], int n, Vec3 x1A[], BOOL trans, Mat4 mapM)
{
  float am[3 * 3], *a[3], w[3], vm[3 * 3], *v[3];
  Vec3 avg0, avg1;
  Vec3 x0, x1;
  float det;
  Mat3 rotM;
  Mat4 rotM4, transM;
  int pointI, i, k;

  for (i = 0; i < 3; i++) {
    a[i] = am + i * 3;
    v[i] = vm + i * 3;
  }

  if (trans) {
    Vec3Zero(avg0);
    Vec3Zero(avg1);
    for (i = 0; i < n; i++) {
      Vec3Add(avg0, x0A[i]);
      Vec3Add(avg1, x1A[i]);
    }
    Vec3Scale(avg0, 1.0f / n);
    Vec3Scale(avg1, 1.0f / n);
  }

  for (i = 0; i < 3; i++)
    for (k = 0; k < 3; k++)
      a[i][k] = 0.0f;

  for (pointI = 0; pointI < n; pointI++) {
    Vec3Copy(x0, x0A[pointI]);
    if (trans)
      Vec3Sub(x0, avg0);

    Vec3Copy(x1, x1A[pointI]);
    if (trans)
      Vec3Sub(x1, avg1);

    for (i = 0; i < 3; i++)
      for (k = 0; k < 3; k++)
	a[i][k] += x0[i] * x1[k];
  }

  (void) SingValDecomp(a, 3, 3, w, v);

  for (i = 0; i < 3; i++)
    for (k = 0; k < 3; k++)
      rotM[i][k] = a[k][0] * v[i][0] + a[k][1] * v[i][1] + a[k][2] * v[i][2];
  
  det = rotM[0][0] * rotM[1][1] * rotM[2][2] +
	rotM[0][1] * rotM[1][2] * rotM[2][0] +
	rotM[0][2] * rotM[1][0] * rotM[2][1] -
	rotM[2][0] * rotM[1][1] * rotM[0][2] -
	rotM[2][1] * rotM[1][2] * rotM[0][0] -
	rotM[2][2] * rotM[1][0] * rotM[0][1];

  if (det < 0.0f) {
    if (w[0] < w[1] && w[0] < w[2])
      for (i = 0; i < 3; i++)
	for (k = 0; k < 3; k++)
	  rotM[i][k] =
	      - a[k][0] * v[i][0] + a[k][1] * v[i][1] + a[k][2] * v[i][2];
    else if (w[1] < w[2])
      for (i = 0; i < 3; i++)
	for (k = 0; k < 3; k++)
	  rotM[i][k] =
	      a[k][0] * v[i][0] - a[k][1] * v[i][1] + a[k][2] * v[i][2];
    else
      for (i = 0; i < 3; i++)
	for (k = 0; k < 3; k++)
	  rotM[i][k] =
	      a[k][0] * v[i][0] + a[k][1] * v[i][1] - a[k][2] * v[i][2];
  }

  Mat3Ortho(rotM);
  Mat4Ident(mapM);

  if (trans) {
    mapM[3][0] = avg0[0];
    mapM[3][1] = avg0[1];
    mapM[3][2] = avg0[2];
  }

  Mat3To4(rotM4, rotM);
  Mat4Mult(mapM, rotM4);

  if (trans) {
    Mat4Ident(transM);
    transM[3][0] = - avg1[0];
    transM[3][1] = - avg1[1];
    transM[3][2] = - avg1[2];
    Mat4Mult(mapM, transM);
  }
}
