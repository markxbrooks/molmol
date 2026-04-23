/*
************************************************************************
*
*   FigUtil.c - utility functions for drawing figures
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdfig/SCCS/s.FigUtil.c
*   SCCS identification       : 1.5
*
************************************************************************
*/

#include "fig_util.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <bool.h>
#include <sg.h>
#include <attr_struc.h>
#include <par_names.h>
#include <par_hand.h>
#include <graph_draw.h>

#define MIN_MATCH_LEN 4

typedef struct {
  char *name;
  char greek;
} TabEntry;

static TabEntry GreekTab[] = {
  {"ALPHA",   'a'},
  {"BETA",    'b'},
  {"CHI",     'c'},
  {"DELTA",   'd'},
  {"EPSILON", 'e'},
  {"PHI" ,    'f'},
  {"GAMMA",   'g'},
  {"ETA",     'h'},
  {"IOTA",    'i'},
  {"KAPPA",   'k'},
  {"LAMBDA",  'l'},
  {"MU",      'm'},
  {"NU",      'n'},
  {"PI",      'p'},
  {"THETA",   'q'},
  {"RHO",     'r'},
  {"SIGMA",   's'},
  {"TAU",     't'},
  {"UPSILON", 'u'},
  {"OMEGA",   'w'},
  {"XI",      'x'},
  {"PSI",     'y'},
  {"ZETA",    'z'}
};

int
FigGetGreek(char *out, char *in)
{
  int inPos, outPos, tabPos, nameLen, outLen;
  BOOL found;
  char *name;

  inPos = 0;
  while (in[inPos] != '\0' && isupper(in[inPos]))
    inPos++;

  found = FALSE;
  for (tabPos = 0; tabPos < sizeof(GreekTab) / sizeof(TabEntry); tabPos++) {
    name = GreekTab[tabPos].name;
    nameLen = strlen(name);

    if (nameLen < inPos)
      continue;

    if (strncmp(in, name, inPos) != 0)
      continue;

    if (inPos == nameLen || inPos >= MIN_MATCH_LEN) {
      found = TRUE;
      break;
    }
  }

  if (! found) {
    (void) strcpy(out, in);
    return strlen(in);
  }

  out[0] = '\\';
  out[1] = GreekTab[tabPos].greek;
  outPos = 2;
  outLen = 1 + strlen(in + inPos);

  if (in[inPos] != '\0') {
    out[outPos++] = '^';
    out[outPos++] = '{';

    while (in[inPos] != '\0')
      out[outPos++] = in[inPos++];

    out[outPos++] = '}';
  }

  out[outPos] = '\0';

  return outLen;
}

void
FigDrawAxis(float x0, float y0, float len, AxisOrient orient,
    char *name, float minVal, float maxVal,
    float xDigitSize, float yDigitSize,
    AxisTicks tickSel, int tickInc, float tickLen,
    AxisLabels labelSel, int labelInc)
{
  float x1[2], x2[2];
  float tickL, tickR, tickMul, pos;
  int currVal, lastLabelVal, nameLen;
  BOOL drawLabel;
  char nameStr[50], buf[20];

  x1[0] = x0;
  x1[1] = y0;

  if (orient == AO_HORIZ) {
    x2[0] = x0 + len;
    x2[1] = y0;
  } else {
    x2[0] = x0;
    x2[1] = y0 + len;
  }

  SgDrawLine2D(x1, x2);

  if (tickSel == AT_LEFT) {
    tickL = - tickLen;
    tickR = 0.0f;
  } else if (tickSel == AT_CENT) {
    tickL = - 0.5f * tickLen;
    tickR = 0.5f * tickLen;
  } else if (tickSel == AT_RIGHT) {
    tickL = 0.0f;
    tickR = tickLen;
  } else {
    tickL = 0.0f;
    tickR = 0.0f;
  }

  /* can't do that easily in one operation because / is not properly
     defined for negative values */
  currVal = ((int) minVal / tickInc) * tickInc;
  if (currVal < minVal)
    currVal += tickInc;
  
  if (labelSel == AL_NOT_FIRST)
    currVal += tickInc;
  
  while (currVal < maxVal + (float) 1.0e-5) {
    drawLabel = (currVal % labelInc == 0);

    if (drawLabel) {
      tickMul = 1.0f;
      lastLabelVal = currVal;
    } else {
      tickMul = 0.5f;
    }

    pos = (currVal - minVal) / (maxVal - minVal) * len;

    if (orient == AO_HORIZ) {
      x1[0] = x0 + pos;
      x1[1] = y0 + tickMul * tickL;
      x2[0] = x1[0];
      x2[1] = y0 + tickMul * tickR;
    } else {
      x1[0] = x0 + tickMul * tickL;
      x1[1] = y0 + pos;
      x2[0] = x0 + tickMul * tickR;
      x2[1] = x1[1];
    }

    SgDrawLine2D(x1, x2);

    if (drawLabel && labelSel != AL_NONE) {
      (void) sprintf(buf, "%d", currVal);

      if (orient == AO_HORIZ) {
	x1[0] -= 0.5f * xDigitSize * strlen(buf);
	x1[1] = y0 - 1.5f * tickLen - yDigitSize;
      } else {
	x1[0] = x0 - 2.0f * tickLen - xDigitSize * strlen(buf);
	x1[1] -= 0.5f * yDigitSize;
      }

      SgDrawText2D(x1, buf);
    }

    currVal += tickInc;
  }

  if (labelSel != AL_NONE) {
    nameLen = FigGetGreek(nameStr, name);
    pos = (lastLabelVal - 0.5f * labelInc - minVal) / (maxVal - minVal) * len;

    if (orient == AO_HORIZ) {
      x1[0] = x0 + pos;
      x1[1] = y0 - 1.5f * tickLen;
      if (nameLen < (int) strlen(name)) {
	/* make greek letters larger */
	x1[0] -= nameLen * xDigitSize;
	x1[1] -= yDigitSize;
	SgSetFontSize(2.0f * yDigitSize);
	SgDrawText2D(x1, nameStr);
	SgSetFontSize(yDigitSize);
      } else {
	x1[0] -= 0.5f * nameLen * xDigitSize;
	x1[1] -= yDigitSize;
	SgDrawText2D(x1, nameStr);
      }
    } else {
      x1[0] = x0 - 2.0f * tickLen;
      x1[1] = y0 + pos;
      if (nameLen < (int) strlen(name)) {
	/* make greek letters larger */
	x1[0] -= 2.0f * nameLen * xDigitSize;
	x1[1] -= yDigitSize;
	SgSetFontSize(2.0f * yDigitSize);
	SgDrawText2D(x1, nameStr);
	SgSetFontSize(yDigitSize);
      } else {
	x1[0] -= nameLen * xDigitSize;
	x1[1] -= 0.5f * yDigitSize;
	SgDrawText2D(x1, nameStr);
      }
    }
  }
}

void
FigSetAngleColor(DhAngleP angleP)
{
  DhBondP bondP;
  AttrP attrP;

  bondP = DhAngleGetBond(angleP);
  if (bondP == NULL) {
    SgSetColor(0.0f, 0.0f, 0.0f);
  } else {
    attrP = DhBondGetAttr(bondP);
    if (attrP->colR == 1.0f && attrP->colG && attrP->colB == 1.0f)
      /* white color on white background is not very well visible... */
      SgSetColor(0.0f, 0.0f, 0.0f);
    else
      SgSetColor(attrP->colR, attrP->colG, attrP->colB);
  }
}

void
FigOff(void)
{
  float r, g, b;

  r = (float) ParGetDoubleArrVal(PN_BACK_COLOR, 0);
  g = (float) ParGetDoubleArrVal(PN_BACK_COLOR, 1);
  b = (float) ParGetDoubleArrVal(PN_BACK_COLOR, 2);

  SgSetBackgroundColor(r, g, b);

  GraphShowAlt(NULL, NULL);
}
