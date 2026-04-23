/*
************************************************************************
*
*   ExDipole.c - AddDipole command
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdprim/SCCS/s.ExDipole.c
*   SCCS identification       : 1.4
*
************************************************************************
*/

#include <cmd_prim.h>

#include <stdio.h>
#include <stdlib.h>

#include <mat_vec.h>
#include <data_hand.h>
#include <data_sel.h>
#include <prim_hand.h>
#include <graph_draw.h>

typedef struct {
  Vec3 cent, sum;
  int atomNo;
} AtomData;

static void
addCoord(DhAtomP atomP, void *clientData)
{
  AtomData *dataP = clientData;
  Vec3 coord;

  DhAtomGetCoord(atomP, coord);

  Vec3Add(dataP->sum, coord);
  dataP->atomNo++;
}

static void
addDipole(DhAtomP atomP, void *clientData)
{
  AtomData *dataP = clientData;
  Vec3 coord;
  float charge;

  DhAtomGetCoord(atomP, coord);
  charge = DhAtomGetCharge(atomP, CK_CHARGE);

  Vec3Sub(coord, dataP->cent);
  Vec3ScaleAdd(dataP->sum, charge, coord);

  dataP->atomNo++;
}

static void
dipoleMol(DhMolP molP, void *clientData)
{
  AtomData data;
  PropRefP refP;
  PrimObjP primP;
  Vec3 x1, v;

  refP = PropGetRef(PROP_SELECTED, FALSE);

  Vec3Zero(data.sum);
  data.atomNo = 0;
  DhMolApplyAtom(refP, molP, addCoord, &data);
  if (data.atomNo == 0)
    return;

  Vec3Copy(data.cent, data.sum);
  Vec3Scale(data.cent, 1.0f / data.atomNo);

  Vec3Zero(data.sum);
  data.atomNo = 0;
  DhMolApplyAtom(refP, molP, addDipole, &data);

  Vec3Copy(x1, data.cent);
  Vec3ScaleAdd(x1, -0.5f, data.sum);
  Vec3Copy(v, data.sum);

  primP = PrimNew(PT_CYLINDER, molP);

  PrimSetCylinderStyle(primP, PCS_ARROW);
  PrimSetPos(primP, x1);
  PrimSetVec(primP, v);

  PrimSetProp(PropGetRef("cylinder", TRUE), primP, TRUE);
  PrimSetProp(PropGetRef("dipole", TRUE), primP, TRUE);
}

ErrCode
ExAddDipole(char *cmd)
{
  DhApplyMol(PropGetRef(PROP_SELECTED, FALSE), dipoleMol, NULL);

  GraphMolChanged(PROP_SELECTED);
  GraphRedrawNeeded();

  return EC_OK;
}
