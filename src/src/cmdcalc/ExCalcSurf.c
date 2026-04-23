/*
************************************************************************
*
*   ExCalcSurf.c - CalcSurf command
*
*   Copyright (c) 1994-97
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdcalc/SCCS/s.ExCalcSurf.c
*   SCCS identification       : 1.11
*
************************************************************************
*/

#include <cmd_calc.h>

#include <stdio.h>
#include <stdlib.h>
#include <values.h>
#include <math.h>

#include <break.h>
#include <bool.h>
#include <mat_vec.h>
#include <sphere.h>
#include <grid.h>
#include <pu.h>
#include <arg.h>
#include <data_hand.h>
#include <data_sel.h>

typedef struct {
  float part, full, perc;
} SurfVal;

typedef struct {
  SurfVal v;
  DSTR name;
} ResData;

typedef struct {
  SurfVal v;
  int resI;
  DSTR name;
} AtomData;

typedef struct {
  PropRefP refP;
  char *formRes;
  ResData *resDataA;
  AtomData *atomDataA;
  float *molSurfA;
  DhAtomP atomP;
  float rad;
  DhAtomP *neighA;
  int neighSize, neighNo;
  int molI, atomI;
  GRID grid;
  float rMax;
} SurfData;

static float SolventRad = 1.4f;
static int SurfPrec = 3;
static BOOL AtomSwitch;

static int SpherePointNo;
static Vec3 *SpherePoints;

static int MolNo, MinResI, MaxResI, AtomNo;

static PuTextWindow TextW;

static void
countRes(DhResP resP, void *clientData)
{
  int num;

  num = DhResGetNumber(resP);
  if (num < MinResI)
    MinResI = num;
  if (num > MaxResI)
    MaxResI = num;
}

static void
countAtoms(DhAtomP atomP, void *clientData)
{
  if (DhAtomGetVdw(atomP) < 0.0f)
    return;

  *(int *) clientData += 1;
}

static void
prepareAtom(DhAtomP atomP, void *clientData)
{
  SurfData *dataP = clientData;
  float rad;
  Vec3 x;

  if (BreakCheck(10000))
    return;

  rad = DhAtomGetVdw(atomP);
  if (rad < 0.0f)
    return;

  DhAtomGetCoord(atomP, x);
  GridPrepareAddEntry(dataP->grid, x);

  if (rad > dataP->rMax)
    dataP->rMax = rad;
}

static void
insertAtom(DhAtomP atomP, void *clientData)
{
  SurfData *dataP = clientData;
  float rad;
  Vec3 x;

  if (BreakCheck(10000))
    return;

  rad = DhAtomGetVdw(atomP);
  if (rad < 0.0f)
    return;

  DhAtomGetCoord(atomP, x);
  GridInsertEntry(dataP->grid, x, atomP);
}

static BOOL
fillNeigh(void *entryP, Vec3 cent, void *clientData)
{
  DhAtomP nAtomP = entryP;
  SurfData *dataP = clientData;
  Vec3 dx;
  float rs;

  if (nAtomP == dataP->atomP)
    return TRUE;

  DhAtomGetCoord(nAtomP, dx);
  Vec3Sub(dx, cent);
  rs = dataP->rad + DhAtomGetVdw(nAtomP) + SolventRad;

  if (dx[0] * dx[0] + dx[1] * dx[1] + dx[2] * dx[2] > rs * rs)
    return TRUE;

  if (dataP->neighNo == dataP->neighSize) {
    dataP->neighSize *= 2;
    dataP->neighA = realloc(dataP->neighA,
	dataP->neighSize * sizeof(*dataP->neighA));
  }

  dataP->neighA[dataP->neighNo++] = nAtomP;

  return TRUE;
}

static void
surfAtom(DhAtomP atomP, void *clientData)
{
  SurfData *dataP = clientData;
  DhResP resP;
  DhAtomP nAtomP;
  int fullNo, partNo;
  BOOL fullInside, partInside;
  Vec3 cent, nCent, x, dx;
  float nRad;
  float fullSurf, partSurf;
  int pointI, atomI, ind;

  dataP->rad = DhAtomGetVdw(atomP);
  if (dataP->rad < 0.0f)
    return;

  if (BreakCheck(10000 / SpherePointNo))
    return;

  resP = DhAtomGetRes(atomP);
  DhAtomGetCoord(atomP, cent);

  dataP->atomP = atomP;
  dataP->rad += SolventRad;
  dataP->neighNo = 0;

  GridFind(dataP->grid, cent, 1, fillNeigh, dataP);

  fullNo = 0;
  partNo = 0;

  for (pointI = 0; pointI < SpherePointNo; pointI++) {
    Vec3Copy(x, cent);
    Vec3ScaleAdd(x, dataP->rad, SpherePoints[pointI]);

    fullInside = FALSE;
    partInside = FALSE;

    for (atomI = 0; atomI < dataP->neighNo; atomI++) {
      nAtomP = dataP->neighA[atomI];

      DhAtomGetCoord(nAtomP, nCent);
      Vec3Copy(dx, x);
      Vec3Sub(dx, nCent);

      nRad = DhAtomGetVdw(nAtomP) + SolventRad;
      if (dx[0] * dx[0] + dx[1] * dx[1] + dx[2] * dx[2] > nRad * nRad)
	continue;
      
      fullInside = TRUE;

      if (resP == DhAtomGetRes(nAtomP)) {
	partInside = TRUE;
	break;
      }
    }

    if (! fullInside)
      fullNo++;

    if (! partInside)
      partNo++;
  }

  fullSurf = fullNo * dataP->rad * dataP->rad;
  partSurf = partNo * dataP->rad * dataP->rad;

  ind = (DhResGetNumber(resP) - MinResI) * MolNo + dataP->molI;
  dataP->resDataA[ind].v.full += fullSurf;
  dataP->resDataA[ind].v.part += partSurf;
  dataP->resDataA[ind].name = DhResGetName(resP);

  if (AtomSwitch) {
    ind = dataP->atomI * MolNo + dataP->molI;
    dataP->atomDataA[ind].v.full = fullSurf;
    dataP->atomDataA[ind].v.part = partSurf;
    dataP->atomDataA[ind].resI = DhResGetNumber(resP);
    dataP->atomDataA[ind].name = DhAtomGetName(atomP);
  }

  dataP->atomI++;
}

static void
surfMol(DhMolP molP, void *clientData)
{
  SurfData *dataP = clientData;

  if (BreakInterrupted())
    return;

  dataP->grid = GridNew();
  dataP->rMax = - MAXFLOAT;
  DhMolApplyAtom(dataP->refP, molP, prepareAtom, dataP);
  if (dataP->rMax > 0.0f) {
    GridInsertInit(dataP->grid, 2.0f * (dataP->rMax + SolventRad));
    DhMolApplyAtom(dataP->refP, molP, insertAtom, dataP);
    dataP->atomI = 0;
    DhMolApplyAtom(dataP->refP, molP, surfAtom, dataP);
  } else {
    /* must be initialized, otherwise GridDestroy crashes */
    GridInsertInit(dataP->grid, 10.0f);
  }

  GridDestroy(dataP->grid);

  dataP->molI++;
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
writeMol(DhMolP molP, void *clientData)
{
  SurfData *dataP = clientData;
  DhResP resP;
  DSTR str;
  char buf[20];
  int resI, atomI, ind;

  if (BreakInterrupted())
    return;

  PuWriteStr(TextW, "\n");
  PuWriteStr(TextW, DStrToStr(DhMolGetName(molP)));
  PuWriteStr(TextW, ":\n\n");

  str = DStrNew();

  dataP->molSurfA[dataP->molI] = 0.0f;
  atomI = 0;

  for (resI = MinResI; resI <= MaxResI; resI++) {
    if (BreakCheck(100))
      break;

    resP = DhResFind(molP, resI);
    if (resP == NULL)
      continue;

    ind = (resI - MinResI) * MolNo + dataP->molI;
    if (dataP->resDataA[ind].name == NULL)
      continue;

    if (dataP->resDataA[ind].v.part == 0.0f)
      dataP->resDataA[ind].v.perc = 0.0f;
    else
      dataP->resDataA[ind].v.perc = 100.0f *
          dataP->resDataA[ind].v.full /
	  dataP->resDataA[ind].v.part;

    (void) sprintf(buf, dataP->formRes, resI);
    DStrAssignStr(str, buf);
    (void) sprintf(buf, " %-5s", DStrToStr(DhResGetName(resP)));
    DStrAppStr(str, buf);
    (void) sprintf(buf, " %5.1f", dataP->resDataA[ind].v.full);
    DStrAppStr(str, buf);
    (void) sprintf(buf, ", %5.1f%%", dataP->resDataA[ind].v.perc);
    DStrAppStr(str, buf);
    (void) sprintf(buf, " of %5.1f", dataP->resDataA[ind].v.part);
    DStrAppStr(str, buf);
    DStrAppStr(str, "\n");

    PuWriteStr(TextW, DStrToStr(str));

    dataP->molSurfA[dataP->molI] += dataP->resDataA[ind].v.full;

    if (AtomSwitch) {
      for (;;) {
	if (atomI == AtomNo)
	  break;

        ind = atomI * MolNo + dataP->molI;
        if (dataP->atomDataA[ind].resI > resI)
	  break;

        if (dataP->atomDataA[ind].v.part == 0.0f)
          dataP->atomDataA[ind].v.perc = 0.0f;
        else
          dataP->atomDataA[ind].v.perc = 100.0f *
	      dataP->atomDataA[ind].v.full /
	      dataP->atomDataA[ind].v.part;

        (void) sprintf(buf, "    %-5s",
	    DStrToStr(dataP->atomDataA[ind].name));
        DStrAssignStr(str, buf);
        (void) sprintf(buf, " %5.1f", dataP->atomDataA[ind].v.full);
        DStrAppStr(str, buf);
        (void) sprintf(buf, ", %5.1f%%", dataP->atomDataA[ind].v.perc);
        DStrAppStr(str, buf);
        (void) sprintf(buf, " of %5.1f", dataP->atomDataA[ind].v.part);
        DStrAppStr(str, buf);
        DStrAppStr(str, "\n");

        PuWriteStr(TextW, DStrToStr(str));

	atomI++;
      }
    }
  }

  DStrAssignStr(str, "\nTotal surface: ");
  (void) sprintf(buf, "%8.1f", dataP->molSurfA[dataP->molI]);
  DStrAppStr(str, buf);
  DStrAppStr(str, "\n");
  PuWriteStr(TextW, DStrToStr(str));

  DStrFree(str);

  dataP->molI++;
}

static void
initSurfVal(SurfVal *minP, SurfVal *maxP, SurfVal *avgP)
{
  minP->full = MAXFLOAT;
  minP->part = MAXFLOAT;
  minP->perc = 100.0f;

  maxP->full = 0.0f;
  maxP->part = 0.0f;
  maxP->perc = 0.0f;

  avgP->full = 0.0f;
  avgP->part = 0.0f;
  avgP->perc = 0.0f;
}

static void
addSurfVal(SurfVal *minP, SurfVal *maxP, SurfVal *avgP, SurfVal *valP)
{
  if (valP->full < minP->full)
    minP->full = valP->full;
  if (valP->part < minP->part)
    minP->part = valP->part;
  if (valP->perc < minP->perc)
    minP->perc = valP->perc;

  if (valP->full > maxP->full)
    maxP->full = valP->full;
  if (valP->part > maxP->part)
    maxP->part = valP->part;
  if (valP->perc > maxP->perc)
    maxP->perc = valP->perc;
      
  avgP->full += valP->full;
  avgP->part += valP->part;
  avgP->perc += valP->perc;
}

static void
appSurfVal(DSTR str, SurfVal *minP, SurfVal *maxP, SurfVal *avgP)
{
  char buf[50];

  (void) sprintf(buf, " %5.1f", avgP->full);
  DStrAppStr(str, buf);
  (void) sprintf(buf, " (%5.1f", minP->full);
  DStrAppStr(str, buf);
  (void) sprintf(buf, "..%5.1f)", maxP->full);
  DStrAppStr(str, buf);
  (void) sprintf(buf, ", %5.1f%%", avgP->perc);
  DStrAppStr(str, buf);
  (void) sprintf(buf, " (%5.1f", minP->perc);
  DStrAppStr(str, buf);
  (void) sprintf(buf, "..%5.1f)", maxP->perc);
  DStrAppStr(str, buf);
  (void) sprintf(buf, " of %5.1f", avgP->part);
  DStrAppStr(str, buf);
  (void) sprintf(buf, " (%5.1f", minP->part);
  DStrAppStr(str, buf);
  (void) sprintf(buf, "..%5.1f)", maxP->part);
  DStrAppStr(str, buf);
}

#define ARG_NUM 3
#define OPT_NUM 1

ErrCode
ExCalcSurface(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  EnumEntryDescr optEntry[OPT_NUM];
  ErrCode errCode;
  DhMolP *molPA;
  PropRefP refP;
  SurfData data;
  int resNo, resI;
  int atomNo, atomI;
  SurfVal minVal, maxVal, avgVal;
  float minSurf, maxSurf, avgSurf;
  DSTR str, resName;
  char buf[50];
  int validNo, molI,ind, i;

  arg[0].type = AT_DOUBLE;
  arg[1].type = AT_INT;
  arg[2].type = AT_MULT_ENUM;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Solvent Radius";
  arg[0].v.doubleVal = SolventRad;

  arg[1].prompt = "Precision";
  arg[1].v.intVal = SurfPrec;

  optEntry[0].str = "by atom";
  optEntry[0].onOff = AtomSwitch;

  arg[2].prompt = "Options";
  arg[2].u.enumD.entryP = optEntry;
  arg[2].u.enumD.n = OPT_NUM;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  SolventRad = (float) arg[0].v.doubleVal;
  SurfPrec = arg[1].v.intVal;
  AtomSwitch = optEntry[0].onOff;

  ArgCleanup(arg, ARG_NUM);

  DhActivateGroups(TRUE);

  MolNo = SelMolGet(NULL, 0);

  refP = PropGetRef(PROP_SELECTED, FALSE);

  MinResI = MAXINT;
  MaxResI = - MAXINT;
  DhApplyRes(refP, countRes, NULL);
  if (MinResI > MaxResI) {
    DhActivateGroups(FALSE);
    CipSetError("at least one residue must be selected");
    return EC_ERROR;
  }
  resNo = MaxResI - MinResI + 1;

  if (AtomSwitch) {
    molPA = malloc(MolNo * sizeof(DhMolP));
    (void) SelMolGet(molPA, MolNo);

    AtomNo = 0;
    DhMolApplyAtom(refP, molPA[0], countAtoms, &AtomNo);
    if (AtomNo == 0) {
      free(molPA);
      DhActivateGroups(FALSE);
      CipSetError("at least one atom must be selected");
      return EC_ERROR;
    }

    for (molI = 1; molI < MolNo; molI++) {
      atomNo = 0;
      DhMolApplyAtom(refP, molPA[molI], countAtoms, &atomNo);
      if (atomNo != AtomNo) {
        free(molPA);
        DhActivateGroups(FALSE);
        CipSetError("number of selected atoms must be equal");
        return EC_ERROR;
      }
    }

    data.atomDataA = malloc(MolNo * AtomNo * sizeof(*data.atomDataA));
    for (i = 0; i < MolNo * AtomNo; i++) {
      data.atomDataA[i].v.full = 0.0f;
      data.atomDataA[i].v.part = 0.0f;
    }
  }

  BreakActivate(TRUE);

  TextW = PuCreateTextWindow(cmd);
  PuWriteStr(TextW, "Solvent accessible surface:\n");

  SphereCalcPoints(SurfPrec, &SpherePoints, &SpherePointNo);

  data.resDataA = malloc(MolNo * resNo * sizeof(*data.resDataA));
  for (i = 0; i < MolNo * resNo; i++) {
    data.resDataA[i].v.full = 0.0f;
    data.resDataA[i].v.part = 0.0f;
    data.resDataA[i].name = NULL;
  }
  data.molSurfA = malloc(MolNo * sizeof(*data.molSurfA));

  data.refP = refP;
  data.neighSize = 5;
  data.neighA = malloc(data.neighSize * sizeof(data.neighA));

  data.molI = 0;
  DhApplyMol(refP, surfMol, &data);

  free(data.neighA);
  free(SpherePoints);

  for (i = 0; i < MolNo * resNo; i++) {
    data.resDataA[i].v.full *= 4.0f * (float) M_PI / SpherePointNo;
    data.resDataA[i].v.part *= 4.0f * (float) M_PI / SpherePointNo;
  }

  for (i = 0; i < MolNo * AtomNo; i++) {
    data.atomDataA[i].v.full *= 4.0f * (float) M_PI / SpherePointNo;
    data.atomDataA[i].v.part *= 4.0f * (float) M_PI / SpherePointNo;
  }

  data.formRes = getForm(MaxResI);

  data.molI = 0;
  DhApplyMol(refP, writeMol, &data);

  PuWriteStr(TextW, "\nSummary:\n\n");
  str = DStrNew();

  atomI = 0;

  for (resI = MinResI; resI <= MaxResI; resI++) {
    if (BreakCheck(100))
      break;

    initSurfVal(&minVal, &maxVal, &avgVal);
    validNo = 0;
    resName = NULL;

    for (molI = 0; molI < MolNo; molI++) {
      ind = (resI - MinResI) * MolNo + molI;
      if (data.resDataA[ind].name == NULL)
	continue;

      addSurfVal(&minVal, &maxVal, &avgVal, &data.resDataA[ind].v);
      validNo++;

      if (resName == NULL)
	resName = data.resDataA[ind].name;
    }

    if (validNo == 0)
      continue;

    avgVal.full /= validNo;
    avgVal.part /= validNo;
    avgVal.perc /= validNo;

    (void) sprintf(buf, data.formRes, resI);
    DStrAssignStr(str, buf);
    (void) sprintf(buf, " %-5s", DStrToStr(resName));
    DStrAppStr(str, buf);
    appSurfVal(str, &minVal, &maxVal, &avgVal);
    DStrAppStr(str, "\n");

    PuWriteStr(TextW, DStrToStr(str));

    if (AtomSwitch) {
      for (;;) {
	if (atomI == AtomNo)
	  break;

        ind = atomI * MolNo;
        if (data.atomDataA[ind].resI > resI)
	  break;

        initSurfVal(&minVal, &maxVal, &avgVal);

        for (molI = 0; molI < MolNo; molI++) {
          ind = atomI * MolNo + molI;
          addSurfVal(&minVal, &maxVal, &avgVal, &data.atomDataA[ind].v);
	}

        avgVal.full /= MolNo;
        avgVal.part /= MolNo;
        avgVal.perc /= MolNo;

        (void) sprintf(buf, "    %-5s",
	    DStrToStr(data.atomDataA[ind].name));
        DStrAssignStr(str, buf);
        appSurfVal(str, &minVal, &maxVal, &avgVal);
        DStrAppStr(str, "\n");

        PuWriteStr(TextW, DStrToStr(str));

	atomI++;
      }
    }
  }

  BreakActivate(FALSE);

  minSurf = MAXFLOAT;
  maxSurf = 0.0f;
  avgSurf = 0.0f;

  for (molI = 0; molI < MolNo; molI++) {
    if (data.molSurfA[molI] < minSurf)
      minSurf = data.molSurfA[molI];
    if (data.molSurfA[molI] > maxSurf)
      maxSurf = data.molSurfA[molI];
    avgSurf += data.molSurfA[molI];
  }
  avgSurf /= MolNo;

  DStrAssignStr(str, "\nTotal surface: ");
  (void) sprintf(buf, "%8.1f", avgSurf);
  DStrAppStr(str, buf);
  (void) sprintf(buf, " (%8.1f", minSurf);
  DStrAppStr(str, buf);
  (void) sprintf(buf, "..%8.1f)", maxSurf);
  DStrAppStr(str, buf);
  DStrAppStr(str, "\n");
  PuWriteStr(TextW, DStrToStr(str));

  if (AtomSwitch) {
    free(molPA);
    free(data.atomDataA);
  }

  DStrFree(str);
  free(data.resDataA);
  free(data.molSurfA);

  DhActivateGroups(FALSE);

  return EC_OK;
}
