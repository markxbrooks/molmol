/*
************************************************************************
*
*   RIBColor.c - RIB color setting
*
*   Copyright (c) 1994-99
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/rib/SCCS/s.RIBColor.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include "rib_color.h"

#include <string.h>

#include <strmatch.h>
#include <sg_plot.h>
#include "rib_world.h"
#include "rib_view.h"

#define AMBIENT_LIGHT_NO 100

static BOOL ColorSet = FALSE;
static BOOL MaterialSet = FALSE;
static BOOL MaterialDirty = FALSE;
static BOOL LightCreatedA[AMBIENT_LIGHT_NO];
static int CurrLightId;
static float CurrR, CurrG, CurrB, CurrAlpha;
static float CurrBackR, CurrBackG, CurrBackB;
static float CurrAmb, CurrDiff, CurrSpec, CurrShin, CurrRefl, CurrRefr;
static float CurrTexSc, CurrBump, CurrBumpSc;
static char CurrTex[SG_TEXTURE_NAME_SIZE];
static BOOL TexIsGIF = FALSE;
static SgShadeModel CurrModel;
static SgFogMode CurrFogMode;
static float CurrFogDens, CurrFogStart, CurrFogEnd;

static void
plotColor(void)
{
  SgPlotOutputStr("Color ");
  SgPlotOutputFloat(CurrR);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(CurrG);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(CurrB);
  SgPlotOutputStr("\n");

  SgPlotOutputStr("Opacity ");
  SgPlotOutputFloat(CurrAlpha);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(CurrAlpha);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(CurrAlpha);
  SgPlotOutputStr("\n");
}

static void
plotAtmosphere(void)
{
  float nearClip, farClip;

  SgPlotOutputStr("Atmosphere ");
  if (CurrFogMode == SG_FOG_MODE_EXP || CurrFogMode == SG_FOG_MODE_EXP2) {
    SgPlotOutputStr("\"fog\" \"distance\" ");
    SgPlotOutputFloat(1.0f / CurrFogDens);
  } else {
    RIBGetClipPlanes(&nearClip, &farClip);
    SgPlotOutputStr("\"depthcue\" \"mindistance\" ");
    if (CurrFogMode == SG_FOG_MODE_LINEAR)
      SgPlotOutputFloat((CurrFogStart - nearClip) / (farClip - nearClip));
    else
      SgPlotOutputFloat(1000.0f);
    SgPlotOutputStr(" \"maxdistance\" ");
    if (CurrFogMode == SG_FOG_MODE_LINEAR)
      SgPlotOutputFloat((CurrFogEnd - nearClip) / (farClip - nearClip));
    else
      SgPlotOutputFloat(1000.0f);
  }
  SgPlotOutputStr(" \"background\" [");
  SgPlotOutputFloat(CurrBackR);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(CurrBackG);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(CurrBackB);
  SgPlotOutputStr("]\n");
}

void
SgRIBSetColor(float r, float g, float b, float a, BOOL fg)
{
  if (fg) {
    CurrR = r;
    CurrG = g;
    CurrB = b;
    CurrAlpha = a;

    if (RIBInWorld())
      plotColor();
    else
      ColorSet = TRUE;
  } else {
    CurrBackR = r;
    CurrBackG = g;
    CurrBackB = b;

    if (RIBInWorld() && CurrFogMode != SG_FOG_MODE_OFF)
      plotAtmosphere();
  }
}

static void
plotMaterial(void)
{
  int lightId;

  lightId = (int) (CurrAmb * AMBIENT_LIGHT_NO);
  if (lightId < 0)
    lightId = 0;
  else if (lightId >= AMBIENT_LIGHT_NO)
    lightId = AMBIENT_LIGHT_NO - 1;

  if (lightId != CurrLightId) {
    if (CurrLightId >= 0) {
      SgPlotOutputStr("Illuminate ");
      SgPlotOutputInt(CurrLightId);
      SgPlotOutputStr(" 0\n");
    }

    if (LightCreatedA[lightId]) {
      SgPlotOutputStr("Illuminate ");
      SgPlotOutputInt(lightId);
      SgPlotOutputStr(" 1\n");
    } else {
      SgPlotOutputStr("LightSource \"ambientlight\" ");
      SgPlotOutputInt(lightId);
      SgPlotOutputStr(" \"intensity\" ");
      SgPlotOutputFloat(CurrAmb);
      SgPlotOutputStr("\n");
      LightCreatedA[lightId] = TRUE;
    }

    CurrLightId = lightId;
  }

  SgPlotOutputStr("Surface \"");
  if (CurrTex[0] == '\0')
    SgPlotOutputStr("plastic");
  else
    SgPlotOutputStr(CurrTex);
  SgPlotOutputStr("\" \"Kd\" ");
  SgPlotOutputFloat(CurrDiff);
  SgPlotOutputStr(" \"Ks\" ");
  SgPlotOutputFloat(CurrSpec);
  SgPlotOutputStr(" \"roughness\" ");
  SgPlotOutputFloat(CurrBump);
  SgPlotOutputStr("\n");
}

void
SgRIBSetMatProp(SgMatProp prop, float val)
{
  switch (prop) {
    case SG_MAT_AMBIENT_FACTOR:
      CurrAmb = val;
      break;
    case SG_MAT_DIFFUSE_FACTOR:
      CurrDiff = val;
      break;
    case SG_MAT_SPECULAR_FACTOR:
      CurrSpec = val;
      break;
    case SG_MAT_SHININESS:
      CurrShin = val;
      break;
    case SG_MAT_REFLECTION:
      CurrRefl = val;
      break;
    case SG_MAT_REFRACTION_INDEX:
      CurrRefr = val;
      break;
    case SG_MAT_TEXTURE_SCALE:
      CurrTexSc = val;
      break;
    case SG_MAT_BUMP_DEPTH:
      CurrBump = val;
      break;
    case SG_MAT_BUMP_SCALE:
      CurrBumpSc = val;
      break;
  }

  if (RIBInWorld())
    MaterialDirty = TRUE;
  else
    MaterialSet = TRUE;
}

void
SgRIBSetTexture(char *name)
{
  (void) strcpy(CurrTex, name);
  TexIsGIF = StrMatch(name, "*.gif");

  if (RIBInWorld())
    plotMaterial();
  else
    MaterialSet = TRUE;
}

void
SgRIBSetShadeModel(SgShadeModel model)
{
  CurrModel = model;
}

void
SgRIBSetFogMode(SgFogMode mode)
{
  CurrFogMode = mode;

  if (RIBInWorld() && mode != SG_FOG_MODE_OFF)
    plotAtmosphere();
}

void
SgRIBSetFogPar(SgFogPar par, float val)
{
  /* this is called before SgRIBSetFogMode, so do the output there */
  switch (par) {
    case SG_FOG_DENSITY:
      CurrFogDens = val;
      break;
    case SG_FOG_START:
      CurrFogStart = val;
      break;
    case SG_FOG_END:
      CurrFogEnd = val;
      break;
  }
}

void
RIBStartColor(void)
{
  int i;

  for (i = 0; i < AMBIENT_LIGHT_NO; i++)
    LightCreatedA[i] = FALSE;
  CurrLightId = -1;

  if (CurrFogMode != SG_FOG_MODE_OFF)
    plotAtmosphere();

  if (ColorSet) {
    plotColor();
    ColorSet = FALSE;
  }

  if (MaterialSet) {
    plotMaterial();
    MaterialSet = FALSE;
  }

  MaterialDirty = FALSE;
}

void
RIBUpdateMaterial(void)
{
  if (MaterialDirty) {
    plotMaterial();
    MaterialDirty = FALSE;
  }
}
