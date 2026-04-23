/*
************************************************************************
*
*   GLLight.c - GL light source handling
*
*   Copyright (c) 1994
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/gl/SCCS/s.GLLight.c
*   SCCS identification       : 1.12
*
************************************************************************
*/

#include "gl_light.h"

#include <stdio.h>
#include <gl/gl.h>

#include <sg_get.h>

static SgLightState LightState = SG_LIGHT_OFF;
static float LightPos[4];
static BOOL LightSetupDone = FALSE, LightPosSet = FALSE;
static float MatCol[4], MatDiff, MatSpec;

static void
bindLight(void)
{
  float lt[6] = {POSITION};
  int i;

  for (i = 0; i < 4; i++)
    lt[i + 1] = LightPos[i];
  lt[5] = LMNULL;
  lmdef(DEFLIGHT, 1, 6, lt);

  lmbind(LIGHT0, 1);
}

void
SgGLSetLight(SgLightState state)
{
  if (! LightSetupDone) {
    lmdef(DEFMATERIAL, 1, 0, NULL);
    lmbind(MATERIAL, 1);

    LightSetupDone = TRUE;
  }

  LightState = state;

  if (state == SG_LIGHT_OFF) {
    lmcolor(LMC_COLOR);
    lmbind(LMODEL, 0);
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
      bindLight();
      LightPosSet = TRUE;
    }

    lmcolor(LMC_AD);
    lmbind(LMODEL, 1);
  }

  c4f(MatCol);
}

void
SgGLSetLightPosition(float x[3])
{
  int i;

  for (i = 0; i < 3; i++)
    LightPos[i] = x[i];

  LightPosSet = FALSE;
}

static void
updateSpec(void)
{
  float mat[5];
  float val;

  /* since we make the light source weaker for adjusting the
     diffuse component (so that we can use lmcolor() for
     updateing the color), we have to make the specular
     component of the material stronger accordingly. */

  if (MatSpec > MatDiff || MatDiff == 0.0f)
    val = 1.0f;
  else
    val = MatSpec / MatDiff;

  mat[0] = SPECULAR;
  mat[1] = val;
  mat[2] = val;
  mat[3] = val;
  mat[4] = LMNULL;
  lmdef(DEFMATERIAL, 1, 5, mat);
}

void
SgGLSetMatProp(SgMatProp prop, float val)
{
  float mat[5];
  float lt[5];

  switch (prop) {
    case SG_MAT_AMBIENT_FACTOR:
      lt[0] = AMBIENT;
      lt[1] = val;
      lt[2] = val;
      lt[3] = val;
      lt[4] = LMNULL;
      lmdef(DEFLMODEL, 1, 5, lt);
      break;
    case SG_MAT_DIFFUSE_FACTOR:
      MatDiff = val;
      lt[0] = LCOLOR;
      lt[1] = val;
      lt[2] = val;
      lt[3] = val;
      lt[4] = LMNULL;
      lmdef(DEFLIGHT, 1, 5, lt);
      updateSpec();
      break;
    case SG_MAT_SPECULAR_FACTOR:
      MatSpec = val;
      updateSpec();
      break;
    case SG_MAT_SHININESS:
      mat[0] = SHININESS;
      mat[1] = val;
      mat[2] = LMNULL;
      lmdef(DEFMATERIAL, 1, 3, mat);
      break;
    default:
      /* not supported */;
  }
}

void
GLSetMaterialColor(float c[4])
{
  MatCol[0] = c[0];
  MatCol[1] = c[1];
  MatCol[2] = c[2];
  MatCol[3] = c[3];
}

long
GLGetColorMode(void)
{
  if (LightState == SG_LIGHT_OFF)
    return LMC_COLOR;
  else
    return LMC_AD;
}
