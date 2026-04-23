/*
************************************************************************
*
*   VRML2Text.c - VRML2 text drawing
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/vrml2/SCCS/s.VRML2Text.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include "vrml2_text.h"

#include <stdlib.h>
#include <string.h>

#include <sg_map.h>
#include <sg_plot.h>
#include "vrml2_util.h"
#include "vrml2_color.h"

static float CurrSize2D, CurrSize;

static void
plotText(float x[3], float fullSize, char *str)
{
  char *cleanStr;
  int i;

  cleanStr = malloc(strlen(str) + 1);

  i = 0;
  while (*str != '\0') {
    if (*str != '_' && *str != '^' && *str != '{' && *str != '}')
      cleanStr[i++] = *str;

    str++;
  }
  cleanStr[i] = '\0';

  SgPlotOutputStr("Transform {\n");
  SgPlotOutputStr("translation ");
  VRML2PlotVect(x);
  SgPlotOutputStr("\n");
  SgPlotOutputStr("children [\n");
  SgPlotOutputStr("Shape {\n");

  VRML2PlotColor();

  SgPlotOutputStr("geometry Text {\n");

  SgPlotOutputStr("string \"");
  SgPlotOutputStr(cleanStr);
  SgPlotOutputStr("\"\n");

  free(cleanStr);

  SgPlotOutputStr("fontStyle FontStyle {\n");
  SgPlotOutputStr("size ");
  SgPlotOutputFloat(fullSize);
  SgPlotOutputStr("\n");
  SgPlotOutputStr("family \"SANS\"\n");
  SgPlotOutputStr("}\n");

  SgPlotOutputStr("}\n");

  SgPlotOutputStr("}\n");
  SgPlotOutputStr("]\n");
  SgPlotOutputStr("}\n");
}

void
SgVRML2DrawAnnot2D(float x[2], char *str)
{
  float xm[3];

  VRML2Map2D(xm, x);
  plotText(xm, CurrSize2D, str);
}

void
SgVRML2DrawText2D(float x[2], char *str)
{
  float xm[3];

  VRML2Map2D(xm, x);
  plotText(xm, CurrSize2D, str);
}

void
SgVRML2DrawAnnot(float x[3], char *str)
{
  float xm[3];

  SgMapModelPoint(xm, x);
  plotText(xm, CurrSize, str);
}

void
SgVRML2DrawText(float x[3], char *str)
{
  float xm[3];

  SgMapModelPoint(xm, x);
  plotText(xm, CurrSize, str);
}

static float
mapSize(float size)
{
  float xm[3], x[2];

  x[0] = 0.0f;
  x[1] = size;

  VRML2Map2D(xm, x);

  return xm[1];
}

void
SgVRML2SetFontSize2D(float size)
{
  CurrSize2D = mapSize(size);
}

void
SgVRML2SetFontSize(float size)
{
  CurrSize = mapSize(size);
}
