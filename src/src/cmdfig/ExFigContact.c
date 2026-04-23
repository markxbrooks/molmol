/*
************************************************************************
*
*   ExFigContact.c - FigContact command
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdfig/SCCS/s.ExFigContact.c
*   SCCS identification       : 1.7
*
************************************************************************
*/

#include <cmd_fig.h>

#include <stdio.h>
#include <stdlib.h>
#include <values.h>
#include <math.h>

#include <break.h>
#include <mat_vec.h>
#include <grid.h>
#include <hashtab.h>
#include <sg.h>
#include <arg.h>
#include <data_hand.h>
#include <data_sel.h>
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
  int res1I, res2I;
  int atom1I, atom2I;
} AtomPair;

typedef struct {
  DhMolP *molPA;
  int molNo;
  HASHTABLE tab;
  GRID grid;
  PropRefP atomRefP;
  int half;
  int resI;
  DhAtomP atomP;
} SearchData;

static float MinDist = 3.0f, MaxDist = 5.0f;
static ResRange RangeX, RangeY;
static float *DistTab = NULL;

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
hashPair(void *p, unsigned size)
{
  AtomPair *pairP = p;
  unsigned idx;

  idx = pairP->res1I;
  idx = 37 * idx + pairP->res2I;
  idx = 37 * idx + pairP->atom1I;
  idx = 37 * idx + pairP->atom2I;

  return idx % size;
}

static int
compPair(void *p1, void *p2)
{
  AtomPair *pair1P = p1;
  AtomPair *pair2P = p2;

  if (pair1P->res1I == pair2P->res1I &&
      pair1P->res2I == pair2P->res2I &&
      pair1P->atom1I == pair2P->atom1I &&
      pair1P->atom2I == pair2P->atom2I)
    return 0;

  return 1;
}

static void
prepAtom(DhAtomP atomP, void *clientData)
{
  SearchData *dataP = clientData;
  Vec3 x;

  DhAtomGetCoord(atomP, x);
  GridPrepareAddEntry(dataP->grid, x);
}

static void
prepResAtom(DhResP resP, void *clientData)
{
  SearchData *dataP = clientData;

  if (BreakCheck(1000))
    return;

  DhResApplyAtom(dataP->atomRefP, resP, prepAtom, dataP);
}

static void
addAtom(DhAtomP atomP, void *clientData)
{
  SearchData *dataP = clientData;
  Vec3 x;

  DhAtomGetCoord(atomP, x);
  GridInsertEntry(dataP->grid, x, atomP);
}

static void
addResAtom(DhResP resP, void *clientData)
{
  SearchData *dataP = clientData;

  if (BreakCheck(1000))
    return;

  DhResApplyAtom(dataP->atomRefP, resP, addAtom, dataP);
}

static BOOL
handlePair(void *entryP, float *x1, void *clientData)
{
  DhAtomP atom2P = entryP;
  SearchData *dataP = clientData;
  int res1I, res2I;
  Vec3 dx;
  float d;
  AtomPair pair;

  res1I = dataP->resI;
  res2I = DhResGetNumber(DhAtomGetRes(atom2P)) - RangeY.min;

  if (dataP->half == 0) {
    if (res1I > res2I)
      return TRUE;
  } else {
    if (res1I <= res2I)
      return TRUE;
  }

  if (BreakCheck(1000))
    return FALSE;

  DhAtomGetCoord(atom2P, dx);
  Vec3Sub(dx, x1);
  d = dx[0] * dx[0] + dx[1] * dx[1] + dx[2] * dx[2];
  if (d > MaxDist * MaxDist)
    return TRUE;
  
  pair.res1I = res1I;
  pair.res2I = res2I;
  pair.atom1I = DhAtomGetNumber(dataP->atomP);
  pair.atom2I = DhAtomGetNumber(atom2P);
  (void) HashtabInsert(dataP->tab, &pair, TRUE);

  return TRUE;
}

static void
findAtom(DhAtomP atomP, void *clientData)
{
  SearchData *dataP = clientData;
  Vec3 x;

  DhAtomGetCoord(atomP, x);
  dataP->atomP = atomP;
  GridFind(dataP->grid, x, 1, handlePair, dataP);
}

static void
findResAtom(DhResP resP, void *clientData)
{
  SearchData *dataP = clientData;

  dataP->resI = DhResGetNumber(resP) - RangeX.min;
  DhResApplyAtom(dataP->atomRefP, resP, findAtom, dataP);
}

static void
checkPair(void *p, void *clientData)
{
  AtomPair *pairP = p;
  SearchData *dataP = clientData;
  float dAvg, d;
  int dNo;
  DhResP res1P, res2P;
  DhAtomP atom1P, atom2P;
  Vec3 x1, dx;
  int molI, ind;

  dAvg = 0.0f;
  dNo = 0;

  for (molI = 0; molI < dataP->molNo; molI++) {
    res1P = DhResFind(dataP->molPA[molI], pairP->res1I + RangeX.min);
    res2P = DhResFind(dataP->molPA[molI], pairP->res2I + RangeY.min);

    atom1P = DhAtomFindNumber(res1P, pairP->atom1I, FALSE);
    if (atom1P == NULL)
      continue;

    atom2P = DhAtomFindNumber(res2P, pairP->atom2I, FALSE);
    if (atom2P == NULL)
      continue;

    DhAtomGetCoord(atom1P, x1);
    DhAtomGetCoord(atom2P, dx);
    Vec3Sub(dx, x1);
    d = Vec3Abs(dx);

    dAvg += d;
    dNo++;
  }

  dAvg /= dNo;

  ind = pairP->res2I * (RangeX.max - RangeX.min + 1) + pairP->res1I;
  if (dAvg < DistTab[ind])
    DistTab[ind] = dAvg;
}

static void
drawContact(void *clientData)
{
  int resNo1, resNo2;
  float sx, sy, cx, cy;
  int resI1, resI2;
  float x1[2], x2[2], x[4][2];
  float d, c;

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
      if (BreakCheck(1000))
	return;

      d = DistTab[resI2 * resNo1 + resI1];
      if (d >= MaxDist)
	continue;
      
      if (d <= MinDist)
	c = 0.0f;
      else
	c = (d - MinDist) / (MaxDist - MinDist);
      
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

#define ARG_NUM 6

ErrCode
ExFigContact(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  PropRefP resRef1P, resRef2P, atomRef1P, atomRef2P;
  SearchData data;
  int tabSize, molI, i;

  arg[0].type = AT_STR;
  arg[1].type = AT_STR;
  arg[2].type = AT_STR;
  arg[3].type = AT_STR;
  arg[4].type = AT_DOUBLE;
  arg[5].type = AT_DOUBLE;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Horiz. Res.";
  DStrAssignStr(arg[0].v.strVal, "selected");

  arg[1].prompt = "Vert. Res.";
  DStrAssignStr(arg[1].v.strVal, "selected");

  arg[2].prompt = "Top Atoms";
  DStrAssignStr(arg[2].v.strVal, "bb");

  arg[3].prompt = "Bottom Atoms";
  DStrAssignStr(arg[3].v.strVal, "all");

  arg[4].prompt = "Black Dist.";
  arg[4].v.doubleVal = MinDist;

  arg[5].prompt = "White Dist.";
  arg[5].v.doubleVal = MaxDist;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  resRef1P = PropGetRef(DStrToStr(arg[0].v.strVal), FALSE);
  resRef2P = PropGetRef(DStrToStr(arg[1].v.strVal), FALSE);
  atomRef1P = PropGetRef(DStrToStr(arg[2].v.strVal), FALSE);
  atomRef2P = PropGetRef(DStrToStr(arg[3].v.strVal), FALSE);
  MinDist = (float) arg[4].v.doubleVal;
  MaxDist = (float) arg[5].v.doubleVal;

  ArgCleanup(arg, ARG_NUM);

  DhActivateGroups(TRUE);

  RangeX.min = MAXINT;
  RangeX.max = - MAXINT;
  DhApplyRes(resRef1P, countRes, &RangeX);
  if (RangeX.min > RangeX.max) {
    DhActivateGroups(FALSE);
    FigOff();
    CipSetError("no residues in horizontal direction");
    return EC_ERROR;
  }

  RangeY.min = MAXINT;
  RangeY.max = - MAXINT;
  DhApplyRes(resRef2P, countRes, &RangeY);
  if (RangeY.min > RangeY.max) {
    DhActivateGroups(FALSE);
    FigOff();
    CipSetError("no residues in vertical direction");
    return EC_ERROR;
  }

  data.molNo = SelMolGet(NULL, 0);
  data.molPA = malloc(data.molNo * sizeof(*data.molPA));
  (void) SelMolGet(data.molPA, data.molNo);

  for (molI = 1; molI < data.molNo; molI++)
    if (! DhMolEqualStruc(data.molPA[0], data.molPA[molI])) {
      free(data.molPA);
      FigOff();
      CipSetError("selected molecules don't have same structure");
      return EC_ERROR;
    }

  if (DistTab != NULL)
    free(DistTab);

  tabSize = (RangeX.max - RangeX.min + 1) * (RangeY.max - RangeY.min + 1);
  DistTab = malloc(tabSize * sizeof(*DistTab));
  if (DistTab == NULL) {
    DhActivateGroups(FALSE);
    free(data.molPA);
    FigOff();
    CipSetError("out of memory");
    return EC_ERROR;
  }

  for (i = 0; i < tabSize; i++)
    DistTab[i] = MAXFLOAT;

  BreakActivate(TRUE);

  for (data.half = 0; data.half < 2; data.half++) {
    if (data.half == 0)
      data.atomRefP = atomRef1P;
    else
      data.atomRefP = atomRef2P;

    data.tab = HashtabOpen(2791, sizeof(AtomPair), hashPair, compPair);

    for (molI = 0; molI < data.molNo; molI++) {
      data.grid = GridNew();

      DhMolApplyRes(resRef2P, data.molPA[molI], prepResAtom, &data);
      if (MaxDist < 2.0f)
	GridInsertInit(data.grid, 2.0f);
      else
	GridInsertInit(data.grid, MaxDist);
      DhMolApplyRes(resRef2P, data.molPA[molI], addResAtom, &data);

      DhMolApplyRes(resRef1P, data.molPA[molI], findResAtom, &data);

      GridDestroy(data.grid);
    }

    HashtabApply(data.tab, checkPair, &data);

    HashtabClose(data.tab);
  }

  BreakActivate(FALSE);
  DhActivateGroups(FALSE);

  free(data.molPA);

  SgSetBackgroundColor(1.0f, 1.0f, 1.0f);
  GraphShowAlt(drawContact, NULL);

  return EC_OK;
}
