/*
************************************************************************
*
*   DefTransf.c - default transformation handling for Sg
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/def/SCCS/s.DefTransf.c
*   SCCS identification       : 1.11
*
************************************************************************
*/

#include <sg_def.h>

#include <stdio.h>
#include <math.h>

#include <mat_vec.h>
#include <sg.h>
#include <sg_error.h>
#include <sg_map.h>
#include <sg_get.h>

#define STACK_SIZE 24

#define IDENT_MAT { \
  {1.0f, 0.0f, 0.0f, 0.0f}, \
  {0.0f, 1.0f, 0.0f, 0.0f}, \
  {0.0f, 0.0f, 1.0f, 0.0f}, \
  {0.0f, 0.0f, 0.0f, 1.0f} \
}

static Mat4 MatrixStack[STACK_SIZE] = {IDENT_MAT};
static int StackPtr = 0;

static const float EpsDist = (float) 1.0e-6;

static SgProjection Proj;
static float VX, VY, VZ;
static float ViewAng;
static float Near, Far, NearLocal, FarLocal;
static Mat4 ViewT, ModelViewT, ProjT, ViewProjT, ModelViewProjT;
static BOOL ViewUpToDate = FALSE, ProjUpToDate = FALSE, ModelUpToDate = FALSE;
static float NearClip, FarClip;

void
SgDefRotateX(float ang)
{
  Mat4 m = IDENT_MAT;
  float c = cosf(ang);
  float s = sinf(ang);

  m[1][1] = c;
  m[2][2] = c;
  m[1][2] = s;
  m[2][1] = -s;

  Mat4Mult(MatrixStack[StackPtr], m);
  ModelUpToDate = FALSE;
}

void
SgDefRotateY(float ang)
{
  Mat4 m = IDENT_MAT;
  float c = cosf(ang);
  float s = sinf(ang);

  m[0][0] = c;
  m[2][2] = c;
  m[0][2] = s;
  m[2][0] = -s;

  Mat4Mult(MatrixStack[StackPtr], m);
  ModelUpToDate = FALSE;
}

void
SgDefRotateZ(float ang)
{
  Mat4 m = IDENT_MAT;
  float c = cosf(ang);
  float s = sinf(ang);

  m[0][0] = c;
  m[1][1] = c;
  m[0][1] = s;
  m[1][0] = -s;

  Mat4Mult(MatrixStack[StackPtr], m);
  ModelUpToDate = FALSE;
}

void
SgDefTranslate(float dx[3])
{
  Mat4 m = IDENT_MAT;

  m[3][0] = dx[0];
  m[3][1] = dx[1];
  m[3][2] = dx[2];

  Mat4Mult(MatrixStack[StackPtr], m);
  ModelUpToDate = FALSE;
}

void
SgDefScale(float sx, float sy, float sz)
{
  Mat4 m = IDENT_MAT;

  m[0][0] = sx;
  m[1][1] = sy;
  m[2][2] = sz;

  Mat4Mult(MatrixStack[StackPtr], m);
  ModelUpToDate = FALSE;
}

void
SgDefSetMatrix(float m[4][4])
{
  Mat4Copy(MatrixStack[StackPtr], m);
  ModelUpToDate = FALSE;
}

void
SgDefMultMatrix(float m[4][4])
{
  Mat4Mult(MatrixStack[StackPtr], m);
  ModelUpToDate = FALSE;
}

void
SgDefPushMatrix(void)
{
  if (StackPtr >= STACK_SIZE - 1) {
    SgRaiseError(SG_ERR_STACKFULL, "SgDefPushMatrix");
    return;
  }
  StackPtr++;
  Mat4Copy(MatrixStack[StackPtr], MatrixStack[StackPtr - 1]);
  ModelUpToDate = FALSE;
}

void
SgDefPopMatrix(void)
{
  if (StackPtr <= 0) {
    SgRaiseError(SG_ERR_STACKEMPTY, "SgDefPopMatrix");
    return;
  }
  StackPtr--;
  ModelUpToDate = FALSE;
}

static void
updateView(void)
{
  Mat4 m;
  float dx, dy, dz;
  float a, axz;
  float c, s;
  
  dx = - VX;
  dy = - VY;
  dz = - VZ;
  a = sqrtf(dx * dx + dy * dy + dz * dz);
  axz = sqrtf(dx * dx + dz * dz);

  Mat4Ident(ViewT);
  c = axz / a;
  s = - dy / a;
  ViewT[1][1] = c;
  ViewT[2][2] = c;
  ViewT[1][2] = s;
  ViewT[2][1] = -s;

  Mat4Ident(m);
  c = - dz / axz;
  s = dx / axz;
  m[0][0] = c;
  m[2][2] = c;
  m[0][2] = -s;
  m[2][0] = s;
  Mat4Mult(ViewT, m);

  Mat4Ident(m);
  m[3][0] = - VX;
  m[3][1] = - VY;
  m[3][2] = - VZ;
  Mat4Mult(ViewT, m);

  ViewUpToDate = TRUE;
}

static void
updateProj(void)
{
  float b, t;

  Mat4Ident(ProjT);
  if (Proj == SG_PROJ_ORTHO) {
    b = SgGetOrthoHeight();
    ProjT[0][0] = 1.0f / b;
    ProjT[1][1] = 1.0f / b;
    ProjT[2][2] = - 2.0f / (Far - Near);
    ProjT[3][2] = - (Far + Near) / (Far - Near);
  } else {
    t = tanf(ViewAng / 2.0f);
    ProjT[0][0] = 1.0f / t;
    ProjT[1][1] = 1.0f / t;
    ProjT[2][2] = - (Far + Near) / (Far - Near);
    ProjT[2][3] = - 1.0f;
    ProjT[3][2] = - 2.0f * Far * Near / (Far - Near);
    ProjT[3][3] = 0.0f;
  }

  ProjUpToDate = TRUE;
}

void
SgDefSetProjection(SgProjection proj)
{
  Proj = proj;
  ProjUpToDate = FALSE;
}

void
SgDefSetViewPoint(float x[3])
{
  VX = x[0];
  VY = x[1];
  VZ = x[2];
  ViewUpToDate = FALSE;
}

void
SgDefSetViewAngle(float ang)
{
  ViewAng = ang;
  ProjUpToDate = FALSE;
}

void
SgDefSetNearPlane(float d, BOOL local)
{
  NearLocal = d;
  if (! local) {
    Near = d;
    if (Near == Far)
      Far = Near + EpsDist;
    ProjUpToDate = FALSE;
  }
}

void
SgDefSetFarPlane(float d, BOOL local)
{
  FarLocal = d;
  if (! local) {
    Far = d;
    if (Far == Near)
      Near = Far - EpsDist;
    ProjUpToDate = FALSE;
  }
}

static void
updateModel(void)
{
  Mat4Copy(ModelViewT, ViewT);
  Mat4Mult(ModelViewT, MatrixStack[StackPtr]);

  Mat4Copy(ModelViewProjT, ViewProjT);
  Mat4Mult(ModelViewProjT, MatrixStack[StackPtr]);

  ModelUpToDate = TRUE;
}

void
SgDefUpdateView(void)
{
  if (! ViewUpToDate)
    updateView();
  if (! ProjUpToDate)
    updateProj();

  Mat4Copy(ViewProjT, ProjT);
  Mat4Mult(ViewProjT, ViewT);

  Mat4Ident(MatrixStack[StackPtr]);
  updateModel();

  if (Proj == SG_PROJ_ORTHO) {
    NearClip = (2.0f * NearLocal - Near - Far) / (Far - Near);
    FarClip = (2.0f * FarLocal - Near - Far) / (Far - Near);
  } else {
    NearClip = (Far + Near) / (Far - Near) -
	2.0f * Far * Near / ((Far - Near) * NearLocal);
    FarClip = (Far + Near) / (Far - Near) -
	2.0f * Far * Near / ((Far - Near) * FarLocal);
  }
}

void
SgDefSetViewport(float x, float y, float width, float height)
{
}

void
SgMapModelInit(void)
{
  Mat4Ident(MatrixStack[StackPtr]);
  updateModel();
}

void
SgMapModelPoint(float xOut[3], float xIn[3])
{
  float xh[4];

  xh[0] = xIn[0];
  xh[1] = xIn[1];
  xh[2] = xIn[2];
  xh[3] = 1.0f;

  Mat4VecMult(xh, MatrixStack[StackPtr]);

  xOut[0] = xh[0];
  xOut[1] = xh[1];
  xOut[2] = xh[2];
}

void
SgMapModelVect(float xOut[3], float xIn[3])
{
  SgMapModelPoint(xOut, xIn);

  xOut[0] -= MatrixStack[StackPtr][3][0];
  xOut[1] -= MatrixStack[StackPtr][3][1];
  xOut[2] -= MatrixStack[StackPtr][3][2];
}

void
SgMapEyePoint(float xOut[3], float xIn[3])
{
  float xh[4];

  xh[0] = xIn[0];
  xh[1] = xIn[1];
  xh[2] = xIn[2];
  xh[3] = 1.0f;

  if (! ModelUpToDate)
    updateModel();

  Mat4VecMult(xh, ModelViewT);

  xOut[0] = xh[0];
  xOut[1] = xh[1];
  xOut[2] = xh[2];
}

void
SgMapEyeVect(float xOut[3], float xIn[3])
{
  SgMapEyePoint(xOut, xIn);

  xOut[0] -= ModelViewT[3][0];
  xOut[1] -= ModelViewT[3][1];
  xOut[2] -= ModelViewT[3][2];
}

BOOL
SgBackfacing(float x[3], float v[3])
{
  if (Proj == SG_PROJ_ORTHO)
    return v[2] < 0.0f;
  else
    return v[0] * x[0] + v[1] * x[1] + v[2] * x[2] > 0.0f;
}

static void
mapOnePoint(float xOut[3], float xIn[3])
{
  float xh[4];

  xh[0] = xIn[0];
  xh[1] = xIn[1];
  xh[2] = xIn[2];
  xh[3] = 1.0f;

  if (! ModelUpToDate)
    updateModel();

  Mat4VecMult(xh, ModelViewProjT);

  if (xh[3] < 0.0f)
    xh[3] = - xh[3];

  xOut[0] = xh[0] / xh[3];
  xOut[1] = xh[1] / xh[3];
  xOut[2] = xh[2] / xh[3];
}

int
SgMapPoint(float xOut[3], float xIn[3])
{
  mapOnePoint(xOut, xIn);

  if (xOut[2] < NearClip || xOut[2] > FarClip)
    return 0;
  else
    return 1;
}

static void
calcClipPoint(float x0[3], float x1[3], float z)
/* clip line, modify x0 so that x0[2] == z */
{
  float t;

  t = (z - x1[2]) / (x0[2] - x1[2]);
  x0[0] = x1[0] + t * (x0[0] - x1[0]);
  x0[1] = x1[1] + t * (x0[1] - x1[1]);
  x0[2] = z;
}

int
SgMapLine(float x0Out[3], float x1Out[3],
    float x0In[3], float x1In[3])
{
  mapOnePoint(x0Out, x0In);
  mapOnePoint(x1Out, x1In);

  if (x0Out[2] < NearClip) {
    if (x1Out[2] < NearClip)  /* completely outside */
      return 0;
    calcClipPoint(x0Out, x1Out, NearClip);
  } else if (x0Out[2] > FarClip) {
    if (x1Out[2] > FarClip)  /* completely outside */
      return 0;
    calcClipPoint(x0Out, x1Out, FarClip);
  }

  if (x1Out[2] < NearClip) {
    calcClipPoint(x1Out, x0Out, NearClip);
  } else if (x1Out[2] > FarClip) {
    calcClipPoint(x1Out, x0Out, FarClip);
  }

  return 2;
}

static int
addClipPoints(float startV[3], float endV[3], float (*outP)[3],
    float s, float z)
{
  if (s * startV[2] > s * z) {
    if (s * endV[2] > s * z) {
      return 0;
    } else {  /* going inside, take intersection point */
      Vec3Copy(outP[0], startV);
      calcClipPoint(outP[0], endV, z);
      return 1;
    }
  } else {
    Vec3Copy(outP[0], startV);
    if (s * endV[2] > s * z) {  /* going outside, take intersection point */
      Vec3Copy(outP[1], startV);
      calcClipPoint(outP[1], endV, z);
      return 2;
    } else {
      return 1;
    }
  }
}

int
SgMapPoly(float xOut[][3], float xIn[][3], int n)
{
  int indIn, indOut;
  float xt[SG_POLY_SIZE][3];
  float firstV[3], startV[3], endV[3];

  mapOnePoint(firstV, xIn[0]);
  Vec3Copy(endV, firstV);
  indOut = 0;
  for (indIn = 1; indIn < n; indIn++) {
    Vec3Copy(startV, endV);
    mapOnePoint(endV, xIn[indIn]);
    indOut += addClipPoints(startV, endV, xt + indOut,
	-1.0f, NearClip);
    if (indOut >= SG_POLY_SIZE - 1)
      break;
  }
  if (indOut < SG_POLY_SIZE - 1)
    indOut += addClipPoints(endV, firstV, xt + indOut,
	-1.0f, NearClip);

  n = indOut;
  if (n == 0)
    return 0;

  indOut = 0;
  for (indIn = 1; indIn < n; indIn++) {
    indOut += addClipPoints(xt[indIn - 1], xt[indIn], xOut + indOut,
	1.0f, FarClip);
    if (indOut >= SG_POLY_SIZE - 1)
      break;
  }
  if (indOut < SG_POLY_SIZE - 1)
    indOut += addClipPoints(xt[n - 1], xt[0], xOut + indOut,
	1.0f, FarClip);

  return indOut;
}

static void
getTransfSize(float *sy, float *sz, float s, float z)
{
  float z0;

  /* multiply size with scaling factor */
  s *= Vec3Abs(MatrixStack[StackPtr][0]);

  if (Proj == SG_PROJ_ORTHO) {
    *sy = s * ProjT[1][1];
    if (sz != NULL)
      *sz = - s * ProjT[2][2];
  } else {
    /* calculate back z value before perspective transformation */
    z0 = ProjT[3][2] / (ProjT[2][2] + z);

    *sy = s * ProjT[1][1] / z0;
    if (sz != NULL)
      *sz = z - (ProjT[3][2] / (z0 - s) - ProjT[2][2]);
  }
}

int
SgMapSphere(float xOut[3], float *rYOutP, float *rZOutP,
    float xIn[3], float rIn)
{
  mapOnePoint(xOut, xIn);

  if (xOut[2] < NearClip || xOut[2] > FarClip)
    return 0;

  getTransfSize(rYOutP, rZOutP, rIn, xOut[2]);

  return 1;
}

int
SgMapCone(float xOut[4][3], float *r0OutP, float *r1OutP,
    float x0In[3], float x1In[3], float r0, float r1)
{
  float x0Out[3], x1Out[3];
  float v[2], a;

  if (SgMapLine(x0Out, x1Out, x0In, x1In) == 0)
    return 0;
  
  getTransfSize(&r0, r0OutP, r0, x0Out[2]);
  getTransfSize(&r1, r1OutP, r1, x1Out[2]);

  v[0] = x1Out[0] - x0Out[0];
  v[1] = x1Out[1] - x0Out[1];
  a = sqrtf(v[0] * v[0] + v[1] * v[1]);
  v[0] /= a;
  v[1] /= a;

  xOut[0][0] = x0Out[0] - r0 * v[1];
  xOut[0][1] = x0Out[1] + r0 * v[0];
  xOut[0][2] = x0Out[2];
  xOut[1][0] = x0Out[0] + r0 * v[1];
  xOut[1][1] = x0Out[1] - r0 * v[0];
  xOut[1][2] = x0Out[2];
  xOut[2][0] = x1Out[0] + r1 * v[1];
  xOut[2][1] = x1Out[1] - r1 * v[0];
  xOut[2][2] = x1Out[2];
  xOut[3][0] = x1Out[0] - r1 * v[1];
  xOut[3][1] = x1Out[1] + r1 * v[0];
  xOut[3][2] = x1Out[2];

  return 4;
}

void
SgDefConvCoord(int xs, int ys, float *xP, float *yP)
{
  float vx, vy, vw, vh;

  SgGetViewport(&vx, &vy, &vw, &vh);

  *xP = (2 * (xs - vx) - vw) / vh;
  *yP = 2 * (ys - vy) / vh - 1.0f;
}
