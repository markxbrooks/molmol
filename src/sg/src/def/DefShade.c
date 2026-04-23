/*
************************************************************************
*
*   DefShade.c - default shading for Sg
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/def/SCCS/s.DefShade.c
*   SCCS identification       : 1.17
*
************************************************************************
*/

#include <sg_def.h>
#include "def_shade.h"

#include <math.h>

#include <mat_vec.h>
#include <sg_get.h>
#include <sg_map.h>

static float CurrAmb, CurrDiff, CurrSpec, CurrShininess;
static SgShadeModel CurrShadeModel;
static SgFogMode CurrFogMode;
static float CurrFogDens, CurrFogStart, CurrFogEnd;
static SgLightState CurrLightState;
static Vec3 CurrLightPos, TransfLightPos, LightV;

void
SgDefSetMatProp(SgMatProp prop, float val)
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
      CurrShininess = val;
      break;
    default:
      /* not supported */;
  }
}

void
SgDefSetShadeModel(SgShadeModel model)
{
  CurrShadeModel = model;
}

SgShadeModel
DefGetShadeModel(void)
{
  return CurrShadeModel;
}

void
SgDefSetFogMode(SgFogMode mode)
{
  CurrFogMode = mode;
}

BOOL
DefFogEnabled(void)
{
  return CurrFogMode != SG_FOG_MODE_OFF;
}

void
SgDefSetFogPar(SgFogPar par, float val)
{
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
SgDefSetLight(SgLightState state)
{
  CurrLightState = state;
}

void
SgDefSetLightPosition(float x[3])
{
  Vec3Copy(CurrLightPos, x);

  SgMapEyePoint(TransfLightPos, CurrLightPos);
  SgMapEyeVect(LightV, CurrLightPos);
  if (LightV[0] != 0.0f || LightV[1] != 0.0f || LightV[2] != 0.0f)
    Vec3Norm(LightV);
}

void
DefShade(float *r, float *g, float *b, float x[3], float nv[3])
{
  Vec3 vLight;
  float prod, fact, spec;

  fact = CurrAmb;

  if (CurrLightState == SG_LIGHT_POINT) {
    Vec3Copy(vLight, TransfLightPos);
    Vec3Sub(vLight, x);
    Vec3Norm(vLight);
  } else {
    Vec3Copy(vLight, LightV);
  }

  prod = Vec3Scalar(vLight, nv);

  if (prod > 0.0f)
    fact += CurrDiff * prod;

  *r *= fact;
  *g *= fact;
  *b *= fact;

  if (prod > 0.0f && CurrSpec > 0.0f) {
    prod = 2.0f * prod * nv[2] - vLight[2];
    if (prod > 0.0f) {
      spec = CurrSpec * powf(prod, CurrShininess);
      *r += spec;
      *g += spec;
      *b += spec;
    }
  }

  if (*r > 1.0f)
    *r = 1.0f;

  if (*g > 1.0f)
    *g = 1.0f;

  if (*b > 1.0f)
    *b = 1.0f;
}

void
DefFog(float *r, float *g, float *b, float z)
{
  float blendFact, br, bg, bb;

  if (CurrFogMode == SG_FOG_MODE_OFF)
    return;

  /* z is in eye coordinates, always negative in front of viewer */
  if (z > 0.0f)
    return;
  
  switch (CurrFogMode) {
    case SG_FOG_MODE_EXP:
      blendFact = expf(CurrFogDens * z);
      break;
    case SG_FOG_MODE_EXP2:
      blendFact = expf(- CurrFogDens * CurrFogDens * z * z);
      break;
    case SG_FOG_MODE_LINEAR:
      if (- z < CurrFogStart)
	return;
      else if (- z > CurrFogEnd)
	blendFact = 0.0f;
      else
	blendFact = (z + CurrFogEnd) / (CurrFogEnd - CurrFogStart);
      break;
  }

  *r *= blendFact;
  *g *= blendFact;
  *b *= blendFact;

  blendFact = 1.0f - blendFact;
  SgGetBackgroundColor(&br, &bg, &bb);

  *r += blendFact * br;
  *g += blendFact * bg;
  *b += blendFact * bb;
}
