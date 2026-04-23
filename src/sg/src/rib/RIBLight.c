/*
************************************************************************
*
*   RIBLight.c - RIB light source handling
*
*   Copyright (c) 1994-98
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
*   Date of last modification : 99/10/16
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/rib/SCCS/s.RIBLight.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include "rib_light.h"

#include <sg_plot.h>
#include "rib_util.h"
#include "rib_world.h"

static BOOL LightDirty = FALSE;
static SgLightState LightState;
static SgLightState LightKind = SG_LIGHT_OFF;
static float LightPos[3];

static void
switchLight(void)
{
  if (LightState == SG_LIGHT_OFF) {
    SgPlotOutputStr("Illuminate 0 1\n");
    SgPlotOutputStr("Illuminate 1 0\n");
  } else {
    SgPlotOutputStr("Illuminate 0 0\n");
    SgPlotOutputStr("Illuminate 1 1\n");
  }
}

static void
plotLight(void)
{
  if (LightKind == SG_LIGHT_OFF) {
    SgPlotOutputStr("LightSource \"ambientlight\" 1 \"intensity\" 0.0\n");
  } else if (LightState == SG_LIGHT_POINT) {
    SgPlotOutputStr("LightSource \"pointlight\" 1 \"from\" [");
    RIBPlotVect(LightPos);
    SgPlotOutputStr("]\n");
  } else {
    SgPlotOutputStr("LightSource \"distantlight\" 1 \"from\" [");
    RIBPlotVect(LightPos);
    SgPlotOutputStr("] \"to\" [0.0 0.0 0.0]");
    SgPlotOutputStr("\n");
  }
}

void
SgRIBSetLight(SgLightState state)
{
  LightState = state;

  if (LightState != SG_LIGHT_OFF)
    LightKind = LightState;

  if (RIBInWorld())
    switchLight();
  else
    LightDirty = TRUE;
}

void
SgRIBSetLightPosition(float x[3])
{
  int i;

  for (i = 0; i < 3; i++)
    LightPos[i] = x[i];
}

void
RIBStartLight(void)
{
  if (LightDirty) {
    plotLight();
    LightDirty = FALSE;
  }
}
