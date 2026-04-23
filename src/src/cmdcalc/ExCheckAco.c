/*
************************************************************************
*
*   ExCheckAco.c - CheckAco command
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdcalc/SCCS/s.ExCheckAco.c
*   SCCS identification       : 1.5
*
************************************************************************
*/

#include <cmd_calc.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <pu.h>
#include <arg.h>
#include <data_hand.h>
#include <data_sel.h>

typedef struct {
  int molNo;
  int angNo;
  int molI;
  int angI;
  float *val;
  char *formMol, *formRes;
} AngData;

static float Cutoff = 5.0f;
static int MinStrucNo = 1;

static PuTextWindow TextW;

static void
countAng(DhAngleP angleP, void *clientData)
{
  (* (int *) clientData)++;
}

static void
countRes(DhResP resP, void *clientData)
{
  (* (int *) clientData)++;
}

static void
getVal(DhAngleP angleP, void *clientData)
{
  AngData *dataP = clientData;
  float minVal, maxVal, val;
  float d1, d2;

  minVal = DhAngleGetMinVal(angleP);
  maxVal = DhAngleGetMaxVal(angleP);
  val = DhAngleGetVal(angleP);

  if (val < minVal) {
    d1 = minVal - val;
    val += 360.0f;
    if (val < maxVal) {
      d1 = 0.0;
    } else {
      d2 = val - maxVal;
      if (d2 < d1)
	d1 = d2;
    }
  } else if (val > maxVal) {
    d1 = val - maxVal;
    val -= 360.0f;
    if (val > minVal) {
      d1 = 0.0f;
    } else {
      d2 = minVal - val;
      if (d2 < d1)
	d1 = d2;
    }
  } else {
    d1 = 0.0f;
  }

  if (d1 < 0.0f)
    d1 = 0.0f;

  dataP->val[dataP->angI * dataP->molNo + dataP->molI] = d1;
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
  int valNo;
  float minVal, maxVal;
  DSTR str;
  DhResP resP;
  char buf[20];
  char ch;
  int i;

  val = dataP->val + dataP->angI * dataP->molNo;
  dataP->angI++;

  valNo = 0;
  maxVal = 0.0f;
  minVal = 360.0f;
  for (i = 0; i < dataP->molNo; i++) {
    if (val[i] > Cutoff)
      valNo++;
    
    if (val[i] > maxVal)
      maxVal = val[i];
    
    if (val[i] < minVal)
      minVal = val[i];
  }

  if (valNo < MinStrucNo)
    return;

  str = DStrNew();
  resP = DhAngleGetRes(angleP);

  (void) sprintf(buf, dataP->formRes, DhResGetNumber(resP));
  DStrAppStr(str, buf);
  (void) sprintf(buf, " %-5s", DStrToStr(DhResGetName(resP)));
  DStrAppStr(str, buf);
  (void) sprintf(buf, "%-6s", DStrToStr(DhAngleGetName(angleP)));
  DStrAppStr(str, buf);

  (void) sprintf(buf, dataP->formMol, valNo);
  DStrAppStr(str, buf);
  DStrAppStr(str, " (");
  (void) sprintf(buf, "%5.1f", minVal);
  DStrAppStr(str, buf);
  DStrAppStr(str, "..");
  (void) sprintf(buf, "%5.1f", maxVal);
  DStrAppStr(str, buf);
  DStrAppStr(str, ") ");

  for (i = 0; i < dataP->molNo; i++) {
    if (val[i] <= Cutoff)
      ch = ' ';
    else if (val[i] == maxVal)
      ch = '+';
    else if ((i + 1) % 10 == 0)
      ch = '0';
    else if ((i + 1) % 5 == 0)
      ch = '5';
    else
      ch = '*';

    DStrAppChar(str, ch);
  }

  DStrAppChar(str, '\n');
  PuWriteStr(TextW, DStrToStr(str));
  DStrFree(str);
}

static BOOL
checkAco(DhMolP *molPA, int molNo)
{
  char *form, buf[20];
  PropRefP refP;
  int angNo, resNo, molI;
  AngData data;

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
    return FALSE;
  }

  for (molI = 1; molI < molNo; molI++) {
    angNo = 0;
    DhMolApplyAngle(refP, molPA[molI], countAng, &angNo);
    if (angNo != data.angNo) {
      CipSetError("same angles must be selected in all structures");
      return FALSE;
    }
  }

  data.molNo = molNo;
  data.formMol = getForm(molNo);

  resNo = 0;
  DhMolApplyRes(refP, molPA[0], countRes, &resNo);
  data.formRes = getForm(resNo);

  data.val = malloc(molNo * data.angNo * sizeof(*data.val));

  for (data.molI = 0; data.molI < molNo; data.molI++) {
    data.angI = 0;
    DhMolApplyAngle(refP, molPA[data.molI], getVal, &data);
  }

  data.angI = 0;
  DhMolApplyAngle(refP, molPA[0], writeVal, &data);

  free(data.val);

  return TRUE;
}

#define ARG_NUM 2

ErrCode
ExCheckAco(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  char buf[20];
  int totMolNo, molNo, molI, i;
  DhMolP *molPA, *eqMolPA;
  BOOL ok;

  arg[0].type = AT_DOUBLE;
  arg[1].type = AT_INT;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Cutoff";
  arg[0].v.doubleVal = Cutoff;

  arg[1].prompt = "Min. Struc. #";
  arg[1].v.intVal = MinStrucNo;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  Cutoff = (float) arg[0].v.doubleVal;
  MinStrucNo = arg[1].v.intVal;

  ArgCleanup(arg, ARG_NUM);

  totMolNo = SelMolGet(NULL, 0);
  if (totMolNo == 0) {
    CipSetError("at least one molecule must be selected");
    return EC_ERROR;
  }

  TextW = PuCreateTextWindow(cmd);

  PuWriteStr(TextW, "Angle constraint violations > ");
  (void) sprintf(buf, "%.2f", Cutoff);
  PuWriteStr(TextW, buf);
  PuWriteStr(TextW, " in at least ");
  (void) sprintf(buf, "%d", MinStrucNo);
  PuWriteStr(TextW, buf);
  PuWriteStr(TextW, " structures:\n");

  molPA = malloc(totMolNo * sizeof(*molPA));
  (void) SelMolGet(molPA, totMolNo);

  eqMolPA = malloc(totMolNo * sizeof(*molPA));

  for (molI = 0; molI < totMolNo; molI++) {
    if (molPA[molI] == NULL)
      continue;

    eqMolPA[0] = molPA[molI];
    molNo = 1;

    for (i = molI + 1; i < totMolNo; i++) {
      if (molPA[i] == NULL)
	continue;

      if (DhMolEqualStruc(molPA[molI], molPA[i])) {
	eqMolPA[molNo++] = molPA[i];
	molPA[i] = NULL;
      }
    }

    molPA[molI] = NULL;

    ok = checkAco(eqMolPA, molNo);
    if (! ok)
      break;
  }

  free(molPA);
  free(eqMolPA);

  if (ok)
    return EC_OK;
  else
    return EC_ERROR;
}
