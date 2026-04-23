/*
************************************************************************
*
*   ExCheckLimit.c - CheckLol and CheckUpl commands
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdcalc/SCCS/s.ExCheckLimit.c
*   SCCS identification       : 1.10
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
#include <pu.h>
#include <arg.h>
#include <data_hand.h>
#include <data_sel.h>
#include <data_dist.h>

typedef struct {
  int res1I, res2I;
  int atom1I, atom2I;
  float limit;
} LimitData;

static float Cutoff = 0.2f;
static int MinStrucNo = 1;

static PuTextWindow TextW;

static void
getMaxResNum(DhResP resP, void *clientData)
{
  int *maxResNumP = clientData;
  int num;

  num = DhResGetNumber(resP);
  if (num > *maxResNumP)
    *maxResNumP = num;
}

static void
fillList(DhDistP distP, void *clientData)
{
  LINLIST list = clientData;
  DhAtomP atom1P, atom2P;
  LimitData entry;

  atom1P = DhDistGetAtom1(distP);
  atom2P = DhDistGetAtom2(distP);

  entry.res1I = DhResGetNumber(DhAtomGetRes(atom1P));
  entry.res2I = DhResGetNumber(DhAtomGetRes(atom2P));

  entry.atom1I = DhAtomGetNumber(atom1P);
  entry.atom2I = DhAtomGetNumber(atom2P);

  entry.limit = DhDistGetLimit(distP);

  (void) ListInsertLast(list, &entry);
}

static int
compareEntry(void *p1, void *p2)
{
  LimitData *entry1P = p1;
  LimitData *entry2P = p2;

  if (entry1P->res1I < entry2P->res1I)
    return -1;

  if (entry1P->res1I > entry2P->res1I)
    return 1;

  if (entry1P->atom1I < entry2P->atom1I)
    return -1;

  if (entry1P->atom1I > entry2P->atom1I)
    return 1;

  if (entry1P->res2I < entry2P->res2I)
    return -1;

  if (entry1P->res2I > entry2P->res2I)
    return 1;

  if (entry1P->atom2I < entry2P->atom2I)
    return -1;

  if (entry1P->atom2I > entry2P->atom2I)
    return 1;

  return 0;
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

static BOOL
checkLimit(DhMolP *molPA, int molNo, DhDistKind distSel)
{
  char *formMol, *formRes, buf[20];
  PropRefP refP;
  DistList tab;
  LINLIST list;
  int maxResNum;
  LimitData *entryP;
  int molI, valNo;
  DSTR str;
  float *valA, minVal, maxVal;
  DhResP res1P, res2P;
  DhAtomP atom1P, atom2P;
  Vec3 x1, x2;
  char ch;

  PuWriteStr(TextW, "\n");
  formMol = getForm(molNo);
  for (molI = 0; molI < molNo; molI++) {
    (void) sprintf(buf, formMol, molI + 1);
    PuWriteStr(TextW, buf);
    PuWriteStr(TextW, " ");
    PuWriteStr(TextW, DStrToStr(DhMolGetName(molPA[molI])));
    PuWriteStr(TextW, "\n");
  }
  PuWriteStr(TextW, "\n");

  refP = PropGetRef(PROP_SELECTED, FALSE);

  list = ListOpen(sizeof(LimitData));

  tab = DistListBuildIntra(refP, molPA, molNo, distSel);
  DistListApply(tab, fillList, list);
  DistListFree(tab);

  ListSort(list, compareEntry);

  maxResNum = 0;
  DhMolApplyRes(refP, molPA[0], getMaxResNum, &maxResNum);
  formRes = getForm(maxResNum);

  valA = malloc(molNo * sizeof(*valA));
  str = DStrNew();

  entryP = ListFirst(list);
  while (entryP != NULL) {
    if (BreakCheck(100))
      break;

    minVal = MAXFLOAT;
    maxVal = 0.0f;
    valNo = 0;

    for (molI = 0; molI < molNo; molI++) {
      res1P = DhResFind(molPA[molI], entryP->res1I);
      res2P = DhResFind(molPA[molI], entryP->res2I);

      atom1P = DhAtomFindNumber(res1P, entryP->atom1I, FALSE);
      atom2P = DhAtomFindNumber(res2P, entryP->atom2I, FALSE);

      if (atom1P == NULL || atom2P == NULL) {
	valA[molI] = 0.0f;
      } else {
	DhAtomGetCoord(atom1P, x1);
	DhAtomGetCoord(atom2P, x2);

	if (distSel == DK_UPPER)
	  valA[molI] = Vec3DiffAbs(x1, x2) - entryP->limit;
	else
	  valA[molI] = entryP->limit - Vec3DiffAbs(x1, x2);

	if (valA[molI] < 0.0f)
	  valA[molI] = 0.0f;
      }

      if (valA[molI] > Cutoff)
	valNo++;

      if (valA[molI] < minVal)
	minVal = valA[molI];
      if (valA[molI] > maxVal)
	maxVal = valA[molI];
    }

    if (valNo >= MinStrucNo) {
      (void) sprintf(buf, formRes, entryP->res1I);
      DStrAssignStr(str, buf);
      (void) sprintf(buf, " %-5s%-5s",
	  DStrToStr(DhResGetName(res1P)), DStrToStr(DhAtomGetName(atom1P)));
      DStrAppStr(str, buf);
      (void) sprintf(buf, formRes, entryP->res2I);
      DStrAppStr(str, buf);
      (void) sprintf(buf, " %-5s%-5s",
	  DStrToStr(DhResGetName(res2P)), DStrToStr(DhAtomGetName(atom2P)));
      DStrAppStr(str, buf);
      (void) sprintf(buf, formMol, valNo);
      DStrAppStr(str, buf);

      DStrAppStr(str, " (");
      (void) sprintf(buf, "%4.2f", minVal);
      DStrAppStr(str, buf);
      DStrAppStr(str, "..");
      (void) sprintf(buf, "%4.2f", maxVal);
      DStrAppStr(str, buf);
      DStrAppStr(str, ") ");

      for (molI = 0; molI < molNo; molI++) {
	if (valA[molI] <= Cutoff)
	  ch = ' ';
	else if (valA[molI] == maxVal)
	  ch = '+';
	else if ((molI + 1) % 10 == 0)
	  ch = '0';
	else if ((molI + 1) % 5 == 0)
	  ch = '5';
	else
	  ch = '*';

	DStrAppChar(str, ch);
      }

      DStrAppChar(str, '\n');
      PuWriteStr(TextW, DStrToStr(str));
    }

    entryP = ListNext(list, entryP);
  }

  free(valA);
  DStrFree(str);

  ListClose(list);

  return TRUE;
}

#define ARG_NUM 2

ErrCode
ExCheckLimit(char *cmd)
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

  DhActivateGroups(TRUE);

  totMolNo = SelMolGet(NULL, 0);
  if (totMolNo == 0) {
    DhActivateGroups(FALSE);
    CipSetError("at least one molecule must be selected");
    return EC_ERROR;
  }

  BreakActivate(TRUE);

  TextW = PuCreateTextWindow(cmd);

  PuWriteStr(TextW, "Constraint violations > ");
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
      if (BreakCheck(10))
	break;

      if (molPA[i] == NULL)
        continue;

      if (DhMolEqualStruc(molPA[molI], molPA[i])) {
        eqMolPA[molNo++] = molPA[i];
        molPA[i] = NULL;
      }
    }

    if (BreakInterrupted())
      break;

    molPA[molI] = NULL;

    if (strcmp(cmd, "CheckUpl") == 0)
      ok = checkLimit(eqMolPA, molNo, DK_UPPER);
    else
      ok = checkLimit(eqMolPA, molNo, DK_LOWER);

    if (! ok)
      break;
  }

  BreakActivate(FALSE);

  free(molPA);
  free(eqMolPA);

  DhActivateGroups(FALSE);

  if (ok)
    return EC_OK;
  else
    return EC_ERROR;
}
