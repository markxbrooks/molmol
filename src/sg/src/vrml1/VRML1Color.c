/*
************************************************************************
*
*   VRML1Color.c - VRML1 color setting
*
*   Copyright (c) 1996
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/vrml1/SCCS/s.VRML1Color.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include "vrml1_color.h"

#include <string.h>

#include <strmatch.h>
#include <sg_plot.h>
#include "vrml1_light.h"

static BOOL ValChanged = FALSE;
static float CurrR, CurrG, CurrB, CurrAlpha;
static float CurrAmb, CurrDiff, CurrSpec, CurrShin;
static char CurrTex[SG_TEXTURE_NAME_SIZE];

void
SgVRML1SetColor(float r, float g, float b, float a, BOOL fg)
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
SgVRML1SetMatProp(SgMatProp prop, float val)
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
SgVRML1SetTexture(char *name)
{
  (void) strcpy(CurrTex, name);
}

void
VRML1PlotColor(void)
{
  if (CurrTex[0] != '\0') {
    SgPlotOutputStr("Texture2 {filename \"");
    SgPlotOutputStr(CurrTex);
    SgPlotOutputStr("}\n");
  }

  if (! ValChanged) {
    SgPlotOutputStr("USE M\n");
    return;
  }

  SgPlotOutputStr("DEF M Material {\n");

  SgPlotOutputStr("ambientColor ");
  SgPlotOutputFloat(CurrAmb * CurrR);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(CurrAmb * CurrG);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(CurrAmb * CurrB);
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

  SgPlotOutputStr("}\n");

  ValChanged = FALSE;
}
