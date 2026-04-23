/*
************************************************************************
*
*   POVLight.c - POV light source handling
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/pov/SCCS/s.POVLight.c
*   SCCS identification       : 1.5
*
************************************************************************
*/

#include "pov_light.h"

#include <sg_plot.h>

static SgLightState LightKind = SG_LIGHT_OFF;
static float LightPos[3];

void
SgPOVSetLight(SgLightState state)
{
  if (state != SG_LIGHT_OFF)
    LightKind = state;
}

void
SgPOVSetLightPosition(float x[3])
{
  int i;

  for (i = 0; i < 3; i++)
    LightPos[i] = x[i];
}

void
POVPlotLightSource(void)
{
  float mult;

  if (LightKind != SG_LIGHT_OFF) {
    if (LightKind == SG_LIGHT_POINT)
      mult = 1.0f;
    else
      mult = 100.0f;

    SgPlotOutputStr("light_source {<");
    SgPlotOutputFloat(mult * LightPos[0]);
    SgPlotOutputStr(", ");
    SgPlotOutputFloat(mult * LightPos[1]);
    SgPlotOutputStr(", ");
    SgPlotOutputFloat(mult * LightPos[2]);
    SgPlotOutputStr("> color White}\n");
  }
}
