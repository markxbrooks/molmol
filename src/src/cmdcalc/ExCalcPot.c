/*
************************************************************************
*
*   ExCalcPot.c - CalcPot command
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdcalc/SCCS/s.ExCalcPot.c
*   SCCS identification       : 1.10
*
************************************************************************
*/

#include <cmd_calc.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <break.h>
#include <potential.h>
#include <g_file.h>
#include <arg.h>
#include <par_names.h>
#include <par_hand.h>
#include <data_hand.h>

typedef struct {
  ExprP radExprP;
  ExprP chargeExprP;
  GFile gf;
  BOOL ok;
  int molNo;
} PotData;

static float MolDiel = 2.0f;
static float SolventDiel = 80.0f;
static float SolventRad = 1.4f;
static float SaltConcentr = 0.3f;
static float SaltRad = 2.0f;
static float BoundSize = 10.0f;
static int BoundCond = 0;
static char FileName[200] = "tt.pot";

static PotAtomDescr *AtomA;
static int AtomNo;

#if 0
BOOL
PotentialCalc(PotAtomDescr descrA[], int atomNo,
    float molDiel, float solvDiel, float solvRad, float saltConcentr,
    float boxScale, PotBoundaryCond boundCond, int prec,
    PotGridDescr *gridP)
{
  FILE *fp;
  int atomI;

  fp = fopen(FileName, "w");
  if (fp == NULL)
    return FALSE;

  (void) fprintf(fp, "%5d %5.2f %5.2f %5.3f %5.2f %1d\n",
      atomNo, molDiel, solvDiel, solvRad, saltConcentr, prec);

  for (atomI = 0; atomI < atomNo; atomI++)
    (void) fprintf(fp, "%8.3f %8.3f %8.3f %5.3f %6.3f\n",
	descrA[atomI].cent[0], descrA[atomI].cent[1], descrA[atomI].cent[2],
	descrA[atomI].rad, descrA[atomI].charge);
  (void) fclose(fp);

  return TRUE;
}
#endif

static void
countAtom(DhAtomP atomP, void *clientData)
{
  (* (int *) clientData)++;
}

static void
fillData(DhAtomP atomP, void *clientData)
{
  PotData *dataP = clientData;
  ExprRes exprRes;

  ExprEval(atomP, dataP->radExprP, &exprRes);
  if (exprRes.resType == ER_INT)
    AtomA[AtomNo].rad = (float) exprRes.u.intVal;
  else
    AtomA[AtomNo].rad = exprRes.u.floatVal;
  if (AtomA[AtomNo].rad <= 0.0f)
    return;

  DhAtomGetCoord(atomP, AtomA[AtomNo].cent);

  ExprEval(atomP, dataP->chargeExprP, &exprRes);
  if (exprRes.resType == ER_INT)
    AtomA[AtomNo].charge = (float) exprRes.u.intVal;
  else
    AtomA[AtomNo].charge = exprRes.u.floatVal;

  AtomNo++;
}

static void
calcPot(DhMolP molP, void *clientData)
{
  PotData *dataP = clientData;
  PropRefP refP;
  int atomNo;
  PotGridDescr potGrid;
  char buf[100];
  int totalNo, i, k;

  if (BreakInterrupted())
    return;

  if (! dataP->ok)
    return;

  refP = PropGetRef(PROP_SELECTED, FALSE);

  atomNo = 0;
  DhMolApplyAtom(refP, molP, countAtom, &atomNo);
  if (atomNo == 0)
    return;

  AtomA = malloc(atomNo * sizeof(*AtomA));
  AtomNo = 0;
  DhMolApplyAtom(refP, molP, fillData, dataP);

  if (! PotentialCalc(AtomA, AtomNo, MolDiel,
      SolventDiel, SolventRad,
      SaltConcentr, SaltRad,
      BoundSize, BoundCond, ParGetIntVal(PN_DRAW_PREC),
      &potGrid)) {
    if (! BreakInterrupted()) {
      CipSetError("out of memory");
      dataP->ok = FALSE;
    }
    return;
  }

  totalNo = 1;
  for (k = 0; k < 3; k++) {
    (void) sprintf(buf, "%8.3f %4d %5.3f", potGrid.origin[k],
	potGrid.pointNo[k], potGrid.spacing);
    dataP->ok = (GFileWriteLine(dataP->gf, buf) == GF_RES_OK);
    if (! dataP->ok)
      break;

    totalNo *= potGrid.pointNo[k];
  }

  for (i = 0; i < totalNo; i++) {
    if (! dataP->ok)
      break;

    (void) sprintf(buf, "%10.3e", potGrid.valueA[i]);
    dataP->ok = (GFileWriteStr(dataP->gf, buf) == GF_RES_OK);
    if ((i % 10 == 9 || i == totalNo - 1) && dataP->ok)
      dataP->ok = (GFileWriteNL(dataP->gf) == GF_RES_OK);
  }

  free(potGrid.valueA);

  free(AtomA);

  dataP->molNo++;
}

#define ARG_NUM 10
#define BOUND_NUM 2

ErrCode
ExCalcPot(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  EnumEntryDescr enumEntry[BOUND_NUM];
  ErrCode errCode;
  PotData potData;

  arg[0].type = AT_DOUBLE;
  arg[1].type = AT_DOUBLE;
  arg[2].type = AT_DOUBLE;
  arg[3].type = AT_DOUBLE;
  arg[4].type = AT_DOUBLE;
  arg[5].type = AT_DOUBLE;
  arg[6].type = AT_DOUBLE;
  arg[7].type = AT_DOUBLE;
  arg[8].type = AT_ENUM;
  arg[9].type = AT_STR;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Atom Radius";
  arg[0].entType = DE_ATOM;
  DStrAssignStr(arg[0].v.strVal, "vdw");

  arg[1].prompt = "Atom Charge";
  arg[1].entType = DE_ATOM;
  DStrAssignStr(arg[1].v.strVal, "charge");

  arg[2].prompt = "Mol. Dielectr.";
  arg[2].v.doubleVal = MolDiel;

  arg[3].prompt = "Solvent Dielectr.";
  arg[3].v.doubleVal = SolventDiel;

  arg[4].prompt = "Solvent Radius";
  arg[4].v.doubleVal = SolventRad;

  arg[5].prompt = "Salt Concentr.";
  arg[5].v.doubleVal = SaltConcentr;

  arg[6].prompt = "Salt Radius";
  arg[6].v.doubleVal = SaltRad;

  arg[7].prompt = "Boundary Size";
  arg[7].v.doubleVal = BoundSize;

  enumEntry[0].str = "zero";
  enumEntry[0].onOff = FALSE;
  enumEntry[1].str = "debye";
  enumEntry[1].onOff = FALSE;

  enumEntry[BoundCond].onOff = TRUE;

  arg[8].prompt = "Boundary Condition";
  arg[8].u.enumD.entryP = enumEntry;
  arg[8].u.enumD.n = BOUND_NUM;
  arg[8].v.intVal = BoundCond;

  arg[9].prompt = "File Name";
  DStrAssignStr(arg[9].v.strVal, FileName);

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  potData.radExprP = arg[0].v.exprP;
  potData.chargeExprP = arg[1].v.exprP;

  MolDiel = (float) arg[2].v.doubleVal;
  SolventDiel = (float) arg[3].v.doubleVal;
  SolventRad = (float) arg[4].v.doubleVal;
  SaltConcentr = (float) arg[5].v.doubleVal;
  SaltRad = (float) arg[6].v.doubleVal;
  BoundSize = (float) arg[7].v.doubleVal;
  BoundCond = arg[8].v.intVal;
  (void) strcpy(FileName, DStrToStr(arg[9].v.strVal));

  if (BoundSize < 0.0f)
    BoundSize = 0.0f;

  potData.gf = GFileOpenNew(FileName, GF_FORMAT_ASCII);
  if (potData.gf == NULL) {
    ArgCleanup(arg, ARG_NUM);
    return EC_ERROR;
  }

  BreakActivate(TRUE);
  DhActivateGroups(TRUE);

  potData.ok = TRUE;
  potData.molNo = 0;
  DhApplyMol(PropGetRef(PROP_SELECTED, FALSE), calcPot, &potData);

  GFileClose(potData.gf);

  ArgCleanup(arg, ARG_NUM);
  BreakActivate(FALSE);
  DhActivateGroups(FALSE);

  if (! potData.ok)
    return EC_ERROR;

  if (potData.molNo > 1) {
    CipSetError("more than one molecule selected");
    return EC_WARNING;
  }

  return EC_OK;
}
