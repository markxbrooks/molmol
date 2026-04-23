/*
************************************************************************
*
*   CalcPair.c - find close distances of atom pairs
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/calc/SCCS/s.CalcPair.c
*   SCCS identification       : 1.18
*
************************************************************************
*/

#include <calc_pair.h>

#include <stdio.h>
#include <stdlib.h>
#include <values.h>

#include <break.h>
#include <linlist.h>
#include <grid.h>
#include <data_hand.h>
#include <data_sel.h>

#define NO_LIMIT -1.0f

typedef struct {
  int res1I, res2I;
  int atom1I, atom2I;
  DhAtomP atom1P, atom2P;
  float *valP;
  int validNo;
  BOOL *validP;
  float *limitP;
} PairData;

static PropRefP Prop1RefP, Prop2RefP, AllRefP;
static float MaxDist, MaxDistS;
static BOOL Symm, InterMol;
static PairExclude ExclSel;
static int MinStrucNo;
static CalcPrepareF Prepare;
static CalcCheckAtomF CheckAtom1;
static CalcCheckAtomF CheckAtom2;
static CalcCheckPairF CheckPair;
static int ValNo;
static float *ValA, *MaxValA;
static CalcGetLimitsF GetLimits;
static int LimitNo;
static float *MaxLimitA;
static int MaxResI;
static CalcListPairF ListPair;
static CalcReportF Report;
static GRID Grid;
static int MolNo, MolI;
static LINLIST PairList;
static PairData *CurrPairP, *FirstPairP;
static DhAtomP CurrAtom1P;
static int ExclSize, ExclNo;
static DhAtomP *ExclA;

static void
freePair(void *p, void *clientData)
{
  PairData *pairP = p;

  free(pairP->valP);
  free(pairP->validP);
  free(pairP->limitP);
}

static void
prepareAtom(DhAtomP atomP, void *clientData)
{
  Vec3 x;

  if (BreakCheck(10000))
    return;

  if (! CheckAtom2(atomP))
    return;

  if (InterMol)
    DhAtomGetCoordTransf(atomP, x);
  else
    DhAtomGetCoord(atomP, x);

  GridPrepareAddEntry(Grid, x);
}

static void
addAtom(DhAtomP atomP, void *clientData)
{
  Vec3 x;

  if (BreakCheck(10000))
    return;

  if (! CheckAtom2(atomP))
    return;

  if (InterMol)
    DhAtomGetCoordTransf(atomP, x);
  else
    DhAtomGetCoord(atomP, x);

  GridInsertEntry(Grid, x, atomP);
}

static void
reallocExcl(void)
{
  if (ExclNo == ExclSize) {
    ExclSize *= 2;
    ExclA = realloc(ExclA, ExclSize * sizeof(*ExclA));
  }
}

static void
fillIndirNeigh(DhBondP bondP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  BOOL in1, in2;
  int i;

  if (CurrAtom1P == atom1P || CurrAtom1P == atom2P)
    return;

  in1 = FALSE;
  in2 = FALSE;
  for (i = 0; i < ExclNo; i++) {
    if (ExclA[i] == atom1P)
      in1 = TRUE;
    if (ExclA[i] == atom2P)
      in2 = TRUE;
  }

  if (in1 && in2)
    return;

  reallocExcl();

  if (in1)
    ExclA[ExclNo++] = atom2P;
  else
    ExclA[ExclNo++] = atom1P;
}

static void
fillNeigh(DhBondP bondP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  reallocExcl();

  if (CurrAtom1P == atom1P) {
    ExclA[ExclNo++] = atom2P;
    if (ExclSel == PE_2_BOND)
      DhAtomApplyBond(AllRefP, atom2P, fillIndirNeigh, NULL);
  } else {
    ExclA[ExclNo++] = atom1P;
    if (ExclSel == PE_2_BOND)
      DhAtomApplyBond(AllRefP, atom1P, fillIndirNeigh, NULL);
  }
}

static BOOL
eqAtom1(PairData *pair1P, PairData *pair2P)
{
  return pair1P->res1I == pair2P->res1I &&
      pair1P->atom1I == pair2P->atom1I;
}

static BOOL
eqAtom2(PairData *pair1P, PairData *pair2P)
{
  return pair1P->res2I == pair2P->res2I &&
      pair1P->atom2I == pair2P->atom2I;
}

static void
fillLimits(PairData *pairP)
{
  int i;

  if (LimitNo > 0) {
    pairP->limitP = malloc(LimitNo * sizeof(*pairP->limitP));
    if (GetLimits(pairP->atom1P, pairP->atom2P, pairP->limitP)) {
      for (i = 0; i < LimitNo; i++)
	if (pairP->limitP[i] > MaxLimitA[i])
	  MaxLimitA[i] = pairP->limitP[i];
    } else {
      for (i = 0; i < LimitNo; i++)
	pairP->limitP[i] = NO_LIMIT;
    }
  } else {
    pairP->limitP = NULL;
  }
}

static BOOL
handleIntraPair(void *entryP, float *x1, void *clientData)
{
  DhAtomP atom2P = entryP;
  Vec3 dx;
  float dxS;
  DhResP res1P, res2P;
  PairData pair, *newPairP;
  BOOL found;
  int i;

  if (atom2P == CurrAtom1P)
    return TRUE;

  if (BreakCheck(1000))
    return FALSE;

  res1P = DhAtomGetRes(CurrAtom1P);
  res2P = DhAtomGetRes(atom2P);

  pair.res1I = DhResGetNumber(res1P);
  pair.res2I = DhResGetNumber(res2P);

  if (Symm && pair.res1I > pair.res2I)
    return TRUE;

  pair.atom1I = DhAtomGetNumber(CurrAtom1P);
  pair.atom2I = DhAtomGetNumber(atom2P);

  if (Symm && pair.res1I == pair.res2I && pair.atom1I > pair.atom2I)
    return TRUE;

  for (i = 0; i < ExclNo; i++)
    if (ExclA[i] == atom2P)
      return TRUE;

  DhAtomGetCoord(atom2P, dx);
  Vec3Sub(dx, x1);
  dxS = dx[0] * dx[0] + dx[1] * dx[1] + dx[2] * dx[2];
  if (dxS > MaxDistS)
    return TRUE;

  if (! CheckPair(CurrAtom1P, atom2P, dxS, ValA))
    return TRUE;

  found = FALSE;
  if (FirstPairP == NULL) {
    while (CurrPairP != NULL && CurrPairP->res1I < pair.res1I)
      CurrPairP = ListNext(PairList, CurrPairP);

    if (CurrPairP != NULL && CurrPairP->res1I == pair.res1I) {
      while (CurrPairP != NULL &&
	  CurrPairP->res1I == pair.res1I && CurrPairP->atom1I < pair.atom1I)
	CurrPairP = ListNext(PairList, CurrPairP);

      if (CurrPairP != NULL && eqAtom1(CurrPairP, &pair)) {
	FirstPairP = CurrPairP;
	found = TRUE;
      }
    }
  } else {
    CurrPairP = FirstPairP;
    found = TRUE;
  }

  if (found) {
    found = FALSE;

    while (CurrPairP != NULL && eqAtom1(CurrPairP, &pair) &&
	CurrPairP->res2I < pair.res2I)
      CurrPairP = ListNext(PairList, CurrPairP);

    if (CurrPairP != NULL && eqAtom1(CurrPairP, &pair) &&
	CurrPairP->res2I == pair.res2I) {
      while (CurrPairP != NULL && eqAtom1(CurrPairP, &pair) &&
	  CurrPairP->res2I == pair.res2I && CurrPairP->atom2I < pair.atom2I)
	CurrPairP = ListNext(PairList, CurrPairP);

      if (CurrPairP != NULL &&
	  eqAtom1(CurrPairP, &pair) && eqAtom2(CurrPairP, &pair))
	found = TRUE;
    }
  }

  if (! found) {
    if (ValNo > 0)
      pair.valP = malloc(MolNo * ValNo * sizeof(*pair.valP));
    else
      pair.valP = NULL;

    pair.validNo = 0;
    pair.validP = malloc(MolNo * sizeof(*pair.validP));
    for (i = 0; i < MolNo; i++)
      pair.validP[i] = FALSE;

    pair.atom1P = CurrAtom1P;
    pair.atom2P = atom2P;

    fillLimits(&pair);

    if (pair.res1I > MaxResI)
      MaxResI = pair.res1I;

    if (pair.res2I > MaxResI)
      MaxResI = pair.res2I;

    if (CurrPairP == NULL)
      newPairP = ListInsertLast(PairList, &pair);
    else
      newPairP = ListInsertBefore(PairList, CurrPairP, &pair);

    if (FirstPairP == NULL || FirstPairP == CurrPairP)
      FirstPairP = newPairP;

    CurrPairP = newPairP;
  }

  for (i = 0; i < ValNo; i++) {
    CurrPairP->valP[MolI * ValNo + i] = ValA[i];
    if (ValA[i] > MaxValA[i])
      MaxValA[i] = ValA[i];
  }

  CurrPairP->validNo++;
  CurrPairP->validP[MolI] = TRUE;

  return TRUE;
}

static BOOL
handleInterPair(void *entryP, float *x1, void *clientData)
{
  DhAtomP atom2P = entryP;
  Vec3 dx;
  float dxS;
  DhResP res1P, res2P;
  PairData pair;
  int i;

  if (BreakCheck(1000))
    return FALSE;

  DhAtomGetCoordTransf(atom2P, dx);
  Vec3Sub(dx, x1);
  dxS = dx[0] * dx[0] + dx[1] * dx[1] + dx[2] * dx[2];
  if (dxS > MaxDistS)
    return TRUE;

  res1P = DhAtomGetRes(CurrAtom1P);
  res2P = DhAtomGetRes(atom2P);

  pair.res1I = DhResGetNumber(res1P);
  pair.res2I = DhResGetNumber(res2P);

  pair.atom1I = DhAtomGetNumber(CurrAtom1P);
  pair.atom2I = DhAtomGetNumber(atom2P);

  if (! CheckPair(CurrAtom1P, atom2P, dxS, ValA))
    return TRUE;

  if (ValNo > 0)
    pair.valP = malloc(ValNo * sizeof(*pair.valP));
  else
    pair.valP = NULL;

  pair.validP = malloc(sizeof(*pair.validP));

  for (i = 0; i < ValNo; i++) {
    pair.valP[i] = ValA[i];
    if (ValA[i] > MaxValA[i])
      MaxValA[i] = ValA[i];
  }

  pair.validNo = 1;
  pair.validP[0] = TRUE;

  pair.atom1P = CurrAtom1P;
  pair.atom2P = atom2P;

  fillLimits(&pair);

  if (pair.res1I > MaxResI)
    MaxResI = pair.res1I;

  if (pair.res2I > MaxResI)
    MaxResI = pair.res2I;

  (void) ListInsertLast(PairList, &pair);

  return TRUE;
}

static void
handleIntraAtom(DhAtomP atomP, void *clientData)
{
  Vec3 x;

  if (BreakInterrupted())
    return;

  if (! CheckAtom1(atomP))
    return;

  CurrAtom1P = atomP;
  FirstPairP = NULL;

  ExclNo = 0;
  if (ExclSel == PE_BOND || ExclSel == PE_2_BOND)
    DhAtomApplyBond(AllRefP, atomP, fillNeigh, NULL);

  DhAtomGetCoord(atomP, x);
  GridFind(Grid, x, 1, handleIntraPair, NULL);
}

static void
handleInterAtom(DhAtomP atomP, void *clientData)
{
  Vec3 x;

  if (BreakInterrupted())
    return;

  if (! CheckAtom1(atomP))
    return;

  CurrAtom1P = atomP;

  DhAtomGetCoordTransf(atomP, x);
  GridFind(Grid, x, 1, handleInterPair, NULL);
}

static char *
getFormInt(int maxVal)
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

static char *
getFormFloat(float maxVal)
{
  if (maxVal >= 100.0f)
    return "%4.0f";

  if (maxVal >= 10.0f)
    return "%4.1f";

  return "%4.2f";
}

static void
writeMolList(DhMolP *molPA, int molNo)
{
  char *form, buf[20];
  int num, i;

  form = getFormInt(molNo);

  Report("\n");
  for (i = 0; i < molNo; i++) {
    if (InterMol)
      num = DhMolGetNumber(molPA[i]) + 1;
    else
      num = i + 1;
    (void) sprintf(buf, form, num);
    Report(buf);
    Report(" ");
    Report(DStrToStr(DhMolGetName(molPA[i])));
    Report("\n");
  }
  Report("\n");
}

static int
compareAtom(DhAtomP atom1P, DhAtomP atom2P)
{
  DhResP res1P, res2P;
  int mol1I, mol2I;
  int res1I, res2I;
  int atom1I, atom2I;

  res1P = DhAtomGetRes(atom1P);
  res2P = DhAtomGetRes(atom2P);

  mol1I = DhMolGetNumber(DhResGetMol(res1P));
  mol2I = DhMolGetNumber(DhResGetMol(res2P));

  if (mol1I < mol2I)
    return -1;

  if (mol1I > mol2I)
    return 1;

  res1I = DhResGetNumber(res1P);
  res2I = DhResGetNumber(res2P);

  if (res1I < res2I)
    return -1;

  if (res1I > res2I)
    return 1;

  atom1I = DhAtomGetNumber(atom1P);
  atom2I = DhAtomGetNumber(atom2P);

  if (atom1I < atom2I)
    return -1;

  if (atom1I > atom2I)
    return 1;
  
  return 0;
}

static int
comparePairInter(void *p1, void *p2)
{
  PairData *pair1P = p1;
  PairData *pair2P = p2;
  int res;

  res = compareAtom(pair1P->atom1P, pair2P->atom1P);
  if (res == 0)
    return compareAtom(pair1P->atom2P, pair2P->atom2P);
  else
    return res;
}

void
writeIntra(void)
{
  char *formMol, *formRes, *formVal;
  DSTR str;
  PairData *pairP;
  char buf[40], ch;
  float val, minVal, maxVal;
  int valI, i;

  formMol = getFormInt(MolNo);
  formRes = getFormInt(MaxResI);

  str = DStrNew();

  pairP = ListFirst(PairList);
  while (pairP != NULL) {
    if (BreakCheck(100))
      break;

    if (pairP->validNo < MinStrucNo) {
      pairP = ListNext(PairList, pairP);
      continue;
    }

    (void) sprintf(buf, formRes, pairP->res1I);
    DStrAssignStr(str, buf);
    (void) sprintf(buf, " %-5s%-5s",
	DStrToStr(DhResGetName(DhAtomGetRes(pairP->atom1P))),
	DStrToStr(DhAtomGetName(pairP->atom1P)));
    DStrAppStr(str, buf);
    (void) sprintf(buf, formRes, pairP->res2I);
    DStrAppStr(str, buf);
    (void) sprintf(buf, " %-5s%-5s",
	DStrToStr(DhResGetName(DhAtomGetRes(pairP->atom2P))),
	DStrToStr(DhAtomGetName(pairP->atom2P)));
    DStrAppStr(str, buf);
    (void) sprintf(buf, formMol, pairP->validNo);
    DStrAppStr(str, buf);

    for (valI = 0; valI < ValNo; valI++) {
      minVal = MAXFLOAT;
      maxVal = -MAXFLOAT;
      for (i = 0; i < MolNo; i++) {
	if (! pairP->validP[i])
	  continue;

	val = pairP->valP[i * ValNo + valI];
	if (val < minVal)
	  minVal = val;
	if (val > maxVal)
	  maxVal = val;
      }

      formVal = getFormFloat(MaxValA[valI]);
      DStrAppStr(str, " (");
      (void) sprintf(buf, formVal, minVal);
      DStrAppStr(str, buf);
      DStrAppStr(str, "..");
      (void) sprintf(buf, formVal, maxVal);
      DStrAppStr(str, buf);
      DStrAppStr(str, ")");
    }

    for (valI = 0; valI < LimitNo; valI++) {
      if (MaxLimitA[valI] == NO_LIMIT) {
	/* do not print empty column */
      } else if (pairP->limitP[valI] == NO_LIMIT) {
	DStrAppStr(str, "     ");
      } else {
	DStrAppStr(str, " ");
	(void) sprintf(buf, getFormFloat(MaxLimitA[valI]),
	    pairP->limitP[valI]);
	DStrAppStr(str, buf);
      }
    }

    DStrAppStr(str, " ");

    for (i = 0; i < MolNo; i++) {
      if (! pairP->validP[i])
	ch = ' ';
      else if (ValNo == 1 && pairP->valP[i] == maxVal)
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
    Report(DStrToStr(str));

    pairP = ListNext(PairList, pairP);
  }

  DStrFree(str);
}

void
writeInter(void)
{
  char *formMol, *formRes;
  DSTR str;
  PairData *pairP;
  DhResP res1P, res2P;
  char buf[40];
  float valSum;
  int valI;

  formMol = getFormInt(MolNo);
  formRes = getFormInt(MaxResI);

  str = DStrNew();

  pairP = ListFirst(PairList);
  while (pairP != NULL) {
    if (BreakCheck(100))
      break;

    res1P = DhAtomGetRes(pairP->atom1P);
    res2P = DhAtomGetRes(pairP->atom2P);

    (void) sprintf(buf, formMol, DhMolGetNumber(DhResGetMol(res1P)) + 1);
    DStrAssignStr(str, buf);
    DStrAppStr(str, " ");
    (void) sprintf(buf, formRes, pairP->res1I);
    DStrAppStr(str, buf);
    (void) sprintf(buf, " %-5s%-5s",
	DStrToStr(DhResGetName(res1P)),
	DStrToStr(DhAtomGetName(pairP->atom1P)));
    DStrAppStr(str, buf);
    (void) sprintf(buf, formMol, DhMolGetNumber(DhResGetMol(res2P)) + 1);
    DStrAppStr(str, buf);
    DStrAppStr(str, " ");
    (void) sprintf(buf, formRes, pairP->res2I);
    DStrAppStr(str, buf);
    (void) sprintf(buf, " %-5s%-5s",
	DStrToStr(DhResGetName(res2P)),
	DStrToStr(DhAtomGetName(pairP->atom2P)));
    DStrAppStr(str, buf);

    for (valI = 0; valI < ValNo; valI++) {
      DStrAppStr(str, " ");
      (void) sprintf(buf, getFormFloat(MaxValA[valI]),
	  pairP->valP[valI]);
      DStrAppStr(str, buf);
    }

    for (valI = 0; valI < LimitNo; valI++) {
      if (MaxLimitA[valI] == NO_LIMIT) {
	/* do not print empty column */
      } else if (pairP->limitP[valI] == NO_LIMIT) {
	DStrAppStr(str, "     ");
      } else {
	DStrAppStr(str, " ");
	(void) sprintf(buf, getFormFloat(MaxLimitA[valI]),
	    pairP->limitP[valI]);
	DStrAppStr(str, buf);
      }
    }

    DStrAppChar(str, '\n');
    Report(DStrToStr(str));

    pairP = ListNext(PairList, pairP);
  }

  DStrAssignStr(str, "\nsum:");
  for (valI = 0; valI < ValNo; valI++) {
    valSum = 0.0f;
    pairP = ListFirst(PairList);
    while (pairP != NULL) {
      valSum += pairP->valP[valI];
      pairP = ListNext(PairList, pairP);
    }
    DStrAppStr(str, " ");
    (void) sprintf(buf, getFormFloat(valSum), valSum);
    DStrAppStr(str, buf);
  }
  DStrAppChar(str, '\n');
  Report(DStrToStr(str));

  DStrFree(str);
}

static void
calcPairIntra(DhMolP molPA[], int molNo)
{
  BOOL *molDoneA;
  DhMolP *eqMolPA;
  PairData *pairP;
  int molI, i;

  molDoneA = malloc(molNo * sizeof(*molDoneA));
  for (molI = 0; molI < molNo; molI++)
    molDoneA[molI] = FALSE;

  eqMolPA = malloc(molNo * sizeof(*eqMolPA));

  for (molI = 0; molI < molNo; molI++) {
    if (molDoneA[molI])
      continue;

    MolNo = 1;
    PairList = ListOpen(sizeof(PairData));
    ListAddDestroyCB(PairList, freePair, NULL, NULL);

    molDoneA[molI] = TRUE;
    eqMolPA[0] = molPA[molI];

    for (i = molI + 1; i < molNo; i++) {
      if (molDoneA[i])
	continue;

      if (! DhMolEqualStruc(molPA[molI], molPA[i]))
	continue;

      molDoneA[i] = TRUE;
      eqMolPA[MolNo++] = molPA[i];
    }

    if (Prepare != NULL)
      Prepare(eqMolPA, MolNo);

    if (Report != NULL)
      writeMolList(eqMolPA, MolNo);

    for (MolI = 0; MolI < MolNo; MolI++) {
      CurrPairP = ListFirst(PairList);

      Grid = GridNew();
      DhMolApplyAtom(Prop2RefP, eqMolPA[MolI], prepareAtom, NULL);
      GridInsertInit(Grid, MaxDist);
      DhMolApplyAtom(Prop2RefP, eqMolPA[MolI], addAtom, NULL);
      DhMolApplyAtom(Prop1RefP, eqMolPA[MolI], handleIntraAtom, NULL);
      GridDestroy(Grid);
    }

    if (ListPair != NULL) {
      pairP = ListFirst(PairList);
      while (pairP != NULL) {
	if (pairP->validNo >= MinStrucNo)
	  ListPair(pairP->atom1P, pairP->atom2P);
	pairP = ListNext(PairList, pairP);
      }
    }

    if (Report != NULL)
      writeIntra();


    ListClose(PairList);
  }

  free(molDoneA);
  free(eqMolPA);
}

static void
calcPairInter(DhMolP molPA[], int molNo)
{
  int startI, mol1I, mol2I;
  
  if (Prepare != NULL)
    Prepare(molPA, molNo);

  if (Report != NULL)
    writeMolList(molPA, molNo);

  PairList = ListOpen(sizeof(PairData));
  ListAddDestroyCB(PairList, freePair, NULL, NULL);

  if (Symm)
    startI = 1;
  else
    startI = 0;

  for (mol2I = startI; mol2I < molNo; mol2I++) {
    Grid = GridNew();
    DhMolApplyAtom(Prop2RefP, molPA[mol2I], prepareAtom, NULL);
    GridInsertInit(Grid, MaxDist);
    DhMolApplyAtom(Prop2RefP, molPA[mol2I], addAtom, NULL);

    for (mol1I = 0; mol1I < mol2I; mol1I++)
      DhMolApplyAtom(Prop1RefP, molPA[mol1I], handleInterAtom, NULL);

    if (! Symm)
      for (mol1I = mol2I + 1; mol1I < molNo; mol1I++)
	DhMolApplyAtom(Prop1RefP, molPA[mol1I], handleInterAtom, NULL);

    GridDestroy(Grid);
  }

  if (Report != NULL) {
    ListSort(PairList, comparePairInter);
    MolNo = molNo;
    writeInter();
  }

  ListClose(PairList);
}

void
CalcPair(float dist, BOOL symm, BOOL interMol,
    PairExclude excl, int minStrucNo,
    CalcPrepareF prepare,
    char *prop1, char *prop2,
    CalcCheckAtomF checkAtom1, CalcCheckAtomF checkAtom2,
    CalcCheckPairF checkPair, int valNo,
    CalcGetLimitsF getLimits, int limitNo,
    CalcListPairF listPair, CalcReportF report)
{
  int molNo;
  DhMolP *molPA;
  int i;

  if (dist <= 0.0f)
    return;

  DhActivateGroups(TRUE);

  molNo = SelMolGet(NULL, 0);
  if (molNo < 1) {
    DhActivateGroups(FALSE);
    return;
  }

  Prop1RefP = PropGetRef(prop1, FALSE);
  Prop2RefP = PropGetRef(prop2, FALSE);
  AllRefP = PropGetRef(PROP_ALL, FALSE);

  MaxDist = dist;
  MaxDistS = dist * dist;
  Symm = symm && Prop1RefP == Prop2RefP;
  InterMol = interMol;
  ExclSel = excl;
  MinStrucNo = minStrucNo;
  Prepare = prepare;
  CheckAtom1 = checkAtom1;
  CheckAtom2 = checkAtom2;
  CheckPair = checkPair;
  ValNo = valNo;
  GetLimits = getLimits;
  LimitNo = limitNo;
  ListPair = listPair;
  Report = report;

  if (ValNo > 0) {
    ValA = malloc(ValNo * sizeof(*ValA));
    MaxValA = malloc(ValNo * sizeof(*MaxValA));
    for (i = 0; i < ValNo; i++)
      MaxValA[i] = 0.0f;
  } else {
    ValA = NULL;
    MaxValA = NULL;
  }

  if (LimitNo > 0) {
    MaxLimitA = malloc(LimitNo * sizeof(*MaxLimitA));
    for (i = 0; i < LimitNo; i++)
      MaxLimitA[i] = NO_LIMIT;
  } else {
    MaxLimitA = NULL;
  }

  MaxResI = 0;

  ExclSize = 1;
  ExclA = malloc(ExclSize * sizeof(*ExclA));

  molPA = malloc(molNo * sizeof(*molPA));
  (void) SelMolGet(molPA, molNo);

  if (InterMol)
    calcPairInter(molPA, molNo);
  else
    calcPairIntra(molPA, molNo);

  free(ValA);
  free(MaxValA);
  free(MaxLimitA);
  free(ExclA);
  free(molPA);

  DhActivateGroups(FALSE);
}
