/*
************************************************************************
*
*   PSColor.c - PostScript color setting
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/ps/SCCS/s.PSColor.c
*   SCCS identification       : 1.11
*
************************************************************************
*/

#include "ps_color.h"

#include <sg.h>
#include <sg_plot.h>
#include <sg_get.h>

static float ColR, ColG, ColB;
static BOOL ColChanged = FALSE;

static void
outputColor(float r, float g, float b)
{
  SgPlotDoGamma(&r, &g, &b);

  SgPlotOutputFloat(r);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(g);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(b);
  SgPlotOutputStr(" co\n");
}

void
SgPSSetColor(float r, float g, float b, float a, BOOL fg)
{
  float fgR, fgG, fgB;
  BOOL isWhite;
  float x, y, w, h;

  if (fg) {
    ColR = r;
    ColG = g;
    ColB = b;
    ColChanged = TRUE;
  } else {
    /* don't paint white background, it's white anyway */
    isWhite  = (r == 1.0f && g == 1.0f && b == 1.0f);

    if (! isWhite) {
      SgGetColor(&fgR, &fgG, &fgB);
      outputColor(r, g, b);
    }

    SgGetViewport(&x, &y, &w, &h);
    SgPlotOutputStr("np\n");
    SgPlotOutputFloat(x);
    SgPlotOutputStr(" ");
    SgPlotOutputFloat(y);
    SgPlotOutputStr(" mt\n");
    SgPlotOutputFloat(x + w);
    SgPlotOutputStr(" ");
    SgPlotOutputFloat(y);
    SgPlotOutputStr(" lt\n");
    SgPlotOutputFloat(x + w);
    SgPlotOutputStr(" ");
    SgPlotOutputFloat(y + h);
    SgPlotOutputStr(" lt\n");
    SgPlotOutputFloat(x);
    SgPlotOutputStr(" ");
    SgPlotOutputFloat(y + h);
    SgPlotOutputStr(" lt\n");

    if (isWhite) {
      SgPlotOutputStr("cp clip\n");
    } else {
      SgPlotOutputStr("cp clip fi\n");
      outputColor(fgR, fgG, fgB);
    }
  }
}

void
PSInvalidateColor(void)
{
  /* Must be called by functions that modify the current color in
     the output file, so that the next call to PSUpdateColor() brings
     it in sync with the current color we have stored. */
  ColChanged = TRUE;
}

void
PSUpdateColor(void)
{
  if (! ColChanged)
    return;

  outputColor(ColR, ColG, ColB);
  ColChanged = FALSE;
}
