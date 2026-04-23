/*
************************************************************************
*
*   ExLenBond.c - LengthBond command
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdrep/SCCS/s.ExLenBond.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <cmd_rep.h>

#include <stdio.h>

#include <mat_vec.h>
#include <arg.h>
#include <data_hand.h>
#include <graph_draw.h>

typedef struct {
  float startPar, endPar;
} LengthData;

static void
setBondLength(DhBondP bondP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  LengthData *dataP = clientData;
  Vec3 x1, x2, v;

  DhAtomGetCoord(atom1P, x1);
  DhAtomGetCoord(atom2P, x2);
  Vec3Copy(v, x2);
  Vec3Sub(v, x1);

  if (dataP->startPar == 0.0f) {
    DhAtomSetAltCoord(atom1P, NULL);
  } else {
    Vec3ScaleAdd(x1, dataP->startPar, v);
    DhAtomSetAltCoord(atom1P, x1);
  }

  if (dataP->endPar == 1.0f) {
    DhAtomSetAltCoord(atom2P, NULL);
  } else {
    Vec3ScaleAdd(x2, dataP->endPar - 1.0f, v);
    DhAtomSetAltCoord(atom2P, x2);
  }
}

#define ARG_NUM 2

ErrCode
ExLengthBond(char *cmd)
{
  PropRefP refP;
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  LengthData data;

  refP = PropGetRef(PROP_SELECTED, FALSE);

  arg[0].type = AT_DOUBLE;
  arg[1].type = AT_DOUBLE;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Start Par.";
  arg[0].v.doubleVal = 0.0;

  arg[1].prompt = "End Par.";
  arg[1].v.doubleVal = 1.0;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  data.startPar = (float) arg[0].v.doubleVal;
  data.endPar = (float) arg[1].v.doubleVal;

  ArgCleanup(arg, ARG_NUM);

  DhApplyBond(refP, setBondLength, &data);

  GraphMolChanged(PROP_SELECTED);
  GraphRedrawNeeded();

  return EC_OK;
}
