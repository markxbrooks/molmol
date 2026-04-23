/*
************************************************************************
*
*   ExCalcDist.c - CalcDist command
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
*   Date of last modification : 01/05/27
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdcalc/SCCS/s.ExCalcDist.c
*   SCCS identification       : 1.12
*
************************************************************************
*/

#include <cmd_calc.h>

#include <stdio.h>
#include <math.h>

#include <break.h>
#include <pu.h>
#include <arg.h>
#include <data_hand.h>
#include <data_dist.h>
#include <calc_pair.h>
#include <graph_draw.h>

static float MaxDist = 5.0f;
static int ResDiff = 1;
static int MinStrucNo = 1;
static BOOL InterMol = FALSE;
static BOOL DoReport = TRUE;
static BOOL DoPeak = FALSE;
static BOOL DoInsert = FALSE;

static PuTextWindow TextW, PeakW;
static int PeakI;
static DistList DList = NULL, LList = NULL;

static void
makeDistList(DhMolP *molPA, int molNo)
{
  PropRefP refP;

  if (DList != NULL)
    DistListFree(DList);

  if (LList != NULL)
    DistListFree(LList);
  
  refP = PropGetRef(PROP_ALL, FALSE);

  if (InterMol) {
    DList = DistListBuildInter(refP, DK_UPPER);
    LList = NULL;
  } else {
    DList = DistListBuildInter(refP, DK_UPPER);
    LList = DistListBuildIntra(refP, molPA, molNo, DK_UPPER);
  }
}

static BOOL
checkAtom(DhAtomP atomP)
{
  return TRUE;
}

static BOOL
checkPair(DhAtomP atom1P, DhAtomP atom2P, float distS, float *valP)
{
  int resNum1, resNum2;
  DhDistP distP;

  if (! InterMol) {
    resNum1 = DhResGetNumber(DhAtomGetRes(atom1P));
    resNum2 = DhResGetNumber(DhAtomGetRes(atom2P));
    if (resNum2 - resNum1 < ResDiff && resNum1 - resNum2 < ResDiff)
      return FALSE;
  }

  *valP = sqrtf(distS);

  if (DoInsert) {
    distP = DistListFind(DList, atom1P, atom2P);
    if (distP != NULL)
      return TRUE;

    distP = DhDistNew(atom1P, atom2P);
    if (distP == NULL)
      return TRUE;

    DhDistSetKind(distP, DK_UPPER);
    DhDistSetLimit(distP, MaxDist);
    DhDistInit(distP);
  }

  return TRUE;
}

static BOOL
getLimit(DhAtomP atom1P, DhAtomP atom2P, float *limitP)
{
  DhDistP distP;

  if (InterMol)
    distP = DistListFind(DList, atom1P, atom2P);
  else
    distP = DistListFind(LList, atom1P, atom2P);

  if (distP == NULL)
    return FALSE;

  *limitP = DhDistGetLimit(distP);

  return TRUE;
}

static void
writeStr(char *str)
{
  PuWriteStr(TextW, str);
}

static void
writePeak(DhAtomP atom1P, DhAtomP atom2P)
{
  float f1, f2, ft;
  int i1, i2, it;
  char line[100];

  f1 = DhAtomGetShift(atom1P);
  f2 = DhAtomGetShift(atom2P);

  if (f1 == DH_SHIFT_UNKNOWN || f2 == DH_SHIFT_UNKNOWN)
    return;

  i1 = DhAtomGetShiftIndex(atom1P);
  i2 = DhAtomGetShiftIndex(atom2P);

  if (f1 < f2) {
    /* produce peaks on left side of diagonal */
    ft = f1;
    f1 = f2;
    f2 = ft;

    it = i1;
    i1 = i2;
    i2 = it;
  }

  PeakI++;

  (void) sprintf(line,
      "%4d %7.3f %7.3f %1d %-9s %10.3e %9.2e %c %3d %4d %4d  0\n",
      PeakI, f1, f2, 1, "N", 0.0, 0.0, '-', 0, i1, i2);
  PuWriteStr(PeakW, line);
}

#define ARG_NUM 6
#define OPT_NUM 4

ErrCode
ExCalcDist(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  EnumEntryDescr enumEntry[OPT_NUM];
  ErrCode errCode;
  DSTR title;
  char buf[20];

  arg[0].type = AT_DOUBLE;
  arg[1].type = AT_INT;
  arg[2].type = AT_INT;
  arg[3].type = AT_MULT_ENUM;
  arg[4].type = AT_STR;
  arg[5].type = AT_STR;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Max. Dist";
  arg[0].v.doubleVal = MaxDist;

  arg[1].prompt = "Res Diff.";
  arg[1].v.intVal = ResDiff;

  arg[2].prompt = "Min. Struc. #";
  arg[2].v.intVal = MinStrucNo;

  enumEntry[0].str = "intermol";
  enumEntry[0].onOff = InterMol;
  enumEntry[1].str = "report";
  enumEntry[1].onOff = DoReport;
  enumEntry[2].str = "peak_list";
  enumEntry[2].onOff = DoPeak;
  enumEntry[3].str = "display";
  enumEntry[3].onOff = DoInsert;

  arg[3].prompt = "Options";
  arg[3].u.enumD.entryP = enumEntry;
  arg[3].u.enumD.n = OPT_NUM;

  arg[4].prompt = "Property 1";
  arg[4].optional = TRUE;
  DStrAssignStr(arg[4].v.strVal, PROP_SELECTED);

  arg[5].prompt = "Property 2";
  arg[5].optional = TRUE;
  DStrAssignStr(arg[5].v.strVal, PROP_SELECTED);

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  MaxDist = (float) arg[0].v.doubleVal;
  ResDiff = arg[1].v.intVal;
  MinStrucNo = arg[2].v.intVal;
  InterMol = enumEntry[0].onOff;
  DoReport = enumEntry[1].onOff;
  DoPeak = enumEntry[2].onOff;
  DoInsert = enumEntry[3].onOff;

  BreakActivate(TRUE);

  if (DoReport) {
    TextW = PuCreateTextWindow(cmd);

    PuWriteStr(TextW, "Distances < ");
    (void) sprintf(buf, "%.2f", MaxDist);
    PuWriteStr(TextW, buf);
    PuWriteStr(TextW, " (residue difference >= ");
    (void) sprintf(buf, "%d", ResDiff);
    PuWriteStr(TextW, buf);
    PuWriteStr(TextW, ")\nin at least ");
    (void) sprintf(buf, "%d", MinStrucNo);
    PuWriteStr(TextW, buf);
    PuWriteStr(TextW, " structures:\n");
  }

  if (DoPeak) {
    title = DStrNew();
    DStrAssignStr(title, cmd);
    DStrAppStr(title, ".peaks");
    PeakW = PuCreateTextWindow(DStrToStr(title));
    DStrFree(title);
    PuWriteStr(PeakW, "# Number of dimensions 2\n");
    PeakI = 0;
  }

  CalcPair(MaxDist, TRUE, InterMol, PE_2_BOND, MinStrucNo, makeDistList,
      DStrToStr(arg[4].v.strVal), DStrToStr(arg[5].v.strVal),
      checkAtom, checkAtom, checkPair, 1,
      getLimit, 1,
      DoPeak ? writePeak : NULL, DoReport ? writeStr : NULL);

  BreakActivate(FALSE);

  ArgCleanup(arg, ARG_NUM);

  if (DList != NULL) {
    DistListFree(DList);
    DList = NULL;
  }

  if (LList != NULL) {
    DistListFree(LList);
    LList = NULL;
  }

  if (DoInsert) {
    GraphMolChanged(PROP_SELECTED);
    GraphRedrawNeeded();
  }

  return EC_OK;
}
