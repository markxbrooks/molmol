/*
************************************************************************
*
*   OGLView.c - OpenGL viewing parameters
*
*   Copyright (c) 1994-2001
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/ogl/SCCS/s.OGLView.c
*   SCCS identification       : 1.11
*
************************************************************************
*/

#include "ogl_view.h"

#include <math.h>
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>

#include <sg_get.h>
#include "ogl_util.h"
#include "ogl_stereo.h"

static SgProjection Proj;
static GLfloat VX, VY, VZ;
static GLfloat ViewAng;
static GLfloat Near, Far, NearGlob, FarGlob;
static float VpX, VpY, VpW, VpH;
static float AspRat;

static GLfloat ProjM[16];

void
SgOGLConvCoord(int xs, int ys, float *xP, float *yP)
{
  *xP = (2 * (xs - VpX) - VpW) / VpH;
  if (OGLFullscreenStereo()) {
    if (ys >= 0.5f * VpH)
      *yP = 4 * (ys - VpY) / VpH - 3.0f;
    else
      *yP = 4 * (ys - VpY) / VpH - 1.0f;
  } else {
    *yP = 2 * (ys - VpY) / VpH - 1.0f;
  }
}

void
SgOGLUpdateView(void)
{
  float b;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  if (Proj == SG_PROJ_ORTHO) {
    b = SgGetOrthoHeight();
    glOrtho(- b * AspRat, b * AspRat, - b, b, Near, Far);
  } else {
    gluPerspective(ViewAng, AspRat, Near, Far);
  }
  glMatrixMode(GL_MODELVIEW);

  glLoadIdentity();
  gluLookAt(VX, VY, VZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

  if (Proj == SG_PROJ_ORTHO)
    glDepthRange((Near - NearGlob) / (FarGlob - NearGlob),
	(Far - NearGlob) / (FarGlob - NearGlob));
  else
    glDepthRange(FarGlob / (FarGlob - NearGlob) -
	FarGlob * NearGlob / ((FarGlob - NearGlob) * Near),
	FarGlob / (FarGlob - NearGlob) -
	FarGlob * NearGlob / ((FarGlob - NearGlob) * Far));
}

void
SgOGLSetProjection(SgProjection proj)
{
  Proj = proj;
}

void
SgOGLSetViewPoint(float x[3])
{
  VX = x[0];
  VY = x[1];
  VZ = x[2];
}

void
SgOGLSetViewAngle(float ang)
{
  /* argument is in rad, GL angles are in tenths of degrees */
  ViewAng = Rad2OGLAng(ang);
}

void
SgOGLSetNearPlane(float d, BOOL local)
{
  Near = d;
  if (! local)
    NearGlob = d;
}

void
SgOGLSetFarPlane(float d, BOOL local)
{
  Far = d;
  if (! local)
    FarGlob = d;
}

static void
setViewport(void)
{
  glViewport((GLuint) (VpX + 0.5f), (GLuint) (VpY + 0.5f),
      (GLuint) (VpW + 0.5f), (GLuint) (VpH + 0.5f));
}

void
SgOGLSetViewport(float x, float y, float width, float height)
{
  VpX = x;
  VpY = y;
  VpW = width;
  VpH = height;
  AspRat = width / height;

  setViewport();
}

void
SgOGLSetBuffer(SgBuffer buf)
{
  if (buf == SG_BUF_FRONT)
    glDrawBuffer(GL_FRONT);
  else
    glDrawBuffer(GL_BACK);
}

void
SgOGLFlushFrame(void)
{
  glFlush();
}

void
SgOGLEndFrame(void)
{
  setViewport();  /* may have been changed for stereo display */

  /* changing of buffers for double buffering is done in the
     corresponding IO function, e. g. IOMotifOGLEndFrame() */
}

void
OGLStart2D(void)
{
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_PROJECTION);
  glGetFloatv(GL_PROJECTION_MATRIX, ProjM);
  glLoadIdentity();
  gluOrtho2D(- AspRat, AspRat, -1.0, 1.0);
  glDisable(GL_DEPTH_TEST);
}

void
OGLEnd2D(void)
{
  glLoadMatrixf(ProjM);
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  if (SgGetFeature(SG_FEATURE_HIDDEN))
    glEnable(GL_DEPTH_TEST);
}
