/*
************************************************************************
*
*   GLFeature.c - GL enable/disable features
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/gl/SCCS/s.GLFeature.c
*   SCCS identification       : 1.6
*
************************************************************************
*/

#include "gl_feature.h"

#include <gl/gl.h>

static BOOL TranspOn = FALSE, AntiOn = FALSE;

void
SgGLSetFeature(SgFeature feature, BOOL onOff)
{
  static float lt[3];

  switch (feature) {
    case SG_FEATURE_TRANSPARENCY:
      if (onOff)
	blendfunction(BF_SA, BF_MSA);
      else if (! AntiOn)
	blendfunction(BF_ONE, BF_ZERO);
      TranspOn = onOff;
      break;
    case SG_FEATURE_ANTI_ALIASING:
      if (onOff) {
	linesmooth(SML_ON);
	blendfunction(BF_SA, BF_MSA);
      } else {
	linesmooth(SML_OFF);
	if (! TranspOn)
	  blendfunction(BF_ONE, BF_ZERO);
      }
      AntiOn = onOff;
      break;
    case SG_FEATURE_BACKFACE:
      backface(onOff);
      break;
    case SG_FEATURE_TWO_SIDE:
      lt[0] = TWOSIDE;
      if (onOff)
	lt[1] = 1.0f;
      else
	lt[1] = 0.0f;
      lt[2] = LMNULL;
      lmdef(DEFLMODEL, 1, 3, lt);
      break;
    case SG_FEATURE_HIDDEN:
      zbuffer(onOff);
      break;
  }
}
