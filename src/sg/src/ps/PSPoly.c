/*
************************************************************************
*
*   PSPoly.c - PostScript polygon drawing
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/ps/SCCS/s.PSPoly.c
*   SCCS identification       : 1.12
*
************************************************************************
*/

#include "ps_poly.h"

#include <stdio.h>
#include <values.h>

#include <sg_map.h>
#include <sg_plot.h>
#include <sg_get.h>
#include "ps_color.h"

static void
plotPoint(float x[2])
{
  SgPlotOutputFloat(x[0]);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(x[1]);
}

void
SgPSDrawPolygon(float x[][2], int n)
{
  float xm[2];
  int i;

  PSUpdateColor();

  if (n == 3) {
    /* special function for most common case to make files shorter */
    SgVpMapPoint(xm, x[0]);
    plotPoint(xm);
    SgPlotOutputStr(" ");
    SgVpMapPoint(xm, x[1]);
    plotPoint(xm);
    SgPlotOutputStr(" ");
    SgVpMapPoint(xm, x[2]);
    plotPoint(xm);
    SgPlotOutputStr(" tr\n");
  } else {
    SgVpMapPoint(xm, x[0]);
    SgPlotOutputStr("np\n");
    plotPoint(xm);
    SgPlotOutputStr(" mt\n");
    for (i = 1; i < n; i++) {
      SgVpMapPoint(xm, x[i]);
      plotPoint(xm);
      SgPlotOutputStr(" lt\n");
    }
    SgPlotOutputStr("cp fi\n");
  }
}

static int
intCol(float f)
{
  int i;

  i = (int) (f * 256.0f);
  /* these should normally be in the range already, but make sure
     to not create an illegal file when something goes wrong */
  if (i < 0)
    i = 0;
  else if (i > 255)
    i = 255;

  return i;
}

static void
imgCol(float r, float g, float b)
{
  char buf[10];

  (void) sprintf(buf, "%02x%02x%02x", intCol(r), intCol(g), intCol(b));
  SgPlotOutputStr(buf);
}

void
SgPSDrawShadedPolygon(float x[][2], float col[][3], int n)
{
  float xm[SG_POLY_SIZE][2];
  float xMin, xMax, yMin, yMax;
  int xMinI, xMaxI, yMinI, yMaxI;
  int prec;
  float xSize, ySize;
  float colg[SG_POLY_SIZE][3];
  float d, grad, xGrad, yGrad, xDist, yDist;
  int xNo, yNo;
  float x0, y0, xInc, yInc;
  float x1, x2, y1, y2, t;
  float colPix[3];
  int clipNo, clipI;
  BOOL isIn;
  float xmc1[SG_POLY_SIZE][2], xmc2[SG_POLY_SIZE][2];
  float colc1[SG_POLY_SIZE][3], colc2[SG_POLY_SIZE][3];
  int xI, yI, i1, i, k;

  for (i = 0; i < n; i++)
    SgVpMapPoint(xm[i], x[i]);

  xMin = MAXFLOAT;
  xMax = - MAXFLOAT;
  yMin = MAXFLOAT;
  yMax = - MAXFLOAT;
  for (i = 0; i < n; i++) {
    if (xm[i][0] < xMin) {
      xMin = xm[i][0];
      xMinI = i;
    }
    if (xm[i][0] > xMax) {
      xMax = xm[i][0];
      xMaxI = i;
    }
    if (xm[i][1] < yMin) {
      yMin = xm[i][1];
      yMinI = i;
    }
    if (xm[i][1] > yMax) {
      yMax = xm[i][1];
      yMaxI = i;
    }
  }

  xSize = xm[xMaxI][0] - xm[xMinI][0];
  ySize = xm[yMaxI][1] - xm[yMinI][1];

  for (i = 0; i < n; i++) {
    for (k = 0; k < 3; k++)
      colg[i][k] = col[i][k];
    SgPlotDoGamma(&colg[i][0], &colg[i][1], &colg[i][2]);
  }

  xGrad = 0.0f;
  xDist = 0.0f;
  yGrad = 0.0f;
  yDist = 0.0f;
  for (i = 0; i < n; i++) {
    grad = 0.0f;
    for (k = 0; k < 3; k++) {
      d = colg[i][k] - colg[(i + 1) % n][k];
      if (d > grad)
	grad = d;
      else if (- d > grad)
	grad = - d;
    }

    d = xm[i][0] - xm[(i + 1) % n][0];
    if (d < 0.0f)
      d = - d;
    if (d * grad > xDist * xGrad) {
      xGrad = grad;
      xDist = d;
    }

    d = xm[i][1] - xm[(i + 1) % n][1];
    if (d < 0.0f)
      d = - d;
    if (d * grad > yDist * yGrad) {
      yGrad = grad;
      yDist = d;
    }
  }

  prec = SgGetPrecision();

  xNo = (int) (256.0f * xSize * xGrad * (1 << prec) / 8.0f);
  if (xNo == 0)
    xNo = 1;
  else if (xNo > 256 * xGrad)
    xNo = 256;

  yNo = (int) (256.0f * ySize * yGrad * (1 << prec) / 8.0f);
  if (yNo == 0)
    yNo = 1;
  else if (yNo > 256 * yGrad)
    yNo = 256;

  if (xNo == 1 && yNo == 1) {
    /* uniform color, draw as normal polygon, with average color */
    for (k = 0; k < 3; k++)
      colPix[k] = 0.0f;

    for (i = 0; i < n; i++)
      for (k = 0; k < 3; k++)
	colPix[k] += colg[i][k];
      
    for (k = 0; k < 3; k++) {
      colPix[k] /= n;
      SgPlotOutputFloat(colPix[k]);
      SgPlotOutputStr(" ");
    }
    SgPlotOutputStr("co\n");
    PSInvalidateColor();

    if (n == 3) {
      plotPoint(xm[0]);
      SgPlotOutputStr(" ");
      plotPoint(xm[1]);
      SgPlotOutputStr(" ");
      plotPoint(xm[2]);
      SgPlotOutputStr(" tr\n");
    } else {
      SgPlotOutputStr("np\n");
      plotPoint(xm[0]);
      SgPlotOutputStr(" mt\n");
      for (i = 1; i < n; i++) {
	plotPoint(xm[i]);
	SgPlotOutputStr(" lt\n");
      }
      SgPlotOutputStr("cp fi\n");
    }

    return;
  }

  if (xNo == 1) {
    xInc = xSize;
    x0 = xm[xMinI][0];
  } else {
    xInc = xSize / (xNo - 1);
    x0 = xm[xMinI][0] - 0.5f * xInc;
    xSize += xInc;
  }

  if (yNo == 1) {
    yInc = ySize;
    y0 = xm[yMinI][1];
  } else {
    yInc = ySize / (yNo - 1);
    y0 = xm[yMinI][1] - 0.5f * yInc;
    ySize += yInc;
  }

  SgPlotOutputStr("sn\n");
  SgPlotOutputFloat(xm[0][0]);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(xm[0][1]);
  SgPlotOutputStr(" mt\n");
  for (i = 1; i < n; i++) {
    SgPlotOutputFloat(xm[i][0]);
    SgPlotOutputStr(" ");
    SgPlotOutputFloat(xm[i][1]);
    SgPlotOutputStr(" lt\n");
  }
  SgPlotOutputStr("cl\n");

  SgPlotOutputInt(xNo);
  SgPlotOutputStr(" ");
  SgPlotOutputInt(yNo);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(xSize);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(ySize);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(x0);
  SgPlotOutputStr(" ");
  SgPlotOutputFloat(y0);
  SgPlotOutputStr(" im\n");

  for (yI = 0; yI < yNo; yI++) {
    y1 = y0 + yI * yInc;
    y2 = y1 + yInc;

    for (xI = 0; xI < xNo; xI++) {
      x1 = x0 + xI * xInc;
      x2 = x1 + xInc;

      /* clip the polgon with rectangle describing the pixel,
         take the average color of this resulting polygon */

      /* clip at x1, original -> c1 */
      clipNo = n;
      clipI = 0;
      for (i = 0; i < clipNo; i++) {
	isIn = (xm[i][0] > x1);
	if (isIn) {
	  xmc1[clipI][0] = xm[i][0];
	  xmc1[clipI][1] = xm[i][1];
	  for (k = 0; k < 3; k++)
	    colc1[clipI][k] = colg[i][k];
	  clipI++;
	}

	i1 = (i + 1) % clipNo;
	if ((xm[i1][0] > x1) != isIn) {
	  t = (x1 - xm[i][0]) / (xm[i1][0] - xm[i][0]);
	  xmc1[clipI][0] = x1;
	  xmc1[clipI][1] = xm[i][1] + t * (xm[i1][1] - xm[i][1]);
	  for (k = 0; k < 3; k++)
	    colc1[clipI][k] = colg[i][k] + t * (colg[i1][k] - colg[i][k]);
	  clipI++;
	}
      }

      /* clip at x2, c1 -> c2 */
      clipNo = clipI;
      clipI = 0;
      for (i = 0; i < clipNo; i++) {
	isIn = (xmc1[i][0] < x2);
	if (isIn) {
	  xmc2[clipI][0] = xmc1[i][0];
	  xmc2[clipI][1] = xmc1[i][1];
	  for (k = 0; k < 3; k++)
	    colc2[clipI][k] = colc1[i][k];
	  clipI++;
	}

	i1 = (i + 1) % clipNo;
	if ((xmc1[i1][0] < x2) != isIn) {
	  t = (x2 - xmc1[i][0]) / (xmc1[i1][0] - xmc1[i][0]);
	  xmc2[clipI][0] = x2;
	  xmc2[clipI][1] = xmc1[i][1] + t * (xmc1[i1][1] - xmc1[i][1]);
	  for (k = 0; k < 3; k++)
	    colc2[clipI][k] = colc1[i][k] + t * (colc1[i1][k] - colc1[i][k]);
	  clipI++;
	}
      }

      /* clip at y1, c2 -> c1 */
      clipNo = clipI;
      clipI = 0;
      for (i = 0; i < clipNo; i++) {
	isIn = (xmc2[i][1] > y1);
	if (isIn) {
	  xmc1[clipI][0] = xmc2[i][0];
	  xmc1[clipI][1] = xmc2[i][1];
	  for (k = 0; k < 3; k++)
	    colc1[clipI][k] = colc2[i][k];
	  clipI++;
	}

	i1 = (i + 1) % clipNo;
	if ((xmc2[i1][1] > y1) != isIn) {
	  t = (y1 - xmc2[i][1]) / (xmc2[i1][1] - xmc2[i][1]);
	  xmc1[clipI][0] = xmc2[i][0] + t * (xmc2[i1][0] - xmc2[i][0]);
	  xmc1[clipI][1] = y1;
	  for (k = 0; k < 3; k++)
	    colc1[clipI][k] = colc2[i][k] + t * (colc2[i1][k] - colc2[i][k]);
	  clipI++;
	}
      }

      /* clip at y2, c1 -> c2 */
      clipNo = clipI;
      clipI = 0;
      for (i = 0; i < clipNo; i++) {
	isIn = (xmc1[i][1] < y2);
	if (isIn) {
	  for (k = 0; k < 3; k++)
	    colc2[clipI][k] = colc1[i][k];
	  clipI++;
	}

	i1 = (i + 1) % clipNo;
	if ((xmc1[i1][1] < y2) != isIn) {
	  t = (y2 - xmc1[i][1]) / (xmc1[i1][1] - xmc1[i][1]);
	  for (k = 0; k < 3; k++)
	    colc2[clipI][k] = colc1[i][k] + t * (colc1[i1][k] - colc1[i][k]);
	  clipI++;
	}
      }
      clipNo = clipI;

      if (clipNo == 0) {
	/* completely outside */
	imgCol(colg[0][0], colg[0][1], colg[0][2]);
	continue;
      }

      for (k = 0; k < 3; k++)
	colPix[k] = 0.0f;

      for (i = 0; i < clipNo; i++)
	for (k = 0; k < 3; k++)
	  colPix[k] += colc2[i][k];
	
      for (k = 0; k < 3; k++)
	colPix[k] /= clipNo;

      imgCol(colPix[0], colPix[1], colPix[2]);
    }

    SgPlotOutputStr("\n");
  }

  SgPlotOutputStr("re\n");
}
