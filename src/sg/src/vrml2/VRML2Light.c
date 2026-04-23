/*
************************************************************************
*
*   VRML2Light.c - VRML2 light source handling
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
*   Date of last modification : 99/10/31
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/sg/src/vrml2/SCCS/s.VRML2Light.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include "vrml2_light.h"

#include <sg_plot.h>
#include "vrml2_util.h"
#include "vrml2_color.h"

static BOOL LightChanged = FALSE;
static SgLightState LightState, LightKind = SG_LIGHT_OFF;
static float LightPos[3], LightDir[3];

void
SgVRML2SetLight(SgLightState state)
{
  LightState = state;

  VRML2ColorSetLight(state != SG_LIGHT_OFF);

  if (state == SG_LIGHT_OFF || state == LightKind)
    return;

  LightKind = state;
  LightChanged = TRUE;
}

void
SgVRML2SetLightPosition(float x[3])
{
  int i;

  for (i = 0; i < 3; i++) {
    LightPos[i] = x[i];
    LightDir[i] = - x[i];
  }

  LightChanged = TRUE;
}

void
VRML2UpdateLight(void)
{
  if (LightKind == SG_LIGHT_OFF || ! LightChanged)
    return;

  if (LightKind == SG_LIGHT_POINT) {
    SgPlotOutputStr("PointLight {");
    SgPlotOutputStr("location ");
    VRML2PlotVect(LightPos);
    SgPlotOutputStr("}\n");
  } else {
    SgPlotOutputStr("DirectionalLight {");
    SgPlotOutputStr("direction ");
    VRML2PlotVect(LightDir);
    SgPlotOutputStr("}\n");
  }

  LightChanged = FALSE;
}
