/*
************************************************************************
*
*   ExCalcMatch.c - CalcMatch command
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
*   Date of last modification : 01/05/27
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdcalc/SCCS/s.ExCalcMatch.c
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include <cmd_calc.h>

#include <stdio.h>
#include <stdlib.h>
#include <values.h>
#include <math.h>

#include <break.h>
#include <map_coord.h>
#include <pu.h>
#include <arg.h>
#include <data_hand.h>
#include <data_sel.h>

#define PROP_NAME "match"

typedef struct {
  Vec3 *coordA;
  int atomI;
} AtomData;

typedef struct {
  DhMolP molP;
  DhResP *resPA;
  int resI;
} MolData;

typedef struct {
  MolData *molDataA;
  int molI;
} MatchData;

static int MinResNo = 30;
static float MaxRmsd = 1.0f;

static void
countAtom(DhAtomP atomP, void *clientData)
{
  *(int *) clientData += 1;
}

static void
countRes(DhResP atomP, void *clientData)
{
  *(int *) clientData += 1;
}

static void
getMol(DhMolP molP, void *clientData)
{
  MatchData *matchDataP = clientData;
  MolData *molDataP = matchDataP->molDataA + matchDataP->molI;

  molDataP->molP = molP;
  matchDataP->molI++;
}

static void
getRes(DhResP resP, void *clientData)
{
  MolData *molDataP = clientData;

  molDataP->resPA[molDataP->resI] = resP;
  molDataP->resI++;
}

static void
getCoord(DhAtomP atomP, void *clientData)
{
  AtomData *atomDataP = clientData;

  DhAtomGetCoord(atomP, atomDataP->coordA[atomDataP->atomI]);
  atomDataP->atomI++;
}

#define ARG_NUM 2

ErrCode
ExCalcMatch(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  PropRefP refP, setRefP;
  MatchData matchData;
  int molNo, molI;
  int resNo, resNo1, resI, maxI;
  int totAtomNo, atomNo, atomNo1, atomI, startI;
  AtomData atomData1, atomData2;
  PuTextWindow textW;
  char buf[20];
  int step;
  float *rmsdA, rmsdAvg;
  float dispAvg, maxDisp, disp, d, s;
  Mat4 *mapMA;
  Vec4 v4;
  Vec3 v3;
  BOOL first;

  arg[0].type = AT_INT;
  arg[1].type = AT_DOUBLE;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Min. Res. #";
  arg[0].v.intVal = MinResNo;

  arg[1].prompt = "Max. RMSD";
  arg[1].v.doubleVal = MaxRmsd;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  MinResNo = arg[0].v.intVal;
  MaxRmsd = (float) arg[1].v.doubleVal;

  ArgCleanup(arg, ARG_NUM);

  DhActivateGroups(TRUE);

  molNo = SelMolGet(NULL, 0);
  if (molNo < 2) {
    DhActivateGroups(FALSE);
    CipSetError("at least 2 molecules must be selected");
    return EC_ERROR;
  }

  refP = PropGetRef(PROP_SELECTED, FALSE);

  matchData.molDataA = malloc(molNo * sizeof(*matchData.molDataA));
  matchData.molI = 0;
  DhApplyMol(refP, getMol, &matchData);

  resNo = 0;
  DhMolApplyRes(refP, matchData.molDataA[0].molP, countRes, &resNo);
  if (resNo < 1) {
    free(matchData.molDataA);
    DhActivateGroups(FALSE);
    CipSetError("at least 1 residue must be selected");
    return EC_ERROR;
  }

  totAtomNo = 0;
  DhMolApplyAtom(refP, matchData.molDataA[0].molP, countAtom, &totAtomNo);
  if (totAtomNo < 1) {
    free(matchData.molDataA);
    DhActivateGroups(FALSE);
    CipSetError("at least 1 atom must be selected");
    return EC_ERROR;
  }

  for (molI = 1; molI < molNo; molI++) {
    resNo1 = 0;
    DhMolApplyRes(refP, matchData.molDataA[molI].molP, countRes, &resNo1);
    if (resNo1 != resNo) {
      free(matchData.molDataA);
      DhActivateGroups(FALSE);
      CipSetError("number of selected residues must be equal");
      return EC_ERROR;
    }
  }

  for (molI = 0; molI < molNo; molI++) {
    matchData.molDataA[molI].resPA = malloc(resNo * sizeof(DhResP));
    matchData.molDataA[molI].resI = 0;
    DhMolApplyRes(refP, matchData.molDataA[molI].molP,
	getRes, matchData.molDataA + molI);
  }

  for (resI = 0; resI < resNo; resI++) {
    atomNo = 0;
    DhResApplyAtom(refP, matchData.molDataA[0].resPA[resI],
	countAtom, &atomNo);
    if (atomNo == 0)
      matchData.molDataA[0].resPA[resI] = NULL;

    for (molI = 1; molI < molNo; molI++) {
      atomNo1 = 0;
      DhResApplyAtom(refP, matchData.molDataA[molI].resPA[resI],
	  countAtom, &atomNo1);
      if (atomNo1 == 0)
	matchData.molDataA[molI].resPA[resI] = NULL;

      if (atomNo1 != atomNo) {
	for (molI = 0; molI < molNo; molI++)
	  free(matchData.molDataA[molI].resPA);
	free(matchData.molDataA);
	DhActivateGroups(FALSE);
	CipSetError("number of selected atoms must be equal");
	return EC_ERROR;
      }
    }
  }

  setRefP = PropGetRef(PROP_NAME, TRUE);
  for (molI = 0; molI < molNo; molI++)
    for (resI = 0; resI < resNo; resI++)
      if (matchData.molDataA[molI].resPA[resI] != NULL)
	DhResSetProp(setRefP, matchData.molDataA[molI].resPA[resI], FALSE);

  textW = PuCreateTextWindow(cmd);

  atomData1.coordA = malloc(totAtomNo * sizeof(Vec3));
  atomData2.coordA = malloc(totAtomNo * sizeof(Vec3));

  mapMA = malloc(molNo * sizeof(*mapMA));
  rmsdA = malloc(molNo * sizeof(*rmsdA));

  BreakActivate(TRUE);

  for (step = 0; step < resNo - MinResNo; step++) {
    atomData1.atomI = 0;
    for (resI = 0; resI < resNo; resI++)
      if (matchData.molDataA[0].resPA[resI] != NULL)
	DhResApplyAtom(refP, matchData.molDataA[0].resPA[resI],
	    getCoord, &atomData1);
    atomNo = atomData1.atomI;

    Mat4Ident(mapMA[0]);

    for (molI = 1; molI < molNo; molI++) {
      if (BreakCheck(1))
	break;

      atomData2.atomI = 0;
      for (resI = 0; resI < resNo; resI++)
	if (matchData.molDataA[molI].resPA[resI] != NULL)
	  DhResApplyAtom(refP, matchData.molDataA[molI].resPA[resI],
	      getCoord, &atomData2);

      MapCoord(atomData1.coordA, atomNo, atomData2.coordA, TRUE, mapMA[molI]);
    }

    if (BreakInterrupted())
      break;

    for (molI = 0; molI < molNo; molI++)
      rmsdA[molI] = 0.0f;

    maxDisp = - 1.0f;

    for (resI = 0; resI < resNo; resI++) {
      if (matchData.molDataA[0].resPA[resI] == NULL)
	continue;

      atomNo1 = 0;
      DhResApplyAtom(refP, matchData.molDataA[0].resPA[resI],
	  countAtom, &atomNo1);

      /* calculate mean */
      for (atomI = 0; atomI < atomNo1; atomI++)
	Vec3Zero(atomData1.coordA[atomI]);

      for (molI = 0; molI < molNo; molI++) {
	atomData2.atomI = 0;
	DhResApplyAtom(refP, matchData.molDataA[molI].resPA[resI],
	    getCoord, &atomData2);

	for (atomI = 0; atomI < atomNo1; atomI++) {
	  Vec3To4(v4, atomData2.coordA[atomI]);
	  Mat4VecMult(v4, mapMA[molI]);
	  Vec4To3(v3, v4);
	  Vec3Add(atomData1.coordA[atomI], v3);
	}
      }

      s = 1.0f / molNo;
      for (atomI = 0; atomI < atomNo1; atomI++)
	Vec3Scale(atomData1.coordA[atomI], s);

      /* calculate displacement */
      dispAvg = 0.0f;
      for (molI = 0; molI < molNo; molI++) {
	atomData2.atomI = 0;
	DhResApplyAtom(refP, matchData.molDataA[molI].resPA[resI],
	    getCoord, &atomData2);

	disp = 0.0f;
	for (atomI = 0; atomI < atomNo1; atomI++) {
	  Vec3To4(v4, atomData2.coordA[atomI]);
	  Mat4VecMult(v4, mapMA[molI]);
	  Vec4To3(v3, v4);
	  Vec3Sub(v3, atomData1.coordA[atomI]);
	  d = v3[0] * v3[0] + v3[1] * v3[1] + v3[2] * v3[2];

	  disp += d;
	  rmsdA[molI] += d;
	}

	dispAvg += sqrtf(disp / atomNo1);
      }

      dispAvg /= molNo;
      if (dispAvg > maxDisp) {
	maxDisp = dispAvg;
	maxI = resI;
      }
    }

    rmsdAvg = 0.0f;
    for (molI = 0; molI < molNo; molI++)
      rmsdAvg += sqrtf(rmsdA[molI] / atomNo);
    rmsdAvg /= molNo;

    PuWriteStr(textW, "rmsd: ");
    (void) sprintf(buf, "%4.2f", rmsdAvg);
    PuWriteStr(textW, buf);

    if (rmsdAvg < MaxRmsd) {
      PuWriteStr(textW, "\n");
      break;
    }

    PuWriteStr(textW, "  eliminate: ");
    (void) sprintf(buf, "%4d",
	DhResGetNumber(matchData.molDataA[0].resPA[maxI]));
    PuWriteStr(textW, buf);
    PuWriteStr(textW, "  displacement: ");
    (void) sprintf(buf, "%4.2f", maxDisp);
    PuWriteStr(textW, buf);
    PuWriteStr(textW, "\n");

    for (molI = 0; molI < molNo; molI++)
      matchData.molDataA[molI].resPA[maxI] = NULL;
  }

  BreakActivate(FALSE);

  if (! BreakInterrupted()) {
    PuWriteStr(textW, "\n");

    for (molI = 0; molI < molNo; molI++) {
      PuWriteStr(textW, "#");
      (void) sprintf(buf, "%d",
	  DhMolGetNumber(matchData.molDataA[molI].molP) + 1);
      PuWriteStr(textW, buf);
      PuWriteStr(textW, ":");

      first = TRUE;
      for (resI = 0; resI < resNo; resI++) {
	if (matchData.molDataA[molI].resPA[resI] == NULL)
	  continue;

	if (resI == 0 ||
	    matchData.molDataA[molI].resPA[resI - 1] == NULL)
	  startI = resI;

	if (resI == resNo - 1 ||
	    matchData.molDataA[molI].resPA[resI + 1] == NULL) {
	  if (! first)
	    PuWriteStr(textW, ",");

	  (void) sprintf(buf, "%d",
	      DhResGetNumber(matchData.molDataA[molI].resPA[startI]));
	  PuWriteStr(textW, buf);

	  if (resI > startI) {
	    (void) sprintf(buf, "-%d",
		DhResGetNumber(matchData.molDataA[molI].resPA[resI]));
	    PuWriteStr(textW, buf);
	  }

	  first = FALSE;
	}

	DhResSetProp(setRefP, matchData.molDataA[molI].resPA[resI], TRUE);
      }

      PuWriteStr(textW, "\n");
    }
  }

  for (molI = 0; molI < molNo; molI++)
    free(matchData.molDataA[molI].resPA);
  free(matchData.molDataA);
  free(atomData1.coordA);
  free(atomData2.coordA);
  free(mapMA);
  free(rmsdA);

  DhActivateGroups(FALSE);

  return EC_OK;
}
