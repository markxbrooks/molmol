/*
************************************************************************
*
*   ExFigDist.c - FigDist command
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdfig/SCCS/s.ExFigDist.c
*   SCCS identification       : 1.10
*
************************************************************************
*/

#include <cmd_fig.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <values.h>
#include <math.h>

#include <break.h>
#include <sg.h>
#include <arg.h>
#include <data_hand.h>
#include <data_sel.h>
#include <graph_draw.h>
#include "fig_util.h"

#define PROP_PLOT "_plot"
#define AXIS_SIZE_X 0.2f
#define AXIS_SIZE_Y 0.12f
#define BORDER_SIZE 0.05f
#define TICK_SIZE 0.03f
#define FONT_SIZE 0.05f
#define DIGIT_SIZE_X (0.8f * FONT_SIZE)
#define DIGIT_SIZE_Y FONT_SIZE

typedef struct {
  DhMolP *molPA;
  int molNo;
} MolList;

typedef struct {
  DhDistP *distPA;
  int distNo;
} DistList;

static int RowNo = 4;
static float MinDist = 0.0f;
static float MaxDist = 9.0f;
static BOOL SubscrSwitch = FALSE;
static BOOL ConstrSwitch = TRUE;
static int MolNo, DistNo;

static void
countDist(DhDistP distP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  *(int *) clientData += 1;
}

static void
fillMol(DhMolP molP, void *clientData)
{
  MolList *listP = clientData;

  if (listP->molNo >= MolNo)
    return;

  listP->molPA[listP->molNo++] = molP;
}

static void
fillDist(DhDistP distP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  DistList *listP = clientData;

  if (listP->distNo >= DistNo)
    return;

  listP->distPA[listP->distNo++] = distP;
}

static void
unmarkMol(DhMolP molP, void *clientData)
{
  PropRefP refP = clientData;

  DhMolSetProp(refP, molP, FALSE);
}

static void
markMol(DhMolP molP, void *clientData)
{
  PropRefP refP = clientData;

  DhMolSetProp(refP, molP, TRUE);
}

static void
unmarkDist(DhDistP angP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  PropRefP refP = clientData;

  DhDistSetProp(refP, angP, FALSE);
}

static void
markDist(DhDistP angP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  PropRefP refP = clientData;

  DhDistSetProp(refP, angP, TRUE);
}

static void
drawDist(void *clientData)
{
  PropRefP refP;
  MolList molList;
  int distNo, breakCount;
  DistList distList;
  float cx, cy, sx, sy;
  float (*xA)[2];
  DSTR labelStr;
  char buf[20];
  int colNo, rowI, colI;
  int tickInc, labelInc;
  DhDistP distP;
  DhResP res1P, res2P;
  int res1I, res2I;
  DhAtomP atom1P, atom2P, a1P, a2P;
  Vec3 coord1, coord2;
  float val;
  float x1[2], x2[2];
  int molI, distI;

  DhActivateGroups(TRUE);

  refP = PropGetRef(PROP_PLOT, FALSE);

  molList.molPA = malloc(MolNo * sizeof(*molList.molPA));
  molList.molNo = 0;
  DhApplyMol(refP, fillMol, &molList);
  if (molList.molNo < 2) {
    free(molList.molPA);
    DhActivateGroups(FALSE);
    return;
  }

  distNo = 0;
  DhMolApplyDist(refP, molList.molPA[0], countDist, &distNo);
  if (distNo < DistNo) {
    free(molList.molPA);
    DhActivateGroups(FALSE);
    return;
  }

  distList.distNo = 0;
  distList.distPA = malloc(DistNo * sizeof(DhDistP));
  DhMolApplyDist(refP, molList.molPA[0], fillDist, &distList);

  DhActivateGroups(FALSE);

  colNo = (DistNo - 1) / RowNo + 1;
  sx = 2.0f / colNo;
  sy = 2.0f / RowNo;

  SgSetLight(SG_LIGHT_OFF);
  SgSetColor(0.0f, 0.0f, 0.0f);
  SgSetLineWidth(0.0f);
  SgSetLineStyle(SG_LINE_SOLID);
  SgSetFontSize(sy * FONT_SIZE);

  xA = malloc(molList.molNo * sizeof(*xA));
  labelStr = DStrNew();
  breakCount = 20000 / (MolNo + 10);

  for (distI = 0; distI < DistNo; distI++) {
    if (BreakCheck(breakCount))
      break;

    distP = distList.distPA[distI];
    SgSetPickObj("Dist", distP);

    atom1P = DhDistGetAtom1(distP);
    atom2P = DhDistGetAtom2(distP);

    res1I = DhResGetNumber(DhAtomGetRes(atom1P));
    res2I = DhResGetNumber(DhAtomGetRes(atom2P));

    rowI = RowNo - 1 - distI / colNo;
    colI = distI % colNo;

    cx = (float) (2 * colI - colNo) / colNo;
    cy = (float) (2 * rowI - RowNo) / RowNo;

    if (MolNo <= 5) {
      tickInc = 1;
      labelInc = 1;
    } else if (MolNo <= 10) {
      tickInc = 1;
      labelInc = 2;
    } else if (MolNo <= 20) {
      tickInc = 1;
      labelInc = 5;
    } else if (MolNo <= 50) {
      tickInc = 5;
      labelInc = 10;
    } else if (MolNo <= 100) {
      tickInc = 10;
      labelInc = 20;
    } else if (MolNo <= 200) {
      tickInc = 10;
      labelInc = 50;
    } else {
      tickInc = 50;
      labelInc = 100;
    }

    FigDrawAxis(cx + AXIS_SIZE_X * sx, cy + AXIS_SIZE_Y * sy,
	sx * (1.0f - AXIS_SIZE_X - BORDER_SIZE), AO_HORIZ,
	"#", 1.0f, (float) MolNo, sy * DIGIT_SIZE_X, sy * DIGIT_SIZE_Y,
	AT_LEFT, tickInc, sy * TICK_SIZE, AL_ALL, labelInc);

    if (MaxDist - MinDist <= 5.0f) {
      tickInc = 1;
      labelInc = 1;
    } else {
      tickInc = 1;
      labelInc = 2;
    }

    FigDrawAxis(cx + AXIS_SIZE_X * sx, cy + AXIS_SIZE_Y * sy,
	sy * (1.0f - AXIS_SIZE_Y - BORDER_SIZE), AO_VERT,
	"d", MinDist, MaxDist, sy * DIGIT_SIZE_X, sy * DIGIT_SIZE_Y,
	AT_LEFT, tickInc, sx * TICK_SIZE, AL_ALL, labelInc);

    x1[0] = cx + sx * 1.2f * AXIS_SIZE_X;
    x1[1] = cy + sy * (1.0f - BORDER_SIZE - DIGIT_SIZE_Y);
    DStrAssignDStr(labelStr, DhAtomGetName(atom1P));
    if (SubscrSwitch)
      DStrAppStr(labelStr, "_{");
    (void) sprintf(buf, "%d", res1I);
    DStrAppStr(labelStr, buf);
    if (SubscrSwitch)
      DStrAppStr(labelStr, "}");
    DStrAppStr(labelStr, "-");
    DStrAppDStr(labelStr, DhAtomGetName(atom2P));
    if (SubscrSwitch)
      DStrAppStr(labelStr, "_{");
    (void) sprintf(buf, "%d", res2I);
    DStrAppStr(labelStr, buf);
    if (SubscrSwitch)
      DStrAppStr(labelStr, "}");
    SgDrawText2D(x1, DStrToStr(labelStr));

    if (ConstrSwitch && (DhDistGetKind(distP) == DK_UPPER ||
        DhDistGetKind(distP) == DK_LOWER)) {
      val = DhDistGetLimit(distP);
      x1[0] = cx + AXIS_SIZE_X * sx;
      x1[1] = cy + sy * AXIS_SIZE_Y +
          sy * (1.0f - AXIS_SIZE_Y - BORDER_SIZE) *
	  (val - MinDist) / (MaxDist - MinDist);
      x2[0] = cx + (1.0f - BORDER_SIZE) * sx;
      x2[1] = x1[1];
      SgDrawLine2D(x1, x2);
    }

    for (molI = 0; molI < molList.molNo; molI++) {
      val = 0.0f;

      res1P = DhResFind(molList.molPA[molI], res1I);
      res2P = DhResFind(molList.molPA[molI], res2I);

      if (res1P != NULL && res2P != NULL) {
	a1P = DhAtomFindNumber(res1P, DhAtomGetNumber(atom1P), FALSE);
	a2P = DhAtomFindNumber(res2P, DhAtomGetNumber(atom2P), FALSE);

	if (a1P != NULL && atom2P != NULL) {
	  DhAtomGetCoord(a1P, coord1);
	  DhAtomGetCoord(a2P, coord2);
	  val = Vec3DiffAbs(coord1, coord2);
	}
      }

      xA[molI][0] = cx + sx * AXIS_SIZE_X +
	  sx * (1.0f - AXIS_SIZE_X - BORDER_SIZE) * molI / (MolNo - 1);
      xA[molI][1] = cy + sy * AXIS_SIZE_Y +
	  sy * (1.0f - AXIS_SIZE_Y - BORDER_SIZE) *
	  (val - MinDist) / (MaxDist - MinDist);
    }

    SgDrawPolyline2D(xA, molList.molNo);
  }

  DStrFree(labelStr);
  free(xA);

  free(molList.molPA);
  free(distList.distPA);
}

#define ARG_NUM 4
#define OPT_NUM 2

ErrCode
ExFigDist(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  EnumEntryDescr optEntry[OPT_NUM];
  ErrCode errCode;
  PropRefP refP;
  DhMolP molP;

  arg[0].type = AT_INT;
  arg[1].type = AT_DOUBLE;
  arg[2].type = AT_DOUBLE;
  arg[3].type = AT_MULT_ENUM;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Row Number";
  arg[0].v.intVal = RowNo;

  arg[1].prompt = "Min. Dist";
  arg[1].v.doubleVal = MinDist;

  arg[2].prompt = "Max. Dist";
  arg[2].v.doubleVal = MaxDist;

  optEntry[0].str = "res # subscript";
  optEntry[0].onOff = SubscrSwitch;
  optEntry[1].str = "show constraints";
  optEntry[1].onOff = ConstrSwitch;

  arg[3].prompt = "Options";
  arg[3].u.enumD.entryP = optEntry;
  arg[3].u.enumD.n = OPT_NUM;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);\
    return errCode;
  }

  RowNo = arg[0].v.intVal;
  if (RowNo < 1)
    RowNo = 1;

  MinDist = (float) arg[1].v.doubleVal;
  MaxDist = (float) arg[2].v.doubleVal;

  SubscrSwitch = optEntry[0].onOff;
  ConstrSwitch = optEntry[1].onOff;

  ArgCleanup(arg, ARG_NUM);

  if (MaxDist < MinDist + 2.0f)
    MaxDist = MinDist + 2.0f;

  DhActivateGroups(TRUE);

  MolNo = SelMolGet(&molP, 1);
  if (MolNo < 2) {
    DhActivateGroups(FALSE);
    CipSetError("at least 2 molecules must be selected");
    return EC_ERROR;
  }

  refP = PropGetRef(PROP_SELECTED, FALSE);

  DistNo = 0;
  DhMolApplyDist(refP, molP, countDist, &DistNo);
  if (DistNo < 1) {
    DhActivateGroups(FALSE);
    CipSetError("at least 1 distance must be selected");
    return EC_ERROR;
  }

  refP = PropGetRef(PROP_PLOT, TRUE);
  DhApplyMol(PropGetRef(PROP_ALL, FALSE), unmarkMol, refP);
  DhApplyDist(PropGetRef(PROP_ALL, FALSE), unmarkDist, refP);
  DhApplyMol(PropGetRef(PROP_ALL, FALSE), markMol, refP);
  DhApplyDist(PropGetRef(PROP_SELECTED, FALSE), markDist, refP);

  DhActivateGroups(FALSE);

  SgSetBackgroundColor(1.0f, 1.0f, 1.0f);
  GraphShowAlt(drawDist, NULL);

  return EC_OK;
}
