/*
************************************************************************
*
*   DefPoly.c - default polygon drawing for Sg
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
*   Date of last modification : 96/03/05
*   Pathname of SCCS file     : /sgiext/molmol/sg/src/def/SCCS/s.DefPoly.c
*   SCCS identification       : 1.6
*
************************************************************************
*/

#include <sg_def.h>

#include <mat_vec.h>
#include <sg.h>
#include <sg_get.h>
#include <sg_map.h>
#include "def_hidden.h"
#include "def_shade.h"

void
SgDefDrawPolygon(float x[][3], int n, float nv[3])
{
  float xm[SG_POLY_SIZE][3];
  BOOL backface, twoSide, hidden, shade, fog;
  Vec3 xEye, v1, v2, nvm;
  float xm2[SG_POLY_SIZE][2];
  float r, g, b, lr, lg, lb;
  int i;

  n = SgMapPoly(xm, x, n);
  if (n == 0)
    return;

  backface = SgGetFeature(SG_FEATURE_BACKFACE);
  twoSide = SgGetFeature(SG_FEATURE_TWO_SIDE);
  hidden = SgGetFeature(SG_FEATURE_DEPTH_SORT) ||
      SgGetFeature(SG_FEATURE_HIDDEN);
  shade = SgGetFeature(SG_FEATURE_SHADE);
  fog = DefFogEnabled();

  if (backface || shade)
    SgMapEyePoint(xEye, x[0]);

  if (backface || twoSide) {
    Vec3Copy(v1, x[1]);
    Vec3Sub(v1, x[0]);
    Vec3Copy(v2, x[2]);
    Vec3Sub(v2, x[0]);
    Vec3Cross(v1, v2);
    SgMapEyeVect(v1, v1);
    if (SgBackfacing(xEye, v1)) {
      if (backface)
	return;
      nvm[0] = - nv[0];
      nvm[1] = - nv[1];
      nvm[2] = - nv[2];
      SgMapEyeVect(nvm, nvm);
    } else if (shade) {
      SgMapEyeVect(nvm, nv);
    }
  } else if (shade) {
    SgMapEyeVect(nvm, nv);
  }

  if (shade) {
    Vec3Norm(nvm);
    SgGetColor(&r, &g, &b);
    lr = r;
    lg = g;
    lb = b;
    DefShade(&lr, &lg, &lb, xEye, nvm);
    if (fog)
      DefFog(&lr, &lg, &lb, xEye[2]);
  } else if (hidden) {
    SgGetColor(&lr, &lg, &lb);
  }

  if (hidden) {
    DefHiddenPolygon(xm, n, lr, lg, lb);
    return;
  }

  for (i = 0; i < n; i++) {
    xm2[i][0] = xm[i][0];
    xm2[i][1] = xm[i][1];
  }

  if (shade) {
    SgSetColor(lr, lg, lb);
    SgDrawPolygon2D(xm2, n);
    SgSetColor(r, g, b);
  } else {
    SgDrawPolygon2D(xm2, n);
  }
}
