/*
************************************************************************
*
*   ExCalcAtom.c - CalcAtom command
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
*   Date of last modification : 95/03/02
*   Pathname of SCCS file     : /sgiext/molmol/src/cmdcalc/SCCS/s.ExCalcAtom.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <cmd_calc.h>

#include <stdio.h>

#include <arg.h>
#include <data_hand.h>
#include <graph_draw.h>

static void
calcAtom(DhResP resP, void *clientData)
{
  DSTR name = clientData;

  DhResCalcAtom(resP, name);
}

static void
calcAngles(DhMolP molP, void *clientData)
{
  DhMolCoordsChanged(molP);
}

#define ARG_NUM 1

ErrCode
ExCalcAtom(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  PropRefP refP;

  arg[0].type = AT_STR;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Atom Name";

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  refP = PropGetRef(PROP_SELECTED, FALSE);

  DhApplyRes(refP, calcAtom, arg[0].v.strVal);
  DhApplyMol(refP, calcAngles, NULL);

  ArgCleanup(arg, ARG_NUM);

  GraphRedrawNeeded();

  return EC_OK;
}
