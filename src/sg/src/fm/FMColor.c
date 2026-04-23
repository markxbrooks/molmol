/*
************************************************************************
*
*   FMColor.c - FrameMaker color setting
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/fm/SCCS/s.FMColor.c
*   SCCS identification       : 1.8
*
************************************************************************
*/

#include "fm_color.h"

#include <stdio.h>

#include <sg.h>
#include <sg_plot.h>

#define STEPS_R 12
#define STEPS_G 18
#define STEPS_B 8

#define TAG_LEN 30

static void
plotViewportRect(void)
{
  float x, y, w, h;

  SgGetViewport(&x, &y, &w, &h);
  SgPlotOutputStr("  <Fill 0>\n");
  SgPlotOutputStr("  <NumPoints 4>\n");
  SgPlotOutputStr("  <Point ");
  SgPlotOutputFloat(x);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(y);
  SgPlotOutputStr(">\n");
  SgPlotOutputStr("  <Point ");
  SgPlotOutputFloat(x + w);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(y);
  SgPlotOutputStr(">\n");
  SgPlotOutputStr("  <Point ");
  SgPlotOutputFloat(x + w);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(y + h);
  SgPlotOutputStr(">\n");
  SgPlotOutputStr("  <Point ");
  SgPlotOutputFloat(x);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(y + h);
  SgPlotOutputStr(">\n");
}

void
SgFM3SetColor(float r, float g, float b, float a, BOOL fg)
{
  int rI, gI, bI, cI;
  char *str;

  SgPlotDoGamma(&r, &g, &b);

  if (r < 0.5f)
    rI = 0;
  else
    rI = 1;
  
  if (g < 0.5f)
    gI = 0;
  else
    gI = 1;
  
  if (b < 0.5f)
    bI = 0;
  else
    bI = 1;

  cI = 4 * rI + 2 * gI + bI;

  switch (cI) {
    case 0:
      str = "0";
      break;
    case 1:
      str = "4";
      break;
    case 2:
      str = "3";
      break;
    case 3:
      str = "5";
      break;
    case 4:
      str = "2";
      break;
    case 5:
      str = "6";
      break;
    case 6:
      str = "7";
      break;
    case 7:
      str = "1";
      break;
  }

  if (fg) {
    SgPlotOutputStr("<Separation ");
    SgPlotOutputStr(str);
    SgPlotOutputStr(">\n");
    SgPlotOutputStr("<Font <FSeparation ");
    SgPlotOutputStr(str);
    SgPlotOutputStr(">>\n");
  } else {
    SgPlotOutputStr("<Polygon <GroupID 1>\n");
    SgPlotOutputStr("  <Separation ");
    SgPlotOutputStr(str);
    SgPlotOutputStr(">\n");
    plotViewportRect();
    SgPlotOutputStr(">\n");
  }
}

static void
getColorTag(char *tag, int ri, int gi, int bi)
{
  (void) sprintf(tag, "rgb_%d_%d_%d", ri, gi, bi);
}

void
FM4WriteColorCatalog(void)
{
  int ri, gi, bi;
  float rf, gf, bf;
  float rg, gg, bg;
  float c, m, y, k;
  char tag[TAG_LEN];

  SgPlotOutputStr("<ColorCatalog\n");

  rf = 1.0f / (STEPS_R - 1);
  gf = 1.0f / (STEPS_G - 1);
  bf = 1.0f / (STEPS_B - 1);

  for (ri = 0; ri < STEPS_R; ri++)
    for (gi = 0; gi < STEPS_G; gi++)
      for (bi = 0; bi < STEPS_B; bi++) {
	getColorTag(tag, ri, gi, bi);

	rg = rf * ri;
	gg = gf * gi;
	bg = bf * bi;
	SgPlotDoGamma(&rg, &gg, &bg);

	c = 1.0f - rg;
	m = 1.0f - gg;
	y = 1.0f - bg;

	k = c;
	if (m < k)
	  k = m;
	if (y < k)
	  k = y;

	c -= k;
	m -= k;
	y -= k;

	SgPlotOutputStr("  <Color\n");
	SgPlotOutputStr("    <ColorTag ");
	SgPlotOutputStr(tag);
	SgPlotOutputStr(">\n");
	SgPlotOutputStr("    <ColorCyan ");
	SgPlotOutputFloat(100.0f * c);
	SgPlotOutputStr(">\n");
	SgPlotOutputStr("    <ColorMagenta ");
	SgPlotOutputFloat(100.0f * m);
	SgPlotOutputStr(">\n");
	SgPlotOutputStr("    <ColorYellow ");
	SgPlotOutputFloat(100.0f * y);
	SgPlotOutputStr(">\n");
	SgPlotOutputStr("    <ColorBlack ");
	SgPlotOutputFloat(100.0f * k);
	SgPlotOutputStr(">\n");
	SgPlotOutputStr("  >\n");
      }

  SgPlotOutputStr(">\n");
}

void
SgFM4SetColor(float r, float g, float b, float a, BOOL fg)
{
  int ri, gi, bi;
  char tag[TAG_LEN];

  /* Gamma corrected values were written to the color catalog, so
     we don't have to worry about gamma correction here. */
  ri = (int) (r * (STEPS_R - 1) + 0.5f);
  gi = (int) (g * (STEPS_G - 1) + 0.5f);
  bi = (int) (b * (STEPS_B - 1) + 0.5f);

  getColorTag(tag, ri, gi, bi);

  if (fg) {
    SgPlotOutputStr("<ObColor ");
    SgPlotOutputStr(tag);
    SgPlotOutputStr(">\n");
    SgPlotOutputStr("<Font <FColor ");
    SgPlotOutputStr(tag);
    SgPlotOutputStr(">>\n");
  } else {
    SgPlotOutputStr("<Polygon <GroupID 1>\n");
    SgPlotOutputStr("  <ObColor ");
    SgPlotOutputStr(tag);
    SgPlotOutputStr(">\n");
    plotViewportRect();
    SgPlotOutputStr(">\n");
  }
}
