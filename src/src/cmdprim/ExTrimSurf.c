/*
************************************************************************
*
*   ExTrimSurf.c - TrimSurface command
*
*   Copyright (c) 1997
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdprim/SCCS/s.ExTrimSurf.c
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include <cmd_prim.h>

#include <stdio.h>
#include <stdlib.h>
#include <values.h>

#include <break.h>
#include <grid.h>
#include <arg.h>
#include <data_hand.h>
#include <prim_hand.h>
#include <graph_draw.h>

typedef struct {
  GRID grid;
  float maxRad;
  DhAtomP atomP;
  float minDist;
} TrimData;

static float DistOffset = 1.5f;

static void
prepareAtom(DhAtomP atomP, void *clientData)
{
  TrimData *dataP = clientData;
  float rad;
  Vec3 x;

  if (BreakCheck(10000))
    return;

  rad = DhAtomGetVdw(atomP);
  if (rad < 0.0f)
    return;

  DhAtomGetCoord(atomP, x);
  GridPrepareAddEntry(dataP->grid, x);

  if (rad > dataP->maxRad)
    dataP->maxRad = rad;
}

static void
insertAtom(DhAtomP atomP, void *clientData)
{
  TrimData *dataP = clientData;
  float rad;
  Vec3 x;

  if (BreakCheck(10000))
    return;

  rad = DhAtomGetVdw(atomP);
  if (rad < 0.0f)
    return;

  DhAtomGetCoord(atomP, x);
  GridInsertEntry(dataP->grid, x, atomP);
}

static BOOL
getClosest(void *entryP, Vec3 x, void *clientData)
{
  DhAtomP atomP = entryP;
  TrimData *dataP = clientData;
  Vec3 dx;
  float rad, dist;

  DhAtomGetCoord(atomP, dx);
  Vec3Sub(dx, x);
  rad = DhAtomGetVdw(atomP);
  dist = dx[0] * dx[0] + dx[1] * dx[1] + dx[2] * dx[2] - rad * rad;

  if (dist < dataP->minDist) {
    dataP->atomP = atomP;
    dataP->minDist = dist;
  }

  return TRUE;
}

static void
trimF(float *distP, Vec3 x, void *clientData)
{
  TrimData *dataP = clientData;
  Vec3 dx;
  float rad;

  if (BreakCheck(10000))
    return;

  dataP->atomP = NULL;
  dataP->minDist = MAXFLOAT;
  GridFind(dataP->grid, x, 1, getClosest, dataP);

  if (dataP->atomP == NULL) {
    *distP = MAXFLOAT;
  } else {
    DhAtomGetCoord(dataP->atomP, dx);
    Vec3Sub(dx, x);
    rad = DhAtomGetVdw(dataP->atomP);
    *distP = Vec3Abs(dx) - rad - DistOffset;
  }
}

static void
trimSurf(PrimObjP primP, void *clientData)
{
  DhMolP molP;
  PropRefP refP;
  TrimData data;

  if (BreakInterrupted())
    return;

  molP = PrimGetMol(primP);

  refP = PropGetRef(PROP_SELECTED, FALSE);

  data.grid = GridNew();
  data.maxRad = 0.0f;
  DhMolApplyAtom(refP, molP, prepareAtom, &data);
  if (data.maxRad > 0.0f) {
    GridInsertInit(data.grid, 1.1f * data.maxRad + DistOffset);
    DhMolApplyAtom(refP, molP, insertAtom, &data);
    PrimSurfaceTrim(primP, trimF, &data);
  } else {
    /* must be initialized, otherwise GridDestroy crashes */
    GridInsertInit(data.grid, 10.0f);
  }

  GridDestroy(data.grid);
}

#define ARG_NUM 1

ErrCode
ExTrimSurface(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  PropRefP refP;

  arg[0].type = AT_DOUBLE;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Dist Offset";
  arg[0].v.doubleVal = DistOffset;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  DistOffset = (float) arg[0].v.doubleVal;

  ArgCleanup(arg, ARG_NUM);

  BreakActivate(TRUE);

  refP = PropGetRef(PROP_SELECTED, FALSE);
  PrimApply(PT_DOT_SURFACE, refP, trimSurf, NULL);
  PrimApply(PT_SURFACE, refP, trimSurf, NULL);

  BreakActivate(FALSE);

  GraphMolChanged(PROP_SELECTED);
  GraphRedrawNeeded();

  return EC_OK;
}
