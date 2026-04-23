/*
************************************************************************
*
*   ExCalcShift.c - CalcShift command
*
*   Copyright (c) 1994-96
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
*   Date of last modification : 01/05/26
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdcalc/SCCS/s.ExCalcShift.c
*   SCCS identification       : 1.6
*
************************************************************************
*/

#include <cmd_calc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <values.h>

#include <break.h>
#include <ell_integ.h>
#include <mat_vec.h>
#include <linlist.h>
#include <grid.h>
#include <pu.h>
#include <arg.h>
#include <par_names.h>
#include <setup_file.h>
#include <data_hand.h>

/* amide requires some special treatment */
#define AMIDE_NAME "HN"

#define PHYS_E ((float) 4.80325e-10)
#define PHYS_M ((float) 9.10956e-28)
#define PHYS_C ((float) 2.99792e10)

typedef enum {
  CM_CURRENT,
  CM_ORBITAL
} CalcMethod;

typedef struct {
  DSTR resName;
  int atomNo;
  DSTR *nameA;
  float ringFact, polFact;
} ModelRing;

typedef struct {
  DSTR name;
  CalcMethod method;
  LINLIST mRingList;
} ModelData;

typedef struct {
  ModelRing *mRingP;
  DhResP resP;
  Vec3 *coordA;
  Vec3 center, norm;
  Mat3 transM;
  float rad;
} RingData;

typedef struct {
  int molNo, molI;
  int atomNo, atomI;
  DSTR *atomNameA;
  float *ringValA, *polValA;
  char *formRes;
  ModelData *modelP;
  LINLIST ringList;
  GRID grid;
  DhAtomP atomP;
  Vec3 polSumV;
} CalcData;

typedef struct {
  DhAtomP atomP;
  DhAtomP neighAtomP;
} NeighData;

static BOOL DoRing = TRUE;
static BOOL DoPol = FALSE;
static int ModelInd = 0;
static float ContribCutoff = 0.5f;
static float MaxDist = 5.0f;

static LINLIST ModelList = NULL;

static PuTextWindow TextW;

static void
freeModelData(void *p, void *clientData)
{
  ModelData *modelP = p;

  DStrFree(modelP->name);
  ListClose(modelP->mRingList);
}

static void
freeModelRing(void *p, void *clientData)
{
  ModelRing *mRingP = p;
  int i;

  DStrFree(mRingP->resName);
  if (mRingP->atomNo > 0) {
    for (i = 0; i < mRingP->atomNo; i++)
      DStrFree(mRingP->nameA[i]);
    free(mRingP->nameA);
  }
}

static void
readModels(void)
{
  GFile gf;
  GFileRes res;
  FLOAT32 f32;
  INT32 i32;
  char buf[100];
  ModelData model, *modelP;
  ModelRing mRing, *mRingP;
  int groupNo, mRingNo;
  int i;

  if (ModelList != NULL)
    return;

  ModelList = ListOpen(sizeof(ModelData));
  ListAddDestroyCB(ModelList, freeModelData, NULL, NULL);

  gf = SetupOpen(PN_RING_SHIFT, "RingShift", FALSE);
  if (gf == NULL)
    return;
  
  res = GF_RES_OK;
  while (! GFileEOF(gf)) {
    model.name = DStrNew();
    model.mRingList = ListOpen(sizeof(ModelRing));
    ListAddDestroyCB(model.mRingList, freeModelRing, NULL, NULL);

    modelP = ListInsertLast(ModelList, &model);

    res = GFileReadStr(gf, buf, sizeof(buf));
    if (res != GF_RES_OK)
      break;
    DStrAssignStr(modelP->name, buf);

    res = GFileReadStr(gf, buf, sizeof(buf));
    if (res != GF_RES_OK)
      break;
    if (strcmp(buf, "current") == 0)
      modelP->method = CM_CURRENT;
    else
      modelP->method = CM_ORBITAL;

    res = GFileReadINT32(gf, &i32);
    if (res != GF_RES_OK)
      break;
    groupNo = i32;

    while (groupNo > 0) {
      res = GFileReadINT32(gf, &i32);
      if (res != GF_RES_OK)
	break;
      mRingNo = i32;

      res = GFileReadFLOAT32(gf, &f32);
      if (res != GF_RES_OK)
	break;
      mRing.polFact = f32 * (float) 1.0e-12;  /* given in 10^-12 esu */

      while (mRingNo > 0) {
	mRing.resName = DStrNew();
	mRing.atomNo = 0;

	mRingP = ListInsertLast(modelP->mRingList, &mRing);

	res = GFileReadStr(gf, buf, sizeof(buf));
	if (res != GF_RES_OK)
	  break;
	DStrAssignStr(mRingP->resName, buf);

	res = GFileReadFLOAT32(gf, &f32);
	if (res != GF_RES_OK)
	  break;
	mRingP->ringFact = f32;

	res = GFileReadINT32(gf, &i32);
	if (res != GF_RES_OK)
	  break;
	mRingP->atomNo = i32;
	mRingP->nameA = malloc(mRingP->atomNo * sizeof(*mRingP->nameA));
	for (i = 0; i < mRingP->atomNo; i++)
	  mRingP->nameA[i] = DStrNew();
	for (i = 0; i < mRingP->atomNo; i++) {
	  res = GFileReadStr(gf, buf, sizeof(buf));
	  if (res != GF_RES_OK)
	    break;
	  DStrAssignStr(mRingP->nameA[i], buf);
	}

	mRingNo--;
      }

      groupNo--;
    }
  }

  if (res != GF_RES_OK) {
    ListClose(ModelList);
    ModelList = NULL;
  }

  GFileClose(gf);
}

static void
countAtoms(DhAtomP atomP, void *clientData)
{
  *(int *) clientData += 1;
}

static void
countMolAtoms(DhMolP molP, void *clientData)
{
  CalcData *dataP = clientData;
  int atomNo;

  atomNo = 0;
  DhMolApplyAtom(PropGetRef(PROP_SELECTED, FALSE), molP, countAtoms, &atomNo);

  if (dataP->molNo == 0)
    dataP->atomNo = atomNo;
  else if (atomNo != dataP->atomNo)
    dataP->atomNo = -1;

  dataP->molNo++;
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
freeRing(void *p, void *clientData)
{
  RingData *ringP = p;

  free(ringP->coordA);
}

static void
calcPlane(RingData *ringP)
{
  Vec3 v1, v2;
  int atomI;

  Vec3Zero(ringP->center);
  for (atomI = 0; atomI < ringP->mRingP->atomNo; atomI++)
    Vec3Add(ringP->center, ringP->coordA[atomI]);
  Vec3Scale(ringP->center, 1.0f / ringP->mRingP->atomNo);

  Vec3Zero(ringP->norm);
  ringP->rad = 0.0f;
  Vec3Copy(v1, ringP->coordA[ringP->mRingP->atomNo - 1]);
  Vec3Sub(v1, ringP->center);
  for (atomI = 0; atomI < ringP->mRingP->atomNo; atomI++) {
    Vec3Copy(v2, ringP->coordA[atomI]);
    Vec3Sub(v2, ringP->center);
    ringP->rad += Vec3Abs(v2);
    Vec3Cross(v1, v2);
    Vec3Add(ringP->norm, v1);
    Vec3Copy(v1, v2);
  }
  Vec3Norm(ringP->norm);
  ringP->rad /= ringP->mRingP->atomNo;

  /* transformation matrix ring plane -> x-y plane */
  ringP->transM[0][2] = ringP->norm[0];
  ringP->transM[1][2] = ringP->norm[1];
  ringP->transM[2][2] = ringP->norm[2];

  /* vector orthgonal to norm */
  if (ringP->norm[0] > 0.5f || ringP->norm[0] < -0.5f) {
    v1[0] = - ringP->norm[1];
    v1[1] = ringP->norm[0];
    v1[2] = 0.0f;
  } else {
    v1[0] = 0.0f;
    v1[1] = - ringP->norm[2];
    v1[2] = ringP->norm[1];
  }
  Vec3Norm(v1);
  ringP->transM[0][1] = v1[0];
  ringP->transM[1][1] = v1[1];
  ringP->transM[2][1] = v1[2];

  Vec3Cross(v1, ringP->norm);
  ringP->transM[0][0] = v1[0];
  ringP->transM[1][0] = v1[1];
  ringP->transM[2][0] = v1[2];
}

static void
getRings(DhResP resP, void *clientData)
{
  CalcData *dataP = clientData;
  DSTR resName;
  ModelRing *mRingP;
  RingData ring;
  int atomI;
  DhAtomP atomP;

  resName = DhResGetName(resP);

  mRingP = ListFirst(dataP->modelP->mRingList);
  while (mRingP != NULL) {
    if (DStrCmp(mRingP->resName, resName) == 0) {
      ring.mRingP = mRingP;
      ring.resP = resP;

      ring.coordA = malloc(mRingP->atomNo * sizeof(*ring.coordA));
      for (atomI = 0; atomI < mRingP->atomNo; atomI++) {
	atomP = DhAtomFindName(resP, mRingP->nameA[atomI], FALSE);
	if (atomP == NULL)
	  break;
	DhAtomGetCoord(atomP, ring.coordA[atomI]);
      }

      if (atomP == NULL) {
	free(ring.coordA);
      } else {
	calcPlane(&ring);
	(void) ListInsertLast(dataP->ringList, &ring);
      }
    }

    mRingP = ListNext(dataP->modelP->mRingList, mRingP);
  }
}

static void
prepareAtom(DhAtomP atomP, void *clientData)
{
  CalcData *dataP = clientData;
  Vec3 x;

  DhAtomGetCoord(atomP, x);
  GridPrepareAddEntry(dataP->grid, x);
}

static void
insertAtom(DhAtomP atomP, void *clientData)
{
  CalcData *dataP = clientData;
  Vec3 x;

  DhAtomGetCoord(atomP, x);
  GridInsertEntry(dataP->grid, x, atomP);
}

static float
ringFact(float rho, float z)
{
  float k, g;

  if (z < 0.0f)
    z = - z;

  /* this sqrt is missing in the paper of Case! */
  k = sqrtf(4.0f * rho / ((1.0f + rho) * (1.0f + rho) + z * z));
  g = (EllInteg1(k) +
      (1.0f - rho * rho - z * z) / ((1 - rho) * (1 - rho) + z * z) *
      EllInteg2(k)) / sqrtf((1.0f + rho) * (1.0f + rho) + z * z);
  
  return g;
}

static float
getCurrentShift(RingData *ringP, Vec3 v)
{
  Vec3 vt;
  float rho, z;
  float g, b;

  Vec3Copy(vt, v);
  Vec3Sub(vt, ringP->center);
  z = Vec3Scalar(vt, ringP->norm);
  Vec3ScaleSub(vt, z, ringP->norm);
  rho = Vec3Abs(vt);

  /* two rings 0.64 A from plane */
  g = ringFact(rho / ringP->rad, (z - 0.64f) / ringP->rad) +
      ringFact(rho / ringP->rad, (z + 0.64f) / ringP->rad);
  b = PHYS_E * PHYS_E /
      (2.0f * (float) M_PI * PHYS_M * (float) 1e-8 * ringP->rad *
      PHYS_C * PHYS_C);

  return ringP->mRingP->ringFact * b * g;
}

static float
getOrbitalShift(RingData *ringP, Vec3 v)
{
  int atomNo, atomI;
  Vec3 vt, v1, v2;
  float area, r1, r2, g, b;

  atomNo = ringP->mRingP->atomNo;
  g = 0.0f;

  for (atomI = 0; atomI < atomNo; atomI++) {
    Vec3Copy(v1, ringP->coordA[atomI]);
    Vec3Copy(v2, ringP->coordA[(atomI + 1) % atomNo]);

    r1 = Vec3DiffAbs(v1, v);
    r2 = Vec3DiffAbs(v2, v);

    Vec3Copy(vt, v);
    Vec3Sub(vt, ringP->center);
    Vec3Sub(v1, ringP->center);
    Vec3Sub(v2, ringP->center);

    Mat3VecMult(vt, ringP->transM);
    Mat3VecMult(v1, ringP->transM);
    Mat3VecMult(v2, ringP->transM);
    Vec3Sub(v1, vt);
    Vec3Sub(v2, vt);

    area = v1[0] * v2[1] - v1[1] * v2[0];
    g += area * (1.0f / (r1 * r1 * r1) + 1.0f / (r2 * r2 * r2));
  }

  g *= 0.5f;
  b = (float) 5.4548e-6;

  return ringP->mRingP->ringFact * b * g;
}

static void
getNeigh(DhBondP bondP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  NeighData *dataP = clientData;

  if (dataP->atomP == atom1P)
    dataP->neighAtomP = atom2P;
  else
    dataP->neighAtomP = atom1P;
}

static BOOL
addPol(void *entryP, Vec3 cent, void *clientData)
{
  DhAtomP nAtomP = entryP;
  CalcData *dataP = clientData;
  Vec3 dx;
  float dist;

  if (nAtomP == dataP->atomP)
    return TRUE;

  if (DhAtomGetRes(nAtomP) == DhAtomGetRes(dataP->atomP) &&
      strcmp(DStrToStr(DhAtomGetName(dataP->atomP)), AMIDE_NAME) != 0)
    return TRUE;

  DhAtomGetCoord(nAtomP, dx);
  Vec3Sub(dx, cent);
  dist = Vec3Abs(dx) * (float) 1.0e-8;  /* esu requires cm */

  if (dist > MaxDist)
    return TRUE;

  Vec3Scale(dx, DhAtomGetCharge(nAtomP, CK_CHARGE) * (float) 1.0e-8 /
      (dist * dist * dist));
  Vec3Add(dataP->polSumV, dx);

  return TRUE;
}

static void
calcAtom(DhAtomP atomP, void *clientData)
{
  CalcData *dataP = clientData;
  Vec3 v, neighV;
  char *atomName;
  RingData *ringP;
  ModelRing *mRingP;
  float ringShiftSum, ringShift, polShift;
  DSTR str;
  DhResP resP;
  char buf[20];
  NeighData neighData;

  DhAtomGetCoord(atomP, v);
  atomName = DStrToStr(DhAtomGetName(atomP));

  str = DStrNew();

  ringP = ListFirst(dataP->ringList);
  ringShiftSum = 0.0f;
  while (ringP != NULL) {
    if (ringP->resP != DhAtomGetRes(atomP) ||
	strcmp(atomName, AMIDE_NAME) == 0) {
      if (dataP->modelP->method == CM_CURRENT)
	ringShift = getCurrentShift(ringP, v);
      else
	ringShift = getOrbitalShift(ringP, v);

      ringShift *= (float) -1.0e6;  /* convert to ppm */
      if (ringShift > ContribCutoff || ringShift < - ContribCutoff) {
	DStrAssignStr(str, "      ");
	(void) sprintf(buf, dataP->formRes, DhResGetNumber(ringP->resP));
	DStrAppStr(str, buf);
	(void) sprintf(buf, " %-5s", DStrToStr(ringP->mRingP->resName));
	DStrAppStr(str, buf);
	(void) sprintf(buf, " %6.3f", ringShift);
	DStrAppStr(str, buf);
	DStrAppChar(str, '\n');
	PuWriteStr(TextW, DStrToStr(str));
      }

      ringShiftSum += ringShift;
    }

    ringP = ListNext(dataP->ringList, ringP);
  }

  dataP->ringValA[dataP->molI * dataP->atomNo + dataP->atomI] = ringShiftSum;

  resP = DhAtomGetRes(atomP);

  (void) sprintf(buf, dataP->formRes, DhResGetNumber(resP));
  DStrAssignStr(str, buf);
  (void) sprintf(buf, " %-5s", DStrToStr(DhResGetName(resP)));
  DStrAppStr(str, buf);
  (void) sprintf(buf, " %-5s", atomName);
  DStrAppStr(str, buf);

  if (dataP->molI == 0)
    DStrAssignDStr(dataP->atomNameA[dataP->atomI], str);

  if (DoRing) {
    (void) sprintf(buf, " %6.3f", ringShiftSum);
    DStrAppStr(str, buf);
  }

  if (DoPol) {
    neighData.atomP = atomP;
    neighData.neighAtomP = NULL;
    DhAtomApplyBond(PropGetRef(PROP_ALL, FALSE), atomP, getNeigh, &neighData);

    if (neighData.neighAtomP != NULL) {
      dataP->atomP = atomP;
      Vec3Zero(dataP->polSumV);
      GridFind(dataP->grid, v, 1, addPol, dataP);

      DhAtomGetCoord(neighData.neighAtomP, neighV);
      Vec3Sub(v, neighV);
      Vec3Norm(v);

      polShift = Vec3Scalar(dataP->polSumV, v);
      polShift *= PHYS_E;

      ringP = ListFirst(dataP->ringList);
      if (ringP == NULL)
	mRingP = ListFirst(dataP->modelP->mRingList);
      else
	mRingP = ringP->mRingP;
      polShift *= mRingP->polFact;

      polShift *= (float) -1.0e6;  /* convert to ppm */
      (void) sprintf(buf, " %6.3f", polShift);
      DStrAppStr(str, buf);
    } else {
      polShift = 0.0f;
    }

    dataP->polValA[dataP->molI * dataP->atomNo + dataP->atomI] = polShift;
  }

  DStrAppChar(str, '\n');
  PuWriteStr(TextW, DStrToStr(str));

  DStrFree(str);

  dataP->atomI++;
}

static void
calcMol(DhMolP molP, void *clientData)
{
  CalcData *dataP = clientData;
  PropRefP allRefP, selRefP;

  if (BreakCheck(1))
    return;

  PuWriteStr(TextW, "\n");
  PuWriteStr(TextW, DStrToStr(DhMolGetName(molP)));
  PuWriteStr(TextW, ":\n\n");

  allRefP = PropGetRef(PROP_ALL, FALSE);
  selRefP = PropGetRef(PROP_SELECTED, FALSE);

  dataP->ringList = ListOpen(sizeof(RingData));
  ListAddDestroyCB(dataP->ringList, freeRing, NULL, NULL);
  DhMolApplyRes(allRefP, molP, getRings, dataP);

  if (DoPol) {
    dataP->grid = GridNew();
    DhMolApplyAtom(allRefP, molP, prepareAtom, dataP);
    GridInsertInit(dataP->grid, MaxDist);
    DhMolApplyAtom(allRefP, molP, insertAtom, dataP);
  }

  dataP->atomI = 0;
  DhMolApplyAtom(selRefP, molP, calcAtom, dataP);

  ListClose(dataP->ringList);

  if (DoPol)
    GridDestroy(dataP->grid);

  dataP->molI++;
}

static void
printSummary(CalcData *dataP)
{
  DSTR str;
  char buf[20];
  float avgRing, minRing, maxRing;
  float avgPol, minPol, maxPol;
  float val;
  int molI, atomI;

  PuWriteStr(TextW, "\nSummary:\n\n");

  str = DStrNew();

  for (atomI = 0; atomI < dataP->atomNo; atomI++) {
    avgRing = 0.0f;
    minRing = MAXFLOAT;
    maxRing = - MAXFLOAT;

    avgPol = 0.0f;
    minPol = MAXFLOAT;
    maxPol = - MAXFLOAT;

    for (molI = 0; molI < dataP->molNo; molI++) {
      if (DoRing) {
	val = dataP->ringValA[molI * dataP->atomNo + atomI];
	avgRing += val;
	if (val < minRing)
	  minRing = val;
	if (val > maxRing)
	  maxRing = val;
      }

      if (DoPol) {
	val = dataP->polValA[molI * dataP->atomNo + atomI];
	avgPol += val;
	if (val < minPol)
	  minPol = val;
	if (val > maxPol)
	  maxPol = val;
      }
    }

    DStrAssignDStr(str, dataP->atomNameA[atomI]);

    if (DoRing) {
      avgRing /= dataP->molNo;
      (void) sprintf(buf, " %6.3f", avgRing);
      DStrAppStr(str, buf);
      (void) sprintf(buf, " (%6.3f", minRing);
      DStrAppStr(str, buf);
      (void) sprintf(buf, "..%6.3f)", maxRing);
      DStrAppStr(str, buf);
    }

    if (DoPol && (minPol != 0.0f || maxPol != 0.0f)) {
      avgPol /= dataP->molNo;
      (void) sprintf(buf, " %6.3f", avgPol);
      DStrAppStr(str, buf);
      (void) sprintf(buf, " (%6.3f", minPol);
      DStrAppStr(str, buf);
      (void) sprintf(buf, "..%6.3f)", maxPol);
      DStrAppStr(str, buf);
    }

    DStrAppStr(str, "\n");
    PuWriteStr(TextW, DStrToStr(str));
  }

  DStrFree(str);
}

#define ARG_NUM 4
#define OPT_NUM 2

ErrCode
ExCalcShift(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  EnumEntryDescr optEntry[OPT_NUM], *modelEntry;
  ModelData *modelP;
  ErrCode errCode;
  PropRefP refP;
  CalcData data;
  char buf[20];
  int modelNo, maxResI, i;

  readModels();
  modelNo = ListSize(ModelList);
  if (modelNo == 0) {
    CipSetError("reading of RingShift file failed");
    return EC_ERROR;
  }

  arg[0].type = AT_MULT_ENUM;
  arg[1].type = AT_ENUM;
  arg[2].type = AT_DOUBLE;
  arg[3].type = AT_DOUBLE;

  ArgInit(arg, ARG_NUM);

  modelNo = ListSize(ModelList);
  modelEntry = malloc(modelNo * sizeof(*modelEntry));
  modelP = ListFirst(ModelList);
  for (i = 0; i < modelNo; i++) {
    modelEntry[i].str = DStrToStr(modelP->name);
    modelEntry[i].onOff = (i == ModelInd);
    modelP = ListNext(ModelList, modelP);
  }

  optEntry[0].str = "Ring Current";
  optEntry[0].onOff = DoRing;
  optEntry[1].str = "Bond Polarization";
  optEntry[1].onOff = DoPol;

  arg[0].prompt = "Options";
  arg[0].u.enumD.entryP = optEntry;
  arg[0].u.enumD.n = OPT_NUM;

  arg[1].prompt = "Model";
  arg[1].u.enumD.entryP = modelEntry;
  arg[1].u.enumD.n = modelNo;
  arg[1].v.intVal = ModelInd;

  arg[2].prompt = "Contrib. Cutoff";
  arg[2].v.doubleVal = ContribCutoff;

  arg[3].prompt = "Max. Dist";
  arg[3].v.doubleVal = MaxDist;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    free(modelEntry);
    return errCode;
  }

  DoRing = optEntry[0].onOff;
  DoPol = optEntry[1].onOff;
  ModelInd = arg[1].v.intVal;
  ContribCutoff = (float) arg[2].v.doubleVal;
  MaxDist = (float) arg[3].v.doubleVal;

  ArgCleanup(arg, ARG_NUM);
  free(modelEntry);

  refP = PropGetRef(PROP_SELECTED, FALSE);

  data.molNo = 0;
  DhApplyMol(refP, countMolAtoms, &data);
  if (data.molNo == 0) {
    CipSetError("at least one molecule must be selected");
    return EC_ERROR;
  }
  if (data.atomNo <= 0) {
    CipSetError("number of selected atoms must be equal");
    return EC_ERROR;
  }

  if (! (DoRing || DoPol)) {
    CipSetError("no calculation selected");
    return EC_WARNING;
  }

  data.ringValA = malloc(data.molNo * data.atomNo * sizeof(*data.ringValA));
  data.polValA = malloc(data.molNo * data.atomNo * sizeof(*data.polValA));
  data.atomNameA = malloc(data.atomNo * sizeof(*data.atomNameA));
  for (i = 0; i < data.atomNo; i++)
    data.atomNameA[i] = DStrNew();

  BreakActivate(TRUE);

  TextW = PuCreateTextWindow(cmd);

  data.modelP = ListPos(ModelList, ModelInd);

  if (DoRing && DoPol)
    PuWriteStr(TextW, "Ring current and bond polarization shifts,\n");
  else if (DoRing)
    PuWriteStr(TextW, "Ring current shifts,\n");
  else
    PuWriteStr(TextW, "Bond polarization shifts,\n");
  
  PuWriteStr(TextW, DStrToStr(data.modelP->name));
  PuWriteStr(TextW, " model,\n");

  PuWriteStr(TextW, "Threshold for contributions: ");
  (void) sprintf(buf, "%.2f", ContribCutoff);
  PuWriteStr(TextW, buf);
  PuWriteStr(TextW, "\n");

  maxResI = -1;
  DhApplyRes(refP, maxRes, &maxResI);
  data.formRes = getForm(maxResI);

  data.molI = 0;
  DhApplyMol(refP, calcMol, &data);

  BreakActivate(FALSE);

  if (data.molNo > 1 && ! BreakInterrupted())
    printSummary(&data);

  free(data.ringValA);
  free(data.polValA);
  for (i = 0; i < data.atomNo; i++)
    DStrFree(data.atomNameA[i]);
  free(data.atomNameA);

  return EC_OK;
}
