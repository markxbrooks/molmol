/*
************************************************************************
*
*   DefMarker.c - default marker drawing for Sg
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/def/SCCS/s.DefMarker.c
*   SCCS identification       : 1.8
*
************************************************************************
*/

#include <sg_def.h>

#include <sg.h>
#include <sg_get.h>
#include <sg_map.h>
#include "def_hidden.h"
#include "def_shade.h"

void
SgDefDrawMarkers2D(float x[][2], int n)
{
  int i;

  for (i = 0; i < n; i++)
    SgDrawDisc2D(x[i], 0.001f);
}

void
SgDefDrawMarkers(float x[][3], int n)
{
  float xm[SG_POLY_SIZE][3], xEye[3], zEye[SG_POLY_SIZE];
  BOOL hidden, fog;
  float r, g, b, fr, fg, fb;
  float xm2[SG_POLY_SIZE][2];
  float zSum;
  int inI, outI, i;

  outI = 0;
  for (inI = 0; inI < n; inI++)
    if (SgMapPoint(xm[outI], x[inI]) > 0) {
      SgMapEyePoint(xEye, x[inI]);
      zEye[outI] = xEye[2];
      outI++;
    }
  n = outI;

  hidden = SgGetFeature(SG_FEATURE_DEPTH_SORT) ||
      SgGetFeature(SG_FEATURE_HIDDEN);
  fog = SgGetFeature(SG_FEATURE_SHADE) && DefFogEnabled();

  if (fog)
    SgGetColor(&r, &g, &b);
  else if (hidden)
    SgGetColor(&fr, &fg, &fb);

  if (hidden) {
    for (i = 0; i < n; i++) {
      if (fog) {
	fr = r;
	fg = g;
	fb = b;
	DefFog(&fr, &fg, &fb, zEye[i]);
      }
      DefHiddenMarker(xm[i], fr, fg, fb);
    }
  } else {
    zSum = 0.0f;
    for (i = 0; i < n; i++) {
      xm2[i][0] = xm[i][0];
      xm2[i][1] = xm[i][1];
      zSum += zEye[i];
    }

    if (fog) {
      fr = r;
      fg = g;
      fb = b;
      DefFog(&fr, &fg, &fb, zSum / n);
      SgSetColor(fr, fg, fb);
    }
    SgDrawMarkers2D(xm2, n);
  }

  if (fog && ! hidden)
    SgSetColor(r, g, b);
}
