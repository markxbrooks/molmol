/*
************************************************************************
*
*   OGLFeature.c - OpenGL enable/disable features
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
*   Date of last modification : 97/02/17
*   Pathname of SCCS file     : /local/home/kor/molmol/sg/src/ogl/SCCS/s.OGLFeature.c
*   SCCS identification       : 1.8
*
************************************************************************
*/

#include "ogl_feature.h"

#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

static BOOL TranspOn = FALSE, AntiOn = FALSE;

void
SgOGLSetFogMode(SgFogMode mode)
{
  switch (mode) {
    case SG_FOG_MODE_OFF:
      glDisable(GL_FOG);
      break;
    case SG_FOG_MODE_EXP:
      glFogi(GL_FOG_MODE, GL_EXP);
      glEnable(GL_FOG);
      break;
    case SG_FOG_MODE_EXP2:
      glFogi(GL_FOG_MODE, GL_EXP2);
      glEnable(GL_FOG);
      break;
    case SG_FOG_MODE_LINEAR:
      glFogi(GL_FOG_MODE, GL_LINEAR);
      glEnable(GL_FOG);
      break;
  }
}

void
SgOGLSetFogPar(SgFogPar par, float val)
{
  switch (par) {
    case SG_FOG_DENSITY:
      glFogf(GL_FOG_DENSITY, val);
      break;
    case SG_FOG_START:
      glFogf(GL_FOG_START, val);
      break;
    case SG_FOG_END:
      glFogf(GL_FOG_END, val);
      break;
  }
}

void
SgOGLSetFeature(SgFeature feature, BOOL onOff)
{
  switch (feature) {
    case SG_FEATURE_TRANSPARENCY:
      if (onOff) {
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
      } else if (! AntiOn) {
	glBlendFunc(GL_ONE, GL_ZERO);
	glDisable(GL_BLEND);
      }
      TranspOn = onOff;
      break;
    case SG_FEATURE_ANTI_ALIASING:
      if (onOff) {
	glEnable(GL_LINE_SMOOTH);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
      } else {
	glDisable(GL_LINE_SMOOTH);
	if (! TranspOn) {
	  glBlendFunc(GL_ONE, GL_ZERO);
	  glDisable(GL_BLEND);
	}
      }
      AntiOn = onOff;
      break;
    case SG_FEATURE_BACKFACE:
      if (onOff) {
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
      } else {
	glDisable(GL_CULL_FACE);
      }
      break;
    case SG_FEATURE_TWO_SIDE:
      if (onOff)
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
      else
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);
      break;
    case SG_FEATURE_HIDDEN:
      if (onOff) {
	glEnable(GL_DEPTH_TEST);
      } else {
	glDisable(GL_DEPTH_TEST);
      }
      break;
  }
}
