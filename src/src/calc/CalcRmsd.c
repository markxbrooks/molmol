/*
************************************************************************
*
*   CalcRmsd.c - calculate RMSD
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/calc/SCCS/s.CalcRmsd.c
*   SCCS identification       : 1.18
*
************************************************************************
*/

#include <calc_rmsd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <break.h>
#include <map_coord.h>
#include <expr.h>
#include <data_hand.h>
#include <data_sel.h>

#define MIN_ATOM_NO 3
#define MAX_NAME_LEN 80

typedef struct {
  float avg;
  float der;
  float min;
  float max;
  int n;
} StatData;

typedef struct {
  char name[MAX_NAME_LEN + 1];
  PropRefP refP;
  int membNo;
  int pairNo;
  StatData stat;
} GroupInfo;

typedef struct {
  int groupI;
  int membI;
} MolInfo;

typedef struct {
  char name[MAX_NAME_LEN + 1];
  PropRefP refP;
  int atomNo;
  float *avgA;
} LocalInfo;

typedef struct {
  ExprP rangeExprP;
  BOOL transf;
  Vec3 *coordA;
  DhResP *resPA;
  int atomI;
} AtomData;

typedef struct {
  ExprP rangeExprP;
  PropRefP refP;
  DhResP *resPA;
  int resI;
} ResData;

static CalcReportF Report;
static CalcErrorF Error;
static DSTR ErrorStr = NULL;

static int
listLen(char *str)
{
  int n, len, i;

  len = strlen(str);

  if (len == 0)
    return 0;

  n = 1;
  for (i = 0; i < len; i++)
    if (str[i] == ' ')
      n++;

  return n;
}

static void
buildGroupInfo(char *str, GroupInfo **infoPP, int *nP)
{
  int n, len, ind, nameLen;
  GroupInfo *infoP;
  int i;

  n = listLen(str);
  if (n == 0) {
    *infoPP = NULL;
    *nP = 0;
    return;
  }

  infoP = malloc(n * sizeof(*infoP));

  len = strlen(str);
  ind = 0;
  nameLen = 0;
  for (i = 0; i <= len; i++) {
    if (str[i] == ' ' || str[i] == '\0') {
      infoP[ind].name[nameLen] = '\0';
      infoP[ind].refP = PropGetRef(infoP[ind].name, FALSE);
      ind++;
      nameLen = 0;
    } else if (nameLen < MAX_NAME_LEN) {
      infoP[ind].name[nameLen++] = str[i];
    }
  }

  *infoPP = infoP;
  *nP = n;
}

static void
buildLocalInfo(char *str, LocalInfo **infoPP, int *nP)
{
  int n, len, ind, nameLen;
  LocalInfo *infoP;
  int i;

  n = listLen(str);
  if (n == 0) {
    *infoPP = NULL;
    *nP = 0;
    return;
  }

  infoP = malloc(n * sizeof(*infoP));

  len = strlen(str);
  ind = 0;
  nameLen = 0;
  for (i = 0; i <= len; i++) {
    if (str[i] == ' ' || str[i] == '\0') {
      infoP[ind].name[nameLen] = '\0';
      infoP[ind].refP = PropGetRef(infoP[ind].name, FALSE);
      ind++;
      nameLen = 0;
    } else if (nameLen < MAX_NAME_LEN) {
      infoP[ind].name[nameLen++] = str[i];
    }
  }

  *infoPP = infoP;
  *nP = n;
}

static void
countAtom(DhAtomP atomP, void *clientData)
{
  AtomData *dataP = clientData;
  ExprRes exprRes;

  if (dataP->rangeExprP == NULL)
    exprRes.u.boolVal = TRUE;
  else
    ExprEval(DhAtomGetRes(atomP), dataP->rangeExprP, &exprRes);

  if (exprRes.u.boolVal)
    dataP->atomI++;
}

static int
getMolAtomNo(PropRefP refP, ExprP rangeExprP, DhMolP molP)
{
  AtomData atomData;

  atomData.rangeExprP = rangeExprP;
  atomData.atomI = 0;
  DhMolApplyAtom(refP, molP, countAtom, &atomData);
  return atomData.atomI;
}

static int
getResAtomNo(PropRefP refP, DhResP resP)
{
  AtomData atomData;

  atomData.rangeExprP = NULL;
  atomData.atomI = 0;
  DhResApplyAtom(refP, resP, countAtom, &atomData);
  return atomData.atomI;
}

static void
countRes(DhResP resP, void *clientData)
{
  ResData *dataP = clientData;
  ExprRes exprRes;

  if (dataP->rangeExprP == NULL)
    exprRes.u.boolVal = TRUE;
  else
    ExprEval(resP, dataP->rangeExprP, &exprRes);

  if (exprRes.u.boolVal && getResAtomNo(dataP->refP, resP) > 0)
    dataP->resI++;
}

static int
getResNo(PropRefP refP, ExprP rangeExprP, DhMolP molP)
{
  ResData resData;

  resData.rangeExprP = rangeExprP;
  resData.refP = refP;
  resData.resI = 0;
  DhMolApplyRes(refP, molP, countRes, &resData);
  return resData.resI;
}

static void
fillRes(DhResP resP, void *clientData)
{
  ResData *dataP = clientData;
  ExprRes exprRes;

  if (dataP->rangeExprP == NULL)
    exprRes.u.boolVal = TRUE;
  else
    ExprEval(resP, dataP->rangeExprP, &exprRes);

  if (exprRes.u.boolVal && getResAtomNo(dataP->refP, resP) > 0) {
    dataP->resPA[dataP->resI] = resP;
    dataP->resI++;
  }
}

static BOOL
compareMol(PropRefP refP, ExprP rangeExprP,
    DhMolP molPA[], int molNo,
    int *resNoP, int *atomNoP)
{
  int resNo;
  ResData resData;
  int *atomNoA;
  int molI, resI;

  resNo = getResNo(refP, rangeExprP, molPA[0]);
  if (resNoP != NULL)
    *resNoP = resNo;

  for (molI = 1; molI < molNo; molI++)
    if (resNo != getResNo(refP, rangeExprP, molPA[molI]))
      return FALSE;

  resData.rangeExprP = rangeExprP;
  resData.resPA = malloc(resNo * sizeof(DhResP));
  atomNoA = malloc(resNo * sizeof(*atomNoA));

  resData.rangeExprP = rangeExprP;
  resData.resI = 0;
  resData.refP = refP;
  DhMolApplyRes(refP, molPA[0], fillRes, &resData);

  for (resI = 0; resI < resNo; resI++)
    atomNoA[resI] = getResAtomNo(refP, resData.resPA[resI]);

  for (molI = 1; molI < molNo; molI++) {
    resData.resI = 0;
    DhMolApplyRes(refP, molPA[molI], fillRes, &resData);

    for (resI = 0; resI < resNo; resI++)
      if (getResAtomNo(refP, resData.resPA[resI]) != atomNoA[resI]) {
	free(resData.resPA);
	free(atomNoA);
	return FALSE;
      }
  }

  if (atomNoP != NULL)
    *atomNoP = getMolAtomNo(refP, rangeExprP, molPA[0]);

  free(resData.resPA);
  free(atomNoA);

  return TRUE;
}

static void
setError(char *propName)
{
  /* must be static because it's used after we return */
  if (ErrorStr == NULL)
    ErrorStr = DStrNew();

  DStrAssignStr(ErrorStr, "different number of ");
  DStrAppStr(ErrorStr, propName);
  DStrAppStr(ErrorStr, " atoms");
  Error(DStrToStr(ErrorStr));
}

static void
fillAtom(DhAtomP atomP, void *clientData)
{
  AtomData *dataP = clientData;
  ExprRes exprRes;

  if (dataP->rangeExprP == NULL)
    exprRes.u.boolVal = TRUE;
  else
    ExprEval(DhAtomGetRes(atomP), dataP->rangeExprP, &exprRes);

  if (exprRes.u.boolVal) {
    if (dataP->transf)
      DhAtomGetCoordTransf(atomP, dataP->coordA[dataP->atomI]);
    else
      DhAtomGetCoord(atomP, dataP->coordA[dataP->atomI]);
    dataP->resPA[dataP->atomI] = DhAtomGetRes(atomP);
    dataP->atomI++;
  }
}

static float
calcSqrDist(Vec3 x0, Vec3 x1, Mat4 mapM, BOOL applyM)
{
  Vec3 v3;
  Vec4 v4;

  Vec3Copy(v3, x1);

  if (applyM) {
    Vec3To4(v4, v3);
    Mat4VecMult(v4, mapM);
    Vec4To3(v3, v4);
  }

  Vec3Sub(v3, x0);

  return v3[0] * v3[0] + v3[1] * v3[1] + v3[2] * v3[2];
}

static float
calcRmsd(Vec3 x0A[], int n, Vec3 x1A[], Mat4 mapM, BOOL applyM)
{
  float rmsd;
  int i;

  rmsd = 0.0f;
  for (i = 0; i < n; i++)
    rmsd += calcSqrDist(x0A[i], x1A[i], mapM, applyM);

  return sqrtf(rmsd / n);
}

static void
initStatData(StatData *statP)
/* avg must be set to the sum of all values, n to the number of values */
{
  if (statP->n > 0)
    statP->avg /= statP->n;
  statP->der = 0.0f;
  statP->min = 99.99f;
  statP->max = 0.0f;
}

static void
addStatData(StatData *statP, float val)
{
  if (val < statP->min)
    statP->min = val;
  if (val > statP->max)
    statP->max = val;

  statP->der += (val - statP->avg) * (val - statP->avg);
}

static void
calcStatData(StatData *statP)
{
  if (statP->n > 1)
    statP->der = sqrtf(statP->der / (statP->n - 1));
}

static void
getStat(StatData *statP, float *valA, int n)
{
  int i;

  statP->avg = 0.0f;
  for (i = 0; i < n; i++)
    statP->avg += valA[i];
  statP->n = n;

  initStatData(statP);
  for (i = 0; i < n; i++)
    addStatData(statP, valA[i]);
  calcStatData(statP);
}

static void
writeInt(int i)
{
  char buf[10];

  (void) sprintf(buf, "%3d", i);
  Report(buf);
}

static void
writeFloat(float f)
{
  char buf[10];

  (void) sprintf(buf, "%#5.2f", f);
  Report(buf);
}

static void
writeStatData(StatData *statP)
{
  writeFloat(statP->avg);
  Report(" +/- ");
  writeFloat(statP->der);
  Report(" A  (");
  writeFloat(statP->min);
  Report("..");
  writeFloat(statP->max);
  Report(" A)\n");
}

void
CalcRmsd(BOOL fitPairs, char *range, char *glob1, char *glob2,
    char *groups, char *dispRef,
    char *globDisp, char *locRmsd, char *locDisp,
    CalcReportF report, CalcErrorF error)
{
  DSTR rangeStr;
  ExprP rangeExprP;
  ExprResType resType;
  PropRefP refP1, refP2, allRefP, dispRefP, refP;
  char name1[MAX_NAME_LEN], name2[MAX_NAME_LEN];
  int len1, len2;
  GroupInfo *groupInfoP;
  int groupNo, groupI1, groupI2;
  MolInfo *molInfoP;
  LocalInfo *globDispInfoP, *locRmsdInfoP, *locDispInfoP;
  int globDispNo, locRmsdNo, locDispNo, rmsdI, dispI;
  int molNo, pairNo, molI1, molI2;
  DhMolP *molPA;
  int atomNo1, atomNo2, atomNo;
  BOOL ok, calc1, calc2;
  AtomData atomData1, atomData2;
  int resNo, allResNo0, allResNo, resI, resI0, resI1, resI2;
  ResData allResData0, allResData1, allResData2;
  ResData resData0, resData1, resData2;
  Mat4 mapM;
  float rmsd, *rmsdA, rmsd1, rmsd2;
  StatData stat1, stat2;
  float *valA;
  int valNo, propI, colI;
  int i;

  Report = report;
  Error = error;

  if (strlen(range) == 0) {
    rangeExprP = NULL;
  } else {
    rangeStr = DStrNew();
    DStrAssignStr(rangeStr, "num = ");
    DStrAppStr(rangeStr, range);

    rangeExprP = ExprCompile(DE_RES, DStrToStr(rangeStr), &resType);
    DStrFree(rangeStr);

    if (rangeExprP == NULL) {
      Error(ExprGetErrorMsg());
      return;
    }
  }

  refP1 = PropGetRef(glob1, FALSE);
  refP2 = PropGetRef(glob2, FALSE);

  (void) strncpy(name1, glob1, sizeof(name1));
  (void) strncpy(name2, glob2, sizeof(name2));

  len1 = strlen(glob1);
  len2 = strlen(glob2);
  if (len1 >= sizeof(name1))
    len1 = sizeof(name1) - 1;
  if (len2 >= sizeof(name2))
    len2 = sizeof(name2) - 1;
  
  if (len1 > len2) {
    for (i = len2; i < len1; i++)
      name2[i] = ' ';
    name1[len1] = '\0';
    name2[len1] = '\0';
  } else {
    for (i = len1; i < len2; i++)
      name1[i] = ' ';
    name1[len2] = '\0';
    name2[len2] = '\0';
  }

  buildLocalInfo(globDisp, &globDispInfoP, &globDispNo);
  buildLocalInfo(locRmsd, &locRmsdInfoP, &locRmsdNo);
  buildLocalInfo(locDisp, &locDispInfoP, &locDispNo);

  DhActivateGroups(TRUE);

  molNo = SelMolGet(NULL, 0);
  if (molNo < 2) {
    free(globDispInfoP);
    free(locRmsdInfoP);
    free(locDispInfoP);
    DhActivateGroups(FALSE);
    Error("at least 2 molecules must be selected");
    return;
  }

  molPA = malloc(molNo * sizeof(DhMolP));
  (void) SelMolGet(molPA, molNo);

  ok = compareMol(refP1, rangeExprP, molPA, molNo, NULL, &atomNo1);
  if (ok) {
    ok = compareMol(refP2, rangeExprP, molPA, molNo, NULL, &atomNo2);
    if (! ok)
      setError(glob2);
  } else {
    setError(glob1);
  }

  for (dispI = 0; ok && dispI < globDispNo; dispI++) {
    ok = compareMol(globDispInfoP[dispI].refP, NULL, molPA, molNo,
	NULL, &globDispInfoP[dispI].atomNo);
    if (! ok)
      setError(globDispInfoP[dispI].name);
  }

  for (rmsdI = 0; ok && rmsdI < locRmsdNo; rmsdI++) {
    ok = compareMol(locRmsdInfoP[rmsdI].refP, NULL, molPA, molNo,
	NULL, &locRmsdInfoP[rmsdI].atomNo);
    if (! ok)
      setError(locRmsdInfoP[dispI].name);
  }

  for (dispI = 0; ok && dispI < locDispNo; dispI++) {
    ok = compareMol(locDispInfoP[dispI].refP, NULL, molPA, molNo,
	NULL, &locDispInfoP[dispI].atomNo);
    if (! ok)
      setError(locDispInfoP[dispI].name);
  }

  if (ok) {
    calc1 = (atomNo1 >= MIN_ATOM_NO);
    calc2 = (atomNo2 >= MIN_ATOM_NO);
    if (! (calc1 || calc2)) {
      Error("number of atoms must be at least 3");
      ok = FALSE;
    }
  }

  if (! ok) {
    free(globDispInfoP);
    free(locRmsdInfoP);
    free(locDispInfoP);
    free(molPA);
    DhActivateGroups(FALSE);
    return;
  }

  buildGroupInfo(groups, &groupInfoP, &groupNo);
  for (groupI1 = 0; groupI1 < groupNo; groupI1++)
    groupInfoP[groupI1].membNo = 0;

  molInfoP = malloc(molNo * sizeof(*molInfoP));
  for (molI1 = 0; molI1 < molNo; molI1++) {
    molInfoP[molI1].groupI = -1;
    for (groupI1 = 0; groupI1 < groupNo; groupI1++)
      if (DhMolGetProp(groupInfoP[groupI1].refP, molPA[molI1])) {
	molInfoP[molI1].groupI = groupI1;
	molInfoP[molI1].membI = groupInfoP[groupI1].membNo;
	groupInfoP[groupI1].membNo++;
	break;
      }
  }

  for (molI1 = 0; molI1 < molNo; molI1++) {
    writeInt(molI1 + 1);
    Report(" ");
    Report(DStrToStr(DhMolGetName(molPA[molI1])));
    groupI1 = molInfoP[molI1].groupI;
    if (groupI1 >= 0) {
      Report(" (");
      Report(groupInfoP[groupI1].name);
      Report(")");
    }
    Report("\n");
  }
  Report("\nRMSD table:\n\n");

  Report("    ");
  for (molI1 = 0; molI1 < molNo; molI1++) {
    Report("  ");
    writeInt(molI1 + 1);
  }
  Report("\n");

  if (atomNo1 > atomNo2)
    atomNo = atomNo1;
  else
    atomNo = atomNo2;

  for (dispI = 0; dispI < globDispNo; dispI++)
    if (globDispInfoP[dispI].atomNo > atomNo)
      atomNo = globDispInfoP[dispI].atomNo;

  for (rmsdI = 0; rmsdI < locRmsdNo; rmsdI++)
    if (locRmsdInfoP[rmsdI].atomNo > atomNo)
      atomNo = locRmsdInfoP[rmsdI].atomNo;

  for (dispI = 0; dispI < locDispNo; dispI++)
    if (locDispInfoP[dispI].atomNo > atomNo)
      atomNo = locDispInfoP[dispI].atomNo;

  atomData1.coordA = malloc(atomNo * sizeof(Vec3));
  atomData1.resPA = malloc(atomNo * sizeof(DhResP));
  atomData2.coordA = malloc(atomNo * sizeof(Vec3));
  atomData2.resPA = malloc(atomNo * sizeof(DhResP));

  atomData1.transf = ! fitPairs;
  atomData2.transf = ! fitPairs;

  allRefP = PropGetRef(PROP_ALL, FALSE);
  allResNo0 = getResNo(allRefP, NULL, molPA[0]);
  allResData0.resPA = malloc(allResNo0 * sizeof(DhResP));
  resData0.resPA = malloc(allResNo0 * sizeof(DhResP));

  allResData0.rangeExprP = NULL;
  allResData0.resI = 0;
  allResData0.refP = allRefP;
  DhMolApplyRes(allRefP, molPA[0], fillRes, &allResData0);

  allResNo = allResNo0;
  for (molI1 = 1; molI1 < molNo; molI1++) {
    resNo = getResNo(allRefP, NULL, molPA[molI1]);
    if (resNo > allResNo)
      allResNo = resNo;
  }

  allResData1.resPA = malloc(allResNo * sizeof(DhResP));
  allResData2.resPA = malloc(allResNo * sizeof(DhResP));
  resData1.resPA = malloc(allResNo * sizeof(DhResP));
  resData2.resPA = malloc(allResNo * sizeof(DhResP));

  if (groupNo == 0)
    valNo = allResNo0;
  else
    valNo = groupNo * allResNo0;

  for (dispI = 0; dispI < globDispNo; dispI++) {
    globDispInfoP[dispI].avgA = malloc(valNo * sizeof(float));
    for (resI = 0; resI < valNo; resI++)
      globDispInfoP[dispI].avgA[resI] = 0.0f;
  }

  for (rmsdI = 0; rmsdI < locRmsdNo; rmsdI++) {
    locRmsdInfoP[rmsdI].avgA = malloc(valNo * sizeof(float));
    for (resI = 0; resI < valNo; resI++)
      locRmsdInfoP[rmsdI].avgA[resI] = 0.0f;
  }

  for (dispI = 0; dispI < locDispNo; dispI++) {
    locDispInfoP[dispI].avgA = malloc(valNo * sizeof(float));
    for (resI = 0; resI < valNo; resI++)
      locDispInfoP[dispI].avgA[resI] = 0.0f;
  }

  if (dispRef[0] != '\0')
    dispRefP = PropGetRef(dispRef, FALSE);
  else
    dispRefP = NULL;

  rmsdA = malloc(molNo * molNo * sizeof(*rmsdA));
  for (i = 0; i < molNo * molNo; i++)
    rmsdA[i] = 0.0f;

  pairNo = molNo * (molNo - 1) / 2;
  stat1.avg = 0.0f;
  stat2.avg = 0.0f;

  for (molI1 = 0; molI1 < molNo; molI1++) {
    if (BreakCheck(1))
      break;

    writeInt(molI1 + 1);
    Report(" ");

    allResData1.rangeExprP = NULL;
    allResData1.resI = 0;
    allResData1.refP = allRefP;
    DhMolApplyRes(allRefP, molPA[molI1], fillRes, &allResData1);

    for (molI2 = 0; molI2 < molNo; molI2++) {
      if (BreakCheck(1))
	break;

      if (molI1 == molI2) {
	Report("     ");
	continue;
      }

      if (molI1 < molI2) {
	refP = refP1;
	atomNo = atomNo1;
      } else {
	refP = refP2;
	atomNo = atomNo2;
      }

      if (atomNo < MIN_ATOM_NO) {
	Report("     ");
	continue;
      }

      atomData1.rangeExprP = rangeExprP;
      atomData1.atomI = 0;
      DhMolApplyAtom(refP, molPA[molI1], fillAtom, &atomData1);
      atomData2.rangeExprP = rangeExprP;
      atomData2.atomI = 0;
      DhMolApplyAtom(refP, molPA[molI2], fillAtom, &atomData2);

      if (fitPairs)
	MapCoord(atomData1.coordA, atomNo, atomData2.coordA, TRUE, mapM);

      rmsd = calcRmsd(atomData1.coordA, atomNo, atomData2.coordA,
	  mapM, fitPairs);

      writeFloat(rmsd);
      rmsdA[molI1 * molNo + molI2] = rmsd;

      if (molI1 < molI2)
	stat1.avg += rmsd;
      else
	stat2.avg += rmsd;

      if (molI1 > molI2)
	continue;

      if (globDispNo == 0 && locRmsdNo == 0 && locDispNo == 0)
	continue;

      groupI1 = molInfoP[molI1].groupI;
      groupI2 = molInfoP[molI2].groupI;
      if (groupNo > 0 && (groupI1 < 0 || groupI1 != groupI2))
	continue;

      if (dispRefP != NULL &&
	  DhMolGetProp(dispRefP, molPA[molI1]) ==
	  DhMolGetProp(dispRefP, molPA[molI2]))
	continue;

      allResData2.rangeExprP = NULL;
      allResData2.resI = 0;
      allResData2.refP = allRefP;
      DhMolApplyRes(allRefP, molPA[molI2], fillRes, &allResData2);

      resData0.rangeExprP = NULL;
      resData1.rangeExprP = NULL;
      resData2.rangeExprP = NULL;

      atomData1.rangeExprP = NULL;
      atomData2.rangeExprP = NULL;

      /* calculate global displacements */
      for (dispI = 0; dispI < globDispNo; dispI++) {
	refP = globDispInfoP[dispI].refP;

	resData0.resI = 0;
	resData0.refP = refP;
	DhMolApplyRes(refP, molPA[0], fillRes, &resData0);
	resNo = resData0.resI;

	resData1.resI = 0;
	resData1.refP = refP;
	DhMolApplyRes(refP, molPA[molI1], fillRes, &resData1);

	resData2.resI = 0;
	resData2.refP = refP;
	DhMolApplyRes(refP, molPA[molI2], fillRes, &resData2);

	resI0 = 0;
	for (resI = 0; resI < resNo; resI++) {
	  while (allResData0.resPA[resI0] != resData0.resPA[resI])
	    resI0++;

	  atomData1.atomI = 0;
	  DhResApplyAtom(refP, resData1.resPA[resI], fillAtom, &atomData1);

	  atomData2.atomI = 0;
	  DhResApplyAtom(refP, resData2.resPA[resI], fillAtom, &atomData2);

	  rmsd = calcRmsd(atomData1.coordA, atomData1.atomI,
	      atomData2.coordA, mapM, fitPairs);

	  if (groupNo == 0)
	    globDispInfoP[dispI].avgA[resI0] += rmsd;
	  else
	    globDispInfoP[dispI].avgA[groupNo * resI0 + groupI1] += rmsd;
	}
      }

      atomData1.rangeExprP = NULL;
      atomData2.rangeExprP = NULL;

      /* calculate local RMSDs */
      for (rmsdI = 0; rmsdI < locRmsdNo; rmsdI++) {
	refP = locRmsdInfoP[rmsdI].refP;

	resData0.resI = 0;
	resData0.refP = refP;
	DhMolApplyRes(refP, molPA[0], fillRes, &resData0);
	resNo = resData0.resI;

	resData1.resI = 0;
	resData1.refP = refP;
	DhMolApplyRes(refP, molPA[molI1], fillRes, &resData1);

	resData2.resI = 0;
	resData2.refP = refP;
	DhMolApplyRes(refP, molPA[molI2], fillRes, &resData2);

	resI0 = 0;
	resI1 = 0;
	resI2 = 0;
	for (resI = 0; resI < resNo; resI++) {
	  while (allResData0.resPA[resI0] != resData0.resPA[resI])
	    resI0++;
	  while (allResData1.resPA[resI1] != resData1.resPA[resI])
	    resI1++;
	  while (allResData2.resPA[resI2] != resData2.resPA[resI])
	    resI2++;

	  if (resI0 == 0 || resI0 == allResNo0 - 1 ||
	      resI1 == 0 || resI1 == allResData1.resI ||
	      resI2 == 0 || resI2 == allResData2.resI)
	    continue;

	  atomData1.atomI = 0;
	  DhResApplyAtom(refP, allResData1.resPA[resI1 - 1],
	      fillAtom, &atomData1);
	  DhResApplyAtom(refP, allResData1.resPA[resI1],
	      fillAtom, &atomData1);
	  DhResApplyAtom(refP, allResData1.resPA[resI1 + 1],
	      fillAtom, &atomData1);

	  atomData2.atomI = 0;
	  DhResApplyAtom(refP, allResData2.resPA[resI2 - 1],
	      fillAtom, &atomData2);
	  DhResApplyAtom(refP, allResData2.resPA[resI2],
	      fillAtom, &atomData2);
	  DhResApplyAtom(refP, allResData2.resPA[resI2 + 1],
	      fillAtom, &atomData2);

	  MapCoord(atomData1.coordA, atomData1.atomI,
	      atomData2.coordA, TRUE, mapM);

	  rmsd = calcRmsd(atomData1.coordA, atomData1.atomI,
	      atomData2.coordA, mapM, fitPairs);

	  if (groupNo == 0)
	    locRmsdInfoP[rmsdI].avgA[resI0] += rmsd;
	  else
	    locRmsdInfoP[rmsdI].avgA[groupNo * resI0 + groupI1] += rmsd;
	}
      }

      if (locDispNo == 0)
	continue;

      /* calculate local displacements */
      resData0.resI = 0;
      resData0.refP = refP1;
      DhMolApplyRes(refP, molPA[0], fillRes, &resData0);
      resNo = resData0.resI;

      resData1.resI = 0;
      resData1.refP = refP1;
      DhMolApplyRes(refP, molPA[molI1], fillRes, &resData1);

      resData2.resI = 0;
      resData2.refP = refP1;
      DhMolApplyRes(refP, molPA[molI2], fillRes, &resData2);

      resI0 = 0;
      resI1 = 0;
      resI2 = 0;
      for (resI = 0; resI < resNo; resI++) {
	while (allResData0.resPA[resI0] != resData0.resPA[resI])
	  resI0++;
	while (allResData1.resPA[resI1] != resData1.resPA[resI])
	  resI1++;
	while (allResData2.resPA[resI2] != resData2.resPA[resI])
	  resI2++;

	if (resI0 == 0 || resI0 == allResNo0 - 1 ||
	    resI1 == 0 || resI1 == allResData1.resI ||
	    resI2 == 0 || resI2 == allResData2.resI)
	  continue;

	atomData1.atomI = 0;
	DhResApplyAtom(refP1, allResData1.resPA[resI1 - 1],
	    fillAtom, &atomData1);
	DhResApplyAtom(refP1, allResData1.resPA[resI1],
	    fillAtom, &atomData1);
	DhResApplyAtom(refP1, allResData1.resPA[resI1 + 1],
	    fillAtom, &atomData1);

	if (atomData1.atomI < 3)
	  continue;

	atomData2.atomI = 0;
	DhResApplyAtom(refP1, allResData2.resPA[resI2 - 1],
	    fillAtom, &atomData2);
	DhResApplyAtom(refP1, allResData2.resPA[resI2],
	    fillAtom, &atomData2);
	DhResApplyAtom(refP1, allResData2.resPA[resI2 + 1],
	    fillAtom, &atomData2);

	MapCoord(atomData1.coordA, atomData1.atomI,
	    atomData2.coordA, TRUE, mapM);

	for (dispI = 0; dispI < locDispNo; dispI++) {
	  refP = locDispInfoP[dispI].refP;

	  atomData1.atomI = 0;
	  DhResApplyAtom(refP, resData1.resPA[resI], fillAtom, &atomData1);

	  if (atomData1.atomI == 0)
	    continue;

	  atomData2.atomI = 0;
	  DhResApplyAtom(refP, resData2.resPA[resI], fillAtom, &atomData2);

	  rmsd = calcRmsd(atomData1.coordA, atomData1.atomI,
	      atomData2.coordA, mapM, fitPairs);

	  if (groupNo == 0)
	    locDispInfoP[dispI].avgA[resI0] += rmsd;
	  else
	    locDispInfoP[dispI].avgA[groupNo * resI0 + groupI1] += rmsd;
	}
      }
    }

    Report("\n");
  }

  Report("\n");

  stat1.n = pairNo;
  initStatData(&stat1);
  stat2.n = pairNo;
  initStatData(&stat2);
  for (molI1 = 0; molI1 < molNo; molI1++)
    for (molI2 = 0; molI2 < molNo; molI2++) {
      rmsd = rmsdA[molI1 * molNo + molI2];

      if (molI1 < molI2)
        addStatData(&stat1, rmsd);
      else if (molI1 > molI2)
        addStatData(&stat2, rmsd);
    }

  if (range[0] != '\0') {
    Report("residues considered: ");
    Report(range);
    Report("\n\n");
  }

  if (calc1) {
    Report("mean global ");
    Report(name1);
    Report(" RMSD: ");
    calcStatData(&stat1);
    writeStatData(&stat1);
  }

  if (calc2) {
    Report("mean global ");
    Report(name2);
    Report(" RMSD: ");
    calcStatData(&stat2);
    writeStatData(&stat2);
  }

  if (groupNo > 0)
    Report("\n");

  for (groupI1 = 0; groupI1 < groupNo; groupI1++) {
    writeInt(groupI1 + 1);
    Report(" ");
    Report(groupInfoP[groupI1].name);
    Report("\n");
  }

  for (propI = 0; propI < 2; propI++) {
    if (groupNo == 0 || (propI == 0 && ! calc1))
      continue;

    if (groupNo == 0 || (propI == 1 && ! calc2))
      continue;

    Report("\nAverage RMSD, dev., min. and max of ");
    if (propI == 0)
      Report(name1);
    else
      Report(name2);
    Report(":\n");

    for (colI = 0; colI < 4; colI++) {
      Report("    ");
      for (groupI1 = 0; groupI1 < groupNo; groupI1++) {
	Report("  ");
	writeInt(groupI1 + 1);
      }
    }
    Report("\n");

    for (groupI1 = 0; groupI1 < groupNo; groupI1++) {
      writeInt(groupI1 + 1);
      Report(" ");

      for (groupI2 = 0; groupI2 < groupNo; groupI2++) {
	if (((groupI1 < groupI2) &&
	    (groupInfoP[groupI1].membNo != groupInfoP[groupI2].membNo)) ||
	    groupInfoP[groupI1].membNo == 0 ||
	    groupInfoP[groupI2].membNo == 0) {
	  groupInfoP[groupI2].stat.avg = 0.0f;
	  groupInfoP[groupI2].stat.der = 0.0f;
	  groupInfoP[groupI2].stat.min = 0.0f;
	  groupInfoP[groupI2].stat.max = 0.0f;
	  continue;
	}

	valA = malloc(
	    groupInfoP[groupI1].membNo * groupInfoP[groupI2].membNo *
	    sizeof(*valA));
	valNo = 0;

	for (molI1 = 0; molI1 < molNo; molI1++) {
	  if (molInfoP[molI1].groupI != groupI1 &&
	      molInfoP[molI1].groupI != groupI2)
	    continue;

	  for (molI2 = 0; molI2 < molNo; molI2++) {
	    if (propI == 0 && molI1 >= molI2)
	      continue;

	    if (propI == 1 && molI1 <= molI2)
	      continue;

	    if (molInfoP[molI1].groupI == groupI1) {
	      if (molInfoP[molI2].groupI != groupI2)
		continue;
	    } else {
	      if (molInfoP[molI2].groupI != groupI1)
		continue;
	    }

	    if (groupI1 < groupI2 &&
		molInfoP[molI1].membI != molInfoP[molI2].membI)
	      continue;

	    valA[valNo++] = rmsdA[molI1 * molNo + molI2];
	  }
	}

	getStat(&groupInfoP[groupI2].stat, valA, valNo);
	free(valA);
      }

      for (groupI2 = 0; groupI2 < groupNo; groupI2++)
	writeFloat(groupInfoP[groupI2].stat.avg);

      Report("    ");
      for (groupI2 = 0; groupI2 < groupNo; groupI2++)
	writeFloat(groupInfoP[groupI2].stat.der);

      Report("    ");
      for (groupI2 = 0; groupI2 < groupNo; groupI2++)
	writeFloat(groupInfoP[groupI2].stat.min);

      Report("    ");
      for (groupI2 = 0; groupI2 < groupNo; groupI2++)
	writeFloat(groupInfoP[groupI2].stat.max);

      Report("\n");
    }
  }

  Report("\naverage RMSD of each structure to the rest:\n");

  for (molI1 = 0; molI1 < molNo; molI1++) {
    stat1.avg = 0.0f;
    stat2.avg = 0.0f;

    for (molI2 = 0; molI2 < molNo; molI2++)
      if (molI1 < molI2) {
	stat1.avg += rmsdA[molI1 * molNo + molI2];
	stat2.avg += rmsdA[molI2 * molNo + molI1];
      } else if (molI1 > molI2) {
	stat1.avg += rmsdA[molI2 * molNo + molI1];
	stat2.avg += rmsdA[molI1 * molNo + molI2];
      }

    stat1.n = molNo - 1;
    initStatData(&stat1);
    stat2.n = molNo - 1;
    initStatData(&stat2);

    for (molI2 = 0; molI2 < molNo; molI2++) {
      if (molI1 == molI2)
	continue;

      if (molI1 < molI2) {
	rmsd1 = rmsdA[molI1 * molNo + molI2];
	rmsd2 = rmsdA[molI2 * molNo + molI1];
      } else {
	rmsd1 = rmsdA[molI2 * molNo + molI1];
	rmsd2 = rmsdA[molI1 * molNo + molI2];
      }

      addStatData(&stat1, rmsd1);
      addStatData(&stat2, rmsd2);
    }

    if (calc1) {
      Report("structure ");
      writeInt(molI1 + 1);
      Report(" (");
      Report(name1);
      Report("):");
      calcStatData(&stat1);
      writeStatData(&stat1);
    }

    if (calc2) {
      if (calc1) {
	Report("             ");
      } else {
	Report("structure ");
	writeInt(molI1 + 1);
      }
      Report(" (");
      Report(name2);
      Report("):");
      calcStatData(&stat2);
      writeStatData(&stat2);
    }
  }

  if (calc1 && (globDispNo > 0 || locRmsdNo > 0 || locDispNo > 0)) {
    allResData1.rangeExprP = NULL;
    allResData1.resI = 0;
    allResData1.refP = allRefP;
    DhMolApplyRes(allRefP, molPA[0], fillRes, &allResData1);

    Report("\nresidue number\n");

    if (globDispNo > 0) {
      Report("average global displacements: ");

      for (dispI = 0; dispI < globDispNo; dispI++) {
	Report(globDispInfoP[dispI].name);
	if (dispI < globDispNo - 1)
	  Report(", ");
      }

      Report("\n");
    }

    if (locRmsdNo > 0) {
      Report("average local RMSDs (3 residues): ");

      for (rmsdI = 0; rmsdI < locRmsdNo; rmsdI++) {
	Report(locRmsdInfoP[rmsdI].name);
	if (rmsdI < locRmsdNo - 1)
	  Report(", ");
      }

      Report("\n");
    }

    if (locDispNo > 0) {
      Report("average local displacements: ");

      for (dispI = 0; dispI < locDispNo; dispI++) {
	Report(locDispInfoP[dispI].name);
	if (dispI < locDispNo - 1)
	  Report(", ");
      }

      Report("\n");
    }

    if (dispRefP == NULL) {
      Report("(pairwise)\n");
    } else {
      Report("(relative to ");
      Report(dispRef);
      Report(")\n");
    }

    Report("residue name\n");

    if (groupNo == 0) {
      if (dispRefP == NULL) {
	valNo = pairNo;
      } else {
	valNo = 0;
	for (molI1 = 0; molI1 < molNo - 1; molI1++)
	  for (molI2 = molI1 + 1; molI2 < molNo; molI2++)
	    if (DhMolGetProp(dispRefP, molPA[molI1]) !=
		DhMolGetProp(dispRefP, molPA[molI2]))
	      valNo++;
      }
    } else {
      if (dispRefP == NULL) {
	for (groupI1 = 0; groupI1 < groupNo; groupI1++)
	  groupInfoP[groupI1].pairNo = groupInfoP[groupI1].membNo *
	      (groupInfoP[groupI1].membNo - 1) / 2;
      } else {
	for (groupI1 = 0; groupI1 < groupNo; groupI1++)
	  groupInfoP[groupI1].pairNo = 0;

	for (molI1 = 0; molI1 < molNo - 1; molI1++)
	  for (molI2 = molI1 + 1; molI2 < molNo; molI2++) {
	    groupI1 = molInfoP[molI1].groupI;
	    groupI2 = molInfoP[molI2].groupI;
	    if (groupI1 < 0 || groupI1 != groupI2)
	      continue;

	    if (DhMolGetProp(dispRefP, molPA[molI1]) !=
		DhMolGetProp(dispRefP, molPA[molI2]))
	      groupInfoP[groupI1].pairNo++;
	  }
      }
    }

    for (resI = 0; resI < allResNo0; resI++) {
      writeInt(DhResGetNumber(allResData1.resPA[resI]));

      if (groupNo == 0) {
	if (valNo > 0) {
	  for (dispI = 0; dispI < globDispNo; dispI++) {
	    globDispInfoP[dispI].avgA[resI] /= valNo;
	    writeFloat(globDispInfoP[dispI].avgA[resI]);
	  }

	  for (rmsdI = 0; rmsdI < locRmsdNo; rmsdI++) {
	    locRmsdInfoP[rmsdI].avgA[resI] /= valNo;
	    writeFloat(locRmsdInfoP[rmsdI].avgA[resI]);
	  }

	  for (dispI = 0; dispI < locDispNo; dispI++) {
	    locDispInfoP[dispI].avgA[resI] /= valNo;
	    writeFloat(locDispInfoP[dispI].avgA[resI]);
	  }
	}

	Report("   ");
      } else {
	for (groupI1 = 0; groupI1 < groupNo; groupI1++) {
	  valNo = groupInfoP[groupI1].pairNo;
	  if (valNo == 0) {
	    Report(" ");
	    continue;
	  }

	  for (dispI = 0; dispI < globDispNo; dispI++) {
	    globDispInfoP[dispI].avgA[groupNo * resI + groupI1] /= valNo;
	    writeFloat(globDispInfoP[dispI].avgA[groupNo * resI + groupI1]);
	  }

	  for (rmsdI = 0; rmsdI < locRmsdNo; rmsdI++) {
	    locRmsdInfoP[rmsdI].avgA[groupNo * resI + groupI1] /= valNo;
	    writeFloat(locRmsdInfoP[rmsdI].avgA[groupNo * resI + groupI1]);
	  }

	  for (dispI = 0; dispI < locDispNo; dispI++) {
	    locDispInfoP[dispI].avgA[groupNo * resI + groupI1] /= valNo;
	    writeFloat(locDispInfoP[dispI].avgA[groupNo * resI + groupI1]);
	  }

	  Report("   ");
	}
      }

      Report(DStrToStr(DhResGetName(allResData1.resPA[resI])));
      Report("\n");
    }
  }

  if (rangeExprP != NULL)
    ExprFree(rangeExprP);

  for (dispI = 0; dispI < globDispNo; dispI++)
    free(globDispInfoP[dispI].avgA);

  for (rmsdI = 0; rmsdI < locRmsdNo; rmsdI++)
    free(locRmsdInfoP[rmsdI].avgA);

  for (dispI = 0; dispI < locDispNo; dispI++)
    free(locDispInfoP[dispI].avgA);

  free(globDispInfoP);
  free(locRmsdInfoP);
  free(locDispInfoP);
  free(molPA);
  free(groupInfoP);
  free(molInfoP);
  free(atomData1.coordA);
  free(atomData1.resPA);
  free(atomData2.coordA);
  free(atomData2.resPA);
  free(allResData0.resPA);
  free(allResData1.resPA);
  free(allResData2.resPA);
  free(resData0.resPA);
  free(resData1.resPA);
  free(resData2.resPA);
  free(rmsdA);

  DhActivateGroups(FALSE);
}
