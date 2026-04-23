/*
************************************************************************
*
*   SgView.c - Sg wrappers for viewing functions
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/sgdev/SCCS/s.SgView.c
*   SCCS identification       : 1.11
*
************************************************************************
*/

#include <sg_dev.h>
#include <sg_get.h>
#include <sg_map.h>
#include "sg_view.h"

#include <values.h>
#include <stdio.h>
#include <math.h>

#define DEF_VP_H 2.0f
#define DEF_VP_W (4.0f * DEF_VP_H / 3.0f)

#define MIN_NEAR 0.1f
#define MIN_DIST 0.01f

static BOOL CurrStereo = FALSE;
static SgEye CurrEye = SG_EYE_LEFT;
static SgProjection CurrProj = SG_PROJ_ORTHO;
static float CurrViewP[3] = {0.0f, 0.0f, 1.0f};
static float CurrViewAng = (float) M_PI / 2.0f;
static float CurrNear = -1.0f, CurrFar = 1.0f;

/* viewport needs reasonable default values because it never
   gets set if the program is run with the TTY device, and the
   application may want to get it e. g. for determining the
   aspect ratio for plots */
static float CurrVpX = - 0.5f * DEF_VP_W, CurrVpY = - 0.5f * DEF_VP_H;
static float CurrVpW = DEF_VP_W, CurrVpH = DEF_VP_H;

static BOOL ViewPChanged = TRUE, ViewAngChanged = TRUE;
static float ViewDist, ViewAngTan;

#define STEREO_SUPPORTED (CurrSgDevP->setStereo != NULL)

void
SgSetStereo(BOOL onOff)
{
  if (STEREO_SUPPORTED)
    CurrSgDevP->setStereo(onOff);

  CurrStereo = onOff;
  SgSetEye(CurrEye);
}

BOOL
SgGetStereo(void)
{
  return CurrStereo && STEREO_SUPPORTED;
}

void
SgSetEye(SgEye eye)
{
  if (CurrStereo && STEREO_SUPPORTED)
    CurrSgDevP->setEye(eye);

  CurrEye = eye;
}

SgEye
SgGetEye(void)
{
  return CurrEye;
}

void
SgSetProjection(SgProjection proj)
{
  CurrSgDevP->setProjection(proj);
  CurrProj = proj;
}

void
SgSetViewPoint(float x[3])
{
  CurrSgDevP->setViewPoint(x);
  CurrViewP[0] = x[0];
  CurrViewP[1] = x[1];
  CurrViewP[2] = x[2];

  ViewPChanged = TRUE;
}

void
SgSetViewAngle(float ang)
{
  CurrSgDevP->setViewAngle(ang);
  CurrViewAng = ang;

  ViewAngChanged = TRUE;
}

void
SgSetNearPlane(float d, BOOL local)
{
  if (d < MIN_NEAR)
    d = MIN_NEAR;

  CurrSgDevP->setNearPlane(d, local);
  if (! local)
    CurrNear = d;
}

void
SgSetFarPlane(float d, BOOL local)
{
  if (local)
    return;

  if (d < CurrNear)
    d = CurrNear + MIN_DIST;

  CurrSgDevP->setFarPlane(d, local);
  if (! local)
    CurrFar = d;
}

static void
updateViewDist(void)
{
  if (ViewPChanged) {
    ViewDist = sqrtf(CurrViewP[0] * CurrViewP[0] +
	CurrViewP[1] * CurrViewP[1] +
	CurrViewP[2] * CurrViewP[2]);
    ViewPChanged = FALSE;
  }
}

static void
updateViewAngTan(void)
{
  if (ViewAngChanged) {
    ViewAngTan = tanf(CurrViewAng / 2.0f);
    ViewAngChanged = FALSE;
  }
}

float
SgGetOrthoHeight(void)
{
  /* make sure that projected image has about the same size as
     with perspective transformation */
  updateViewDist();
  updateViewAngTan();

  return ViewDist * ViewAngTan;
}

float
SgGetDepthFact(float z)
{
  updateViewAngTan();

  if (CurrProj == SG_PROJ_ORTHO)
    return CurrViewP[2] * ViewAngTan;
  else
    return (CurrViewP[2] - z) * ViewAngTan;
}

void
SgUpdateView(void)
{
  CurrSgDevP->updateView();
}

void
SgSetView(void)
{
  SgSetStereo(CurrStereo);
  SgSetEye(CurrEye);
  SgSetProjection(CurrProj);
  SgSetViewPoint(CurrViewP);
  SgSetViewAngle(CurrViewAng );
  SgSetNearPlane(CurrNear, FALSE);
  SgSetFarPlane(CurrFar, FALSE);
}

void
SgSetViewport(float x, float y, float w, float h)
{
  CurrSgDevP->setViewport(x, y, w, h);

  CurrVpX = x;
  CurrVpY = y;
  CurrVpW = w;
  CurrVpH = h;
}

void
SgUpdateViewport(void)
{
  CurrSgDevP->setViewport(CurrVpX, CurrVpY, CurrVpW, CurrVpH);
}

void
SgGetViewport(float *xP, float *yP, float *wP, float *hP)
{
  *xP = CurrVpX;
  *yP = CurrVpY;
  *wP = CurrVpW;
  *hP = CurrVpH;
}

void
SgVpMapPoint(float xOut[2], float xIn[2])
{
  xOut[0] = CurrVpX + 0.5f * (xIn[0] * CurrVpH + CurrVpW);
  if (CurrSgDevP->vpHandling == VPH_UPSIDE_DOWN)
    xOut[1] = CurrVpY + 0.5f * (- xIn[1] + 1.0f) * CurrVpH;
  else
    xOut[1] = CurrVpY + 0.5f * (xIn[1] + 1.0f) * CurrVpH;
}

float
SgVpMapSize(float s)
{
  return 0.5f * s * CurrVpH;
}

void
SgConvCoord(int xs, int ys, float *xP, float *yP)
{
  CurrSgDevP->convCoord(xs, ys, xP, yP);
}
