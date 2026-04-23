/*
************************************************************************
*
*   ExAtom.c - AddAtom, AddPseudo and RemoveAtom commands
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdstruc/SCCS/s.ExAtom.c
*   SCCS identification       : 1.8
*
************************************************************************
*/

#include <cmd_struc.h>

#include <stdio.h>
#include <stdlib.h>
#include <values.h>
#include <math.h>

#include <mat_vec.h>
#include <arg.h>
#include <data_hand.h>
#include <graph_draw.h>

typedef struct {
  DSTR name, attName, axisName, refName;
  Vec3 xt;
} AtomData;

typedef struct {
  DSTR name;
} PseudoData;

typedef struct {
  DhAtomP *atomPA;
  int atomNo;
} AtomList;

static void
addAtom(DhResP resP, void *clientData)
{
  AtomData *dataP = clientData;
  DhAtomP atomP, attAtomP;
  DhBondP bondP;
  Vec3 x1, x2, x3, x4;
  Vec3 vx, vy, vz;
  Mat3 m;
  int i;

  atomP = DhAtomFindName(resP, dataP->attName, FALSE);
  if (atomP == NULL)
    return;
  DhAtomGetCoord(atomP, x2);

  atomP = DhAtomFindName(resP, dataP->axisName, FALSE);
  if (atomP == NULL)
    return;
  DhAtomGetCoord(atomP, x1);

  atomP = DhAtomFindName(resP, dataP->refName, FALSE);
  if (atomP == NULL)
    return;
  DhAtomGetCoord(atomP, x3);

  atomP = DhAtomNew(resP, dataP->name);
  if (atomP == NULL)
    return;

  Vec3Copy(vz, x1);
  Vec3Sub(vz, x2);
  Vec3Norm(vz);

  Vec3Copy(vx, x3);
  Vec3Sub(vx, x2);
  Vec3ScaleSub(vx, Vec3Scalar(vz, vx), vz);
  Vec3Norm(vx);

  Vec3Copy(vy, vz);
  Vec3Cross(vy, vx);

  for (i = 0; i < 3; i++) {
    m[0][i] = vx[i];
    m[1][i] = vy[i];
    m[2][i] = vz[i];
  }

  Vec3Copy(x4, dataP->xt);
  Mat3VecMult(x4, m);
  Vec3Add(x4, x2);

  DhAtomSetCoord(atomP, x4);

  DhAtomInit(atomP);

  attAtomP = DhAtomFindName(resP, dataP->attName, FALSE);
  bondP = DhBondNew(attAtomP, atomP);
  DhBondInit(bondP);
}

#define ARG_NUM 7

ErrCode
ExAddAtom(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  AtomData data;
  float phi, cphi, sphi;
  float theta, ctheta, stheta;
  float dist;

  arg[0].type = AT_STR;
  arg[1].type = AT_STR;
  arg[2].type = AT_STR;
  arg[3].type = AT_STR;
  arg[4].type = AT_DOUBLE;
  arg[5].type = AT_DOUBLE;
  arg[6].type = AT_DOUBLE;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "New Atom";
  arg[1].prompt = "Attach Atom";
  arg[2].prompt = "Axis Atom";
  arg[3].prompt = "Ref. Atom";
  arg[4].prompt = "PHI";
  arg[4].v.doubleVal = 120.0;
  arg[5].prompt = "THETA";
  arg[5].v.doubleVal = 120.0;
  arg[6].prompt = "Bond Lenght";
  arg[6].v.doubleVal = 1.0;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  data.name = arg[0].v.strVal;
  data.attName = arg[1].v.strVal;
  data.axisName = arg[2].v.strVal;
  data.refName = arg[3].v.strVal;

  phi = (float) arg[4].v.doubleVal / 180.0f * (float) M_PI;
  theta = (float) arg[5].v.doubleVal / 180.0f * (float) M_PI;
  dist = (float) arg[6].v.doubleVal;

  cphi = cosf(phi);
  sphi = sinf(phi);
  ctheta = cosf(theta);
  stheta = sinf(theta);

  data.xt[0] = dist * cphi * stheta;
  data.xt[1] = - dist * sphi * stheta;
  data.xt[2] = dist * ctheta;

  DhApplyRes(PropGetRef(PROP_SELECTED, FALSE), addAtom, &data);

  ArgCleanup(arg, ARG_NUM);

  GraphMolChanged(PROP_SELECTED);
  GraphRedrawNeeded();

  return EC_OK;
}

static void
countAtoms(DhAtomP atomP, void *clientData)
{
  *(int *) clientData += 1;
}

static void
fillAtoms(DhAtomP atomP, void *clientData)
{
  AtomList *listP = clientData;

  listP->atomPA[listP->atomNo++] = atomP;
}

static void
addPseudo(DhResP resP, void *clientData)
{
  AtomData *dataP = clientData;
  PropRefP refP;
  AtomList list;
  DhAtomP pseudoP;

  refP = PropGetRef(PROP_SELECTED, FALSE);

  list.atomNo = 0;
  DhResApplyAtom(refP, resP, countAtoms, &list.atomNo);
  if (list.atomNo < 2)
    return;

  list.atomPA = malloc(list.atomNo * sizeof(*list.atomPA));
  list.atomNo = 0;
  DhResApplyAtom(refP, resP, fillAtoms, &list);

  pseudoP = DhAtomNewPseudo(list.atomPA, list.atomNo, dataP->name);
  if (pseudoP == NULL)
    return;

  DhAtomInit(pseudoP);

  free(list.atomPA);
}

#undef ARG_NUM
#define ARG_NUM 1

ErrCode
ExAddPseudo(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  PseudoData data;

  arg[0].type = AT_STR;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Name";

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  data.name = arg[0].v.strVal;

  DhApplyRes(PropGetRef(PROP_SELECTED, FALSE), addPseudo, &data);

  ArgCleanup(arg, ARG_NUM);

  GraphMolChanged(PROP_SELECTED);
  GraphRedrawNeeded();

  return EC_OK;
}

static void
removeAtom(DhAtomP atomP, void *clientData)
{
  DhAtomDestroy(atomP);
}

ErrCode
ExRemoveAtom(char *cmd)
{
  DhApplyAtom(PropGetRef(PROP_SELECTED, FALSE), removeAtom, NULL);

  GraphMolChanged(PROP_SELECTED);
  GraphRedrawNeeded();

  return EC_OK;
}
