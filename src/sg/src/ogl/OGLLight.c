/*
************************************************************************
*
*   OGLLight.c - OpenGL light source handling
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/ogl/SCCS/s.OGLLight.c
*   SCCS identification       : 1.17
*
************************************************************************
*/

#include "ogl_light.h"

#include <stdio.h>
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>

#include <sg_get.h>

static float LightPos[4];
static BOOL LightPosSet = FALSE;
static float MatCol[4];
static GLenum CurrFace = GL_FRONT;

static void
updateFace(void)
{
  GLenum face;

  if (SgGetFeature(SG_FEATURE_TWO_SIDE))
    face = GL_FRONT_AND_BACK;
  else
    face = GL_FRONT;
  
  if (face != CurrFace) {
    glColorMaterial(face, GL_AMBIENT_AND_DIFFUSE);
    CurrFace = face;
  }
}

void
SgOGLSetLight(SgLightState state)
{
  if (state == SG_LIGHT_OFF) {
    glDisable(GL_COLOR_MATERIAL);
    glDisable(GL_LIGHTING);
  } else {
    if (state == SG_LIGHT_POINT) {
      if (LightPos[3] != 1.0f) {
	LightPos[3] = 1.0f;
	LightPosSet = FALSE;
      }
    } else {
      if (LightPos[3] != 0.0f) {
	LightPos[3] = 0.0f;
	LightPosSet = FALSE;
      }
    }

    if (! LightPosSet) {
      glLightfv(GL_LIGHT0, GL_POSITION, LightPos);
      LightPosSet = TRUE;
    }

    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
  }

  updateFace();
  glColor4fv(MatCol);
}

void
SgOGLSetLightPosition(float x[3])
{
  int i;

  for (i = 0; i < 3; i++)
    LightPos[i] = x[i];

  LightPosSet = FALSE;
}

void
SgOGLSetMatProp(SgMatProp prop, float val)
{
  float v[4];

  switch (prop) {
    case SG_MAT_AMBIENT_FACTOR:
      v[0] = val;
      v[1] = val;
      v[2] = val;
      v[3] = 1.0f;
      glLightModelfv(GL_LIGHT_MODEL_AMBIENT, v);
      break;
    case SG_MAT_DIFFUSE_FACTOR:
      v[0] = val;
      v[1] = val;
      v[2] = val;
      v[3] = 1.0f;
      glLightfv(GL_LIGHT0, GL_DIFFUSE, v);
      break;
    case SG_MAT_SPECULAR_FACTOR:
      v[0] = val;
      v[1] = val;
      v[2] = val;
      v[3] = 1.0f;
      glLightfv(GL_LIGHT0, GL_SPECULAR, v);
      break;
    case SG_MAT_SHININESS:
      glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, val);
      break;
  }
}

void
OGLSetMaterialColor(float c[4])
{
  MatCol[0] = c[0];
  MatCol[1] = c[1];
  MatCol[2] = c[2];
  MatCol[3] = c[3];

  updateFace();
}
