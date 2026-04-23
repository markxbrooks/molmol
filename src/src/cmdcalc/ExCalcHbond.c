/*
************************************************************************
*
*   ExCalcHbond.c - CalcHbond command
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdcalc/SCCS/s.ExCalcHbond.c
*   SCCS identification       : 1.11
*
************************************************************************
*/

#include <cmd_calc.h>

#include <stdio.h>
#include <math.h>
#include <values.h>

#include <break.h>
#include <mat_vec.h>
#include <pu.h>
#include <arg.h>
#include <data_hand.h>
#include <data_dist.h>
#include <calc_pair.h>
#include <graph_draw.h>

typedef struct {
  DhAtomP atomP;
  DhAtomP neighAtomP;
} NeighData;

static float MaxDist = 2.4f;
static float MaxAngle = 35.0f;
static int MinStrucNo = 1;
static BOOL InterMol = FALSE;
static BOOL DoReport = TRUE;
static BOOL DoMacro = FALSE;
static BOOL DoInsert = FALSE;

static PuTextWindow TextW, MacroW;
static PropRefP AllRefP;
static DistList DList = NULL;

static void
makeDistList(DhMolP *molPA, int molNo)
{
  if (DList != NULL)
    DistListFree(DList);

  DList = DistListBuildInter(AllRefP, DK_HBOND);
}

static BOOL
checkAtom1(DhAtomP atomP)
{
  return DhAtomGetHBondCap(atomP) > 0;
}

static BOOL
checkAtom2(DhAtomP atomP)
{
  return DhAtomGetHBondCap(atomP) < 0;
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
checkPair(DhAtomP atom1P, DhAtomP atom2P, float distS, float *valP)
{
  NeighData data;
  DhDistP distP;
  Vec3 x0, v1, v2;
  float c, s, ang;

  data.atomP = atom1P;
  data.neighAtomP = NULL;
  DhAtomApplyBond(AllRefP, atom1P, getNeigh, &data);
  if (data.neighAtomP == NULL)
    return FALSE;

  if (InterMol) {
    DhAtomGetCoordTransf(data.neighAtomP, x0);
    DhAtomGetCoordTransf(atom1P, v1);
    DhAtomGetCoordTransf(atom2P, v2);
  } else {
    DhAtomGetCoord(data.neighAtomP, x0);
    DhAtomGetCoord(atom1P, v1);
    DhAtomGetCoord(atom2P, v2);
  }

  Vec3Sub(v1, x0);
  Vec3Sub(v2, x0);

  Vec3Norm(v1);
  Vec3Norm(v2);

  c = Vec3Scalar(v1, v2);
  Vec3Cross(v1, v2);
  s = Vec3Abs(v1);
  ang = atan2f(s, c) * 180.0f / (float) M_PI;

  if (ang > MaxAngle)
    return FALSE;

  valP[0] = sqrtf(distS);
  valP[1] = ang;

  if (DoInsert) {
    distP = DistListFind(DList, atom1P, atom2P);
    if (distP != NULL)
      return TRUE;

    distP = DhDistNew(atom1P, atom2P);
    if (distP == NULL)
      return TRUE;

    DhDistSetKind(distP, DK_HBOND);
    DhDistInit(distP);
  }

  return TRUE;
}

static void
writeStr(char *str)
{
  PuWriteStr(TextW, str);
}

static void
writeMacro(DhAtomP atom1P, DhAtomP atom2P)
{
  char buf[20];

  PuWriteStr(MacroW, "SelectAtom 'mol.selected & (res.num = ");
  (void) sprintf(buf, "%d", DhResGetNumber(DhAtomGetRes(atom1P)));
  PuWriteStr(MacroW, buf);
  PuWriteStr(MacroW, " & name = \"");
  PuWriteStr(MacroW, DStrToStr(DhAtomGetName(atom1P)));
  PuWriteStr(MacroW, "\" | res.num = ");
  (void) sprintf(buf, "%d", DhResGetNumber(DhAtomGetRes(atom2P)));
  PuWriteStr(MacroW, buf);
  PuWriteStr(MacroW, " & name = \"");
  PuWriteStr(MacroW, DStrToStr(DhAtomGetName(atom2P)));
  PuWriteStr(MacroW, "\")'\n");
  PuWriteStr(MacroW, "AddHbond\n");
}

#define ARG_NUM 4
#define OPT_NUM 4

ErrCode
ExCalcHbond(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  EnumEntryDescr enumEntry[OPT_NUM];
  ErrCode errCode;
  DSTR title;
  char buf[20];

  arg[0].type = AT_DOUBLE;
  arg[1].type = AT_DOUBLE;
  arg[2].type = AT_INT;
  arg[3].type = AT_MULT_ENUM;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Max. Dist";
  arg[0].v.doubleVal = MaxDist;

  arg[1].prompt = "Max. Angle";
  arg[1].v.doubleVal = MaxAngle;

  arg[2].prompt = "Min. Struc. #";
  arg[2].v.intVal = MinStrucNo;

  enumEntry[0].str = "intermol";
  enumEntry[0].onOff = InterMol;
  enumEntry[1].str = "report";
  enumEntry[1].onOff = DoReport;
  enumEntry[2].str = "macro";
  enumEntry[2].onOff = DoMacro;
  enumEntry[3].str = "display";
  enumEntry[3].onOff = DoInsert;

  arg[3].prompt = "Options";
  arg[3].u.enumD.entryP = enumEntry;
  arg[3].u.enumD.n = OPT_NUM;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  MaxDist = (float) arg[0].v.doubleVal;
  MaxAngle = (float) arg[1].v.doubleVal;
  MinStrucNo = arg[2].v.intVal;
  InterMol = enumEntry[0].onOff;
  DoReport = enumEntry[1].onOff;
  DoMacro = enumEntry[2].onOff;
  DoInsert = enumEntry[3].onOff;

  ArgCleanup(arg, ARG_NUM);

  BreakActivate(TRUE);

  if (DoReport) {
    TextW = PuCreateTextWindow(cmd);

    PuWriteStr(TextW, "H-bonds (distance < ");
    (void) sprintf(buf, "%.2f", MaxDist);
    PuWriteStr(TextW, buf);
    PuWriteStr(TextW, ", angle < ");
    (void) sprintf(buf, "%.1f", MaxAngle);
    PuWriteStr(TextW, buf);
    PuWriteStr(TextW, ")");
    PuWriteStr(TextW, " in at least ");
    (void) sprintf(buf, "%d", MinStrucNo);
    PuWriteStr(TextW, buf);
    PuWriteStr(TextW, " structures:\n");
  }

  if (DoMacro) {
    title = DStrNew();
    DStrAssignStr(title, cmd);
    DStrAppStr(title, ".mac");
    MacroW = PuCreateTextWindow(DStrToStr(title));
    DStrFree(title);
  }

  AllRefP = PropGetRef(PROP_ALL, FALSE);

  CalcPair(MaxDist, FALSE, InterMol, PE_BOND,
      MinStrucNo, DoInsert ? makeDistList : NULL,
      PROP_SELECTED, PROP_SELECTED, checkAtom1, checkAtom2, checkPair, 2,
      NULL, 0,
      DoMacro ? writeMacro : NULL, DoReport ? writeStr : NULL);

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
