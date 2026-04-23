/*
************************************************************************
*
*   DefText.c - default text drawing for Sg
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/def/SCCS/s.DefText.c
*   SCCS identification       : 1.12
*
************************************************************************
*/

#include <sg_def.h>

#include <string.h>

#include <sg.h>
#include <sg_get.h>
#include <sg_map.h>
#include "def_hidden.h"
#include "def_shade.h"
#include "hershey.h"

#define MAX_STROKE_SIZE 256
#define MAX_COORD 22

#define SUP_SCALE 0.65f
#define SUP_OFFS 0.4f
#define SUB_OFFS -0.25f

static float FontScale2D, FontSize;

void
SgDefDrawAnnot2D(float x[2], char *str)
{
  SgDrawText2D(x, str);
}

static char *
getDescr(char ch, BOOL greek)
{
  int pos;

  pos = ch - ' ';

  if (greek) {
    if (pos < 0 || pos >= sizeof(GreekCoord) / sizeof(*GreekCoord))
      pos = 0;
    return GreekCoord[pos];
  } else {
    if (pos < 0 || pos >= sizeof(FontCoord) / sizeof(*FontCoord))
      pos = 0;
    return FontCoord[pos];
  }
}

static float
drawText(float *x, char *str, float fullScale, int dim)
{
  float xPos, yPos;
  BOOL inPar;
  float scale, offs;
  char *descr;
  float stroke2[MAX_STROKE_SIZE][2];
  float stroke3[MAX_STROKE_SIZE][3];
  int pointNo, strokeSize;
  int i;

  xPos = x[0];
  yPos = x[1];
  inPar = FALSE;

  while (*str != '\0') {
    if (*str == '_') {
      str++;
      if (*str == '{') {
	inPar = TRUE;
	str++;
      }
      scale = SUP_SCALE * fullScale;
      offs = SUB_OFFS * MAX_COORD * fullScale;
    } else if (*str == '^') {
      str++;
      if (*str == '{') {
	inPar = TRUE;
	str++;
      }
      scale = SUP_SCALE * fullScale;
      offs = SUP_OFFS * MAX_COORD * fullScale;
    } else {
      if (*str == '}') {
	inPar = FALSE;
	str++;
	continue;
      }
      if (! inPar) {
	scale = fullScale;
	offs = 0.0f;
      }
    }

    if (*str == '\\') {
      str++;
      descr = getDescr(*str, TRUE);
    } else {
      descr = getDescr(*str, FALSE);
    }


    pointNo = strlen(descr) / 2 - 1;
    strokeSize = 0;

    for (i = 1; i <= pointNo; i++) {
      if (dim == 2) {
	if (descr[2 * i] == ' ') {
	  SgDrawPolyline2D(stroke2, strokeSize);
	  strokeSize = 0;
	} else {
	  stroke2[strokeSize][0] =
	      xPos + (descr[2 * i] - descr[0]) * scale;
	  stroke2[strokeSize][1] =
	      yPos + (92 - descr[2 * i + 1]) * scale + offs;
	  strokeSize++;
	}
      } else if (dim == 3) {
	if (descr[2 * i] == ' ') {
	  SgDrawPolyline(stroke3, strokeSize);
	  strokeSize = 0;
	} else {
	  stroke3[strokeSize][0] =
	      xPos + (descr[2 * i] - descr[0]) * scale;
	  stroke3[strokeSize][1] =
	      yPos + (92 - descr[2 * i + 1]) * scale + offs;
	  stroke3[strokeSize][2] = x[2];
	  strokeSize++;
	}
      }
    }

    if (strokeSize > 0) {
      if (dim == 2)
	SgDrawPolyline2D(stroke2, strokeSize);
      else if (dim == 3)
	SgDrawPolyline(stroke3, strokeSize);
    }

    xPos += (descr[1] - descr[0]) * scale;

    str++;
  }

  return xPos - x[0];
}

void
SgDefDrawText2D(float x[2], char *str)
{
  (void) drawText(x, str, FontScale2D, 2);
}

void
SgDefDrawAnnot(float x[3], char *str)
{
  float xm[3], xEye[3];
  BOOL hidden, fog;
  float r, g, b, fr, fg, fb;

  if (SgMapPoint(xm, x) == 0)
    return;
  
  hidden = SgGetFeature(SG_FEATURE_DEPTH_SORT) ||
      SgGetFeature(SG_FEATURE_HIDDEN);
  fog = SgGetFeature(SG_FEATURE_SHADE) && DefFogEnabled();

  if (fog) {
    SgGetColor(&r, &g, &b);
    fr = r;
    fg = g;
    fb = b;
    SgMapEyePoint(xEye, x);
    DefFog(&fr, &fg, &fb, xEye[2]);
  } else if (hidden) {
    SgGetColor(&fr, &fg, &fb);
  }

  if (hidden) {
    DefHiddenAnnot(xm, str, fr, fg, fb);
  } else if (fog) {
    SgSetColor(fr, fg, fb);
    SgDrawAnnot2D(xm, str);
    SgSetColor(r, g, b);
  } else {
    SgDrawAnnot2D(xm, str);
  }
}

void
SgDefDrawText(float x[3], char *str)
{
  float xm[3], xEye[3], prevSize;
  BOOL hidden, fog;
  float r, g, b, fr, fg, fb;

  if (SgGetDim() == 3) {
    (void) drawText(x, str, FontSize * SgGetDepthFact(0.0f) / MAX_COORD, 3);
    return;
  }

  if (SgMapPoint(xm, x) == 0)
    return;

  prevSize = FontSize;
  SgSetFontSize(FontSize * SgGetDepthFact(0.0f) / SgGetDepthFact(x[2]));

  hidden = SgGetFeature(SG_FEATURE_DEPTH_SORT) ||
      SgGetFeature(SG_FEATURE_HIDDEN);
  fog = SgGetFeature(SG_FEATURE_SHADE) && DefFogEnabled();

  if (fog) {
    SgGetColor(&r, &g, &b);
    fr = r;
    fg = g;
    fb = b;
    SgMapEyePoint(xEye, x);
    DefFog(&fr, &fg, &fb, xEye[2]);
  } else if (hidden) {
    SgGetColor(&fr, &fg, &fb);
  }

  if (hidden) {
    DefHiddenText(xm, str, fr, fg, fb);
  } else if (fog) {
    SgSetColor(fr, fg, fb);
    SgDrawText2D(xm, str);
    SgSetColor(r, g, b);
  } else {
    SgDrawText2D(xm, str);
  }

  SgSetFontSize(prevSize);
}

void
SgDefSetFont(char *name)
{
}

void
SgDefSetFontSize2D(float size)
{
  FontScale2D = size / MAX_COORD;
}

void
SgDefSetFontSize(float size)
{
  FontSize = size;
}

float
SgDefGetTextWidth(char *str, float size)
{
  float x;

  x = 0.0f;
  return drawText(&x, str, size / MAX_COORD, 1);
}
