/*
************************************************************************
*
*   PSText.c - PostScript text drawing
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/ps/SCCS/s.PSText.c
*   SCCS identification       : 1.7
*
************************************************************************
*/

#include "ps_text.h"

#include <sg.h>
#include <sg_map.h>
#include <sg_plot.h>
#include "ps_color.h"

#define SUP_SCALE 0.65f
#define SUP_OFFS 0.4f
#define SUB_OFFS -0.25f

static float FullSize, CurrSize, Offs;
static BOOL GreekOn = FALSE;

static void
setFontSize(float size)
{
  SgPlotOutputStr("/Helvetica ");
  SgPlotOutputFloat(size);
  SgPlotOutputStr(" sf\n");

  CurrSize = size;
}

static void
setGreek(BOOL onOff, BOOL inStr)
{
  if (onOff == GreekOn)
    return;

  if (inStr)
    SgPlotOutputStr(") sh\n");

  if (onOff)
    SgPlotOutputStr("/Symbol ");
  else
    SgPlotOutputStr("/Helvetica ");

  SgPlotOutputFloat(CurrSize);
  SgPlotOutputStr(" sf\n");

  if (inStr)
    SgPlotOutputStr("(");

  GreekOn = onOff;
}

static void
doOffs(float r)
{
  Offs += r;
  SgPlotOutputFloat(r);
  SgPlotOutputStr(" my\n");
}

static void
setSup(void)
{
  setFontSize(SUP_SCALE * FullSize);
  doOffs(SUP_OFFS * FullSize);
}

static void
setSub(void)
{
  setFontSize(SUP_SCALE * FullSize);
  doOffs(SUB_OFFS * FullSize);
}

static void
setNorm(void)
{
  setFontSize(FullSize);
  doOffs(- Offs);
}

static void
showStr(char *str, int n)
{
  char buf[2];
  BOOL greekNext;
  int i;

  if (n == 0)
    return;

  SgPlotOutputStr("(");

  buf[1] = '\0';
  greekNext = FALSE;
  for (i = 0; i < n; i++) {
    if (str[i] == '\\') {
      greekNext = TRUE;
    } else {
      setGreek(greekNext, TRUE);
      buf[0] = str[i];
      SgPlotOutputStr(buf);
      greekNext = FALSE;
    }
  }

  SgPlotOutputStr(") sh\n");

  setGreek(FALSE, FALSE);
}

void
SgPSDrawText(float x[2], char *str)
{
  float xm[2];
  int startI, currI;

  PSUpdateColor();

  SgVpMapPoint(xm, x);
  SgPlotOutputFloat(xm[0]);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(xm[1]);
  SgPlotOutputStr(" mt\n");

  Offs = 0.0f;

  startI = 0;
  currI = 0;
  for (;;) {
    if (str[currI] == '_' || str[currI] == '^') {
      showStr(str + startI, currI - startI);

      if (str[currI] == '_')
	setSub();
      else
	setSup();

      currI++;

      if (str[currI] == '{') {
	currI++;
	startI = currI;
	while (str[currI] != '}' && str[currI] != '\0')
	  currI++;
	showStr(str + startI, currI - startI);
      } else if (str[currI] == '\\') {
	showStr(str + currI, 2);
	currI++;
      } else {
	showStr(str + currI, 1);
      }
      setNorm();

      if (str[currI] != '\0')
	currI++;

      startI = currI;
    } else if (str[currI] == '\0') {
      showStr(str + startI, currI - startI);
      break;
    } else {
      currI++;
    }
  }
}

void
SgPSSetFontSize(float size)
{
  float x, y, w, h;

  SgGetViewport(&x, &y, &w, &h);
  FullSize = 1.5f * size * (0.5f * h);
  setFontSize(FullSize);
}
