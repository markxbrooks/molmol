/*
************************************************************************
*
*   GLView.c - GL viewing parameters
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
*   Date of last modification : 01/06/02
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/gl/SCCS/s.GLView.c
*   SCCS identification       : 1.8
*
************************************************************************
*/

#include "gl_view.h"

#include <math.h>
#include <gl/gl.h>
#include <gl/get.h>

#include <sg_get.h>
#include "gl_util.h"

/* there seems to be no clean way to get these values */
#define YMAXSTEREO 492
#define YOFFSET 532

static BOOL StereoSwitch = FALSE, StereoState = FALSE;
static long MonoMonitor;

static SgProjection Proj;
static Coord VX, VY, VZ;
static Angle ViewAng;
static Coord Near, Far, NearGlob, FarGlob;
static float VpX, VpY, VpW, VpH;
static float AspRat;

static Matrix IdentM = {
  {1.0f, 0.0f, 0.0f, 0.0f},
  {0.0f, 1.0f, 0.0f, 0.0f},
  {0.0f, 0.0f, 1.0f, 0.0f},
  {0.0f, 0.0f, 0.0f, 1.0f}
};

static Matrix ProjM;

void
SgGLSetStereo(BOOL onOff)
{
  if (onOff == StereoSwitch)
    return;

  if (onOff && getgdesc(GD_STEREO)) {
    MonoMonitor = getmonitor();
    setmonitor(STR_RECT);
    StereoState = TRUE;
  } else {
    setmonitor((short) MonoMonitor);
    StereoState = FALSE;
  }

  StereoSwitch = onOff;
}

void
SgGLSetEye(SgEye eye)
{
  Screencoord xMin, xMax;

  xMin = (Screencoord) (VpX + 0.5f);
  xMax = (Screencoord) (VpX + VpW + 0.5f);

  if (eye == SG_EYE_LEFT)
    viewport(xMin, xMax, YOFFSET, YOFFSET + YMAXSTEREO);
  else
    viewport(xMin, xMax, 0, YMAXSTEREO);
}

void
SgGLConvCoord(int xs, int ys, float *xP, float *yP)
{
  *xP = (2 * (xs - VpX) - VpW) / VpH;
  if (StereoState) {
    if (ys >= 0.5f * VpH)
      *yP = 4 * (ys - VpY) / VpH - 3.0f;
    else
      *yP = 4 * (ys - VpY) / VpH - 1.0f;
  } else {
    *yP = 2 * (ys - VpY) / VpH - 1.0f;
  }
}

void
SgGLUpdateView(void)
{
  float b;
  long zMin, zMax;
  float zNear, zFar;

  mmode(MPROJECTION);
  if (Proj == SG_PROJ_ORTHO) {
    b = SgGetOrthoHeight();
    ortho(- b * AspRat, b * AspRat, - b, b, Near, Far);
  } else {
    perspective(ViewAng, AspRat, Near, Far);
  }
  mmode(MVIEWING);

  loadmatrix(IdentM);
  lookat(VX, VY, VZ, 0.0f, 0.0f, 0.0f, 0.0f);

  zMin = getgconfig(GC_ZMIN);
  zMax = getgconfig(GC_ZMAX);
  if (Proj == SG_PROJ_ORTHO) {
    zNear = (Near - NearGlob) / (FarGlob - NearGlob);
    zFar = (Far - NearGlob) / (FarGlob - NearGlob);
  } else {
    zNear = FarGlob / (FarGlob - NearGlob) -
        FarGlob * NearGlob / ((FarGlob - NearGlob) * Near);
    zFar = FarGlob / (FarGlob - NearGlob) -
        FarGlob * NearGlob / ((FarGlob - NearGlob) * Far);
  }
  lsetdepth(zMin + (long) (zNear * (float) (zMax - zMin)),
      zMin + (long) (zFar * (float) (zMax - zMin)));
}

void
SgGLSetProjection(SgProjection proj)
{
  Proj = proj;
}

void
SgGLSetViewPoint(float x[3])
{
  VX = x[0];
  VY = x[1];
  VZ = x[2];
}

void
SgGLSetViewAngle(float ang)
{
  /* argument is in rad, GL angles are in tenths of degrees */
  ViewAng = Rad2GLAng(ang);
}

void
SgGLSetNearPlane(float d, BOOL local)
{
  Near = d;
  if (! local)
    NearGlob = d;
}

void
SgGLSetFarPlane(float d, BOOL local)
{
  Far = d;
  if (! local)
    FarGlob = d;
}

static void
setViewport(void)
{
  viewport((Screencoord) (VpX + 0.5f), (Screencoord) (VpX + VpW + 0.5f),
      (Screencoord) (VpY + 0.5f), (Screencoord) (VpY + VpH + 0.5f));
}

void
SgGLSetViewport(float x, float y, float width, float height)
{
  VpX = x;
  VpY = y;
  VpW = width;
  VpH = height;
  AspRat = width / height;

  setViewport();
}

void
SgGLSetBuffer(SgBuffer buf)
{
  if (buf == SG_BUF_FRONT) {
    backbuffer(FALSE);
    frontbuffer(TRUE);
  } else {
    frontbuffer(FALSE);
    backbuffer(TRUE);
  }
}

void
SgGLFlushFrame(void)
{
  gflush();
}

void
SgGLEndFrame(void)
{
  setViewport();  /* may have been changed for stereo display */
  swapbuffers();
}

void
GLStart2D(void)
{
  pushmatrix();
  loadmatrix(IdentM);
  mmode(MPROJECTION);
  getmatrix(ProjM);
  ortho2(- AspRat, AspRat, -1.0f, 1.0f);
  zbuffer(FALSE);
}

void
GLEnd2D(void)
{
  loadmatrix(ProjM);
  mmode(MVIEWING);
  popmatrix();
  if (SgGetFeature(SG_FEATURE_HIDDEN))
    zbuffer(TRUE);
}
