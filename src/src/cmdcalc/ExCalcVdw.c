/*
************************************************************************
*
*   ExCalcVdw.c - CalcVdw command
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdcalc/SCCS/s.ExCalcVdw.c
*   SCCS identification       : 1.16
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

static float Cutoff = 0.5f;
static int MinStrucNo = 1;
static BOOL InterMol = FALSE;
static BOOL DoReport = TRUE;
static BOOL DoInsert = FALSE;

static PuTextWindow TextW;
static DistList DList = NULL;

static void
maxRadAtom(DhAtomP atomP, void *clientData)
{
  float *maxRadP = clientData;
  float rad;

  rad = DhAtomGetVdw(atomP);
  if (rad > *maxRadP)
    *maxRadP = rad;
}

static void
makeDistList(DhMolP *molPA, int molNo)
{
  if (DList != NULL)
    DistListFree(DList);

  DList = DistListBuildInter(PropGetRef(PROP_ALL, FALSE), DK_LOWER);
}

static BOOL
checkAtom(DhAtomP atomP)
{
  return DhAtomGetVdw(atomP) > 0.0f;
}

static BOOL
checkPair(DhAtomP atom1P, DhAtomP atom2P, float distS, float *valP)
{
  float radSum;
  DhDistP distP;

  radSum = DhAtomGetVdw(atom1P) + DhAtomGetVdw(atom2P);
  if (DhAtomGetHBondCap(atom1P) * DhAtomGetHBondCap(atom2P) < 0)
    radSum -= 1.0f;  /* subtract 1A for possible hydrogen bond partners */

  if (distS >= (radSum - Cutoff) * (radSum - Cutoff) )
    return FALSE;

  *valP = radSum - sqrtf(distS);

  if (DoInsert) {
    distP = DistListFind(DList, atom1P, atom2P);
    if (distP != NULL)
      return TRUE;

    distP = DhDistNew(atom1P, atom2P);
    if (distP == NULL)
      return TRUE;

    DhDistSetKind(distP, DK_LOWER);
    DhDistSetLimit(distP, radSum);
    DhDistInit(distP);
  }

  return TRUE;
}

static void
writeStr(char *str)
{
  PuWriteStr(TextW, str);
}

#define ARG_NUM 3
#define OPT_NUM 3

ErrCode
ExCalcVdw(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  EnumEntryDescr enumEntry[OPT_NUM];
  ErrCode errCode;
  float maxRad;
  char buf[20];

  arg[0].type = AT_DOUBLE;
  arg[1].type = AT_INT;
  arg[2].type = AT_MULT_ENUM;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Cutoff";
  arg[0].v.doubleVal = Cutoff;

  arg[1].prompt = "Min. Struc. #";
  arg[1].v.intVal = MinStrucNo;

  enumEntry[0].str = "intermol";
  enumEntry[0].onOff = InterMol;
  enumEntry[1].str = "report";
  enumEntry[1].onOff = DoReport;
  enumEntry[2].str = "display";
  enumEntry[2].onOff = DoInsert;

  arg[2].prompt = "Options";
  arg[2].u.enumD.entryP = enumEntry;
  arg[2].u.enumD.n = OPT_NUM;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  Cutoff = (float) arg[0].v.doubleVal;
  MinStrucNo = arg[1].v.intVal;
  InterMol = enumEntry[0].onOff;
  DoReport = enumEntry[1].onOff;
  DoInsert = enumEntry[2].onOff;

  ArgCleanup(arg, ARG_NUM);

  BreakActivate(TRUE);

  if (DoReport) {
    TextW = PuCreateTextWindow(cmd);

    PuWriteStr(TextW, "Van der Waals violations > ");
    (void) sprintf(buf, "%.2f", Cutoff);
    PuWriteStr(TextW, buf);
    PuWriteStr(TextW, " in at least ");
    (void) sprintf(buf, "%d", MinStrucNo);
    PuWriteStr(TextW, buf);
    PuWriteStr(TextW, " structures:\n");
  }

  maxRad = 0.0f;
  DhApplyAtom(PropGetRef(PROP_SELECTED, FALSE), maxRadAtom, &maxRad);

  CalcPair(2.0f * maxRad - Cutoff, TRUE, InterMol, PE_2_BOND,
      MinStrucNo, DoInsert ? makeDistList : NULL,
      PROP_SELECTED, PROP_SELECTED, checkAtom, checkAtom, checkPair, 1,
      NULL, 0,
      NULL, DoReport ? writeStr : NULL);

  BreakActivate(FALSE);

  if (DList != NULL) {
    DistListFree(DList);
    DList = NULL;
  }

  if (DoInsert) {
    GraphMolChanged(PROP_SELECTED);
    GraphRedrawNeeded();
  }

  return EC_OK;
}
