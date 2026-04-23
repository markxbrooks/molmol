/*
************************************************************************
*
*   ExCalcAngle.c - CalcAngle command
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdcalc/SCCS/s.ExCalcAngle.c
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include <cmd_calc.h>

#include <stdio.h>
#include <stdlib.h>
#include <values.h>
#include <math.h>

#include <pu.h>
#include <data_hand.h>
#include <data_sel.h>

typedef struct {
  int molNo;
  int angNo;
  int molI;
  int angI;
  float *val;
  char *formRes;
} AngData;

static PuTextWindow TextW;

static void
countAng(DhAngleP angleP, void *clientData)
{
  (* (int *) clientData)++;
}

static void
maxRes(DhResP resP, void *clientData)
{
  int *maxResIP = clientData;
  int resI;

  resI = DhResGetNumber(resP);
  if (resI > *maxResIP)
    *maxResIP = resI;
}

static void
getVal(DhAngleP angleP, void *clientData)
{
  AngData *dataP = clientData;

  dataP->val[dataP->angI * dataP->molNo + dataP->molI] =
      DhAngleGetVal(angleP) * (float) M_PI / 180.0f;
  dataP->angI++;
}

static char *
getForm(int maxVal)
{
  if (maxVal >= 100000)
    return "%6d";

  if (maxVal >= 10000)
    return "%5d";

  if (maxVal >= 1000)
    return "%4d";

  if (maxVal >= 100)
    return "%3d";

  if (maxVal >= 10)
    return "%2d";

  return "%1d";
}

static void
writeVal(DhAngleP angleP, void *clientData)
{
  AngData *dataP = clientData;
  float *val;
  DSTR str;
  DhResP resP;
  char buf[20];
  float xs, ys, ang, len, dev, d;
  int i;

  val = dataP->val + dataP->angI * dataP->molNo;
  dataP->angI++;

  str = DStrNew();
  resP = DhAngleGetRes(angleP);

  (void) sprintf(buf, dataP->formRes, DhResGetNumber(resP));
  DStrAppStr(str, buf);
  (void) sprintf(buf, " %-5s", DStrToStr(DhResGetName(resP)));
  DStrAppStr(str, buf);
  (void) sprintf(buf, "%-6s ", DStrToStr(DhAngleGetName(angleP)));
  DStrAppStr(str, buf);

  xs = 0.0f;
  ys = 0.0f;
  for (i = 0; i < dataP->molNo; i++) {
    xs += cosf(val[i]);
    ys += sinf(val[i]);
  }

  ang = atan2f(ys, xs);
  len = sqrtf(xs * xs + ys * ys) / dataP->molNo;

  dev = 0.0f;
  if (dataP->molNo > 1) {
    for (i = 0; i < dataP->molNo; i++) {
      d = val[i] - ang;
      if (d > (float) M_PI)
	d -= 2.0f * (float) M_PI;
      else if (d < - (float) M_PI)
	d += 2.0f * (float) M_PI;
      dev += d * d;
    }
    dev = sqrtf(dev / (dataP->molNo - 1));
  }

  (void) sprintf(buf, "%6.1f", ang * 180.0f / (float) M_PI);
  DStrAppStr(str, buf);
  (void) sprintf(buf, " +/- %6.1f", dev * 180.0f / (float) M_PI);
  DStrAppStr(str, buf);
  (void) sprintf(buf, "  %5.3f", len);
  DStrAppStr(str, buf);

  DStrAppChar(str, '\n');
  PuWriteStr(TextW, DStrToStr(str));
  DStrFree(str);
}

ErrCode
ExCalcAngle(char *cmd)
{
  int molNo, molI;
  DhMolP *molPA;
  char *form, buf[20];
  PropRefP refP;
  int angNo;
  int maxResI;
  AngData data;

  molNo = SelMolGet(NULL, 0);
  if (molNo == 0) {
    CipSetError("at least one molecule must be selected");
    return EC_ERROR;
  }

  TextW = PuCreateTextWindow(cmd);

  PuWriteStr(TextW, "Average angle and angular order parameters:\n");

  molPA = malloc(molNo * sizeof(*molPA));
  (void) SelMolGet(molPA, molNo);

  PuWriteStr(TextW, "\n");
  form = getForm(molNo);
  for (molI = 0; molI < molNo; molI++) {
    (void) sprintf(buf, form, molI + 1);
    PuWriteStr(TextW, buf);
    PuWriteStr(TextW, " ");
    PuWriteStr(TextW, DStrToStr(DhMolGetName(molPA[molI])));
    PuWriteStr(TextW, "\n");
  }
  PuWriteStr(TextW, "\n");

  refP = PropGetRef(PROP_SELECTED, FALSE);

  data.angNo = 0;
  DhMolApplyAngle(refP, molPA[0], countAng, &data.angNo);
  if (data.angNo == 0) {
    CipSetError("at least one angle must be selected");
    free(molPA);
    return EC_ERROR;
  }

  for (molI = 1; molI < molNo; molI++) {
    angNo = 0;
    DhMolApplyAngle(refP, molPA[molI], countAng, &angNo);
    if (angNo != data.angNo) {
      CipSetError("same angles must be selected in all structures");
      free(molPA);
      return EC_ERROR;
    }
  }

  data.molNo = molNo;

  maxResI = -1;
  DhMolApplyRes(refP, molPA[0], maxRes, &maxResI);
  data.formRes = getForm(maxResI);

  data.val = malloc(molNo * data.angNo * sizeof(*data.val));

  for (data.molI = 0; data.molI < molNo; data.molI++) {
    data.angI = 0;
    DhMolApplyAngle(refP, molPA[data.molI], getVal, &data);
  }

  data.angI = 0;
  DhMolApplyAngle(refP, molPA[0], writeVal, &data);

  free(data.val);
  free(molPA);

  return EC_OK;
}
