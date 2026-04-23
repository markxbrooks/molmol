/*
************************************************************************
*
*   RIBView.c - RIB viewing parameters
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/rib/SCCS/s.RIBView.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include "rib_view.h"

#include <stdio.h>
#include <math.h>
#include <values.h>

#include <mat_vec.h>
#include <sg.h>
#include <sg_map.h>
#include <sg_plot.h>
#include <sg_get.h>
#include "rib_world.h"

static BOOL ViewChanged;
static SgProjection Proj;
static float ViewPoint[3];
static float ViewAng;
static float Near, Far;

void
RIBInitView(void)
{
  ViewChanged = TRUE;
}

static void
plotCamera(void)
{
  float x, y, w, h;
  float aspRat;
  float vx, vz, va;
  float oh;

  SgGetViewport(&x, &y, &w, &h);
  if (SgPlotGetOrient() == SG_PLOT_ROT90)
    aspRat = h / w;
  else
    aspRat = w / h;

  SgPlotOutputStr("FrameAspectRatio ");
  SgPlotOutputFloat(aspRat);
  SgPlotOutputStr("\n");

  SgPlotOutputStr("Clipping ");
  SgPlotOutputFloat(Near);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(Far);
  SgPlotOutputStr("\n");

  vx = ViewPoint[0];
  vz = ViewPoint[2];
  SgPlotOutputStr("Rotate ");
  va = sqrtf(vx * vx + vz * vz);
  vx /= va;
  vz /= va;
  SgPlotOutputFloat(atan2f(vx, vz) * 180.0f / (float) M_PI);
  SgPlotOutputStr(" 0.0 1.0 0.0\n");

  SgPlotOutputStr("Scale 1.0 1.0 -1.0\n");

  SgPlotOutputStr("Translate ");
  SgPlotOutputFloat(- ViewPoint[0]);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(- ViewPoint[1]);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(- ViewPoint[2]);
  SgPlotOutputStr("\n");

  if (SgPlotGetOrient() == SG_PLOT_ROT90)
    SgPlotOutputStr("Rotate 90.0 0.0 0.0 1.0\n");

  if (Proj == SG_PROJ_ORTHO) {
    oh = SgGetOrthoHeight();
    SgPlotOutputStr("ScreenWindow ");
    if (SgPlotGetOrient() == SG_PLOT_ROT90) {
      SgPlotOutputFloat(- oh);
      SgPlotOutputStr(" ");
      SgPlotOutputFloat(oh);
      SgPlotOutputStr(" ");
      SgPlotOutputFloat(- oh / aspRat);
      SgPlotOutputStr(" ");
      SgPlotOutputFloat(oh / aspRat);
    } else {
      SgPlotOutputFloat(- aspRat * oh);
      SgPlotOutputStr(" ");
      SgPlotOutputFloat(aspRat * oh);
      SgPlotOutputStr(" ");
      SgPlotOutputFloat(- oh);
      SgPlotOutputStr(" ");
      SgPlotOutputFloat(oh);
    }
    SgPlotOutputStr("\n");
    SgPlotOutputStr("Projection \"orthographic\"\n");
  } else {
    SgPlotOutputStr("ScreenWindow ");
    if (SgPlotGetOrient() == SG_PLOT_ROT90) {
      SgPlotOutputStr("-1.0 1.0");
      SgPlotOutputFloat(- 1.0f / aspRat);
      SgPlotOutputStr(" ");
      SgPlotOutputFloat(1.0f / aspRat);
      SgPlotOutputStr("\n");
    } else {
      SgPlotOutputFloat(- aspRat);
      SgPlotOutputStr(" ");
      SgPlotOutputFloat(aspRat);
      SgPlotOutputStr("-1.0 1.0\n");
    }
    SgPlotOutputStr("Projection \"perspective\" \"fov\" ");
    SgPlotOutputFloat(ViewAng * 180.0f / (float) M_PI);
    SgPlotOutputStr("\n");
  }
}

void
SgRIBUpdateView(void)
{
  if (ViewChanged) {
    plotCamera();
    RIBWorldStart();
    ViewChanged = FALSE;
  }

  SgMapModelInit();
}

void
SgRIBSetProjection(SgProjection proj)
{
  if (proj == Proj)
    return;

  Proj = proj;
  ViewChanged = TRUE;
}

void
SgRIBSetViewPoint(float x[3])
{
  if (x[0] == ViewPoint[0] && x[1] == ViewPoint[1] && x[2] == ViewPoint[2])
    return;

  Vec3Copy(ViewPoint, x);
  ViewChanged = TRUE;
}

void
SgRIBSetViewAngle(float ang)
{
  if (ang == ViewAng)
    return;

  ViewAng = ang;
  ViewChanged = TRUE;
}

void
SgRIBSetNearPlane(float d, BOOL local)
{
  if (d == Near)
    return;

  Near = d;
}

void
SgRIBSetFarPlane(float d, BOOL local)
{
  if (d == Far)
    return;

  Far = d;
}

extern void RIBGetClipPlanes(float *nearP, float *farP)
{
  *nearP = Near;
  *farP = Far;
}
