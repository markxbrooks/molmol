/*
************************************************************************
*
*   ExFigAngles.c - FigAngles command
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdfig/SCCS/s.ExFigAngles.c
*   SCCS identification       : 1.13
*
************************************************************************
*/

#include <cmd_fig.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <values.h>

#include <break.h>
#include <sg.h>
#include <arg.h>
#include <data_hand.h>
#include <data_sel.h>
#include <graph_draw.h>
#include "fig_util.h"

#define MAX_ANGLE_NO 20

#define PROP_PLOT "_plot"
#define AXIS_SIZE_X 0.2f
#define AXIS_SIZE_Y 0.1f
#define BORDER_SIZE 0.05f
#define TICK_SIZE 0.03f
#define TICK_INC_X 5
#define LABEL_INC_X 10
#define TICK_INC_Y 20
#define LABEL_INC_Y 60
#define FONT_SIZE 0.035f
#define DIGIT_SIZE_X (0.8f * FONT_SIZE)
#define DIGIT_SIZE_Y FONT_SIZE
#define GAP_X 0.001f
#define GAP_Y 0.01f

typedef struct {
  DhAngleP angP;
  float val;
} ValDescr;

typedef struct {
  int valNo;
  ValDescr *valA;
} ResDescr;

typedef struct {
  DSTR name;
  ResDescr *resA;
} AngleDescr;

static DSTR AngleNames = NULL;
static AngleDescr AngleA[MAX_ANGLE_NO];
static float GapSize = 60.0f;
static int MolNo, AngleNo = 0;
static int MinResI, MaxResI;

static void
unmarkRes(DhResP resP, void *clientData)
{
  PropRefP refP = clientData;

  DhResSetProp(refP, resP, FALSE);
}

static void
unmarkAngle(DhAngleP angP, void *clientData)
{
  PropRefP refP = clientData;

  DhAngleSetProp(refP, angP, FALSE);
}

static void
markAngle(DhAngleP angP, void *clientData)
{
  PropRefP refP = clientData;

  DhAngleSetProp(refP, angP, TRUE);
}

static void
countRes(DhResP resP, void *clientData)
{
  int num;

  num = DhResGetNumber(resP);
  if (num < MinResI)
    MinResI = num;
  if (num > MaxResI)
    MaxResI = num;
}

static void
fillVal(DhAngleP angP, void *clientData)
{
  DSTR name;
  int angI;
  ResDescr *descrP;

  if (BreakCheck(10000))
    return;

  name = DhAngleGetName(angP);
  for (angI = 0; angI < AngleNo; angI++)
    if (DStrCmp(name, AngleA[angI].name) == 0)
      break;
  
  if (angI == AngleNo)
    return;

  descrP = AngleA[angI].resA + DhResGetNumber(DhAngleGetRes(angP)) - MinResI;
  descrP->valA[descrP->valNo].angP = angP;
  descrP->valA[descrP->valNo].val = DhAngleGetVal(angP);
  descrP->valNo++;
}

static void
sortVal(ValDescr *valA, int n)
{
  /* small sizes, simple algorithm is fast enough */
  int i, k;
  ValDescr t;

  for (i = 0; i < n - 1; i++)
    for (k = i + 1; k < n; k++)
      if (valA[k].val < valA[i].val) {
	t = valA[k];
	valA[k] = valA[i];
	valA[i] = t;
      }
}

static void
drawBar(float x0, float w, float y0, float y1, float cy, float sy)
{
  float x1;
  float x[5][2];

  x1 = x0 + w - GAP_X;
  x0 += GAP_X;

  y0 = sy * y0 / 180.0f - GAP_Y;
  if (y0 < - sy)
    y0 = - sy;
  y0 += cy;

  y1 = sy * y1 / 180.0f + GAP_Y;
  if (y1 > sy)
    y1 = sy;
  y1 += cy;

  x[0][0] = x0;
  x[0][1] = y1;
  x[1][0] = x0;
  x[1][1] = y0;
  x[2][0] = x1;
  x[2][1] = y0;
  x[3][0] = x1;
  x[3][1] = y1;
  x[4][0] = x0;
  x[4][1] = y1;

  SgDrawPolyline2D(x, 5);
}

static void
drawVal(ValDescr *valA, int n, float x0, float w, float cy, float sy)
{
  float x[2];
  float gap, vDiff;
  int gapI, startI, i1, i2, i3, i;

  if (n == 0)
    return;

  x[0] = x0;
  for (i = 0; i < n; i++) {
    x[1] = cy + sy * valA[i].val / 180.0f;
    SgSetPickObj("Angle", valA[i].angP);
    FigSetAngleColor(valA[i].angP);
    SgDrawDisc2D(x, 0.005f);
  }

  SgSetColor(0.0f, 0.0f, 0.0f);
  SgSetPickObj(NULL, NULL);

  gap = 0.0f;
  for (i = 0; i < n - 1; i++)
    if (valA[i + 1].val - valA[i].val > gap) {
      gap = valA[i + 1].val - valA[i].val;
      gapI = i;
    }

  if (360.0f - (valA[n - 1].val - valA[0].val) > gap)
    startI = 0;
  else
    startI = gapI + 1;

  i1 = startI;
  i2 = startI;
  for (;;) {
    i3 = i2 + 1;
    if (i3 == n) {
      i3 = 0;
      vDiff = valA[0].val - valA[i2].val + 360.0f;
    } else {
      vDiff = valA[i3].val - valA[i2].val;
    }

    if (i3 == startI || vDiff > GapSize) {
      if (i1 <= i2) {
	drawBar(x0 - 0.5f * w, w, valA[i1].val, valA[i2].val, cy, sy);
      } else {
	drawBar(x0 - 0.5f * w, w, valA[i1].val, 180.0f, cy, sy);
	drawBar(x0 - 0.5f * w, w, -180.0f, valA[i2].val, cy, sy);
      }

      if (i3 == startI)
	break;

      i1 = i3;
    }

    i2 = i3;
  }
}

static void
drawAngles(void *clientData)
{
  int resNo;
  float cx, cy, sx, sy;
  int resI, angI, breakCount;
  ResDescr *descrP;

  DhActivateGroups(TRUE);

  MinResI = MAXINT;
  MaxResI = - MAXINT;
  DhApplyRes(PropGetRef(PROP_PLOT, FALSE), countRes, NULL);
  resNo = MaxResI - MinResI + 1;

  for (angI = 0; angI < AngleNo; angI++) {
    AngleA[angI].resA = malloc(resNo * sizeof(ResDescr));
    for (resI = 0; resI < resNo; resI++) {
      AngleA[angI].resA[resI].valA = malloc(MolNo * sizeof(ValDescr));
      AngleA[angI].resA[resI].valNo = 0;
    }
  }

  DhApplyAngle(PropGetRef(PROP_PLOT, FALSE), fillVal, NULL);

  DhActivateGroups(FALSE);

  /* sort algorithm has quadratic behaviour */
  breakCount = 500000 / (2 * MolNo * MolNo);
  for (angI = 0; angI < AngleNo; angI++)
    for (resI = 0; resI < resNo; resI++) {
      if (BreakCheck(breakCount))
	break;
      sortVal(AngleA[angI].resA[resI].valA, AngleA[angI].resA[resI].valNo);
    }

  SgSetLight(SG_LIGHT_OFF);
  SgSetColor(0.0f, 0.0f, 0.0f);
  SgSetLineWidth(0.0f);
  SgSetLineStyle(SG_LINE_SOLID);
  SgSetFontSize(FONT_SIZE);

  FigDrawAxis(-1.0f + AXIS_SIZE_X, -1.0f + AXIS_SIZE_Y,
      2.0f - AXIS_SIZE_X - BORDER_SIZE, AO_HORIZ,
      "Seq.", MinResI - 0.5f, MinResI + resNo - 0.5f,
      DIGIT_SIZE_X, DIGIT_SIZE_Y,
      AT_LEFT, TICK_INC_X, TICK_SIZE, AL_ALL, LABEL_INC_X);

  FigDrawAxis(-1.0f + AXIS_SIZE_X, 1.0f - BORDER_SIZE,
      2.0f - AXIS_SIZE_X - BORDER_SIZE, AO_HORIZ,
      "", MinResI - 0.5f, MinResI + resNo - 0.5f,
      DIGIT_SIZE_X, DIGIT_SIZE_Y,
      AT_RIGHT, TICK_INC_X, TICK_SIZE, AL_NONE, LABEL_INC_X);

  sx = 2.0f - (AXIS_SIZE_X + BORDER_SIZE);
  cx = -1.0f + AXIS_SIZE_X + sx * 0.5f / resNo;

  breakCount = 1000 / MolNo;

  for (angI = 0; angI < AngleNo; angI++) {
    sy = 0.5f * (2.0f - (AXIS_SIZE_Y + BORDER_SIZE)) / AngleNo;
    cy = 1.0f - BORDER_SIZE - (2 * angI + 1) * sy;

    if (angI > 0)
      FigDrawAxis(-1.0f + AXIS_SIZE_X, cy + sy,
	  2.0f - AXIS_SIZE_X - BORDER_SIZE, AO_HORIZ,
	  "", MinResI - 0.5f, MinResI + resNo - 0.5f,
	  DIGIT_SIZE_X, DIGIT_SIZE_Y,
	  AT_CENT, TICK_INC_X, TICK_SIZE, AL_NONE, LABEL_INC_X);

    FigDrawAxis(-1.0f + AXIS_SIZE_X, cy - sy, 2.0f * sy, AO_VERT,
	DStrToStr(AngleA[angI].name), -180.0f, 180.0f,
	DIGIT_SIZE_X, DIGIT_SIZE_Y,
	AT_LEFT, TICK_INC_Y, TICK_SIZE,
	angI == AngleNo - 1 ? AL_ALL : AL_NOT_FIRST, LABEL_INC_Y);

    FigDrawAxis(1.0f - BORDER_SIZE, cy - sy, 2.0f * sy, AO_VERT,
	"", -180.0f, 180.0f,
	DIGIT_SIZE_X, DIGIT_SIZE_Y,
	AT_RIGHT, TICK_INC_Y, TICK_SIZE, AL_NONE, LABEL_INC_Y);

    for (resI = 0; resI < resNo; resI++) {
      if (BreakCheck(breakCount))
	break;

      descrP = AngleA[angI].resA + resI;
      drawVal(descrP->valA, descrP->valNo,
	  cx + sx * resI / resNo, sx / resNo, cy, sy);
    }
  }

  for (angI = 0; angI < AngleNo; angI++) {
    for (resI = 0; resI < resNo; resI++)
      free(AngleA[angI].resA[resI].valA);
    free(AngleA[angI].resA);
  }
}

#define ARG_NUM 2

ErrCode
ExFigAngles(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  int angI;
  char *str;
  PropRefP refP;

  arg[0].type = AT_STR;
  arg[1].type = AT_DOUBLE;

  ArgInit(arg, ARG_NUM);

  if (AngleNames == NULL) {
    AngleNames = DStrNew();
    DStrAssignStr(AngleNames, "PHI PSI");
  }

  arg[0].prompt = "Angles";
  DStrAssignDStr(arg[0].v.strVal, AngleNames);

  arg[1].prompt = "Gap Size";
  arg[1].v.doubleVal = GapSize;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);\
    return errCode;
  }

  DStrAssignDStr(AngleNames, arg[0].v.strVal);
  GapSize = (float) arg[1].v.doubleVal;

  ArgCleanup(arg, ARG_NUM);

  if (SelResGet(NULL, 0) == 0) {
    CipSetError("at least 1 residue must be selected");
    return EC_ERROR;
  }

  for (angI = 0; angI < AngleNo; angI++)
    DStrFree(AngleA[angI].name);

  str = DStrToStr(AngleNames);
  AngleNo = 0;
  AngleA[0].name = DStrNew();
  while (*str != '\0') {
    if (*str == ' ') {
      if (AngleNo == MAX_ANGLE_NO - 1)
	break;
      AngleNo++;
      AngleA[AngleNo].name = DStrNew();
    } else {
      DStrAppChar(AngleA[AngleNo].name, *str);
    }
    str++;
  }
  AngleNo++;

  DhActivateGroups(TRUE);

  MolNo = SelMolGet(NULL, 0);

  refP = PropGetRef(PROP_PLOT, TRUE);
  DhApplyRes(PropGetRef(PROP_ALL, FALSE), unmarkRes, refP);
  DhApplyAngle(PropGetRef(PROP_ALL, FALSE), unmarkAngle, refP);
  DhApplyAngle(PropGetRef(PROP_SELECTED, FALSE), markAngle, refP);

  DhActivateGroups(FALSE);

  SgSetBackgroundColor(1.0f, 1.0f, 1.0f);
  GraphShowAlt(drawAngles, NULL);

  return EC_OK;
}
