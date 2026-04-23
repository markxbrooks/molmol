/*
************************************************************************
*
*   POVColor.c - POV color setting
*
*   Copyright (c) 1994-2000
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/pov/SCCS/s.POVColor.c
*   SCCS identification       : 1.13
*
************************************************************************
*/

#include "pov_color.h"

#include <string.h>

#include <strmatch.h>
#include <sg_plot.h>
#include "pov_light.h"

static BOOL ValChanged = FALSE;
static float CurrR, CurrG, CurrB, CurrAlpha;
static float CurrBackR, CurrBackG, CurrBackB;
static float CurrAmb, CurrDiff, CurrSpec, CurrShin, CurrRefl, CurrRefr;
static float CurrTexSc, CurrBump, CurrBumpSc;
static char CurrTex[SG_TEXTURE_NAME_SIZE];
static BOOL TexIsGIF = FALSE;
static SgShadeModel CurrModel;
static float CurrFogDens;

void
SgPOVSetColor(float r, float g, float b, float a, BOOL fg)
{
  if (fg) {
    CurrR = r;
    CurrG = g;
    CurrB = b;
    CurrAlpha = a;

    ValChanged = TRUE;
  } else {
    CurrBackR = r;
    CurrBackG = g;
    CurrBackB = b;
  }
}

void
SgPOVSetMatProp(SgMatProp prop, float val)
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
      CurrShin = 0.3f * val;
      break;
    case SG_MAT_REFLECTION:
      CurrRefl = val;
      break;
    case SG_MAT_REFRACTION_INDEX:
      CurrRefr = val;
      break;
    case SG_MAT_TEXTURE_SCALE:
      CurrTexSc = 0.1f * val;
      break;
    case SG_MAT_BUMP_DEPTH:
      CurrBump = val;
      break;
    case SG_MAT_BUMP_SCALE:
      CurrBumpSc = 0.02f * val;
      break;
  }

  ValChanged = TRUE;
}

void
SgPOVSetTexture(char *name)
{
  (void) strcpy(CurrTex, name);
  TexIsGIF = StrMatch(name, "*.gif");
  ValChanged = TRUE;
}

void
SgPOVSetShadeModel(SgShadeModel model)
{
  CurrModel = model;
  ValChanged = TRUE;
}

void
SgPOVSetFogMode(SgFogMode mode)
{
  /* linear fog is not easily possible, ignore that */
  if (mode != SG_FOG_MODE_EXP && mode != SG_FOG_MODE_EXP2)
    return;

  /* assume that parameters are set already */
  SgPlotOutputStr("fog {\n");
  SgPlotOutputStr("color rgb <");
  SgPlotOutputFloat(CurrBackR);
  SgPlotOutputStr(", ");
  SgPlotOutputFloat(CurrBackG);
  SgPlotOutputStr(", ");
  SgPlotOutputFloat(CurrBackB);
  SgPlotOutputStr(">\n");
  SgPlotOutputStr("distance ");
  SgPlotOutputFloat(1.0f / CurrFogDens);
  SgPlotOutputStr("\n");
  SgPlotOutputStr("}\n");
}

void
SgPOVSetFogPar(SgFogPar par, float val)
{
  /* this is called before SgPOVSetFogMode, so do the output there */
  if (par == SG_FOG_DENSITY)
    CurrFogDens = val;
}

void
POVPlotTexture(void)
{
  SgPlotOutputStr("texture {\n");

  if (CurrTex[0] == '\0') {
    if (CurrAlpha < 1.0f)
      SgPlotOutputStr("pigment {color rgbf <");
    else
      SgPlotOutputStr("pigment {color rgb <");

    SgPlotOutputFloat(CurrR);
    SgPlotOutputStr(", ");
    SgPlotOutputFloat(CurrG);
    SgPlotOutputStr(", ");
    SgPlotOutputFloat(CurrB);

    if (CurrAlpha < 1.0f) {
      SgPlotOutputStr(", ");
      SgPlotOutputFloat(1.0f - CurrAlpha);
    }

    SgPlotOutputStr(">}\n");
  } else if (TexIsGIF) {
    SgPlotOutputStr("pigment {image_map {gif \"");
    SgPlotOutputStr(CurrTex);
    SgPlotOutputStr("\" interpolate 2} scale ");
    SgPlotOutputFloat(5.0f * CurrTexSc);
    SgPlotOutputStr("}\n");
  }

  if (CurrTex[0] == '\0' || TexIsGIF) {
    SgPlotOutputStr("finish {");
    SgPlotOutputStr("ambient ");
    SgPlotOutputFloat(CurrAmb);
    SgPlotOutputStr(" diffuse ");
    SgPlotOutputFloat(CurrDiff);
    if (CurrSpec > 0.0f) {
      SgPlotOutputStr(" phong ");
      SgPlotOutputFloat(CurrSpec);
      SgPlotOutputStr(" phong_size ");
      SgPlotOutputFloat(CurrShin);
    }
    if (CurrRefl > 0.0f) {
      SgPlotOutputStr(" reflection ");
      SgPlotOutputFloat(CurrRefl);
    }

    if (CurrAlpha < 1.0f && CurrRefr != 1.0f) {
      SgPlotOutputStr(" refraction 1 ior ");
      SgPlotOutputFloat(CurrRefr);
    }

    SgPlotOutputStr("}\n");
  } else {
    SgPlotOutputStr(CurrTex);
    if (CurrTexSc != 1.0f) {
      SgPlotOutputStr(" scale ");
      SgPlotOutputFloat(CurrTexSc);
    }
    SgPlotOutputStr("\n");
  }

  if (CurrBump > 0.0f) {
    SgPlotOutputStr("normal {bumps ");
    SgPlotOutputFloat(CurrBump);
    if (CurrBumpSc != 1.0f) {
      SgPlotOutputStr(" scale ");
      SgPlotOutputFloat(CurrBumpSc);
    }
    SgPlotOutputStr("}\n");
  }

  SgPlotOutputStr("}");

  ValChanged = FALSE;
}

void
POVUpdateColor(void)
{
  if (! ValChanged)
    return;

  SgPlotOutputStr("#declare T = ");
  POVPlotTexture();
  SgPlotOutputStr("\n");
}

void
POVPlotColor(void)
{
  SgPlotOutputStr("texture {T}\n");
}
