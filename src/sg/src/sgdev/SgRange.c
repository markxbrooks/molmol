/*
************************************************************************
*
*   SgRange.c - Sg range determination
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
*   Date of last modification : 01/05/19
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/sgdev/SCCS/s.SgRange.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <sg_dev.h>
#include <sg_range.h>

#include <stdio.h>
#include <values.h>

#include <sg.h>
#include "sg_name.h"
#include "sg_set.h"
#include "sg_view.h"

static SgDev *OldDevP;
static char *OldDevName;
static float XMin, XMax, YMin, YMax;

void
SgStartRange(void)
{
  XMin = MAXFLOAT;
  XMax = - MAXFLOAT;
  YMin = MAXFLOAT;
  YMax = - MAXFLOAT;

  OldDevP = CurrSgDevP;
  OldDevName = SgGetDeviceName();
  SgSetDevice("Range");
  SgInit(NULL, 0);
  SgUpdateViewport();
  SgUpdateView();
}

void
SgEndRange(float *xMinP, float *xMaxP, float *yMinP, float *yMaxP)
{
  (void) SgCleanup();
  CurrSgDevP = OldDevP;
  SgSetDeviceName(OldDevName);
  SgSetAttr();

  *xMinP = XMin;
  *xMaxP = XMax;
  *yMinP = YMin;
  *yMaxP = YMax;
}

void
SgRangePoint(float x, float y)
{
  if (x < XMin)
    XMin = x;

  if (x > XMax)
    XMax = x;

  if (y < YMin)
    YMin = y;

  if (y > YMax)
    YMax = y;
}

void
SgRangeRect(float xMin, float xMax, float yMin, float yMax)
{
  if (xMin < XMin)
    XMin = xMin;

  if (xMax > XMax)
    XMax = xMax;

  if (yMin < YMin)
    YMin = yMin;

  if (yMax > YMax)
    YMax = yMax;
}
