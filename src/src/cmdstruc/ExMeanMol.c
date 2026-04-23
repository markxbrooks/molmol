/*
************************************************************************
*
*   ExMeanMol.c - MeanMol command
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
*   Date of last modification : 98/07/21
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homea/rkoradi/molmol-master/src/cmdstruc/SCCS/s.ExMeanMol.c
*   SCCS identification       : 1.11
*
************************************************************************
*/

#include <cmd_struc.h>

#include <stdio.h>
#include <stdlib.h>

#include <arg.h>
#include <data_hand.h>
#include <data_sel.h>
#include <graph_draw.h>

#define ARG_NUM 3
#define BFACT_NUM 5

static int CurrBFact = 2;
static BOOL CurrSetAll = FALSE;

ErrCode
ExMeanMol(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  EnumEntryDescr bfactEntry[BFACT_NUM];
  EnumEntryDescr setEntry[2];
  int molNo;
  DhMolP *molPA, newMolP;
  Vec3 transV;
  int i;

  arg[0].type = AT_STR;
  arg[1].type = AT_ENUM;
  arg[2].type = AT_ENUM;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Molecule Name";
  DStrAssignStr(arg[0].v.strVal, "mean");

  bfactEntry[0].str = "avg_bfactor";
  bfactEntry[0].onOff = FALSE;
  bfactEntry[1].str = "max_disp";
  bfactEntry[1].onOff = FALSE;
  bfactEntry[2].str = "avg_disp";
  bfactEntry[2].onOff = FALSE;
  bfactEntry[3].str = "rms_disp";
  bfactEntry[3].onOff = FALSE;
  bfactEntry[4].str = "x_ray";
  bfactEntry[4].onOff = FALSE;

  bfactEntry[CurrBFact].onOff = TRUE;

  arg[1].prompt = "B-Factor calculation";
  arg[1].u.enumD.entryP = bfactEntry;
  arg[1].u.enumD.n = BFACT_NUM;
  arg[1].u.enumD.lineNo = 2;
  arg[1].v.intVal = CurrBFact;

  setEntry[0].str = "mean_only";
  setEntry[0].onOff = ! CurrSetAll;
  setEntry[1].str = "set_all";
  setEntry[1].onOff = CurrSetAll;

  arg[2].prompt = "Set B-Factor";
  arg[2].u.enumD.entryP = setEntry;
  arg[2].u.enumD.n = 2;
  if (CurrSetAll)
    arg[2].v.intVal = 1;
  else
    arg[2].v.intVal = 0;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  molNo = SelMolGet(NULL, 0);

  if (molNo < 1) {
    ArgCleanup(arg, ARG_NUM);
    CipSetError("at least 1 molecule must be selected");
    return EC_ERROR;
  }

  CurrBFact = arg[1].v.intVal;
  CurrSetAll = (arg[2].v.intVal == 1);

  molPA = malloc(molNo * sizeof(DhMolP));
  (void) SelMolGet(molPA, molNo);

  for (i = 1; i < molNo; i++)
    if (! DhMolEqualStruc(molPA[0], molPA[i])) {
      ArgCleanup(arg, ARG_NUM);
      CipSetError("selected molecules don't have same structure");
      return EC_ERROR;
    }

  newMolP = DhMolCopy(molPA[0]);
  DhMolSetName(newMolP, arg[0].v.strVal);

  DhMolCalcMeanStruc(newMolP, molPA, molNo, CurrBFact, CurrSetAll);
  DhMolInit(newMolP);

  DhMolSetProp(PropGetRef(PROP_MEAN, TRUE), newMolP, TRUE);

  /* Translation vectors must all be equal so that molecules
     rotate together afterwards. New molecule has identity
     rotation matrix and zero rotation and translation vectors.
     So taking the translation vector of molPA[0] can be
     compensated by taking the same vector as rotation point. */
  DhMolGetTransVect(molPA[0], transV);
  DhMolSetTransVect(newMolP, transV);
  DhMolSetRotPoint(newMolP, transV);

  free(molPA);

  ArgCleanup(arg, ARG_NUM);

  GraphMolAdd(newMolP);
  GraphRedrawNeeded();

  return EC_OK;
}
