/*
************************************************************************
*
*   MatVec.c - utilities for matrices and vectors
*
*   Copyright (c) 1994-97
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/tools/src/SCCS/s.MatVec.c
*   SCCS identification       : 1.6
*
************************************************************************
*/
#include <mat_vec.h>

#include <values.h>
#include <math.h>

void
Vec3Zero(Vec3 v)
{
  v[0] = 0.0f;
  v[1] = 0.0f;
  v[2] = 0.0f;
}

void
Vec4Zero(Vec4 v)
{
  v[0] = 0.0f;
  v[1] = 0.0f;
  v[2] = 0.0f;
  v[3] = 1.0f;
}

void
Vec3Copy(Vec3 v1, Vec3 v2)
{
  int i;

  for (i = 0; i < 3; i++)
    v1[i] = v2[i];
}

void
Vec4Copy(Vec4 v1, Vec4 v2)
{
  int i;

  for (i = 0; i < 4; i++)
    v1[i] = v2[i];
}

void
Vec3To4(Vec4 v1, Vec3 v2)
{
  int i;

  for (i = 0; i < 3; i++)
    v1[i] = v2[i];
  v1[3] = 1.0f;
}

void
Vec4To3(Vec3 v1, Vec4 v2)
{
  int i;

  for (i = 0; i < 3; i++)
    v1[i] = v2[i] / v2[3];
}

float Vec3Abs(Vec3 v)
{
  return sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

float
Vec3DiffAbs(Vec3 v1, Vec3 v2)
{
  return sqrtf((v1[0] - v2[0]) * (v1[0] - v2[0]) +
      (v1[1] - v2[1]) * (v1[1] - v2[1]) +
      (v1[2] - v2[2]) * (v1[2] - v2[2]));
}

float
Vec4Abs(Vec4 v)
{
  return sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]) / v[3];
}

float
Vec4DiffAbs(Vec4 v1, Vec4 v2)
{
  return sqrtf(
      (v1[0] / v1[3] - v2[0] / v2[3]) * (v1[0] / v1[3] - v2[0] / v2[3]) +
      (v1[1] / v1[3] - v2[1] / v2[3]) * (v1[1] / v1[3] - v2[1] / v2[3]) +
      (v1[2] / v1[3] - v2[2] / v2[3]) * (v1[2] / v1[3] - v2[2] / v2[3]));
}

void
Vec3Norm(Vec3 v)
{
  float a;
 
  a = Vec3Abs(v);
  v[0] /= a;
  v[1] /= a;
  v[2] /= a;
}

void
Vec4Norm(Vec4 v)
{
  v[3] = Vec3Abs(v);
}

void
Vec3Scale(Vec3 v, float s)
{
  int i;

  for (i = 0; i < 3; i++)
    v[i] *= s;
}

void
Vec3Add(Vec3 v1, Vec3 v2)
{
  int i;

  for (i = 0; i < 3; i++)
    v1[i] += v2[i];
}

void
Vec3ScaleAdd(Vec3 v1, float s, Vec3 v2)
{
  int i;

  for (i = 0; i < 3; i++)
    v1[i] += s * v2[i];
}

void
Vec3Sub(Vec3 v1, Vec3 v2)
{
  int i;

  for (i = 0; i < 3; i++)
    v1[i] -= v2[i];
}

void
Vec3ScaleSub(Vec3 v1, float s, Vec3 v2)
{
  int i;

  for (i = 0; i < 3; i++)
    v1[i] -= s * v2[i];
}

void
Vec4Scale(Vec4 v, float s)
{
  v[3] /= s;
}

void
Vec4Add(Vec4 v1, Vec4 v2)
{
  int i;

  for (i = 0; i < 3; i++)
    v1[i] += v2[i] / v2[3] * v1[3];
}

void
Vec4ScaleAdd(Vec4 v1, float s, Vec4 v2)
{
  int i;

  for (i = 0; i < 3; i++)
    v1[i] += s * v2[i] / v2[3] * v1[3];
}

void
Vec4Sub(Vec4 v1, Vec4 v2)
{
  int i;

  for (i = 0; i < 3; i++)
    v1[i] -= v2[i] / v2[3] * v1[3];
}

void
Vec4ScaleSub(Vec4 v1, float s, Vec4 v2)
{
  int i;

  for (i = 0; i < 3; i++)
    v1[i] -= s * v2[i] / v2[3] * v1[3];
}

float
Vec3Scalar(Vec3 v1, Vec3 v2)
{
  return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

float
Vec4Scalar(Vec4 v1, Vec4 v2)
{
  int i;
  float s;

  s = 0.0f;
  for (i = 0; i < 3; i++)
    s += v1[i] / v1[3] * v2[i] / v2[3];
  
  return s;
}

void
Vec3Cross(Vec3 v1, Vec3 v2)
{
  Vec3 vRes;

  vRes[0] = v1[1] * v2[2] - v1[2] * v2[1];
  vRes[1] = v1[2] * v2[0] - v1[0] * v2[2];
  vRes[2] = v1[0] * v2[1] - v1[1] * v2[0];
  Vec3Copy(v1, vRes);
}

void
Vec4Cross(Vec4 v1, Vec4 v2)
{
  Vec3Cross(v1, v2);
  v1[3] *= v2[3];
}

void
Mat3Ident(Mat3 m)
{
  int i, j;
 
  for (i = 0; i < 3; i++) {
    for (j = 0; j < 3; j++)
      m[i][j] = 0.0f;
    m[i][i] = 1.0f;
  }
}

void
Mat4Ident(Mat4 m)
{
  int i, j;
 
  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++)
      m[i][j] = 0.0f;
    m[i][i] = 1.0f;
  }
}

void
Mat3Copy(Mat3 m1, Mat3 m2)
{
  int i, j;
 
  for (i = 0; i < 3; i++)
    for (j = 0; j < 3; j++)
      m1[i][j] = m2[i][j];
}

void
Mat4Copy(Mat4 m1, Mat4 m2)
{
  int i, j;
 
  for (i = 0; i < 4; i++)
    for (j = 0; j < 4; j++)
      m1[i][j] = m2[i][j];
}

void
Mat3To4(Mat4 m1, Mat3 m2)
{
  int i, j;
 
  for (i = 0; i < 3; i++)
    for (j = 0; j < 3; j++)
      m1[i][j] = m2[i][j];
  for (i = 0; i < 3; i++) {
    m1[i][3] = 0.0f;
    m1[3][i] = 0.0f;
  }
  m1[3][3] = 1.0f;
}

void
Mat3Mult(Mat3 m1, Mat3 m2)
/* m1 = m2 * m1 */
{
  Mat3 mRes;
  int i, j, k;

  for (i = 0; i < 3; i++)
    for (j = 0; j < 3; j++) {
      mRes[i][j] = 0.0f;
      for (k = 0; k < 3; k++)
        mRes[i][j] += m2[i][k] * m1[k][j];
    }
  Mat3Copy(m1, mRes);
}

void
Mat4Mult(Mat4 m1, Mat4 m2)
/* m1 = m2 * m1 */
{
  Mat4 mRes;
  int i, j, k;

  for (i = 0; i < 4; i++)
    for (j = 0; j < 4; j++) {
      mRes[i][j] = 0.0f;
      for (k = 0; k < 4; k++)
        mRes[i][j] += m2[i][k] * m1[k][j];
    }
  Mat4Copy(m1, mRes);
}

void
Mat3VecMult(Vec3 v, Mat3 m)
{
  Vec3 vRes;
 
  /* loop unrolling (efficiency) */
  vRes[0] = v[0] * m[0][0] + v[1] * m[1][0] + v[2] * m[2][0];
  vRes[1] = v[0] * m[0][1] + v[1] * m[1][1] + v[2] * m[2][1];
  vRes[2] = v[0] * m[0][2] + v[1] * m[1][2] + v[2] * m[2][2];
 
  v[0] = vRes[0];
  v[1] = vRes[1];
  v[2] = vRes[2];
}

void
Mat4VecMult(Vec4 v, Mat4 m)
{
  Vec4 vRes;
 
  /* loop unrolling (efficiency) */
  vRes[0] = v[0] * m[0][0] + v[1] * m[1][0] + v[2] * m[2][0] + v[3] * m[3][0];
  vRes[1] = v[0] * m[0][1] + v[1] * m[1][1] + v[2] * m[2][1] + v[3] * m[3][1];
  vRes[2] = v[0] * m[0][2] + v[1] * m[1][2] + v[2] * m[2][2] + v[3] * m[3][2];
  vRes[3] = v[0] * m[0][3] + v[1] * m[1][3] + v[2] * m[2][3] + v[3] * m[3][3];
 
  v[0] = vRes[0];
  v[1] = vRes[1];
  v[2] = vRes[2];
  v[3] = vRes[3];
}

void
Mat3Transp(Mat3 m)
{
  int i, k;
  float t;

  for (i = 0; i < 2; i++)
    for (k = i + 1; k < 3; k++) {
      t = m[i][k];
      m[i][k] = m[k][i];
      m[k][i] = t;
    }
}

void
Mat4Transp(Mat4 m)
{
  int i, k;
  float t;

  for (i = 0; i < 3; i++)
    for (k = i + 1; k < 4; k++) {
      t = m[i][k];
      m[i][k] = m[k][i];
      m[k][i] = t;
    }
}

void
Mat3Ortho(Mat3 m)
{
  int i, k;
  Vec3 vCorr, v;

  Vec3Norm(m[0]);
  for (i = 1; i < 3; i++) {
    vCorr[0] = 0.0f;
    vCorr[1] = 0.0f;
    vCorr[2] = 0.0f;
    for (k = 0; k < i; k++) {
      Vec3Copy(v, m[k]);
      Vec3Scale(v, Vec3Scalar(m[i], m[k]));
      Vec3Add(vCorr, v);
    }
    Vec3Sub(m[i], vCorr);
    Vec3Norm(m[i]);
  }
}

void
Mat4Ortho(Mat4 m)
{
  int i, k;
  Vec3 vCorr, v;

  Vec3Norm(m[0]);
  for (i = 1; i < 3; i++) {
    vCorr[0] = 0.0f;
    vCorr[1] = 0.0f;
    vCorr[2] = 0.0f;
    for (k = 0; k < i; k++) {
      Vec3Copy(v, m[k]);
      Vec3Scale(v, Vec3Scalar(m[i], m[k]));
      Vec3Add(vCorr, v);
    }
    Vec3Sub(m[i], vCorr);
    Vec3Norm(m[i]);
  }

  m[0][3] = 0.0f;
  m[1][3] = 0.0f;
  m[2][3] = 0.0f;
  m[3][3] = 1.0f;
}

void
Mat3Inv(Mat3 m)
{
  float d;
  float m00, m01, m02, m10, m11, m12, m20, m21, m22;

  m00 = m[0][0]; m01 = m[0][1]; m02 = m[0][2];
  m10 = m[1][0]; m11 = m[1][1]; m12 = m[1][2];
  m20 = m[2][0]; m21 = m[2][1]; m22 = m[2][2];

  d = m00 * m11 * m22 +
      m10 * m21 * m02 +
      m20 * m01 * m12 -
      m00 * m21 * m12 -
      m10 * m01 * m22 -
      m20 * m11 * m02;

  m[0][0] = (m11 * m22 - m21 * m12) / d;
  m[1][0] = (m20 * m12 - m10 * m22) / d;
  m[2][0] = (m10 * m21 - m20 * m11) / d;

  m[0][1] = (m21 * m02 - m01 * m22) / d;
  m[1][1] = (m00 * m22 - m20 * m02) / d;
  m[2][1] = (m20 * m01 - m00 * m21) / d;

  m[0][2] = (m01 * m12 - m11 * m02) / d;
  m[1][2] = (m10 * m02 - m00 * m12) / d;
  m[2][2] = (m00 * m11 - m10 * m01) / d;
}

void
Mat3GetAng(Mat3 m, float *xAngP, float *yAngP, float *zAngP)
/* decompose rotation matrix into 3 rotations around axis (x, y, z) */
{
  if (m[0][0] == 0.0f && m[0][1] == 0.0f) {
    *xAngP = atan2f(m[1][0], m[2][0]);
    *yAngP = 0.5f * (float) M_PI;
    *zAngP = 0.0f;
  } else {
    *xAngP = atan2f(m[1][2], m[2][2]);
    *yAngP = asinf(- m[0][2]);
    *zAngP = atan2f(m[0][1], m[0][0]);
  }
}

void
Mat4GetAng(Mat4 m, float *xAngP, float *yAngP, float *zAngP)
/* decompose rotation matrix into 3 rotations around axis (x, y, z) */
{
  if (m[0][0] == 0.0f && m[0][1] == 0.0f) {
    *xAngP = atan2f(m[1][0], m[2][0]);
    *yAngP = 0.5f * (float) M_PI;
    *zAngP = 0.0f;
  } else {
    *xAngP = atan2f(m[1][2], m[2][2]);
    *yAngP = asinf(- m[0][2]);
    *zAngP = atan2f(m[0][1], m[0][0]);
  }
}
