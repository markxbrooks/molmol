/*
************************************************************************
*
*   SgText.c - Sg wrappers for text functions
*
*   Copyright (c) 1994-95
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/sg/src/sgdev/SCCS/s.SgText.c
*   SCCS identification       : 1.7
*
************************************************************************
*/

#include <sg_dev.h>
#include "sg_set.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sg_get.h>
#include <sg_map.h>

static char *CurrFont = NULL;
static float CurrFontSize = 0.0f, Last2DFontSize = 0.0f;

static void
set2DFontSize(void)
{
  if (CurrFontSize != Last2DFontSize) {
    CurrSgDevP->setFontSize2D(CurrFontSize);
    Last2DFontSize = CurrFontSize;
  }
}

void
SgDrawAnnot2D(float x[2], char *str)
{
  set2DFontSize();
  CurrSgDevP->drawAnnot2D(x, str);
}

void
SgDrawText2D(float x[2], char *str)
{
  set2DFontSize();
  CurrSgDevP->drawText2D(x, str);
}

void
SgDrawAnnot(float x[3], char *str)
{
  CurrSgDevP->drawAnnot(x, str);
}

void
SgDrawText(float x[3], float dx[2], char *str)
{
  float fact, xm[3];

  fact = SgGetDepthFact(0.0f);
  xm[0] = x[0] + fact * dx[0];
  xm[1] = x[1] + fact * dx[1];
  xm[2] = x[2];
  CurrSgDevP->drawText(xm, str);
}

void
SgSetFont(char *name)
{
  CurrSgDevP->setFont(name);
  if (CurrFont != NULL)
    free(CurrFont);

  if (name == NULL) {
    CurrFont = NULL;
  } else {
    CurrFont = malloc(strlen(name) + 1);
    (void) strcpy(CurrFont, name);
  }
}

void
SgSetFontSize(float size)
{
  if (size == CurrFontSize)
    return;

  CurrSgDevP->setFontSize(size);
  CurrFontSize = size;
}

float
SgGetFontSize(void)
{
  return CurrFontSize;
}


void
SgSetTextAttr(void)
{
  SgSetFont(CurrFont);
  CurrSgDevP->setFontSize(CurrFontSize);
  Last2DFontSize = 0.0f;
}

float
SgGetTextWidth(char *str, float size)
{
  return CurrSgDevP->getTextWidth(str, size);
}

void
SgGetTextBox(float ll3[3], float ur3[3], float ll2[2], float ur2[2],
    float x[3], float dx[2], char *str, float h)
{
  float fact, factInv, factRel, w;

  fact = SgGetDepthFact(x[2]);
  factInv = 1.0f / fact;
  factRel = SgGetDepthFact(0.0f) * factInv;
  w = CurrSgDevP->getTextWidth(str, h);

  ll2[0] = x[0] * factInv + dx[0] * factRel;
  ll2[1] = x[1] * factInv + dx[1] * factRel;

  ur2[0] = ll2[0] + w * factRel;
  ur2[1] = ll2[1] + h * factRel;

  ll3[0] = fact * ll2[0];
  ll3[1] = fact * ll2[1];
  ll3[2] = x[2];

  ur3[0] = fact * ur2[0];
  ur3[1] = fact * ur2[1];
  ur3[2] = x[2];
}
