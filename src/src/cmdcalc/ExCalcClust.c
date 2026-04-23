/*
************************************************************************
*
*   ExCalcClust.c - CalcClust command
*
*   Copyright (c) 1994-98
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdcalc/SCCS/s.ExCalcClust.c
*   SCCS identification       : 1.6
*
************************************************************************
*/

#include <cmd_calc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <values.h>
#include <math.h>

#include <break.h>
#include <linlist.h>
#include <cluster.h>
#include <pu.h>
#include <arg.h>
#include <prop_tab.h>
#include <data_hand.h>
#include <data_sel.h>

#define PROP_NAME "cluster"

typedef struct {
  Vec3 *coordA;
  int atomI;
} AtomData;

typedef struct {
  DhMolP *molPA;
  int molNo;
  float *rmsdA;
  int clusterNo;
  PropRefP refP;
  int *molIA;
} ClusterData;

static int ClusterNo = 5;
static float MinRmsd = 0.05f, MaxRmsd = 1.0f;

static void
countAtoms(DhAtomP atomP, void *clientData)
{
  *(int *) clientData += 1;
}

static void
getCoord(DhAtomP atomP, void *clientData)
{
  AtomData *atomDataP = clientData;

  DhAtomGetCoordTransf(atomP, atomDataP->coordA[atomDataP->atomI]);
  atomDataP->atomI++;
}

static void
getProp(char *name, void *clientData)
{
  LINLIST propList = clientData;
  DSTR str;

  if (strncmp(name, PROP_NAME, strlen(PROP_NAME)) == 0) {
    str = DStrNew();
    DStrAssignStr(str, name);
    (void) ListInsertLast(propList, &str);
  }
}

static void
setCluster(int idx, int clusterSize, void *entryP,
    float maxDiff, float minDiff, void *clientData)
{
  DhMolP molP = entryP;
  ClusterData *dataP = clientData;
  float maxRmsd, bestRmsd, rmsd;
  int bestI;
  int molI, i1, i2;
  char propName[20];
  PropRefP refP;

  molI = 0;
  while (dataP->molPA[molI] != molP)
    molI++;

  dataP->molIA[idx] = molI;

  if (idx < clusterSize - 1)
    return;

  bestRmsd = MAXFLOAT;
  for (i1 = 0; i1 < clusterSize; i1++) {
    maxRmsd = 0.0f;
    for (i2 = 0; i2 < clusterSize; i2++) {
      if (i2 == i1)
	continue;

      rmsd = dataP->rmsdA[dataP->molIA[i1] * dataP->molNo + dataP->molIA[i2]];
      if (rmsd > maxRmsd)
	maxRmsd = rmsd;
    }

    if (maxRmsd < bestRmsd) {
      bestRmsd = maxRmsd;
      bestI = i1;
    }
  }

  (void) sprintf(propName, "%s%d", PROP_NAME, dataP->clusterNo + 1);
  refP = PropGetRef(propName, TRUE);
  for (i1 = 0; i1 < clusterSize; i1++) {
    molP = dataP->molPA[dataP->molIA[i1]];
    if (i1 == bestI)
      DhMolSetProp(dataP->refP, molP, TRUE);
    DhMolSetProp(refP, molP, TRUE);
  }

  dataP->clusterNo++;
}

#define ARG_NUM 3

ErrCode
ExCalcCluster(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  DhMolP *molPA;
  PropRefP refP;
  int molNo, atomNo, atomNo1;
  LINLIST propList;
  DSTR *strP;
  float *rmsdA, rmsd, d;
  AtomData atomData1, atomData2;
  ClusterEntryP *entryPA;
  ClusterData clusterData;
  DSTR statStr;
  char numBuf[20];
  int molI, molI1, molI2, atomI, i;

  arg[0].type = AT_INT;
  arg[1].type = AT_DOUBLE;
  arg[2].type = AT_DOUBLE;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Cluster #";
  arg[0].v.intVal = ClusterNo;

  arg[1].prompt = "Min. RMSD";
  arg[1].v.doubleVal = MinRmsd;

  arg[2].prompt = "Max. RMSD";
  arg[2].v.doubleVal = MaxRmsd;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  ClusterNo = arg[0].v.intVal;
  MinRmsd = (float) arg[1].v.doubleVal;
  MaxRmsd = (float) arg[2].v.doubleVal;

  ArgCleanup(arg, ARG_NUM);

  DhActivateGroups(TRUE);

  molNo = SelMolGet(NULL, 0);
  if (molNo < 2) {
    DhActivateGroups(FALSE);
    CipSetError("at least 2 molecules must be selected");
    return EC_ERROR;
  }

  molPA = malloc(molNo * sizeof(DhMolP));
  (void) SelMolGet(molPA, molNo);

  refP = PropGetRef(PROP_SELECTED, FALSE);

  atomNo = 0;
  DhMolApplyAtom(refP, molPA[0], countAtoms, &atomNo);
  if (atomNo < 1) {
    free(molPA);
    DhActivateGroups(FALSE);
    CipSetError("at least 1 atom must be selected");
    return EC_ERROR;
  }

  for (molI = 1; molI < molNo; molI++) {
    atomNo1 = 0;
    DhMolApplyAtom(refP, molPA[molI], countAtoms, &atomNo1);
    if (atomNo1 != atomNo) {
      free(molPA);
      DhActivateGroups(FALSE);
      CipSetError("number of selected atoms must be equal");
      return EC_ERROR;
    }
  }

  /* clear old properties */
  for (molI = 0; molI < molNo; molI++) {
    propList = ListOpen(sizeof(DSTR));
    PropList(DhMolGetPropTab(molPA[molI]), getProp, propList);
    strP = ListFirst(propList);
    while (strP != NULL) {
      DhMolSetProp(PropGetRef(DStrToStr(*strP), FALSE), molPA[molI], FALSE);
      DStrFree(*strP);
      strP = ListNext(propList, strP);
    }
    ListClose(propList);
  }

  BreakActivate(TRUE);

  rmsdA = malloc(molNo * molNo * sizeof(*rmsdA));

  atomData1.coordA = malloc(atomNo * sizeof(Vec3));
  atomData2.coordA = malloc(atomNo * sizeof(Vec3));

  for (molI1 = 0; molI1 < molNo - 1; molI1++) {
    atomData1.atomI = 0;
    DhMolApplyAtom(refP, molPA[molI1], getCoord, &atomData1);

    for (molI2 = molI1 + 1; molI2 < molNo; molI2++) {
      if (BreakCheck(1))
	break;

      atomData2.atomI = 0;
      DhMolApplyAtom(refP, molPA[molI2], getCoord, &atomData2);

      rmsd = 0.0f;
      for (atomI = 0; atomI < atomNo; atomI++)
	for (i = 0; i < 3; i++) {
	  d = atomData1.coordA[atomI][i] - atomData2.coordA[atomI][i];
	  rmsd += d * d;
	}

      rmsd = sqrtf(rmsd / atomNo);
      rmsdA[molI1 * molNo + molI2] = rmsd;
      rmsdA[molI2 * molNo + molI1] = rmsd;
    }

    if (BreakInterrupted())
      break;
  }

  BreakActivate(FALSE);

  if (! BreakInterrupted()) {
    entryPA = malloc(molNo * sizeof(*entryPA));

    for (molI = 0; molI < molNo; molI++)
      entryPA[molI] = ClusterAddEntry(molPA[molI]);

    for (molI1 = 0; molI1 < molNo - 1; molI1++)
      for (molI2 = molI1 + 1; molI2 < molNo; molI2++) {
	rmsd = rmsdA[molI1 * molNo + molI2];
	if (rmsd <= MaxRmsd)
	  ClusterAddPair(entryPA[molI1], entryPA[molI2], rmsd);
      }

    free(entryPA);

    clusterData.molPA = molPA;
    clusterData.molNo = molNo;
    clusterData.rmsdA = rmsdA;
    clusterData.clusterNo = 0;
    clusterData.refP = PropGetRef(PROP_NAME, TRUE);
    clusterData.molIA = malloc(molNo * sizeof(*clusterData.molIA));

    ClusterCalc(ClusterNo, MinRmsd, setCluster, &clusterData);

    statStr = DStrNew();
    (void) sprintf(numBuf, "%d", clusterData.clusterNo);
    DStrAppStr(statStr, numBuf);
    DStrAppStr(statStr, " molecules ");
    DStrAppStr(statStr, PROP_NAME);
    PuSetTextField(PU_TF_STATUS, DStrToStr(statStr));
    DStrFree(statStr);

    free(clusterData.molIA);
  }

  free(atomData1.coordA);
  free(atomData2.coordA);
  free(rmsdA);
  free(molPA);

  DhActivateGroups(FALSE);

  return EC_OK;
}
