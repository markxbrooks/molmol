/*
************************************************************************
*
*   X11Color.c - X11 color setting
*
*   Copyright (c) 1994-99
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
*   Date of last modification : 99/10/30
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/x11/SCCS/s.X11Color.c
*   SCCS identification       : 1.8
*
************************************************************************
*/

#include "x11_color.h"

#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>

#include <hashtab.h>

#include <sg_get.h>
#include <sg_error.h>
#include "x11_access.h"

#define EPS 1.0E-4

typedef unsigned short ColorInt;

#define BIT_NO_SHORT (8 * sizeof(ColorInt))
#define MAX_COLORS 256
#define BIT_NO_R 2
#define BIT_NO_G 2
#define BIT_NO_B 2

#define FLOAT2SHORT(f) ((ColorInt) (f * ((1 << BIT_NO_SHORT) - EPS)))

typedef struct {
  ColorInt r, g, b;
  unsigned long pix;
} ColorDescr;

static BOOL ShadingOn = FALSE;
static unsigned long *ShadeColors = NULL;

/* Use a hash table for storing allocated colors. XAllocColor is
   a round trip request and degrades performance badly. */
static HASHTABLE ColorTab = NULL;

static int CellNo;
static XColor *FullColorMap = NULL;
static unsigned long BGCol;
static float BGColR, BGColG, BGColB;

static unsigned
hashFunc(void *p, unsigned size)
{
  ColorDescr *colP = p;

  return (37 * (37 * colP->r + colP->g) + colP->b) % size;
}

static int
compFunc(void *p1, void *p2)
{
  ColorDescr *col1P = p1;
  ColorDescr *col2P = p2;

  return ! (col1P->r == col2P->r &&
      col1P->g == col2P->g &&
      col1P->b == col2P->b);
}

static BOOL
allocColor(unsigned long *pixP, ColorInt r, ColorInt g, ColorInt b)
{
  Display *dpy = X11GetDisplay();
  Colormap cMap = X11GetColormap();
  ColorDescr descr, *descrP;
  XColor col;
  Status ret;

  descr.r = r;
  descr.g = g;
  descr.b = b;

  if (ColorTab == NULL) {
    ColorTab = HashtabOpen(257, sizeof(ColorDescr), hashFunc, compFunc);
  } else {
    descrP = HashtabSearch(ColorTab, &descr);
    if (descrP != NULL) {
      *pixP = descrP->pix;
      return TRUE;
    }
  }

  col.red = r;
  col.green = g;
  col.blue = b;
  ret = XAllocColor(dpy, cMap, &col);

  if (ret == 0) {
    return FALSE;
  } else {
    descr.pix = col.pixel;
    (void) HashtabInsert(ColorTab, &descr, FALSE);
    *pixP = col.pixel;
    return TRUE;
  }
}

static unsigned long
approxColor(ColorInt r, ColorInt g, ColorInt b, BOOL avoidBG)
{
  Display *dpy = X11GetDisplay();
  Colormap cMap = X11GetColormap();
  BOOL distSet;
  int i, minInd;
  long dist, minDist;

  if (FullColorMap == NULL) {
    CellNo = DisplayCells(dpy, DefaultScreen(dpy));
    FullColorMap = malloc(CellNo * sizeof(XColor));
    if (FullColorMap == NULL) {
      SgRaiseError(SG_ERR_NOMEM, "SgX11SetColor");
      return 0;
    }
    for (i = 0; i < CellNo; i++)
      FullColorMap[i].pixel = i;
    XQueryColors(dpy, cMap, FullColorMap, CellNo);
  }

  distSet = FALSE;
  for (i = 0; i < CellNo; i++) {
    if (! (avoidBG && FullColorMap[i].pixel == BGCol)) {
      dist = abs(r - FullColorMap[i].red) +
	     abs(g - FullColorMap[i].green) +
	     abs(b - FullColorMap[i].blue);
      if (dist < minDist || ! distSet) {
	minDist = dist;
	minInd = i;
	distSet = TRUE;
      }
    }
  }

  return FullColorMap[minInd].pixel;
}

unsigned long
getColor(float r, float g, float b, BOOL fg)
{
  ColorInt rs, gs, bs;
  unsigned long pix;

  rs = FLOAT2SHORT(r),
  gs = FLOAT2SHORT(g);
  bs = FLOAT2SHORT(b);

  if (allocColor(&pix, rs, gs, bs)) {
    if (fg && pix == BGCol) {
      /* avoid that color is approximated by background color */
      return approxColor(rs, gs, bs, TRUE);
    } else {
      return pix;
    }
  } else {
    return approxColor(rs, gs, bs, fg);
  }
}

void
SgX11SetFeature(SgFeature feature, BOOL onOff)
{
  int colNo, colInd, colR, colG, colB;

  if (feature == SG_FEATURE_SHADE) {
    ShadingOn = onOff;
    if (ShadingOn && ShadeColors == NULL) {
      colNo = (1 << BIT_NO_R) * (1 << BIT_NO_G) * (1 << BIT_NO_B);
      ShadeColors = malloc(colNo * sizeof(*ShadeColors));
      colInd = 0;
      for (colR = 0; colR < (1 << BIT_NO_R); colR++)
	for (colG = 0; colG < (1 << BIT_NO_G); colG++)
	  for (colB = 0; colB < (1 << BIT_NO_B); colB++) {
	    ShadeColors[colInd] = getColor(
		(float) colR / ((1 << BIT_NO_R) - 1),
		(float) colG / ((1 << BIT_NO_G) - 1),
		(float) colB / ((1 << BIT_NO_B) - 1),
		FALSE);
	    colInd++;
	  }
    }
  }
}

void
SgX11SetColor(float r, float g, float b, float a, BOOL fg)
{
  Display *dpy = X11GetDisplay();
  Window w = X11GetWindow();
  GC gc = X11GetGC();
  ColorInt ri, bi, gi;
  unsigned long pix;
  XGCValues gcv;

  if (fg) {
    if (r == BGColR && g == BGColG && b == BGColB) {
      pix = BGCol;
    } else if (ShadingOn) {
      ri = (ColorInt) (r * ((1 << BIT_NO_R) - EPS));
      gi = (ColorInt) (g * ((1 << BIT_NO_G) - EPS));
      bi = (ColorInt) (b * ((1 << BIT_NO_B) - EPS));
      pix = ShadeColors[(((ri << BIT_NO_G) + gi) << BIT_NO_B) + bi];
    } else {
      pix = getColor(r, g, b, TRUE);
    }

    gcv.foreground = pix;
    XChangeGC(dpy, gc, GCForeground, &gcv);
  } else {
    pix = getColor(r, g, b, FALSE);

    BGColR = r;
    BGColG = g;
    BGColB = b;
    
    gcv.background = pix;
    XChangeGC(dpy, gc, GCBackground, &gcv);
    XSetWindowBackground(dpy, w, pix);
    BGCol = pix;
  }
}
