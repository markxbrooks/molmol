/*
************************************************************************
*
*   PlateCalc.c - calculate plates
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/prim/SCCS/s.PlateCalc.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include "plate_calc.h"

#include <stdio.h>
#include <stdlib.h>

#include "prim_struc.h"

static void
calcCent(Vec3 xCent, DhAtomP atomA[], int atomNo)
{
  Vec3 x;
  int i;

  Vec3Zero(xCent);
  for (i = 0; i < atomNo; i++) {
    DhAtomGetCoord(atomA[i], x);
    Vec3Add(xCent, x);
  }
  Vec3Scale(xCent, 1.0f / atomNo);
}

static void
calcNorm(Vec3 nv, DhAtomP atomA[], int atomNo, int atomI)
{
  Vec3 x, v1, v2;

  DhAtomGetCoord(atomA[atomI], v1);
  DhAtomGetCoord(atomA[(atomI + atomNo - 1) % atomNo], x);
  Vec3Sub(v1, x);

  DhAtomGetCoord(atomA[(atomI + 1) % atomNo], v2);
  DhAtomGetCoord(atomA[atomI], x);
  Vec3Sub(v2, x);

  Vec3Copy(nv, v1);
  Vec3Cross(nv, v2);
  Vec3Norm(nv);
}

static void
calcAvgNorm(Vec3 nAvg, DhAtomP atomA[], int atomNo)
{
  Vec3 nv;
  int i;

  Vec3Zero(nAvg);
  for (i = 0; i < atomNo; i++) {
    calcNorm(nv, atomA, atomNo, i);
    Vec3Add(nAvg, nv);
  }
  Vec3Norm(nAvg);
}

BOOL
PlateCheckAtoms(DhAtomP atomA[], int atomNo)
{
  Vec3 nCent, nv;
  int i;

  calcAvgNorm(nCent, atomA, atomNo);
  for (i = 0; i < atomNo; i++) {
    calcNorm(nv, atomA, atomNo, i);
    if (Vec3Scalar(nCent, nv) <= 0.0f)
      return FALSE;
  }

  return TRUE;
}

void
PlateCalcPoints(PrimObjP primP)
{
  PrimPlate *plateP;
  int n;
  Vec3 nTop, nBot, xCent, xTop, xBot;
  Vec3 x, v, nv;
  int i;

  plateP = &primP->u.plate;

  if (plateP->pointValid)
    return;

  if (plateP->triA != NULL)
    free(plateP->triA);

  if (plateP->quadA != NULL)
    free(plateP->quadA);
  
  n = plateP->atomNo;

  calcAvgNorm(nTop, plateP->atomA, n);
  Vec3Copy(nBot, nTop);
  Vec3Scale(nBot, -1.0f);

  calcCent(xCent, plateP->atomA, n);
  Vec3Copy(xTop, xCent);
  Vec3ScaleAdd(xTop, 0.5f * plateP->thick, nTop);
  Vec3Copy(xBot, xCent);
  Vec3ScaleAdd(xBot, 0.5f * plateP->thick, nBot);

  plateP->triNo = 2 * n;
  plateP->triA = malloc(plateP->triNo * sizeof(*plateP->triA));

  plateP->quadNo = n;
  plateP->quadA = malloc(plateP->quadNo * sizeof(*plateP->quadA));

  for (i = 0; i < n; i++) {
    Vec3Copy(plateP->triA[i].xA[0], xTop);
    Vec3Copy(plateP->triA[i].nvA[0], nTop);
    Vec3Copy(plateP->triA[n + i].xA[0], xBot);
    Vec3Copy(plateP->triA[n + i].nvA[0], nBot);

    DhAtomGetCoord(plateP->atomA[i], v);
    Vec3Sub(v, xCent);

    Vec3Copy(x, xTop);
    Vec3Add(x, v);
    calcNorm(nv, plateP->atomA, n, i);

    Vec3Copy(plateP->triA[i].xA[2], x);
    Vec3Copy(plateP->triA[i].nvA[2], nv);
    Vec3Copy(plateP->triA[(i + 1) % n].xA[1], x);
    Vec3Copy(plateP->triA[(i + 1) % n].nvA[1], nv);

    Vec3Copy(plateP->quadA[i].xA[0], x);
    Vec3Copy(plateP->quadA[(i + 1) % n].xA[1], x);

    Vec3Copy(x, xBot);
    Vec3Add(x, v);
    Vec3Scale(nv, -1.0f);

    Vec3Copy(plateP->triA[n + i].xA[1], x);
    Vec3Copy(plateP->triA[n + i].nvA[1], nv);
    Vec3Copy(plateP->triA[n + (i + 1) % n].xA[2], x);
    Vec3Copy(plateP->triA[n + (i + 1) % n].nvA[2], nv);

    Vec3Copy(plateP->quadA[i].xA[3], x);
    Vec3Copy(plateP->quadA[(i + 1) % n].xA[2], x);

    DhAtomGetCoord(plateP->atomA[i], nv);
    DhAtomGetCoord(plateP->atomA[(i + n - 1) % n], x);
    Vec3Sub(nv, x);
    Vec3Cross(nv, nTop);
    Vec3Norm(nv);
    Vec3Copy(plateP->quadA[i].nv, nv);
  }

  plateP->pointValid = TRUE;
}
