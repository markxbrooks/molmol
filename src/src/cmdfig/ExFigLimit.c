/*
************************************************************************
*
*   ExFigLimit.c - FigLimit command
*
*   Copyright (c) 1996-99
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdfig/SCCS/s.ExFigLimit.c
*   SCCS identification       : 1.7
*
************************************************************************
*/

#include <cmd_fig.h>

#include <stdio.h>
#include <stdlib.h>
#include <values.h>

#include <hashtab.h>
#include <sg.h>
#include <arg.h>
#include <data_hand.h>
#include <graph_draw.h>
#include "fig_util.h"

#define AXIS_SIZE_X 0.2f
#define AXIS_SIZE_Y 0.1f
#define BORDER_SIZE 0.05f
#define TICK_SIZE 0.03f
#define TICK_INC_X 5
#define LABEL_INC_X 10
#define TICK_INC_Y 5
#define LABEL_INC_Y 10
#define FONT_SIZE 0.035f
#define DIGIT_SIZE_X (0.8f * FONT_SIZE)
#define DIGIT_SIZE_Y FONT_SIZE
#define GAP 0.05f

typedef struct {
  int min, max;
} ResRange;

typedef struct {
  DhDistKind kind;
  int res1I, res2I;
  int atom1I, atom2I;
} DistDescr;

typedef struct {
  PropRefP resRef1P, resRef2P;
  HASHTABLE tab;
} DistData;

static int MinNo = 1, MaxNo = 5;
static float MinGrey = 0.7f;
static ResRange RangeX, RangeY;
static int *NoTab = NULL;

static void
countRes(DhResP resP, void *clientData)
{
  ResRange *rangeP = clientData;
  int num;

  num = DhResGetNumber(resP);
  if (num < rangeP->min)
    rangeP->min = num;
  if (num > rangeP->max)
    rangeP->max = num;
}

static unsigned
hashDist(void *p, unsigned size)
{
  DistDescr *descrP = p;
  unsigned idx;

  idx = descrP->res1I;
  idx = 37 * idx + descrP->res2I;
  idx = 37 * idx + descrP->atom1I;
  idx = 37 * idx + descrP->atom2I;

  return idx % size;
}

static int
compDist(void *p1, void *p2)
{
  DistDescr *descr1P = p1;
  DistDescr *descr2P = p2;

  if (descr1P->res1I == descr2P->res1I &&
      descr1P->res2I == descr2P->res2I &&
      descr1P->atom1I == descr2P->atom1I &&
      descr1P->atom2I == descr2P->atom2I)
    return 0;

  return 1;
}

static void
addDist(DhDistP distP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  DistData *dataP = clientData;
  DhResP res1P, res2P;
  DistDescr descr;

  res1P = DhAtomGetRes(atom1P);
  res2P = DhAtomGetRes(atom2P);

  descr.kind = DhDistGetKind(distP);

  if (dataP->resRef1P == dataP->resRef2P) {
    if (DhResGetProp(dataP->resRef1P, res1P) &&
        DhResGetProp(dataP->resRef1P, res2P)) {
      if (descr.kind == DK_UPPER) {
	descr.res1I = DhResGetNumber(res1P);
	descr.res2I = DhResGetNumber(res2P);
	descr.atom1I = DhAtomGetNumber(atom1P);
	descr.atom2I = DhAtomGetNumber(atom2P);
	(void) HashtabInsert(dataP->tab, &descr, TRUE);
      } else if (descr.kind == DK_LOWER) {
	descr.res1I = DhResGetNumber(res2P);
	descr.res2I = DhResGetNumber(res1P);
	descr.atom1I = DhAtomGetNumber(atom2P);
	descr.atom2I = DhAtomGetNumber(atom1P);
	(void) HashtabInsert(dataP->tab, &descr, TRUE);
      }
    }
  } else if (descr.kind == DK_UPPER || descr.kind == DK_LOWER) {
    if (DhResGetProp(dataP->resRef1P, res1P) &&
        DhResGetProp(dataP->resRef2P, res2P)) {
      descr.res1I = DhResGetNumber(res1P);
      descr.res2I = DhResGetNumber(res2P);
      descr.atom1I = DhAtomGetNumber(atom1P);
      descr.atom2I = DhAtomGetNumber(atom2P);
      (void) HashtabInsert(dataP->tab, &descr, TRUE);
    } else if (DhResGetProp(dataP->resRef1P, res2P) &&
        DhResGetProp(dataP->resRef2P, res1P)) {
      descr.res1I = DhResGetNumber(res2P);
      descr.res2I = DhResGetNumber(res1P);
      descr.atom1I = DhAtomGetNumber(atom2P);
      descr.atom2I = DhAtomGetNumber(atom1P);
      (void) HashtabInsert(dataP->tab, &descr, TRUE);
    }
  }
}

static void
addMol(DhMolP molP, void *clientData)
{
  DhMolApplyDist(PropGetRef(PROP_SELECTED, FALSE), molP, addDist, clientData);
}

static void
handleDist(void *p, void *clientData)
{
  DistDescr *descrP = p;
  int ind;

  ind = (descrP->res2I - RangeY.min) * (RangeX.max - RangeX.min + 1) +
      descrP->res1I - RangeX.min;
  NoTab[ind]++;
}

static void
drawLimit(void *clientData)
{
  int resNo1, resNo2;
  float sx, sy, cx, cy;
  int resI1, resI2;
  float x1[2], x2[2], x[4][2];
  int no;
  float c;

  SgSetLight(SG_LIGHT_OFF);
  SgSetColor(0.0f, 0.0f, 0.0f);
  SgSetLineWidth(0.0f);
  SgSetLineStyle(SG_LINE_SOLID);
  SgSetFontSize(FONT_SIZE);

  FigDrawAxis(-1.0f + AXIS_SIZE_X, -1.0f + AXIS_SIZE_Y,
      2.0f - AXIS_SIZE_X - BORDER_SIZE, AO_HORIZ,
      "Seq.", RangeX.min - 0.5f, RangeX.max + 0.5f,
      DIGIT_SIZE_X, DIGIT_SIZE_Y,
      AT_LEFT, TICK_INC_X, TICK_SIZE, AL_ALL, LABEL_INC_X);

  FigDrawAxis(-1.0f + AXIS_SIZE_X, -1.0f + AXIS_SIZE_Y,
      2.0f - AXIS_SIZE_Y - BORDER_SIZE, AO_VERT,
      "Seq.", RangeY.min - 0.5f, RangeY.max + 0.5f,
      DIGIT_SIZE_X, DIGIT_SIZE_Y,
      AT_LEFT, TICK_INC_Y, TICK_SIZE, AL_ALL, LABEL_INC_Y);

  resNo1 = RangeX.max - RangeX.min + 1;
  resNo2 = RangeY.max - RangeY.min + 1;

  cx = -1.0f + AXIS_SIZE_X;
  sx = (2.0f - (AXIS_SIZE_X + BORDER_SIZE)) / resNo1;

  cy = -1.0f + AXIS_SIZE_Y;
  sy = (2.0f - (AXIS_SIZE_Y + BORDER_SIZE)) / resNo2;

  x1[0] = cx + (RangeX.max - RangeX.min + 1) * sx;
  x2[0] = x1[0];
  x1[1] = cy;
  x2[1] = 1.0f - BORDER_SIZE;
  SgDrawLine2D(x1, x2);

  x1[0] = cx;
  x2[0] = 1.0f - BORDER_SIZE;
  x1[1] = cy + (RangeY.max - RangeY.min + 1) * sy;
  x2[1] = x1[1];
  SgDrawLine2D(x1, x2);

  SgSetLineStyle(SG_LINE_DASHED);

  x1[1] = cy;
  x2[1] = 1.0f - BORDER_SIZE;
  for (resI1 = RangeX.min; resI1 <= RangeX.max; resI1++) {
    if (resI1 % 5 != 0)
      continue;

    x1[0] = cx + (resI1 - RangeX.min + 0.5f) * sx;
    x2[0] = x1[0];

    SgDrawLine2D(x1, x2);
  }

  x1[0] = cx;
  x2[0] = 1.0f - BORDER_SIZE;
  for (resI2 = RangeY.min; resI2 <= RangeY.max; resI2++) {
    if (resI2 % 5 != 0)
      continue;

    x1[1] = cy + (resI2 - RangeY.min + 0.5f) * sy;
    x2[1] = x1[1];

    SgDrawLine2D(x1, x2);
  }

  SgSetLineStyle(SG_LINE_SOLID);

  for (resI1 = 0; resI1 < resNo1; resI1++)
    for (resI2 = 0; resI2 < resNo2; resI2++) {
      no = NoTab[resI2 * resNo1 + resI1];
      if (no < MinNo)
	continue;
      
      if (no >= MaxNo)
	c = 0.0f;
      else
	c = (MinGrey * (MaxNo - no)) / (MaxNo - MinNo);
      
      SgSetColor(c, c, c);

      x[0][0] = cx + (resI1 + GAP) * sx;
      x[0][1] = cy + (resI2 + GAP) * sy;
      x[1][0] = cx + (resI1 + 1 - GAP) * sx;
      x[1][1] = cy + (resI2 + GAP) * sy;
      x[2][0] = cx + (resI1 + 1 - GAP) * sx;
      x[2][1] = cy + (resI2 + 1 - GAP) * sy;
      x[3][0] = cx + (resI1 + GAP) * sx;
      x[3][1] = cy + (resI2 + 1 - GAP) * sy;

      SgDrawPolygon2D(x, 4);
    }
}

#define ARG_NUM 5

ErrCode
ExFigLimit(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  DistData data;
  int tabSize, i;

  arg[0].type = AT_STR;
  arg[1].type = AT_STR;
  arg[2].type = AT_INT;
  arg[3].type = AT_INT;
  arg[4].type = AT_DOUBLE;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Horiz. Res.";
  DStrAssignStr(arg[0].v.strVal, "selected");

  arg[1].prompt = "Vert. Res.";
  DStrAssignStr(arg[1].v.strVal, "selected");

  arg[2].prompt = "Black No.";
  arg[2].v.intVal = MaxNo;

  arg[3].prompt = "Min. No.";
  arg[3].v.intVal = MinNo;

  arg[4].prompt = "Min. Grey";
  arg[4].v.doubleVal = MinGrey;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  data.resRef1P = PropGetRef(DStrToStr(arg[0].v.strVal), FALSE);
  data.resRef2P = PropGetRef(DStrToStr(arg[1].v.strVal), FALSE);
  MaxNo = arg[2].v.intVal;
  MinNo = arg[3].v.intVal;
  MinGrey = (float) arg[4].v.doubleVal;

  ArgCleanup(arg, ARG_NUM);

  DhActivateGroups(TRUE);

  RangeX.min = MAXINT;
  RangeX.max = - MAXINT;
  DhApplyRes(data.resRef1P, countRes, &RangeX);
  if (RangeX.min > RangeX.max) {
    DhActivateGroups(FALSE);
    FigOff();
    CipSetError("no residues in horizontal direction");
    return EC_ERROR;
  }

  RangeY.min = MAXINT;
  RangeY.max = - MAXINT;
  DhApplyRes(data.resRef2P, countRes, &RangeY);
  if (RangeY.min > RangeY.max) {
    DhActivateGroups(FALSE);
    FigOff();
    CipSetError("no residues in vertical direction");
    return EC_ERROR;
  }

  if (NoTab != NULL)
    free(NoTab);

  tabSize = (RangeX.max - RangeX.min + 1) * (RangeY.max - RangeY.min + 1);
  NoTab = malloc(tabSize * sizeof(*NoTab));
  if (NoTab == NULL) {
    DhActivateGroups(FALSE);
    FigOff();
    CipSetError("out of memory");
    return EC_ERROR;
  }

  for (i = 0; i < tabSize; i++)
    NoTab[i] = 0;

  data.tab = HashtabOpen(997, sizeof(DistDescr), hashDist, compDist);
  DhApplyMol(PropGetRef(PROP_SELECTED, FALSE), addMol, &data);
  HashtabApply(data.tab, handleDist, &data);
  HashtabClose(data.tab);

  DhActivateGroups(FALSE);

  SgSetBackgroundColor(1.0f, 1.0f, 1.0f);
  GraphShowAlt(drawLimit, NULL);

  return EC_OK;
}
