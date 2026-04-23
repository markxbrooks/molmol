/*
************************************************************************
*
*   POVShape.c - POV discs, spheres and cones
*
*   Copyright (c) 1994-96
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/pov/SCCS/s.POVShape.c
*   SCCS identification       : 1.9
*
************************************************************************
*/

#include "pov_shape.h"

#include <math.h>

#include <mat_vec.h>
#include <sg_map.h>
#include <sg_plot.h>
#include "pov_util.h"
#include "pov_color.h"
#include "pov_clip.h"

void
SgPOVDrawDisc2D(float x[2], float r)
{
  float xm[3];
  float x2[2], xm2[3];

  POVUpdateColor();

  x2[0] = x[0] + r;
  x2[1] = x[1];

  POVMap2D(xm, x);
  POVMap2D(xm2, x2);

  SgPlotOutputStr("disc {\n");
  POVPlotVect(xm);
  SgPlotOutputStr(", z, ");
  SgPlotOutputFloat(xm2[0] - xm[0]);
  SgPlotOutputStr("\n");
  POVPlotColor();
  SgPlotOutputStr("}\n");
}

void
SgPOVDrawDisc(float x[3], float v[3], float r)
{
  float xm[3], vm[3];

  POVUpdateColor();

  SgMapModelPoint(xm, x);
  SgMapModelVect(vm, v);

  SgPlotOutputStr("disc {\n");
  POVPlotVect(xm);
  SgPlotOutputStr(", ");
  POVPlotVect(vm);
  SgPlotOutputStr(", ");
  SgPlotOutputFloat(r);
  SgPlotOutputStr("\n");
  POVPlotColor();
  (void) POVClip(xm, r);
  SgPlotOutputStr("}\n");
}

static void
drawSphere(float x[3], float r)
{
  SgPlotOutputStr("sphere {\n");
  POVPlotVect(x);
  SgPlotOutputStr(", ");
  SgPlotOutputFloat(r);
  SgPlotOutputStr("\n");
  POVPlotColor();
  (void) POVClip(x, r);
  SgPlotOutputStr("}\n");
}

void
SgPOVDrawSphere(float x[3], float r)
{
  float xm[3], v[3], vm[3];

  POVUpdateColor();

  SgMapModelPoint(xm, x);

  v[0] = r;
  v[1] = 0.0f;
  v[2] = 0.0f;
  SgMapModelVect(vm, v);
  r = Vec3Abs(vm);

  drawSphere(xm, r);
}

void
SgPOVDrawCone(float x0[3], float x1[3], float r0, float r1,
    SgConeEnd end0, SgConeEnd end1)
{
  float xm0[3], xm1[3], v[3], vm[3], x[3];
  float c, s, a;
  BOOL isCylinder;

  POVUpdateColor();

  SgMapModelPoint(xm0, x0);
  SgMapModelPoint(xm1, x1);

  isCylinder = (r0 == r1);

  v[0] = r0;
  v[1] = 0.0f;
  v[2] = 0.0f;
  SgMapModelVect(vm, v);
  r0 = Vec3Abs(vm);

  if (isCylinder) {
    r1 = r0;
  } else {
    v[0] = r1;
    SgMapModelVect(vm, v);
    r1 = Vec3Abs(vm);
  }

  if (isCylinder) {
    SgPlotOutputStr("cylinder {\n");
    POVPlotVect(xm0);
    SgPlotOutputStr(", ");
    POVPlotVect(xm1);
    SgPlotOutputStr(", ");
    SgPlotOutputFloat(r0);
    SgPlotOutputStr("\n");
  } else {
    Vec3Copy(v, xm1);
    Vec3Sub(v, xm0);
    a = Vec3Abs(v);
    Vec3Scale(v, 1.0f / a);

    c = (r0 - r1) / a;
    if (c < -1.0f || c > 1.0f)  /* degenerate */
      c = 0.0f;
    s = sqrtf(1.0f - c * c);

    SgPlotOutputStr("cone {\n");
    if (end0 == SG_CONE_CAP_FLAT) {
      POVPlotVect(xm0);
      SgPlotOutputStr(", ");
      SgPlotOutputFloat(r0);
    } else {
      Vec3Copy(x, xm0);
      Vec3ScaleAdd(x, c * r0, v);
      POVPlotVect(x);
      SgPlotOutputStr(", ");
      SgPlotOutputFloat(s * r0);
    }
    SgPlotOutputStr("\n");

    if (end1 == SG_CONE_CAP_FLAT) {
      POVPlotVect(xm1);
      SgPlotOutputStr(", ");
      SgPlotOutputFloat(r1);
    } else {
      Vec3Copy(x, xm1);
      Vec3ScaleAdd(x, c * r1, v);
      POVPlotVect(x);
      SgPlotOutputStr(", ");
      SgPlotOutputFloat(s * r1);
    }
    SgPlotOutputStr("\n");
  }

  if (end0 == SG_CONE_OPEN && end1 == SG_CONE_OPEN)
    SgPlotOutputStr("open\n");

  POVPlotColor();
  (void) (POVClip(xm0, r0) && POVClip(xm1, r1));;
  SgPlotOutputStr("}\n");

  if (end0 == SG_CONE_CAP_ROUND)
    drawSphere(xm0, r0);

  if (end1 == SG_CONE_CAP_ROUND)
    drawSphere(xm1, r1);
}
