/*
************************************************************************
*
*   SgPlot.c - Sg plotting
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/sgdev/SCCS/s.SgPlot.c
*   SCCS identification       : 1.11
*
************************************************************************
*/

#include <stdio.h>
#include <math.h>

#include <sg_dev.h>
#include <sg_plot.h>
#include "sg_name.h"
#include "sg_set.h"
#include "sg_view.h"

static SgDev *OldDevP;
static char *OldDevName;
static float OldVpX, OldVpY, OldVpW, OldVpH;

static FILE *PlotFileP;
static SgPlotOrient Orient;
static float Gamma;

SgRes
SgStartPlot(char *dev, char *fileName,
    SgPlotOrient orient, float gamma, float x, float y, float w, float h)
{
  PlotFileP = fopen(fileName, "w");
  if (PlotFileP == NULL)
    return SG_RES_ERR;

  Orient = orient;
  Gamma = gamma;

  OldDevP = CurrSgDevP;
  OldDevName = SgGetDeviceName();
  SgGetViewport(&OldVpX, &OldVpY, &OldVpW, &OldVpH);
  SgSetDevice(dev);
  SgSetViewport(x, y, w, h);  /* must be set for calculating font size */
  SgInit(NULL, 0);
  SgUpdateViewport();
  SgUpdateView();

  return SG_RES_OK;
}

void
SgEndPlot(void)
{
  (void) SgCleanup();
  (void) fclose(PlotFileP);
  CurrSgDevP = OldDevP;
  SgSetDeviceName(OldDevName);
  SgSetViewport(OldVpX, OldVpY, OldVpW, OldVpH);
  SgUpdateView();
  SgSetAttr();
}

SgPlotOrient
SgPlotGetOrient(void)
{
  return Orient;
}

float
SgPlotGetGamma(void)
{
  return Gamma;
}

void
SgPlotDoGamma(float *rP, float *gP, float *bP)
{
  if (Gamma == 1.0f)
    return;

  *rP = powf(*rP, Gamma);
  *gP = powf(*gP, Gamma);
  *bP = powf(*bP, Gamma);
}

void
SgPlotOutputStr(char *s)
{
  (void) fprintf(PlotFileP, "%s", s);
}

void
SgPlotOutputInt(int i)
{
  (void) fprintf(PlotFileP, "%d", i);
}

void
SgPlotOutputFloat(float f)
{
  if (f < (float) 1.0e-30 && f > (float) -1.0e-30)
    /* some PostScript printers don't like very small numbers */
    f = 0.0f;

  (void) fprintf(PlotFileP, "%.4g", f);
}
