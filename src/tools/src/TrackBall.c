/*
************************************************************************
*
*   TrackBall.c - virtual trackball
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/tools/src/SCCS/s.TrackBall.c
*   SCCS identification       : 1.2
*
************************************************************************
*/
#include "track_ball.h"

#include <math.h>
#include <mat_vec.h>

static void
calcSphereVec(Vec3 v)
{
  float a;

  a = v[0] * v[0] + v[1] * v[1];
  if (a < 1.0f) {
    v[2] = sqrtf(1.0f - a);
  } else {
    a = sqrtf(a);
    v[0] /= a;
    v[1] /= a;
    v[2] = 0.0f;
  }
}

void TrackBallCalcMat(float v0x, float v0y, float v1x, float v1y,
    Mat4 trackMat)
{
  Vec3 v0, v1, y, z, v1t;
  int i;
  float c, s;
  Mat3 m, mf, mb, mr;

  v0[0] = v0x;
  v0[1] = v0y;
  calcSphereVec(v0);

  v1[0] = v1x;
  v1[1] = v1y;
  calcSphereVec(v1);

  if (Vec3DiffAbs(v0, v1) < 1E-6) {
    Mat4Ident(trackMat);
    return;
  }

  Vec3Copy(z, v0);
  Vec3Cross(z, v1);
  Vec3Norm(z);

  Vec3Copy(y, z);
  Vec3Cross(y, v0);
  Vec3Norm(y);

  for (i = 0; i < 3; i++)
    mf[i][0] = v0[i];
  for (i = 0; i < 3; i++)
    mf[i][1] = y[i];
  for (i = 0; i < 3; i++)
    mf[i][2] = z[i];

  for (i = 0; i < 3; i++)
    mb[0][i] = v0[i];
  for (i = 0; i < 3; i++)
    mb[1][i] = y[i];
  for (i = 0; i < 3; i++)
    mb[2][i] = z[i];

  Vec3Copy(v1t, v1);
  Mat3VecMult(v1t, mf);
  c = v1t[0]; s = v1t[1];
  mr[0][0] =    c; mr[0][1] =    s; mr[0][2] = 0.0f;
  mr[1][0] =   -s; mr[1][1] =    c; mr[1][2] = 0.0f;
  mr[2][0] = 0.0f; mr[2][1] = 0.0f; mr[2][2] = 1.0f;

  Mat3Copy(m, mb);
  Mat3Mult(m, mr);
  Mat3Mult(m, mf);

  Mat3To4(trackMat, m);
}
