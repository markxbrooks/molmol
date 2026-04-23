/*
************************************************************************
*
*   POVView.c - POV viewing parameters
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/pov/SCCS/s.POVView.c
*   SCCS identification       : 1.16
*
************************************************************************
*/

#include "pov_view.h"
#include "pov_clip.h"

#include <math.h>

#include <mat_vec.h>
#include <sg.h>
#include <sg_map.h>
#include <sg_plot.h>
#include <sg_get.h>
#include "pov_util.h"

static BOOL ViewChanged, ClipChanged;
static SgProjection Proj;
static float ViewPoint[3], ViewNorm[3], ViewDist;
static float ViewAng;
static float Near, Far;

void
POVInitView(void)
{
  ViewChanged = TRUE;
  ClipChanged = TRUE;
}

static void
plotCamera(void)
{
  float x, y, w, h;
  float ow, oh;

  SgGetViewport(&x, &y, &w, &h);

  SgPlotOutputStr("camera {\n");

  if (Proj == SG_PROJ_ORTHO)
    SgPlotOutputStr("orthographic\n");

  SgPlotOutputStr("location ");
  POVPlotVect(ViewPoint);
  SgPlotOutputStr("\n");

  if (Proj == SG_PROJ_ORTHO) {
    SgPlotOutputStr("direction <0, 0, 1>\n");

    oh = 2.0f * SgGetOrthoHeight();
    ow = oh * w / h;

    if (SgPlotGetOrient() == SG_PLOT_ROT90) {
      SgPlotOutputStr("up <0, ");
      SgPlotOutputFloat(ow);
      SgPlotOutputStr(", 0>\n");
      SgPlotOutputStr("right <");
      SgPlotOutputFloat(- oh);
      SgPlotOutputStr(", 0, 0>\n");
      SgPlotOutputStr("sky <1, 0, 0>\n");
    } else {
      SgPlotOutputStr("up <0, ");
      SgPlotOutputFloat(oh);
      SgPlotOutputStr(", 0>\n");
      SgPlotOutputStr("right <");
      SgPlotOutputFloat(- ow);
      SgPlotOutputStr(", 0, 0>\n");
    }
  } else {
    SgPlotOutputStr("direction <0, 0, ");
    SgPlotOutputFloat(0.5f / tanf(0.5f * ViewAng));
    SgPlotOutputStr(">\n");

    if (SgPlotGetOrient() == SG_PLOT_ROT90) {
      SgPlotOutputStr("up <0, ");
      SgPlotOutputFloat(w / h);
      SgPlotOutputStr(", 0>\n");
      SgPlotOutputStr("right <-1, 0, 0>\n");
      SgPlotOutputStr("sky <1, 0, 0>\n");
    } else {
      SgPlotOutputStr("right <");
      SgPlotOutputFloat(- w / h);
      SgPlotOutputStr(", 0, 0>\n");
    }
  }

  SgPlotOutputStr("look_at <0, 0, 0>\n");

  SgPlotOutputStr("}\n");
}

static void
plotBackground(void)
{
  float x, y, w, h;
  float r, g, b;
  Vec3 discV;

  SgGetViewport(&x, &y, &w, &h);
  SgGetBackgroundColor(&r, &g, &b);

  if (r != 0.0f || g != 0.0f || b != 0.0f) {
    SgPlotOutputStr("disc {");
    Vec3Copy(discV, ViewNorm);
    Vec3Scale(discV, ViewDist - Far);
    POVPlotVect(discV);
    SgPlotOutputStr(", ");
    POVPlotVect(ViewNorm);
    SgPlotOutputStr(" ");
    SgPlotOutputFloat(
	1.01f * Far * tanf(0.5f * ViewAng) * sqrtf(1.0f + w * w / (h * h)));
    SgPlotOutputStr("\n");
    SgPlotOutputStr("pigment {color rgb <");
    SgPlotOutputFloat(r);
    SgPlotOutputStr(", ");
    SgPlotOutputFloat(g);
    SgPlotOutputStr(", ");
    SgPlotOutputFloat(b);
    SgPlotOutputStr(">}\n");
    SgPlotOutputStr("finish {ambient 1.0 diffuse 0.0}\n");
    SgPlotOutputStr("}\n");
  }
}

static void
plotClip(void)
{
  SgPlotOutputStr("#declare C = difference {\n");
  SgPlotOutputStr("plane {");
  POVPlotVect(ViewNorm);
  SgPlotOutputStr(", ");
  SgPlotOutputFloat(ViewDist - Near);
  SgPlotOutputStr("}\n");
  SgPlotOutputStr("plane {");
  POVPlotVect(ViewNorm);
  SgPlotOutputStr(", ");
  SgPlotOutputFloat(ViewDist - Far);
  SgPlotOutputStr("}\n");
  SgPlotOutputStr("}\n");
}

void
SgPOVUpdateView(void)
{
  if (ViewChanged) {
    plotCamera();
    plotBackground();
    ViewChanged = FALSE;
  }

  SgMapModelInit();

  if (ClipChanged) {
    plotClip();
    ClipChanged = FALSE;
  }
}

void
SgPOVSetProjection(SgProjection proj)
{
  if (proj == Proj)
    return;

  Proj = proj;
  ViewChanged = TRUE;
}

void
SgPOVSetViewPoint(float x[3])
{
  if (x[0] == ViewPoint[0] && x[1] == ViewPoint[1] && x[2] == ViewPoint[2])
    return;

  Vec3Copy(ViewPoint, x);
  ViewDist = Vec3Abs(x);
  Vec3Copy(ViewNorm, x);
  Vec3Scale(ViewNorm, 1.0f / ViewDist);

  ViewChanged = TRUE;
}

void
SgPOVSetViewAngle(float ang)
{
  if (ang == ViewAng)
    return;

  ViewAng = ang;
  ViewChanged = TRUE;
}

void
SgPOVSetNearPlane(float d, BOOL local)
{
  if (d == Near)
    return;

  Near = d;
  ClipChanged = TRUE;
}

void
SgPOVSetFarPlane(float d, BOOL local)
{
  if (d == Far)
    return;

  Far = d;
  ClipChanged = TRUE;
}

BOOL
POVClipTest(float x[3], float r)
{
  float d;

  d = Vec3Scalar(x, ViewNorm);
  return d + r < ViewDist - Near && d - r > ViewDist - Far;
}

void
POVClipOutput(void)
{
  SgPlotOutputStr("clipped_by {C}\n");
}

BOOL
POVClip(float x[3], float r)
{
  if (POVClipTest(x, r)) {
    return TRUE;
  } else {
    POVClipOutput();
    return FALSE;
  }
}
