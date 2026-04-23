/*
************************************************************************
*
*   VRML2Shape.c - VRML2 spheres and cones
*
*   Copyright (c) 1996-99
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/vrml2/SCCS/s.VRML2Shape.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include "vrml2_shape.h"

#include <math.h>

#include <mat_vec.h>
#include <sg_map.h>
#include <sg_plot.h>
#include "vrml2_util.h"
#include "vrml2_color.h"
#include "vrml2_light.h"

static void
drawSphere(float x[3], float r)
{
  SgPlotOutputStr("Transform {\n");
  SgPlotOutputStr("translation ");
  VRML2PlotVect(x);
  SgPlotOutputStr("\n");
  SgPlotOutputStr("children [\n");
  SgPlotOutputStr("Shape {\n");

  VRML2PlotColor();

  SgPlotOutputStr("geometry Sphere {");
  SgPlotOutputStr("radius ");
  SgPlotOutputFloat(r);
  SgPlotOutputStr("}\n");

  SgPlotOutputStr("}\n");
  SgPlotOutputStr("]\n");
  SgPlotOutputStr("}\n");
}

void
SgVRML2DrawSphere(float x[3], float r)
{
  float xm[3], v[3], vm[3];

  VRML2UpdateLight();

  SgMapModelPoint(xm, x);

  v[0] = r;
  v[1] = 0.0f;
  v[2] = 0.0f;
  SgMapModelVect(vm, v);
  r = Vec3Abs(vm);

  drawSphere(xm, r);
}

void
SgVRML2DrawCone(float x0[3], float x1[3], float r0, float r1,
    SgConeEnd end0, SgConeEnd end1)
{
  float xm0[3], xm1[3], xm2[3], v[3], vm[3];
  float r, h;

  VRML2UpdateLight();

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

  Vec3Copy(xm2, xm0);
  Vec3ScaleAdd(xm2, 0.5f, vm);

  h = Vec3Abs(vm);
  Vec3Scale(vm, 1.0f / h);

  SgPlotOutputStr("Transform {\n");
  SgPlotOutputStr("translation ");
  VRML2PlotVect(xm2);
  SgPlotOutputStr("\n");
  SgPlotOutputStr("rotation ");
  SgPlotOutputFloat(vm[2]);
  SgPlotOutputStr(" 0 ");
  SgPlotOutputFloat(- vm[0]);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(atan2f(sqrtf(vm[0] * vm[0] + vm[2] * vm[2]), vm[1]));
  SgPlotOutputStr("\n");

  SgPlotOutputStr("children [\n");
  SgPlotOutputStr("Shape {\n");

  VRML2PlotColor();

  SgPlotOutputStr("geometry Cylinder {\n");
  if (end0 != SG_CONE_CAP_FLAT)
    SgPlotOutputStr("bottom FALSE\n");
  if (end1 != SG_CONE_CAP_FLAT)
    SgPlotOutputStr("top FALSE\n");
  SgPlotOutputStr("radius ");
  SgPlotOutputFloat(r);
  SgPlotOutputStr("\n");
  SgPlotOutputStr("height ");
  SgPlotOutputFloat(h);
  SgPlotOutputStr("\n");
  SgPlotOutputStr("}\n");

  SgPlotOutputStr("}\n");
  SgPlotOutputStr("]\n");
  SgPlotOutputStr("}\n");

  if (end0 == SG_CONE_CAP_ROUND)
    drawSphere(xm0, r);

  if (end1 == SG_CONE_CAP_ROUND)
    drawSphere(xm1, r);
}
