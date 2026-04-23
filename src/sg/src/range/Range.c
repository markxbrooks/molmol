/*
************************************************************************
*
*   Range.c - range determination
*
*   Copyright (c) 1994-99
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/range/SCCS/s.Range.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include "range.h"

#include <values.h>
#include <math.h>

#include <mat_vec.h>
#include <sg_map.h>
#include <sg_range.h>

SgRes
SgRangeInit(int *argcP, char *argv[])
{
  return SG_RES_OK;
}

SgRes
SgRangeCleanup(void)
{
  return SG_RES_OK;
}

void
SgRangeDrawMarkers2D(float x[][2], int n)
{
  int i;

  for (i = 0; i < n; i++)
    SgRangePoint(x[i][0], x[i][1]);
}

void
SgRangeDrawMarkers(float x[][3], int n)
{
  float xm[3];
  int i;

  for (i = 0; i < n; i++) {
    (void) SgMapPoint(xm, x[i]);
    SgRangePoint(xm[0], xm[1]);
  }
}

void
SgRangeDrawLine2D(float x0[2], float x1[2])
{
  SgRangePoint(x0[0], x0[1]);
  SgRangePoint(x1[0], x1[1]);
}

void
SgRangeDrawLine(float x0[3], float x1[3])
{
  float x0m[3], x1m[3];

  (void) SgMapPoint(x0m, x0);
  (void) SgMapPoint(x1m, x1);

  SgRangePoint(x0m[0], x0m[1]);
  SgRangePoint(x1m[0], x1m[1]);
}

void
SgRangeDrawPolyline2D(float x[][2], int n)
{
  int i;

  for (i = 0; i < n - 1; i++)
    SgRangePoint(x[i][0], x[i][1]);
}

void
SgRangeDrawPolyline(float x[][3], int n)
{
  float xm[3];
  int i;

  for (i = 0; i < n; i++) {
    (void) SgMapPoint(xm, x[i]);
    SgRangePoint(xm[0], xm[1]);
  }
}

void
SgRangeDrawPolygon2D(float x[][2], int n)
{
  int i;

  for (i = 0; i < n - 1; i++)
    SgRangePoint(x[i][0], x[i][1]);
}

void
SgRangeDrawPolygon(float x[][3], int n, float nv[3])
{
  float xm[3];
  int i;

  for (i = 0; i < n; i++) {
    (void) SgMapPoint(xm, x[i]);
    SgRangePoint(xm[0], xm[1]);
  }
}

void
SgRangeDrawCircle2D(float x[2], float r)
{
  SgRangeRect(x[0] - r, x[0] + r, x[1] - r, x[1] + r);
}

void
SgRangeDrawDisc2D(float x[2], float r)
{
  SgRangeRect(x[0] - r, x[0] + r, x[1] - r, x[1] + r);
}

void
SgRangeDrawAnnot2D(float x[2], char *str)
{
  SgRangePoint(x[0], x[1]);
}

void
SgRangeDrawText2D(float x[2], char *str)
{
  SgRangePoint(x[0], x[1]);
}

void
SgRangeDrawAnnot(float x[3], char *str)
{
  float xm[3];

  (void) SgMapPoint(xm, x);
  SgRangePoint(xm[0], xm[1]);
}

void
SgRangeDrawText(float x[3], char *str)
{
  float xm[3];

  (void) SgMapPoint(xm, x);
  SgRangePoint(xm[0], xm[1]);
}

void
SgRangeDrawSphere(float x[3], float r)
{
  float xm[3], ry, rz;

  ry = 0.0f;
  (void) SgMapSphere(xm, &ry, &rz, x, r);
  SgRangeRect(xm[0] - ry, xm[0] + ry, xm[1] - ry, xm[1] + ry);
}

void
SgRangeDrawCone(float x0[3], float x1[3], float r0, float r1,
    SgConeEnd end0, SgConeEnd end1)
{
  SgRangeDrawSphere(x0, r0);
  SgRangeDrawSphere(x1, r1);
}
