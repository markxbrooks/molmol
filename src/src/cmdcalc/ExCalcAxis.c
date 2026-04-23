/*
************************************************************************
*
*   ExCalcAxis.c - CalcAxis command
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
*   Date of last modification : 01/06/02
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdcalc/SCCS/s.ExCalcAxis.c
*   SCCS identification       : 1.9
*
************************************************************************
*/

#include <cmd_calc.h>

#include <stdio.h>
#include <stdlib.h>
#include <values.h>
#include <math.h>

#include <break.h>
#include <princip_axis.h>
#include <pu.h>
#include <arg.h>
#include <data_hand.h>
#include <data_sel.h>
#include <prim_hand.h>
#include <graph_draw.h>

typedef struct {
  Vec3 *coordA;
  int atomI;
} AtomData;

typedef struct {
  int molNo;
  int bondNo;
  int molI;
  int bondI;
  Vec3 vA[3];
  float *val;
  char *formRes;
} BondData;

static BOOL ListBonds = FALSE;
static BOOL DoDisplay = FALSE;

static PuTextWindow TextW;

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
countAtoms(DhAtomP atomP, void *clientData)
{
  *(int *) clientData += 1;
}

static void
fillAtom(DhAtomP atomP, void *clientData)
{
  AtomData *dataP = clientData;

  DhAtomGetCoord(atomP, dataP->coordA[dataP->atomI]);
  dataP->atomI++;
}

static void
countBonds(DhBondP atomP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  *(int *) clientData += 1;
}

static void
getBondVal(DhBondP bondP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  BondData *dataP = clientData;
  Vec3 x1, v;
  int axisI;
  float c;

  DhAtomGetCoord(atom1P, x1);
  DhAtomGetCoord(atom2P, v);
  Vec3Sub(v, x1);
  Vec3Norm(v);

  for (axisI = 0; axisI < 3; axisI++) {
    c = Vec3Scalar(v, dataP->vA[axisI]);
    if (c < 0.0f)
      c = - c;
    dataP->val[3 * (dataP->bondI * dataP->molNo + dataP->molI) + axisI] =
      acosf(c) * 180.0f / (float) M_PI;
  }

  dataP->bondI++;
}

static void
writeBondVal(DhBondP bondP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  BondData *dataP = clientData;
  float *val;
  DSTR str;
  DhResP resP;
  char buf[20];
  float v, avg, der, min, max;
  int axisI, i;

  val = dataP->val + 3 * dataP->bondI * dataP->molNo;
  dataP->bondI++;

  str = DStrNew();
  resP = DhBondGetRes(bondP);

  (void) sprintf(buf, dataP->formRes, DhResGetNumber(resP));
  DStrAppStr(str, buf);
  (void) sprintf(buf, " %-5s", DStrToStr(DhResGetName(resP)));
  DStrAppStr(str, buf);
  (void) sprintf(buf, " %-5s", DStrToStr(DhAtomGetName(atom1P)));
  DStrAppStr(str, buf);
  (void) sprintf(buf, "-%-5s", DStrToStr(DhAtomGetName(atom2P)));
  DStrAppStr(str, buf);

  for (axisI = 0; axisI < 3; axisI++) {
    avg = 0.0f;
    min = MAXFLOAT;
    max = - MAXFLOAT;

    for (i = 0; i < dataP->molNo; i++) {
      v = val[3 * i + axisI];
      avg += v;
      if (v < min)
	min = v;
      if (v > max)
	max = v;
    }

    avg /= dataP->molNo;

    if (dataP->molNo > 1) {
      der = 0.0f;
      for (i = 0; i < dataP->molNo; i++) {
	v = val[3 * i + axisI];
	der += (v - avg) * (v - avg);
      }
      der = sqrtf(der / (dataP->molNo - 1));
    } else {
      der = 0.0f;
    }

    if (axisI > 0)
      DStrAppStr(str, " ");
    (void) sprintf(buf, " %4.1f", avg);
    DStrAppStr(str, buf);
    (void) sprintf(buf, " +/- %4.1f", der);
    DStrAppStr(str, buf);
    (void) sprintf(buf, " (%4.1f", min);
    DStrAppStr(str, buf);
    (void) sprintf(buf, "..%4.1f)", max);
    DStrAppStr(str, buf);
  }

  DStrAppChar(str, '\n');
  PuWriteStr(TextW, DStrToStr(str));
  DStrFree(str);
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
addCylinder(DhMolP molP, Vec3 cent, Vec3 dir, float len)
{
  Vec3 x1, v;
  PrimObjP  primP;

  Vec3Copy(x1, cent);
  Vec3ScaleSub(x1, len, dir);
  Vec3Copy(v, dir);
  Vec3Scale(v, 2.0f * len);

  primP = PrimNew(PT_CYLINDER, molP);
  PrimSetPos(primP, x1);
  PrimSetVec(primP, v);

  PrimSetProp(PropGetRef("cylinder", TRUE), primP, TRUE);
  PrimSetProp(PropGetRef("axis", TRUE), primP, TRUE);
}

#define ARG_NUM 1
#define OPT_NUM 2

ErrCode
ExCalcAxis(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  EnumEntryDescr enumEntry[OPT_NUM];
  ErrCode errCode;
  int molNo, molI, switchNo;
  DhMolP *molPA;
  int maxAtomNo, atomNo, bondNo, maxResI;
  char *form, buf[20];
  PropRefP refP;
  AtomData atomData;
  BondData bondData;
  Vec3 cent, dir;
  Mat4 mapM;
  float lenA[3], avgLen0, avgLen1, avgLen2, avgRel0, avgRel1;
  int i, k;

  arg[0].type = AT_MULT_ENUM;

  ArgInit(arg, ARG_NUM);

  enumEntry[0].str = "list bonds";
  enumEntry[0].onOff = ListBonds;
  enumEntry[1].str = "display";
  enumEntry[1].onOff = DoDisplay;

  arg[0].prompt = "Options";
  arg[0].u.enumD.entryP = enumEntry;
  arg[0].u.enumD.n = OPT_NUM;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  ListBonds = enumEntry[0].onOff;
  DoDisplay = enumEntry[1].onOff;

  DhActivateGroups(TRUE);

  molNo = SelMolGet(NULL, 0);
  if (molNo == 0) {
    DhActivateGroups(FALSE);
    CipSetError("at least one molecule must be selected");
    return EC_ERROR;
  }

  molPA = malloc(molNo * sizeof(*molPA));
  (void) SelMolGet(molPA, molNo);

  refP = PropGetRef(PROP_SELECTED, FALSE);

  /* ignore molecules with no atoms selected */
  switchNo = 0;
  maxAtomNo = 0;
  for (molI = 0; molI < molNo; molI++) {
    atomNo = 0;
    DhMolApplyAtom(refP, molPA[molI], countAtoms, &atomNo);
    if (atomNo > 0) {
      if (atomNo < 3) {
        DhActivateGroups(FALSE);
        CipSetError("at least 3 atoms must be selected");
        free(molPA);
        return EC_ERROR;
      }
      molPA[switchNo++] = molPA[molI];
      if (atomNo > maxAtomNo)
        maxAtomNo = atomNo;
    }
  }
  molNo = switchNo;

  if (molNo == 0) {
    DhActivateGroups(FALSE);
    CipSetError("at least one molecule must be selected");
    free(molPA);
    return EC_ERROR;
  }

  if (ListBonds) {
    bondData.bondNo = 0;
    DhMolApplyBond(refP, molPA[0], countBonds, &bondData.bondNo);
    for (molI = 1; molI < molNo; molI++) {
      bondNo = 0;
      DhMolApplyBond(refP, molPA[molI], countBonds, &bondNo);
      if (bondNo != bondData.bondNo) {
	DhActivateGroups(FALSE);
	CipSetError("same bonds must be selected in all structures");
	free(molPA);
	return EC_ERROR;
      }
    }
  }

  BreakActivate(TRUE);

  TextW = PuCreateTextWindow(cmd);

  PuWriteStr(TextW, "Absolute (relative) lengths of principal axes:\n");

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

  atomData.coordA = malloc(maxAtomNo * sizeof(Vec3));

  if (ListBonds && bondData.bondNo > 0) {
    bondData.molNo = molNo;

    maxResI = -1;
    DhMolApplyRes(refP, molPA[0], maxRes, &maxResI);
    bondData.formRes = getForm(maxResI);

    bondData.val = malloc(3 * molNo * bondData.bondNo * sizeof(*bondData.val));
  }

  avgLen0 = 0.0f;
  avgLen1 = 0.0f;
  avgLen2 = 0.0f;
  avgRel0 = 0.0f;
  avgRel1 = 0.0f;

  for (molI = 0; molI < molNo; molI++) {
    if (BreakCheck(1))
      break;

    (void) sprintf(buf, form, molI + 1);
    PuWriteStr(TextW, buf);

    atomData.atomI = 0;
    DhMolApplyAtom(refP, molPA[molI], fillAtom, &atomData);

    Vec3Zero(cent);
    for (i = 0; i < atomData.atomI; i++)
      Vec3Add(cent, atomData.coordA[i]);
    Vec3Scale(cent, 1.0f / atomData.atomI);
    for (i = 0; i < atomData.atomI; i++)
      Vec3Sub(atomData.coordA[i], cent);

    PrincipAxis(atomData.coordA, atomData.atomI, FALSE, mapM, lenA);

    (void) sprintf(buf, " %5.3f", lenA[0]);
    PuWriteStr(TextW, buf);
    (void) sprintf(buf, " %5.3f", lenA[1]);
    PuWriteStr(TextW, buf);
    (void) sprintf(buf, " %5.3f", lenA[2]);
    PuWriteStr(TextW, buf);
    (void) sprintf(buf, " (%5.3f", lenA[0] / lenA[2]);
    PuWriteStr(TextW, buf);
    (void) sprintf(buf, " %5.3f", lenA[1] / lenA[2]);
    PuWriteStr(TextW, buf);
    (void) sprintf(buf, " %5.3f)", 1.0);
    PuWriteStr(TextW, buf);
    PuWriteStr(TextW, "\n");

    avgLen0 += lenA[0];
    avgLen1 += lenA[1];
    avgLen2 += lenA[2];
    avgRel0 += lenA[0] / lenA[2];
    avgRel1 += lenA[1] / lenA[2];

    if (ListBonds && bondData.bondNo > 0) {
      bondData.molI = molI;
      bondData.bondI = 0;

      for (i = 0; i < 3; i++) {
	for (k = 0; k < 3; k++)
	  bondData.vA[i][k] = mapM[k][i];
	Vec3Norm(bondData.vA[i]);
      }

      DhMolApplyBond(refP, molPA[molI], getBondVal, &bondData);
    }

    if (DoDisplay) {
      for (i = 0; i < 3; i++) {
	for (k = 0; k < 3; k++)
	  dir[k] = mapM[k][i];
        addCylinder(molPA[molI], cent, dir, lenA[i]);
      }
    }
  }

  BreakActivate(FALSE);

  free(atomData.coordA);

  PuWriteStr(TextW, "\n");
  PuWriteStr(TextW, "average:");
  (void) sprintf(buf, " %5.3f", avgLen0 / molNo);
  PuWriteStr(TextW, buf);
  (void) sprintf(buf, " %5.3f", avgLen1 / molNo);
  PuWriteStr(TextW, buf);
  (void) sprintf(buf, " %5.3f", avgLen2 / molNo);
  PuWriteStr(TextW, buf);
  (void) sprintf(buf, " (%5.3f", avgRel0 / molNo);
  PuWriteStr(TextW, buf);
  (void) sprintf(buf, " %5.3f", avgRel1 / molNo);
  PuWriteStr(TextW, buf);
  (void) sprintf(buf, " %5.3f)", 1.0f);
  PuWriteStr(TextW, buf);
  PuWriteStr(TextW, "\n");

  if (ListBonds) {
    PuWriteStr(TextW,
	"\nAngles of bonds to principal axes (longest..shortest):\n");

    if (bondData.bondNo > 0) {
      bondData.bondI = 0;
      DhMolApplyBond(refP, molPA[0], writeBondVal, &bondData);

      free(bondData.val);
    }
  }

  free(molPA);

  DhActivateGroups(FALSE);

  if (DoDisplay) {
    GraphMolChanged(PROP_SELECTED);
    GraphRedrawNeeded();
  }

  return EC_OK;
}
