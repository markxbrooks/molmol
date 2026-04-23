/*
************************************************************************
*
*   POVText.c - POV text drawing
*
*   Copyright (c) 1994-96
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/pov/SCCS/s.POVText.c
*   SCCS identification       : 1.18
*
************************************************************************
*/

#include "pov_text.h"

#include <string.h>

#include <sg_map.h>
#include <sg_plot.h>
#include "pov_util.h"
#include "pov_color.h"
#include "pov_clip.h"

#define CHAR_W 0.4f  /* approximate width in definiton file */
#define CHAR_H 0.7f  /* height in definiton file */

#define SUP_SCALE 0.65f
#define SUP_OFFS 0.4f
#define SUB_OFFS -0.25f
#define CHAR_THICK 0.01f

static float CurrSize2D, CurrSize;

static float
showStr(char *str, int n, float x[3], float pos, float offs, float size)
{
  char buf[2];
  float newPos;
  int i;

  buf[1] = '\0';
  newPos = pos;
  i = 0;

  while (i < n) {
    SgPlotOutputStr("text {\n");

    if (str[i] == '\\' && i < n - 1) {
      SgPlotOutputStr("ttf \"symbol.ttf\"\n");
      SgPlotOutputStr("\"");

      while (i < n && str[i] == '\\') {
	i++;
	if (i == n)
	  break;
	buf[0] = str[i];
	SgPlotOutputStr(buf);
	newPos += CHAR_W * size;
	i++;
      }
    } else {
      SgPlotOutputStr("ttf \"arial.ttf\"\n");
      SgPlotOutputStr("\"");

      while (i < n && str[i] != '\\') {
	buf[0] = str[i];
	SgPlotOutputStr(buf);
	newPos += CHAR_W * size;
	i++;
      }
    }

    SgPlotOutputStr("\"\n");

    SgPlotOutputFloat(CHAR_THICK);
    SgPlotOutputStr(", <0, 0, 0>\n");

    SgPlotOutputStr("scale ");
    SgPlotOutputFloat(size);
    SgPlotOutputStr("\n");
    SgPlotOutputStr("translate <");
    SgPlotOutputFloat(x[0] + pos);
    SgPlotOutputStr(", ");
    SgPlotOutputFloat(x[1] + offs);
    SgPlotOutputStr(", ");
    SgPlotOutputFloat(x[2]);
    SgPlotOutputStr(">\n");
    POVPlotColor();
    (void) POVClip(x, 0.0f);
    SgPlotOutputStr("}\n");

    pos = newPos;
  }

  return newPos;
}

static void
plotText(float x[3], float fullSize, char *str)
{
  float pos;
  int startI, currI;
  float size, offs;

  pos = 0.0f;
  startI = 0;
  currI = 0;

  for (;;) {
    if (str[currI] == '_' || str[currI] == '^') {
      pos = showStr(str + startI, currI - startI, x, pos, 0.0f, fullSize);

      size = SUP_SCALE * fullSize;
      if (str[currI] == '_')
	offs = SUB_OFFS * CHAR_H * fullSize;
      else
	offs = SUP_OFFS * CHAR_H * fullSize;

      currI++;

      if (str[currI] == '{') {
	currI++;
	startI = currI;
	while (str[currI] != '}' && str[currI] != '\0')
	  currI++;
	pos = showStr(str + startI, currI - startI, x, pos, offs, size);
      } else if (str[currI] == '\\') {
	pos = showStr(str + currI, 2, x, pos, offs, size);
	currI++;
      } else {
	pos = showStr(str + currI, 1, x, pos, offs, size);
      }

      if (str[currI] != '\0')
	currI++;
      
      startI = currI;
    } else if (str[currI] == '\0') {
      pos = showStr(str + startI, currI - startI, x, pos, 0.0f, fullSize);
      break;
    } else {
      currI++;
    }
  }
}

void
SgPOVDrawAnnot2D(float x[2], char *str)
{
  float xm[3];

  POVUpdateColor();
  POVMap2D(xm, x);
  plotText(xm, CurrSize2D, str);
}

void
SgPOVDrawText2D(float x[2], char *str)
{
  float xm[3];

  POVUpdateColor();
  POVMap2D(xm, x);
  plotText(xm, CurrSize2D, str);
}

void
SgPOVDrawAnnot(float x[3], char *str)
{
  float xm[3];

  POVUpdateColor();
  SgMapModelPoint(xm, x);
  plotText(xm, CurrSize, str);
}

void
SgPOVDrawText(float x[3], char *str)
{
  float xm[3];

  POVUpdateColor();
  SgMapModelPoint(xm, x);
  plotText(xm, CurrSize, str);
}

static float
mapSize(float size)
{
  float xm[3], x[2];

  x[0] = 0.0f;
  x[1] = size;

  POVMap2D(xm, x);

  return xm[1] / CHAR_H;
}

void
SgPOVSetFontSize2D(float size)
{
  CurrSize2D = mapSize(size);
}

void
SgPOVSetFontSize(float size)
{
  CurrSize = mapSize(size);
}
