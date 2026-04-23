/*
************************************************************************
*
*   SgPick.c - Sg picking
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
*   Date of last modification : 01/05/27
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/sgdev/SCCS/s.SgPick.c
*   SCCS identification       : 1.7
*
************************************************************************
*/

#include <sg_dev.h>
#include <sg_pick.h>

#include <stdio.h>
#include <values.h>
#include <math.h>

#include <sg.h>
#include "sg_name.h"
#include "sg_set.h"
#include "sg_view.h"
#include "sg_get.h"

static SgDev *OldDevP;
static char *OldDevName;
static float PointX, PointY;
static void *CurrTypeP, *CurrObjP;
static void *MinTypeP, *MinObjP;
static float MinDist, MinAlpha, MinZ;
static int MinCloseInd;

void
SgStartPick(float x, float y)
{
  float vpx, vpy, vpw, vph;

  SgGetViewport(&vpx, &vpy, &vpw, &vph);
  PointX = x;
  PointY = y;

  MinDist = MAXFLOAT;
  MinAlpha = 0.0f;
  MinZ = MAXFLOAT;
  CurrTypeP = NULL;
  CurrObjP = NULL;
  MinTypeP = NULL;
  MinObjP = NULL;
  MinCloseInd = 0;

  OldDevP = CurrSgDevP;
  OldDevName = SgGetDeviceName();
  SgSetDevice("Pick");
  SgInit(NULL, 0);
  SgUpdateViewport();
  SgUpdateView();
}

void
SgEndPick(void **typePP, void **objPP, int *closeIndP, float *distP)
{
  (void) SgCleanup();
  CurrSgDevP = OldDevP;
  SgSetDeviceName(OldDevName);
  SgSetAttr();

  if (typePP != NULL)
    *typePP = MinTypeP;

  if (objPP != NULL)
    *objPP = MinObjP;

  if (closeIndP != NULL)
    *closeIndP = MinCloseInd;

  if (distP != NULL)
    *distP = sqrtf(MinDist);
}

void
SgPickGetPoint(float x[2])
{
  x[0] = PointX;
  x[1] = PointY;
}

void
SgSetPickObj(void *typeP, void *objP)
{
  CurrTypeP = typeP;
  CurrObjP = objP;
}

void
SgPickDist(float d, float z, int closeInd)
{
  float alpha;

  if (CurrObjP == NULL)
    return;

  if (d > MinDist)
    return;

  if (d == 0.0f) {
    alpha = SgGetMatProp(SG_MAT_ALPHA);
    if (alpha < MinAlpha)
      return;
    if (alpha == MinAlpha) {
      if (z > MinZ)
        return;
    } else {
      MinAlpha = alpha;
    }
    MinZ = z;
  }

  MinDist = d;
  MinTypeP = CurrTypeP;
  MinObjP = CurrObjP;
  MinCloseInd = closeInd;
}
