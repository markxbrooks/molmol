/*
************************************************************************
*
*   PSInit.c - init PostScript device
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/ps/SCCS/s.PSInit.c
*   SCCS identification       : 1.9
*
************************************************************************
*/

#include "ps_init.h"

#include <sg.h>
#include <sg_plot.h>

/* 1 inch = 72 points */
#define POINT_SCALE 72

SgRes
SgPSInit(int *argcP, char *argv[])
{
  float x, y, w, h, xm, ym;

  SgPlotOutputStr("%!PS-Adobe-3.0 EPSF-3.0\n");
  SgPlotOutputStr("%%Creator: MOLMOL\n");
  SgPlotOutputStr("%%Pages: 1\n");

  SgGetViewport(&x, &y, &w, &h);
  xm = x + 0.5f * w;
  ym = y + 0.5f * h;

  SgPlotOutputStr("%%BoundingBox: ");
  if (SgPlotGetOrient() == SG_PLOT_ROT90) {
    SgPlotOutputInt((int) ((xm - 0.5f * h) * POINT_SCALE));
    SgPlotOutputStr(" ");
    SgPlotOutputInt((int) ((ym - 0.5f * w) * POINT_SCALE));
    SgPlotOutputStr(" ");
    SgPlotOutputInt((int) ((xm + 0.5f * h) * POINT_SCALE));
    SgPlotOutputStr(" ");
    SgPlotOutputInt((int) ((ym + 0.5f * w) * POINT_SCALE));
  } else {
    SgPlotOutputInt((int) (x * POINT_SCALE));
    SgPlotOutputStr(" ");
    SgPlotOutputInt((int) (y * POINT_SCALE));
    SgPlotOutputStr(" ");
    SgPlotOutputInt((int) ((x + w) * POINT_SCALE));
    SgPlotOutputStr(" ");
    SgPlotOutputInt((int) ((y + h) * POINT_SCALE));
  }
  SgPlotOutputStr("\n");

  SgPlotOutputStr("%%EndComments\n");

  SgPlotOutputStr("%%BeginProlog\n");

  SgPlotOutputStr("/cl /clip load def\n");
  SgPlotOutputStr("/co /setrgbcolor load def\n");
  SgPlotOutputStr("/cp /closepath load def\n");
  SgPlotOutputStr("/fi /fill load def\n");
  SgPlotOutputStr("/lw /setlinewidth load def\n");
  SgPlotOutputStr("/lt /lineto load def\n");
  SgPlotOutputStr("/mt /moveto load def\n");
  SgPlotOutputStr("/np /newpath load def\n");
  SgPlotOutputStr("/re /grestore load def\n");
  SgPlotOutputStr("/sh /show load def\n");
  SgPlotOutputStr("/st /stroke load def\n");

  SgPlotOutputStr("/do { 2 array astore 0 setdash } def\n");
  SgPlotOutputStr("/df { [] 0 setdash } def\n");

  SgPlotOutputStr("/sn { gsave newpath } def\n");
  SgPlotOutputStr("/ci { newpath 0 360 arc stroke } def\n");
  SgPlotOutputStr("/di { newpath 0 360 arc fill } def\n");
  SgPlotOutputStr("/ls { newpath moveto lineto stroke } def\n");
  SgPlotOutputStr("/my { 0 exch rmoveto } def\n");
  SgPlotOutputStr("/sf { exch findfont exch scalefont setfont } def\n");
  SgPlotOutputStr("/tr { newpath moveto lineto lineto closepath fill } def\n");

  SgPlotOutputStr("/im {\n");
  SgPlotOutputStr("  translate scale\n");
  SgPlotOutputStr("  /istr 2 index 3 mul string def\n");
  SgPlotOutputStr("  8 2 index 2 index matrix scale\n");
  SgPlotOutputStr("  {currentfile istr readhexstring pop} false 3\n");
  SgPlotOutputStr("  colorimage\n");
  SgPlotOutputStr("} def\n");

  /* copied from tops output, compatibility for devices without colorimage */
  SgPlotOutputStr("/bwproc {\n");
  SgPlotOutputStr("  rgbproc\n");
  SgPlotOutputStr("  dup length 3 idiv string 0 3 0\n");
  SgPlotOutputStr("  5 -1 roll {\n");
  SgPlotOutputStr("  add 2 1 roll 1 sub dup 0 eq\n");
  SgPlotOutputStr("  { pop 3 idiv 3 -1 roll dup 4 -1 roll dup\n");
  SgPlotOutputStr("    3 1 roll 5 -1 roll put 1 add 3 0 }\n");
  SgPlotOutputStr("  { 2 1 roll } ifelse\n");
  SgPlotOutputStr("  } forall\n");
  SgPlotOutputStr("  pop pop pop\n");
  SgPlotOutputStr("} def\n");
  SgPlotOutputStr("systemdict /colorimage known not {\n");
  SgPlotOutputStr("  /colorimage {\n");
  SgPlotOutputStr("    pop pop\n");
  SgPlotOutputStr("    /rgbproc exch def\n");
  SgPlotOutputStr("    { bwproc } image\n");
  SgPlotOutputStr("  } def\n");
  SgPlotOutputStr("} if\n");

  SgPlotOutputStr("%%EndProlog\n");

  SgPlotOutputStr("%%BeginSetup\n");

  SgPlotOutputInt(POINT_SCALE);
  SgPlotOutputStr(" ");
  SgPlotOutputInt(POINT_SCALE);
  SgPlotOutputStr(" ");
  SgPlotOutputStr("scale\n");

  if (SgPlotGetOrient() == SG_PLOT_ROT90) {
    SgPlotOutputFloat(xm);
    SgPlotOutputStr(" ");
    SgPlotOutputFloat(ym);
    SgPlotOutputStr(" translate\n");
    SgPlotOutputStr("90 rotate\n");
    SgPlotOutputFloat(- xm);
    SgPlotOutputStr(" ");
    SgPlotOutputFloat(- ym);
    SgPlotOutputStr(" translate\n");
  }

  /* save transformation matrix for stereo display */
  SgPlotOutputStr("/trmtx matrix currentmatrix def\n");

  /* set round line ends */
  SgPlotOutputStr("1 setlinecap\n");
  SgPlotOutputStr("1 setlinejoin\n");

  SgPlotOutputStr("%%EndSetup\n");

  return SG_RES_OK;
}

SgRes
SgPSCleanup(void)
{
  SgPlotOutputStr("%%EOF\n");

  return SG_RES_OK;
}
