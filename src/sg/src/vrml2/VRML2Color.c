/*
************************************************************************
*
*   VRML2Color.c - VRML2 color setting
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
*   Date of last modification : 01/05/27
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/vrml2/SCCS/s.VRML2Color.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include "vrml2_color.h"

#include <string.h>

#include <sg_plot.h>

static BOOL ValChanged = FALSE;
static float CurrR, CurrG, CurrB, CurrAlpha;
static float CurrAmb, CurrDiff, CurrSpec, CurrShin;
static char CurrTex[SG_TEXTURE_NAME_SIZE];
static BOOL LightOn = FALSE;

void
SgVRML2SetColor(float r, float g, float b, float a, BOOL fg)
{
  if (! fg)
    return;

  CurrR = r;
  CurrG = g;
  CurrB = b;
  CurrAlpha = a;

  ValChanged = TRUE;
}

void
SgVRML2SetMatProp(SgMatProp prop, float val)
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
      CurrShin = val / 128.0f;
      break;
    default:
      /* not supported */
      break;
  }

  ValChanged = TRUE;
}

void
SgVRML2SetTexture(char *name)
{
  (void) strcpy(CurrTex, name);
}

void
VRML2ColorSetLight(BOOL onOff)
{
  if (onOff == LightOn)
    return;

  LightOn = onOff;

  ValChanged = TRUE;
}

void
VRML2PlotColor(void)
{
  SgPlotOutputStr("appearance Appearance {\n");

  if (CurrTex[0] != '\0') {
    SgPlotOutputStr("texture ImageTexture {url \"");
    SgPlotOutputStr(CurrTex);
    SgPlotOutputStr("}\n");
  }

  if (ValChanged) {
    SgPlotOutputStr("material DEF M Material {\n");

    if (LightOn) {
      SgPlotOutputStr("ambientIntensity ");
      SgPlotOutputFloat(CurrAmb);
      SgPlotOutputStr("\n");

      SgPlotOutputStr("diffuseColor ");
      SgPlotOutputFloat(CurrDiff * CurrR);
      SgPlotOutputStr(" ");
      SgPlotOutputFloat(CurrDiff * CurrG);
      SgPlotOutputStr(" ");
      SgPlotOutputFloat(CurrDiff * CurrB);
      SgPlotOutputStr("\n");

      SgPlotOutputStr("specularColor ");
      SgPlotOutputFloat(CurrSpec);
      SgPlotOutputStr(" ");
      SgPlotOutputFloat(CurrSpec);
      SgPlotOutputStr(" ");
      SgPlotOutputFloat(CurrSpec);
      SgPlotOutputStr("\n");

      SgPlotOutputStr("shininess ");
      SgPlotOutputFloat(CurrShin);
      SgPlotOutputStr("\n");

      SgPlotOutputStr("transparency ");
      SgPlotOutputFloat(1.0f - CurrAlpha);
      SgPlotOutputStr("\n");
    } else {
      SgPlotOutputStr("emissiveColor ");
      SgPlotOutputFloat(CurrR);
      SgPlotOutputStr(" ");
      SgPlotOutputFloat(CurrG);
      SgPlotOutputStr(" ");
      SgPlotOutputFloat(CurrB);
      SgPlotOutputStr("\n");
    }

    SgPlotOutputStr("}\n");
  } else {
    SgPlotOutputStr("material USE M\n");
  }

  SgPlotOutputStr("}\n");

  ValChanged = FALSE;
}
