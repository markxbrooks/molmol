/*
************************************************************************
*
*   RIBShape.c - RIB discs, spheres and cones
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/rib/SCCS/s.RIBShape.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include "rib_shape.h"

#include <math.h>
#include <values.h>

#include <mat_vec.h>
#include <sg_map.h>
#include <sg_plot.h>
#include "rib_util.h"
#include "rib_color.h"

void
SgRIBDrawDisc2D(float x[2], float r)
{
  float xm[3];
  float x2[2], xm2[3];

  RIBUpdateMaterial();

  x2[0] = x[0] + r;
  x2[1] = x[1];

  RIBMap2D(xm, x);
  RIBMap2D(xm2, x2);
}

void
SgRIBDrawDisc(float x[3], float v[3], float r)
{
  float xm[3], vm[3];

  RIBUpdateMaterial();

  SgMapModelPoint(xm, x);
  SgMapModelVect(vm, v);
}

static void
drawSphere(float x[3], float r)
{
  SgPlotOutputStr("Translate ");
  RIBPlotVect(x);
  SgPlotOutputStr("\n");
  SgPlotOutputStr("Sphere ");
  SgPlotOutputFloat(r);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(-r);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(r);
  SgPlotOutputStr(" 360\n");
  SgPlotOutputStr("Identity\n");
}

void
SgRIBDrawSphere(float x[3], float r)
{
  float xm[3], v[3], vm[3];

  RIBUpdateMaterial();

  SgMapModelPoint(xm, x);

  v[0] = r;
  v[1] = 0.0f;
  v[2] = 0.0f;
  SgMapModelVect(vm, v);
  r = Vec3Abs(vm);

  drawSphere(xm, r);
}

void
SgRIBDrawCone(float x0[3], float x1[3], float r0, float r1,
    SgConeEnd end0, SgConeEnd end1)
{
  float xm0[3], xm1[3], v[3], vm[3];
  float r, h, ang;

  RIBUpdateMaterial();

  /* only support for cylinders */
  r = 0.5f * (r0 + r1);

  SgMapModelPoint(xm0, x0);
  SgMapModelPoint(xm1, x1);

  v[0] = r;
  v[1] = 0.0f;
  v[2] = 0.0f;
  SgMapModelVect(vm, v);
  r = Vec3Abs(vm);

  Vec3Copy(vm, xm1);
  Vec3Sub(vm, xm0);

  h = Vec3Abs(vm);
  Vec3Scale(vm, 1.0f / h);

  SgPlotOutputStr("Translate ");
  RIBPlotVect(xm0);
  SgPlotOutputStr("\n");

  SgPlotOutputStr("Rotate ");
  ang = atan2f(sqrtf(vm[0] * vm[0] + vm[1] * vm[1]), vm[2]);
  SgPlotOutputFloat(ang * 180.0f / (float) M_PI);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(- vm[1]);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(vm[0]);
  SgPlotOutputStr(" 0\n");

  SgPlotOutputStr("Cylinder ");
  SgPlotOutputFloat(r);
  SgPlotOutputStr(" 0.0 ");
  SgPlotOutputFloat(h);
  SgPlotOutputStr(" 360\n");

  SgPlotOutputStr("Identity\n");

  if (end0 == SG_CONE_CAP_ROUND)
    drawSphere(xm0, r);

  if (end1 == SG_CONE_CAP_ROUND)
    drawSphere(xm1, r);
}
