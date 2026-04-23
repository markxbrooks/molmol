/*
************************************************************************
*
*   ExFigRamach.c - FigRamach command
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdfig/SCCS/s.ExFigRamach.c
*   SCCS identification       : 1.23
*
************************************************************************
*/

#include <cmd_fig.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <break.h>
#include <hashtab.h>
#include <sg.h>
#include <arg.h>
#include <setup_file.h>
#include <data_hand.h>
#include <graph_draw.h>
#include <graph_draw.h>
#include "fig_util.h"

#define PROP_PLOT "_plot"
#define AXIS_SIZE_X 0.2f
#define AXIS_SIZE_Y 0.1f
#define BORDER_SIZE 0.05f
#define TICK_SIZE 0.03f
#define TICK_INC 20
#define LABEL_INC 60
#define AXIS_FONT_SIZE 0.035f
#define DIGIT_REL 0.8f
#define MARKER_FONT_SIZE 0.025f
#define MARKER_BORDER 0.01f

typedef enum {
  RB_NONE,
  RB_NORM,
  RB_VAL_ILE_THR,
  RB_GLY
} RamachBack;

typedef enum {
  RM_DOT,
  RM_PLUS,
  RM_CROSS,
  RM_RES,
  RM_RES_BOX,
  RM_MOL,
  RM_MOL_BOX
} RamachMarker;

typedef struct {
  DhResP resP;
  DhAngleP ang1P, ang2P;
} ResData;

static RamachBack BackChoice = RB_NORM;
static BOOL AngConstrSwitch = TRUE;
static BOOL ColorSwitch = TRUE;
static RamachMarker Marker = RM_DOT, AltMarker = RM_PLUS;

static DSTR Angle1 = NULL, Angle2 = NULL, AltRes = NULL;
static float CentX, CentY, ScaleX, ScaleY;
static BOOL Constr1Set, Constr2Set;
static float Constr1Min, Constr1Max, Constr2Min, Constr2Max;

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

static unsigned
hashFunc(void *p, unsigned size)
{
  ResData *dataP = p;

  return (unsigned) dataP->resP % size;
}

static int
compFunc(void *p1, void *p2)
{
  ResData *data1P = p1;
  ResData *data2P = p2;

  if (data1P->resP == data2P->resP)
    return 0;
  else
    return 1;
}

static void
addAngle(DhAngleP angP, void *clientData)
{
  HASHTABLE resTab = clientData;
  DSTR name;
  BOOL eq1;
  ResData data, *dataP;

  if (BreakCheck(1000))
    return;

  name = DhAngleGetName(angP);
  eq1 = (DStrCmp(name, Angle1) == 0);
  if (! eq1 && DStrCmp(name, Angle2) != 0)
    return;

  data.resP = DhAngleGetRes(angP);
  dataP = HashtabSearch(resTab, &data);

  if (dataP == NULL) {
    if (eq1) {
      data.ang1P = angP;
      data.ang2P = NULL;
    } else {
      data.ang1P = NULL;
      data.ang2P = angP;
    }

    (void) HashtabInsert(resTab, &data, FALSE);
  } else {
    if (eq1)
      dataP->ang1P = angP;
    else
      dataP->ang2P = angP;
  }

  if (eq1) {
    if (Constr1Set) {
      if (DhAngleGetMinVal(angP) != Constr1Min)
	Constr1Min = DH_ANGLE_MIN;
      if (DhAngleGetMaxVal(angP) != Constr1Max)
	Constr1Max = DH_ANGLE_MAX;
    } else {
      Constr1Min = DhAngleGetMinVal(angP);
      Constr1Max = DhAngleGetMaxVal(angP);
      Constr1Set = TRUE;
    }
  } else {
    if (Constr2Set) {
      if (DhAngleGetMinVal(angP) != Constr2Min)
	Constr2Min = DH_ANGLE_MIN;
      if (DhAngleGetMaxVal(angP) != Constr2Max)
	Constr2Max = DH_ANGLE_MAX;
    } else {
      Constr2Min = DhAngleGetMinVal(angP);
      Constr2Max = DhAngleGetMaxVal(angP);
      Constr2Set = TRUE;
    }
  }
}

static void
drawRegions(void)
{
  GFile gf;
  GFileRes res;
  INT32 i32;
  FLOAT32 f32;
  float colR, colG, colB;
  int levelNo, contNo, holeNo, pointNo;
  int levelI, contI, pointI;
  float (*points)[2];

  if (BackChoice == RB_NORM)
    gf = SetupOpen("", "RamachX", FALSE);
  else if (BackChoice == RB_VAL_ILE_THR)
    gf = SetupOpen("", "RamachVIT", FALSE);
  else
    gf = SetupOpen("", "RamachG", FALSE);

  if (gf == NULL)
    return;
  
  res = GFileReadINT32(gf, &i32);
  if (res != GF_RES_OK) {
    GFileClose(gf);
    return;
  }
  levelNo = i32;

  for (levelI = 0; levelI < levelNo; levelI++) {
    res = GFileReadINT32(gf, &i32);
    if (res != GF_RES_OK)
      break;
    contNo = i32;

    res = GFileReadINT32(gf, &i32);
    if (res != GF_RES_OK)
      break;
    holeNo = i32;

    res = GFileReadFLOAT32(gf, &f32);
    if (res != GF_RES_OK)
      break;
    colR = f32;

    res = GFileReadFLOAT32(gf, &f32);
    if (res != GF_RES_OK)
      break;
    colG = f32;

    res = GFileReadFLOAT32(gf, &f32);
    if (res != GF_RES_OK)
      break;
    colB = f32;

    SgSetColor(colR, colG, colB);

    for (contI = 0; contI < contNo + holeNo; contI++) {
      res = GFileReadINT32(gf, &i32);
      if (res != GF_RES_OK)
	break;
      pointNo = i32;

      if (contI == contNo)
	SgSetColor(1.0f, 1.0f, 1.0f);

      points = malloc(pointNo * sizeof(*points));

      for (pointI = 0; pointI < pointNo; pointI++) {
	res = GFileReadFLOAT32(gf, &f32);
	if (res != GF_RES_OK)
	  break;
	
	points[pointI][0] = CentX + ScaleX * f32 / 180.0f;

	res = GFileReadFLOAT32(gf, &f32);
	if (res != GF_RES_OK)
	  break;
	
	points[pointI][1] = CentY + ScaleY * f32 / 180.0f;
      }

      if (pointI < pointNo) {
	free(points);
	break;
      }

      SgDrawPolygon2D(points, pointNo);

      free(points);
    }

    if (contI < contNo)
      break;
  }

  GFileClose(gf);
}

static void
drawRect(float min1, float max1, float min2, float max2)
{
  float x[4][2];

  x[0][0] = CentX + ScaleX * min1 / 180.0f;
  x[0][1] = CentY + ScaleY * min2 / 180.0f;
  x[1][0] = CentX + ScaleX * max1 / 180.0f;
  x[1][1] = CentY + ScaleY * min2 / 180.0f;
  x[2][0] = CentX + ScaleX * max1 / 180.0f;
  x[2][1] = CentY + ScaleY * max2 / 180.0f;
  x[3][0] = CentX + ScaleX * min1 / 180.0f;
  x[3][1] = CentY + ScaleY * max2 / 180.0f;

  SgDrawPolygon2D(x, 4);
}

static void
drawAngConstr(void)
{
  if (Constr1Min == DH_ANGLE_MIN && Constr2Min == DH_ANGLE_MIN)
    return;

  if (Constr1Min == DH_ANGLE_MIN)
    Constr1Min = -180.0f;
  else if (Constr1Min < -180.0f)
    Constr1Min += 360.0f;

  if (Constr1Max == DH_ANGLE_MAX)
    Constr1Max = 180.0f;
  else if (Constr1Max > 180.0f)
    Constr1Max -= 360.0f;

  if (Constr2Min == DH_ANGLE_MIN)
    Constr2Min = -180.0f;
  else if (Constr2Min < -180.0f)
    Constr2Min += 360.0f;

  if (Constr2Max == DH_ANGLE_MAX)
    Constr2Max = 180.0f;
  else if (Constr2Max > 180.0f)
    Constr2Max -= 360.0f;

  SgSetColor(0.8f, 0.8f, 0.8f);

  if (Constr1Min < Constr1Max) {
    if (Constr2Min < Constr2Max) {
      drawRect(Constr1Min, Constr1Max, Constr2Min, Constr2Max);
    } else {
      drawRect(Constr1Min, Constr1Max, Constr2Min, 180.0f);
      drawRect(Constr1Min, Constr1Max, -180.0f, Constr2Max);
    }
  } else {
    if (Constr2Min < Constr2Max) {
      drawRect(Constr1Min, 180.0f, Constr2Min, Constr2Max);
      drawRect(-180.0f, Constr1Max, Constr2Min, Constr2Max);
    } else {
      drawRect(Constr1Min, 180.0f, Constr2Min, 180.0f);
      drawRect(Constr1Min, 180.0f, -180.0f, Constr2Max);
      drawRect(-180.0f, Constr1Max, Constr2Min, 180.0f);
      drawRect(-180.0f, Constr1Max, -180.0f, Constr2Max);
    }
  }
}

static BOOL
isAltRes(DSTR name)
{
  DSTR altName;
  char *s;
  BOOL ret;

  altName = DStrNew();
  s = DStrToStr(AltRes);

  ret = FALSE;
  for (;;) {
    if (*s == ' ') {
      ret = (DStrCmp(altName, name) == 0);
      if (ret)
	break;
      DStrAssignStr(altName, "");
    } else if (*s == '\0') {
      ret = (DStrCmp(altName, name) == 0);
      break;
    } else {
      DStrAppChar(altName, *s);
    }

    s++;
  }

  DStrFree(altName);

  return ret;
}

static void
drawNum(float x[2], int num, BOOL box)
{
  char buf[10];
  int len;
  float xl, xr, yb, yt, x0[2], xp[5][2];

  SgSetLineWidth(0.0f);
  SgSetFontSize(MARKER_FONT_SIZE);

  (void) sprintf(buf, "%d", num);
  len = strlen(buf);

  xl = x[0] - 0.5f * len * DIGIT_REL * MARKER_FONT_SIZE;
  yb = x[1] - 0.5f * MARKER_FONT_SIZE;

  x0[0] = xl;
  x0[1] = yb;

  SgDrawText2D(x0, buf);

  if (box) {
    xr = xl + len * DIGIT_REL * MARKER_FONT_SIZE;
    yt = yb + MARKER_FONT_SIZE;

    xl -= MARKER_BORDER;
    yb -= MARKER_BORDER;
    xr += MARKER_BORDER;
    yt += MARKER_BORDER;

    xp[0][0] = xl;
    xp[0][1] = yb;
    xp[1][0] = xr;
    xp[1][1] = yb;
    xp[2][0] = xr;
    xp[2][1] = yt;
    xp[3][0] = xl;
    xp[3][1] = yt;
    xp[4][0] = xl;
    xp[4][1] = yb;

    SgDrawPolyline2D(xp, 5);
  }
}

static void
drawMarker(float x[2], DhResP resP, RamachMarker marker)
{
  float x1[2], x2[2];

  switch (marker) {
    case RM_DOT:
      SgDrawDisc2D(x, 0.005f);
      break;
    case RM_PLUS:
      SgSetLineWidth(2.0f);
      x1[0] = x[0] - 0.01f;
      x1[1] = x[1];
      x2[0] = x[0] + 0.01f;
      x2[1] = x[1];
      SgDrawLine2D(x1, x2);
      x1[0] = x[0];
      x1[1] = x[1] - 0.01f;
      x2[0] = x[0];
      x2[1] = x[1] + 0.01f;
      SgDrawLine2D(x1, x2);
      break;
    case RM_CROSS:
      SgSetLineWidth(2.0f);
      x1[0] = x[0] - 0.01f;
      x1[1] = x[1] - 0.01f;
      x2[0] = x[0] + 0.01f;
      x2[1] = x[1] + 0.01f;
      SgDrawLine2D(x1, x2);
      x1[0] = x[0] - 0.01f;
      x1[1] = x[1] + 0.01f;
      x2[0] = x[0] + 0.01f;
      x2[1] = x[1] - 0.01f;
      SgDrawLine2D(x1, x2);
      break;
    case RM_RES:
      drawNum(x, DhResGetNumber(resP), FALSE);
      break;
    case RM_RES_BOX:
      drawNum(x, DhResGetNumber(resP), TRUE);
      break;
    case RM_MOL:
      drawNum(x, DhMolGetNumber(DhResGetMol(resP)) + 1, FALSE);
      break;
    case RM_MOL_BOX:
      drawNum(x, DhMolGetNumber(DhResGetMol(resP)) + 1, TRUE);
      break;
  }
}

static void
drawRes(void *p, void *clientData)
{
  ResData *dataP = p;
  float x[2];

  if (dataP->ang1P == NULL || dataP->ang2P == NULL)
    return;

  if (BreakCheck(1000))
    return;

  SgSetPickObj("Res", dataP->resP);

  if (ColorSwitch)
    FigSetAngleColor(dataP->ang1P);

  x[0] = CentX + ScaleX * DhAngleGetVal(dataP->ang1P) / 180.0f;
  x[1] = CentY + ScaleY * DhAngleGetVal(dataP->ang2P) / 180.0f;

  if (isAltRes(DhResGetName(dataP->resP)))
    drawMarker(x, dataP->resP, AltMarker);
  else
    drawMarker(x, dataP->resP, Marker);
}

static void
drawRamach(void *clientData)
{
  HASHTABLE resTab;

  resTab = HashtabOpen(997, sizeof(ResData), hashFunc, compFunc);
  
  Constr1Set = FALSE;
  Constr1Min = DH_ANGLE_MIN;
  Constr1Max = DH_ANGLE_MAX;
  Constr2Set = FALSE;
  Constr2Min = DH_ANGLE_MIN;
  Constr2Max = DH_ANGLE_MAX;
  DhApplyAngle(PropGetRef(PROP_PLOT, FALSE), addAngle, resTab);

  SgSetLight(SG_LIGHT_OFF);
  SgSetLineWidth(0.0f);
  SgSetLineStyle(SG_LINE_SOLID);

  CentX = 0.5f * (AXIS_SIZE_X - BORDER_SIZE);
  CentY = 0.5f * (AXIS_SIZE_Y - BORDER_SIZE);
  ScaleX = 1.0f - 0.5f * (AXIS_SIZE_X + BORDER_SIZE);
  ScaleY = 1.0f - 0.5f * (AXIS_SIZE_Y + BORDER_SIZE);

  if (BackChoice != RB_NONE)
    drawRegions();

  if (AngConstrSwitch)
    drawAngConstr();

  SgSetColor(0.0f, 0.0f, 0.0f);
  SgSetFontSize(AXIS_FONT_SIZE);

  FigDrawAxis(-1.0f + AXIS_SIZE_X, -1.0f + AXIS_SIZE_Y,
      2.0f - AXIS_SIZE_X - BORDER_SIZE, AO_HORIZ,
      DStrToStr(Angle1), -180.0f, 180.0f,
      DIGIT_REL * AXIS_FONT_SIZE, AXIS_FONT_SIZE,
      AT_LEFT, TICK_INC, TICK_SIZE, AL_ALL, LABEL_INC);

  FigDrawAxis(-1.0f + AXIS_SIZE_X, 1.0f - BORDER_SIZE,
      2.0f - AXIS_SIZE_X - BORDER_SIZE, AO_HORIZ,
      "", -180.0f, 180.0f,
      DIGIT_REL * AXIS_FONT_SIZE, AXIS_FONT_SIZE,
      AT_RIGHT, TICK_INC, TICK_SIZE, AL_NONE, LABEL_INC);

  FigDrawAxis(-1.0f + AXIS_SIZE_X, -1.0f + AXIS_SIZE_Y,
      2.0f - AXIS_SIZE_Y - BORDER_SIZE, AO_VERT,
      DStrToStr(Angle2), -180.0f, 180.0f,
      DIGIT_REL * AXIS_FONT_SIZE, AXIS_FONT_SIZE,
      AT_LEFT, TICK_INC, TICK_SIZE, AL_ALL, LABEL_INC);

  FigDrawAxis(1.0f - BORDER_SIZE, -1.0f + AXIS_SIZE_Y,
      2.0f - AXIS_SIZE_Y - BORDER_SIZE, AO_VERT,
      "", -180.0f, 180.0f,
      DIGIT_REL * AXIS_FONT_SIZE, AXIS_FONT_SIZE,
      AT_RIGHT, TICK_INC, TICK_SIZE, AL_NONE, LABEL_INC);

  HashtabApply(resTab, drawRes, NULL);

  HashtabClose(resTab);
}

#define ARG_NUM 7
#define BACK_NUM 4
#define MARKER_NUM 7
#define OPT_NUM 2

static void
fillMarkerEntry(EnumEntryDescr *entryA, RamachMarker marker)
{
  entryA[0].str = "dot";
  entryA[0].onOff = FALSE;
  entryA[1].str = "plus";
  entryA[1].onOff = FALSE;
  entryA[2].str = "cross";
  entryA[2].onOff = FALSE;
  entryA[3].str = "res";
  entryA[3].onOff = FALSE;
  entryA[4].str = "res_box";
  entryA[4].onOff = FALSE;
  entryA[5].str = "mol";
  entryA[5].onOff = FALSE;
  entryA[6].str = "mol_box";
  entryA[6].onOff = FALSE;

  entryA[marker].onOff = TRUE;
}

ErrCode
ExFigRamach(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  EnumEntryDescr backEntry[BACK_NUM];
  EnumEntryDescr markerEntry[MARKER_NUM];
  EnumEntryDescr altMarkerEntry[MARKER_NUM];
  EnumEntryDescr optEntry[OPT_NUM];
  ErrCode errCode;
  PropRefP refP;

  arg[0].type = AT_STR;
  arg[1].type = AT_STR;
  arg[2].type = AT_STR;
  arg[3].type = AT_ENUM;
  arg[4].type = AT_ENUM;
  arg[5].type = AT_ENUM;
  arg[6].type = AT_MULT_ENUM;

  ArgInit(arg, ARG_NUM);

  if (Angle1 == NULL) {
    Angle1 = DStrNew();
    DStrAssignStr(Angle1, "PHI");
    Angle2 = DStrNew();
    DStrAssignStr(Angle2, "PSI");
    AltRes = DStrNew();
    DStrAssignStr(AltRes, "GLY");
  }

  arg[0].prompt = "Angle 1";
  DStrAssignDStr(arg[0].v.strVal, Angle1);

  arg[1].prompt = "Angle 2";
  DStrAssignDStr(arg[1].v.strVal, Angle2);

  arg[2].prompt = "Alt. Res";
  DStrAssignDStr(arg[2].v.strVal, AltRes);

  backEntry[0].str = "none";
  backEntry[0].onOff = FALSE;
  backEntry[1].str = "normal";
  backEntry[1].onOff = FALSE;
  backEntry[2].str = "VAL-ILE-THR";
  backEntry[2].onOff = FALSE;
  backEntry[3].str = "GLY";
  backEntry[3].onOff = FALSE;
  backEntry[BackChoice].onOff = TRUE;

  arg[3].prompt = "Background";
  arg[3].u.enumD.entryP = backEntry;
  arg[3].u.enumD.n = BACK_NUM;
  arg[3].v.intVal = BackChoice;

  fillMarkerEntry(markerEntry, Marker);

  arg[4].prompt = "Marker";
  arg[4].u.enumD.entryP = markerEntry;
  arg[4].u.enumD.n = MARKER_NUM;
  arg[4].u.enumD.lineNo = 2;
  arg[4].v.intVal = Marker;

  fillMarkerEntry(altMarkerEntry, AltMarker);

  arg[5].prompt = "Alt. Marker";
  arg[5].u.enumD.entryP = altMarkerEntry;
  arg[5].u.enumD.n = MARKER_NUM;
  arg[5].u.enumD.lineNo = 2;
  arg[5].v.intVal = AltMarker;

  optEntry[0].str = "color";
  optEntry[0].onOff = ColorSwitch;
  optEntry[1].str = "angle constr.";
  optEntry[1].onOff = AngConstrSwitch;

  arg[6].prompt = "Options";
  arg[6].u.enumD.entryP = optEntry;
  arg[6].u.enumD.n = OPT_NUM;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  DStrAssignDStr(Angle1, arg[0].v.strVal);
  DStrAssignDStr(Angle2, arg[1].v.strVal);
  DStrAssignDStr(AltRes, arg[2].v.strVal);
  BackChoice = arg[3].v.intVal;
  Marker = arg[4].v.intVal;
  AltMarker = arg[5].v.intVal;
  ColorSwitch = optEntry[0].onOff;
  AngConstrSwitch = optEntry[1].onOff;

  ArgCleanup(arg, ARG_NUM);

  refP = PropGetRef(PROP_PLOT, TRUE);
  DhApplyRes(PropGetRef(PROP_ALL, FALSE), unmarkRes, refP);
  DhApplyAngle(PropGetRef(PROP_ALL, FALSE), unmarkAngle, refP);
  DhApplyAngle(PropGetRef(PROP_SELECTED, FALSE), markAngle, refP);

  SgSetBackgroundColor(1.0f, 1.0f, 1.0f);
  GraphShowAlt(drawRamach, NULL);

  return EC_OK;
}
