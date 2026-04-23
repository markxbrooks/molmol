/*
************************************************************************
*
*   ExFigCircles.c - FigCircles command
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdfig/SCCS/s.ExFigCircles.c
*   SCCS identification       : 1.9
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
#define BORDER_SIZE 0.1f
#define INTERPOL_NO 32

typedef struct {
  DhMolP *molPA;
  int molNo;
} MolList;

typedef struct {
  DhAngleP *anglePA;
  int angleNo;
} AngleList;

static int RowNo = 8;
static float FontSize = 0.3f;
static int MolNo, AngleNo;

static void
countAngle(DhAngleP angleP, void *clientData)
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
fillAngle(DhAngleP angleP, void *clientData)
{
  AngleList *listP = clientData;
  int angleI, i;

  if (BreakCheck(10000))
    return;

  if (listP->angleNo >= AngleNo)
    return;

  angleI = listP->angleNo - 1;
  if (DhAngleIsBackbone(angleP)) {
    /* insert backbone angle of same residue before
       non-backbone angles */
    while (angleI >= 0 &&
	! DhAngleIsBackbone(listP->anglePA[angleI]) &&
	DhAngleGetRes(angleP) == DhAngleGetRes(listP->anglePA[angleI]))
      angleI--;
    for (i = listP->angleNo - 1; i > angleI; i--)
      listP->anglePA[i + 1] = listP->anglePA[i];
  }

  listP->anglePA[angleI + 1] = angleP;
  listP->angleNo++;
}

static void
unmarkMol(DhMolP molP, void *clientData)
{
  PropRefP refP = clientData;

  DhMolSetProp(refP, molP, FALSE);
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
drawCircles(void *clientData)
{
  PropRefP refP;
  MolList molList;
  int angleNo, breakCount;
  AngleList *listA;
  float (*xA)[2];
  DSTR labelStr;
  char buf[20];
  DhAngleP angleP;
  float val, lastVal, t, ang;
  int xNo, interpolNo;
  float cx, cy, rad, rInc, tickSize, fontSize, labelOffs;
  int colNo, rowI, colI;
  float x1[2], x2[2], xTri[3][2];
  int molI, angleI, i;

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

  for (molI = 0; molI < molList.molNo; molI++) {
    angleNo = 0;
    DhMolApplyAngle(refP, molList.molPA[molI], countAngle, &angleNo);
    if (angleNo < AngleNo) {
      free(molList.molPA);
      DhActivateGroups(FALSE);
      return;
    }
  }

  listA = malloc(molList.molNo * sizeof(*listA));

  for (molI = 0; molI < molList.molNo; molI++) {
    listA[molI].angleNo = 0;
    listA[molI].anglePA = malloc(AngleNo * sizeof(DhAngleP));
    DhMolApplyAngle(refP, molList.molPA[molI], fillAngle, listA + molI);
  }

  DhActivateGroups(FALSE);

  colNo = (AngleNo - 1) / RowNo + 1;

  rad = (1.0f - BORDER_SIZE) / RowNo;
  tickSize = 0.5f * BORDER_SIZE / RowNo;
  fontSize = FontSize * rad;
  labelOffs = BORDER_SIZE / RowNo;

  if (FontSize > 0.3f) {
    rad -= (FontSize - 0.3f) / RowNo;
    labelOffs += 0.25f * (FontSize - 0.3f) / RowNo;
  }

  rInc = rad / molList.molNo;

  SgSetLight(SG_LIGHT_OFF);
  SgSetColor(0.0f, 0.0f, 0.0f);
  SgSetLineWidth(0.0f);
  SgSetLineStyle(SG_LINE_SOLID);
  SgSetFontSize(fontSize);

  xA = malloc(molList.molNo * INTERPOL_NO * sizeof(*xA));
  labelStr = DStrNew();
  breakCount = 20000 / (molList.molNo * INTERPOL_NO + 20);

  angleI = 0;
  for (angleI = 0; angleI < AngleNo; angleI++) {
    if (BreakCheck(breakCount))
      break;

    angleP = listA[0].anglePA[angleI];
    SgSetPickObj("Angle", angleP);

    rowI = RowNo - 1 - angleI / colNo;
    colI = angleI % colNo;

    cx = (float) (2 * colI - colNo + 1) / RowNo;
    cy = (float) (2 * rowI - RowNo + 1) / RowNo;

    x1[0] = cx;
    x1[1] = cy;
    SgDrawCircle2D(x1, rad);
    SgDrawDisc2D(x1, 0.5f * rInc);

    x1[0] = cx;
    x2[0] = cx;
    x1[1] = cy + rad;
    x2[1] = cy + rad + tickSize;
    SgDrawLine2D(x1, x2);
    x1[1] = cy - rad;
    x2[1] = cy - rad - tickSize;
    SgDrawLine2D(x1, x2);

    x1[1] = cy;
    x2[1] = cy;
    x1[0] = cx + rad;
    x2[0] = cx + rad + tickSize;
    SgDrawLine2D(x1, x2);
    x1[0] = cx - rad;
    x2[0] = cx - rad - tickSize;
    SgDrawLine2D(x1, x2);

    x1[0] = cx;
    x2[0] = cx;
    x1[1] = cy;
    x2[1] = cy + rad;
    SgSetLineStyle(SG_LINE_DASHED);
    SgDrawLine2D(x1, x2);
    SgSetLineStyle(SG_LINE_SOLID);

    xTri[0][0] = cx;
    xTri[0][1] = cy + rad + 1.5f * tickSize;
    xTri[1][0] = cx;
    xTri[1][1] = cy + rad - 1.5f * tickSize;
    xTri[2][0] = cx + 3.0f * tickSize;
    xTri[2][1] = cy + rad;
    SgDrawPolygon2D(xTri, 3);

    x1[0] = cx + 0.7f * rad;
    x1[1] = cy - 1.0f / RowNo + labelOffs;
    FigGetGreek(buf, DStrToStr(DhAngleGetName(angleP)));
    DStrAssignStr(labelStr, buf);
    (void) sprintf(buf, "%d", DhResGetNumber(DhAngleGetRes(angleP)));
    DStrAppStr(labelStr, "_{");
    DStrAppStr(labelStr, buf);
    DStrAppStr(labelStr, "}");
    SgDrawText2D(x1, DStrToStr(labelStr));

    xNo = 0;
    lastVal = 0.0f;

    for (molI = 0; molI < molList.molNo; molI++) {
      angleP = listA[molI].anglePA[angleI];
      val = DhAngleGetVal(angleP) * (float) M_PI / 180.0f;

      if (molI == 0) {
	interpolNo = 1;
      } else {
	if (val > lastVal + (float) M_PI)
	  lastVal += 2.0f * (float) M_PI;
	else if (val < lastVal - (float) M_PI)
	  lastVal -= 2.0f * (float) M_PI;

	if (val > lastVal)
	  interpolNo = (int) ((val - lastVal) / (float) M_PI * INTERPOL_NO);
	else
	  interpolNo = (int) ((lastVal - val) / (float) M_PI * INTERPOL_NO);
	
	if (interpolNo == 0)
	  interpolNo = 1;
      }

      for (i = 0; i < interpolNo; i++) {
	t = (float) (i + 1) / interpolNo;
	ang = (1.0f - t) * lastVal + t * val;

	xA[xNo][0] = cx + (molI + t) * rInc * sinf(ang);
	xA[xNo][1] = cy + (molI + t) * rInc * cosf(ang);

	xNo++;
      }

      lastVal = val;
    }

    SgDrawPolyline2D(xA, xNo);
  }

  DStrFree(labelStr);
  free(xA);

  free(molList.molPA);
  for (molI = 0; molI < molList.molNo; molI++)
    free(listA[molI].anglePA);
  free(listA);
}

#define ARG_NUM 2

ErrCode
ExFigCircles(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  PropRefP refP;
  DhMolP *molPA;
  int molI, angleNo1;

  arg[0].type = AT_INT;
  arg[1].type = AT_DOUBLE;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Row Number";
  arg[0].v.intVal = RowNo;

  arg[1].prompt = "Font Size";
  arg[1].v.doubleVal = FontSize;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);\
    return errCode;
  }

  RowNo = arg[0].v.intVal;
  if (RowNo < 1)
    RowNo = 1;
  FontSize = (float) arg[1].v.doubleVal;

  ArgCleanup(arg, ARG_NUM);

  DhActivateGroups(TRUE);

  MolNo = SelMolGet(NULL, 0);
  if (MolNo < 2) {
    DhActivateGroups(FALSE);
    CipSetError("at least 2 molecules must be selected");
    return EC_ERROR;
  }

  molPA = malloc(MolNo * sizeof(DhMolP));
  (void) SelMolGet(molPA, MolNo);

  refP = PropGetRef(PROP_SELECTED, FALSE);

  AngleNo = 0;
  DhMolApplyAngle(refP, molPA[0], countAngle, &AngleNo);
  if (AngleNo < 1) {
    free(molPA);
    DhActivateGroups(FALSE);
    CipSetError("at least 1 angle must be selected");
    return EC_ERROR;
  }

  for (molI = 1; molI < MolNo; molI++) {
    angleNo1 = 0;
    DhMolApplyAngle(refP, molPA[molI], countAngle, &angleNo1);
    if (angleNo1 != AngleNo) {
      free(molPA);
      DhActivateGroups(FALSE);
      CipSetError("number of selected angles must be equal");
      return EC_ERROR;
    }
  }

  free(molPA);

  refP = PropGetRef(PROP_PLOT, TRUE);
  DhApplyMol(PropGetRef(PROP_ALL, FALSE), unmarkMol, refP);
  DhApplyAngle(PropGetRef(PROP_ALL, FALSE), unmarkAngle, refP);
  DhApplyAngle(PropGetRef(PROP_SELECTED, FALSE), markAngle, refP);

  DhActivateGroups(FALSE);

  SgSetBackgroundColor(1.0f, 1.0f, 1.0f);
  GraphShowAlt(drawCircles, NULL);

  return EC_OK;
}
