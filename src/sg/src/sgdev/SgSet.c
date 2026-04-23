/*
************************************************************************
*
*   SgSet.c - Sg wrappers for set/get value functions
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/sgdev/SCCS/s.SgSet.c
*   SCCS identification       : 1.14
*
************************************************************************
*/

#include <sg_dev.h>
#include <sg_get.h>
#include "sg_set.h"

#include <string.h>

static BOOL Features[SG_FEATURE_NO];
static float CurrBColR, CurrBColG, CurrBColB;
static float CurrColR, CurrColG, CurrColB, CurrColA;
static BOOL AlphaChanged = FALSE;
static float MatProp[SG_MAT_PROP_NO];
static char Texture[SG_TEXTURE_NAME_SIZE];
static int CurrPrec;
static float CurrLineWidth;
static SgLineStyle CurrLineStyle;
static SgFogMode CurrFogMode;
static float CurrFogPar[SG_FOG_PAR_NO];

void
SgSetFeature(SgFeature feature, BOOL onOff)
{
  Features[feature] = onOff;

  CurrSgDevP->setFeature(feature, onOff);
}

BOOL
SgGetFeature(SgFeature feature)
{
  return Features[feature];
}

void
SgSetBackgroundColor(float r, float g, float b)
{
  if (r == CurrBColR && g == CurrBColG && b == CurrBColB)
    return;

  CurrSgDevP->setColor(r, g, b, 1.0f, FALSE);
  CurrBColR = r;
  CurrBColG = g;
  CurrBColB = b;
}

void
SgGetBackgroundColor(float *rP, float *gP, float *bP)
{
  *rP = CurrBColR;
  *gP = CurrBColG;
  *bP = CurrBColB;
}

void
SgSetColor(float r, float g, float b)
{
  if (r == CurrColR && g == CurrColG && b == CurrColB && ! AlphaChanged)
    return;

  CurrSgDevP->setColor(r, g, b, CurrColA, TRUE);
  CurrColR = r;
  CurrColG = g;
  CurrColB = b;
  AlphaChanged = FALSE;
}

void
SgGetColor(float *rP, float *gP, float *bP)
{
  *rP = CurrColR;
  *gP = CurrColG;
  *bP = CurrColB;
}

void
SgSetMatProp(SgMatProp prop, float val)
{
  if (prop == SG_MAT_ALPHA) {
    /* Some devices can only set alpha together with color, so we
       treat it specially and pass the new value at the next color
       change. */
    if (val == CurrColA)
      return;

    CurrColA = val;
    AlphaChanged = TRUE;
  } else {
    if (val == MatProp[prop])
      return;

    CurrSgDevP->setMatProp(prop, val);
  }

  MatProp[prop] = val;
}

float
SgGetMatProp(SgMatProp prop)
{
  return MatProp[prop];
}

void
SgSetTexture(char *name)
{
  if (strcmp(name, Texture) == 0)
    return;
  
  CurrSgDevP->setTexture(name);
  (void) strcpy(Texture, name);
}

void
SgSetPrecision(int prec)
{
  if (prec == CurrPrec)
    return;

  CurrSgDevP->setPrecision(prec);
  CurrPrec = prec;
}

int
SgGetPrecision(void)
{
  return CurrPrec;
}

void
SgSetLineWidth(float w)
{
  if (w == CurrLineWidth)
    return;

  CurrSgDevP->setLineWidth(w);
  CurrLineWidth = w;
}

float
SgGetLineWidth(void)
{
  return CurrLineWidth;
}

void
SgSetLineStyle(SgLineStyle lineStyle)
{
  if (lineStyle == CurrLineStyle)
    return;

  CurrSgDevP->setLineStyle(lineStyle);
  CurrLineStyle = lineStyle;
}

SgLineStyle
SgGetLineStyle(void)
{
  return CurrLineStyle;
}

void
SgSetFogMode(SgFogMode mode)
{
  if (mode == CurrFogMode)
    return;

  CurrSgDevP->setFogMode(mode);
  CurrFogMode = mode;
}

void
SgSetFogPar(SgFogPar par, float val)
{
  if (val == CurrFogPar[par])
    return;

  CurrSgDevP->setFogPar(par, val);
  CurrFogPar[par] = val;
}

void
SgUpdateColor(void)
{
  CurrSgDevP->setColor(CurrColR, CurrColG, CurrColB, CurrColA, TRUE);
}

void
SgSetAttr(void)
{
  int i;

  CurrSgDevP->setColor(CurrBColR, CurrBColG, CurrBColB, 1.0f, FALSE);
  SgUpdateColor();
  for (i = 0; i < SG_MAT_PROP_NO; i++)
    CurrSgDevP->setMatProp(i, MatProp[i]);
  CurrSgDevP->setTexture(Texture);
  CurrSgDevP->setPrecision(CurrPrec);
  CurrSgDevP->setLineWidth(CurrLineWidth);
  CurrSgDevP->setLineStyle(CurrLineStyle);
  for (i = 0; i < SG_FOG_PAR_NO; i++)
    CurrSgDevP->setFogPar(i, CurrFogPar[i]);
  CurrSgDevP->setFogMode(CurrFogMode);

  SgSetShadeAttr();
  SgSetTextAttr();
}

int
SgGetDim(void)
{
  return CurrSgDevP->dim;
}
