/*
************************************************************************
*
*   ExRotAngle.c - RotateAngle command
*
*   Copyright (c) 1994
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdstruc/SCCS/s.ExRotAngle.c
*   SCCS identification       : 1.5
*
************************************************************************
*/

#include <cmd_struc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arg.h>
#include <data_hand.h>
#include <data_sel.h>
#include <graph_draw.h>

#define MAX_ANGLE_NO 20

static DhAngleP AnglePA[MAX_ANGLE_NO];
static int AngleNo;

static BOOL
verifyIndex(ArgDescrP argP)
{
  int idx, i;

  idx = argP->v.intVal;
  for (i = 0; i < AngleNo; i++)
    DhRotAddAngle(AnglePA[i], i >= idx);

  return TRUE;
}

static BOOL
verifyAngle(ArgDescrP argP)
{
  DhAngleP angleP = argP->userData;

  DhRotSetAngle(angleP, (float) argP->v.doubleVal);
  GraphRedraw();

  return TRUE;
}

static void
recalcCoord(DhMolP molP, void *clientData)
{
  DhMolAnglesChanged(molP);
}

ErrCode
ExRotateAngle(char *cmd)
{
  ArgDescr arg[1 + MAX_ANGLE_NO];
  ErrCode errCode;
  DSTR dstr;
  char numStr[7];
  int i;

  AngleNo = SelAngleGet(AnglePA, MAX_ANGLE_NO);
  if (AngleNo > MAX_ANGLE_NO)
    AngleNo = MAX_ANGLE_NO;

  if (AngleNo == 0) {
    CipSetError("at least 1 angle must be selected");
    return EC_ERROR;
  }

  arg[0].type = AT_INT;
  for (i = 0; i < AngleNo; i++)
    arg[1 + i].type = AT_DOUBLE;

  ArgInit(arg, AngleNo + 1);

  dstr = DStrNew();

  arg[0].prompt = "backwards";
  arg[0].v.intVal = 0;
  arg[0].verifyFunc = verifyIndex;
  arg[0].useSlider = TRUE;
  arg[0].u.sliderD.minVal = 0.0f;
  arg[0].u.sliderD.maxVal = (float) AngleNo;
  arg[0].u.sliderD.digits = 0;

  for (i = 0; i < AngleNo; i++) {
    DStrAssignDStr(dstr, DhResGetName(DhAngleGetRes(AnglePA[i])));
    (void) sprintf(numStr, "%d", DhResGetNumber(DhAngleGetRes(AnglePA[i])));
    DStrAppStr(dstr, numStr);
    DStrAppChar(dstr, ' ');
    DStrAppDStr(dstr, DhAngleGetName(AnglePA[i]));
    arg[1 + i].prompt = malloc(DStrLen(dstr) + 1);
    (void) strcpy(arg[1 + i].prompt, DStrToStr(dstr));

    arg[1 + i].verifyFunc = verifyAngle;
    arg[1 + i].v.doubleVal = DhAngleGetVal(AnglePA[i]);
    arg[1 + i].useSlider = TRUE;
    arg[1 + i].u.sliderD.minVal = -180.0f;
    arg[1 + i].u.sliderD.maxVal = 180.0f;
    arg[1 + i].u.sliderD.digits = 1;

    arg[1 + i].userData = AnglePA[i];

    DhRotAddAngle(AnglePA[i], TRUE);
  }

  DStrFree(dstr);

  errCode = ArgGet(arg, AngleNo + 1);

  if (errCode == EC_OK) {
    DhRotEnd(TRUE);
    DhApplyMol(PropGetRef(PROP_SELECTED, FALSE), recalcCoord, NULL);
    GraphMolChanged(PROP_SELECTED);
  } else {
    DhRotEnd(FALSE);
  }

  for (i = 0; i < AngleNo; i++)
    free(arg[i + 1].prompt);

  ArgCleanup(arg, AngleNo + 1);
  GraphRedrawNeeded();

  return errCode;
}
