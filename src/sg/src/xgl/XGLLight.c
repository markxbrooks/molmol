/*
************************************************************************
*
*   XGLLight.c - XGL light source handling
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
*   Date of last modification : 95/12/12
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/xgl/SCCS/s.XGLLight.c
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include "xgl_light.h"

#include <stdio.h>
#include <xgl/xgl.h>

#include "xgl_access.h"
#include "xgl_proj.h"

#define LIGHT_NUM 2

static BOOL LightSetupDone = FALSE;
static SgShadeModel ShadeModel;
static Xgl_pt_d3d LightPos;
static Xgl_pt_f3d LightDir;

static void
setShadeModel(Xgl_object ctx)
{
  switch (ShadeModel) {
    case SG_SHADE_FLAT:
      xgl_object_set(ctx,
	  XGL_3D_CTX_SURF_FRONT_ILLUMINATION, XGL_ILLUM_PER_FACET,
	  NULL);
      break;
    case SG_SHADE_GOURAUD:
    case SG_SHADE_PHONG:
      xgl_object_set(ctx,
	  XGL_3D_CTX_SURF_FRONT_ILLUMINATION, XGL_ILLUM_PER_VERTEX,
	  NULL);
      break;
  }
}

void
SgXGLSetLight(SgLightState state)
{
  Xgl_object ctx = XGLGetContext3D();
  Xgl_light lightA[LIGHT_NUM];
  Xgl_color col;
  Xgl_boolean lightSwitch[2];

  if (LightSetupDone) {
    xgl_object_get(ctx, XGL_3D_CTX_LIGHTS, lightA);
  } else {
    xgl_object_set(ctx,
	XGL_3D_CTX_LIGHT_NUM, LIGHT_NUM,
	NULL);

    xgl_object_get(ctx, XGL_3D_CTX_LIGHTS, lightA);

    col.rgb.r = 1.0;
    col.rgb.g = 1.0;
    col.rgb.b = 1.0;
    xgl_object_set(lightA[0],
	XGL_LIGHT_TYPE, XGL_LIGHT_AMBIENT,
	XGL_LIGHT_COLOR, &col,
	NULL);

    lightSwitch[0] = TRUE;
    lightSwitch[1] = TRUE;

    xgl_object_set(ctx,
	XGL_3D_CTX_LIGHT_SWITCHES, lightSwitch,
	NULL);

    LightSetupDone = TRUE;
  }

  if (state == SG_LIGHT_OFF) {
    xgl_object_set(ctx,
	XGL_3D_CTX_SURF_FRONT_ILLUMINATION, XGL_ILLUM_NONE,
	NULL);
  } else if (state == SG_LIGHT_POINT) {
    xgl_object_set(lightA[1],
	XGL_LIGHT_TYPE, XGL_LIGHT_POSITIONAL,
	XGL_LIGHT_POSITION, LightPos,
	NULL);
    setShadeModel(ctx);
  } else {
    xgl_object_set(lightA[1],
	XGL_LIGHT_TYPE, XGL_LIGHT_DIRECTIONAL,
	XGL_LIGHT_DIRECTION, LightDir,
	NULL);
    setShadeModel(ctx);
  }
}

void
SgXGLSetLightPosition(float x[3])
{
  LightPos.x = x[0];
  LightPos.y = x[1];
  LightPos.z = x[2];

  LightDir.x = - x[0];
  LightDir.y = - x[1];
  LightDir.z = - x[2];
}

void
SgXGLSetShadeModel(SgShadeModel model)
{
  ShadeModel = model;
}
