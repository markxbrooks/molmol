/*
************************************************************************
*
*   ExCircles.c - AddCircles command
*
*   Copyright (c) 1994-99
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdprim/SCCS/s.ExCircles.c
*   SCCS identification       : 1.6
*
************************************************************************
*/

#include <cmd_prim.h>

#include <stdio.h>

#include <hashtab.h>
#include <arg.h>
#include <data_hand.h>
#include <data_sel.h>
#include <prim_hand.h>
#include <attr_struc.h>
#include <attr_mng.h>
#include <graph_draw.h>

typedef struct {
  DhAtomP atomP;
  Vec3 v;
} AtomData;

static HASHTABLE AtomTab;
static float CurrRad = 0.2f;

static unsigned
hashFunc(void *p, unsigned size)
{
  AtomData *dataP = p;

  return (unsigned) dataP->atomP % size;
}

static int
compFunc(void *p1, void *p2)
{
  AtomData *data1P = p1;
  AtomData *data2P = p2;

  if (data1P->atomP == data2P->atomP)
    return 0;
  else
    return 1;
}

static void
addAtom(DhAtomP atomP, void *clientData)
{
  AtomData data;

  data.atomP = atomP;
  Vec3Zero(data.v);

  (void) HashtabInsert(AtomTab, &data, FALSE);
}

static void
addBond(DhBondP bondP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  AtomData data, *data1P, *data2P;
  Vec3 v, x;

  data.atomP = atom1P;
  data1P = HashtabSearch(AtomTab, &data);

  data.atomP = atom2P;
  data2P = HashtabSearch(AtomTab, &data);

  if (data1P == NULL && data2P == NULL)
    return;

  DhAtomGetCoord(atom2P, v);
  DhAtomGetCoord(atom1P, x);
  Vec3Sub(v, x);
  Vec3Norm(v);

  if (data1P != NULL)
    Vec3Add(data1P->v, v);

  if (data2P != NULL)
    Vec3Add(data2P->v, v);
}

static void
addCircle(void *p, void *clientData)
{
  AtomData *dataP = p;
  ExprP exprP = clientData;
  PrimObjP primP;
  Vec3 x;
  ExprRes exprRes;
  AttrP attrP;
  struct AttrS attr;

  if (dataP->v[0] == 0.0f && dataP->v[1] == 0.0f && dataP->v[2] == 0.0f)
    /* no bonds to determine direction of circle */
    return;

  primP = PrimNew(PT_CIRCLE, DhResGetMol(DhAtomGetRes(dataP->atomP)));
  DhAtomGetCoord(dataP->atomP, x);
  PrimSetPos(primP, x);
  Vec3Norm(dataP->v);
  PrimSetVec(primP, dataP->v);

  attrP = PrimGetAttr(primP);
  AttrCopy(&attr, attrP);
  AttrReturn(attrP);

  ExprEval(dataP->atomP, exprP, &exprRes);
  if (exprRes.resType == ER_INT)
    attr.radius = (float) exprRes.u.intVal;
  else
    attr.radius = exprRes.u.floatVal;

  if (attr.radius < 0.0f)
    attr.radius = 0.0f;
  
  PrimSetAttr(primP, AttrGet(&attr));

  PrimSetProp(PropGetRef("circle", TRUE), primP, TRUE);

  CurrRad = attr.radius;
}

#define ARG_NUM 1

ErrCode
ExAddCircles(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;
  PropRefP refP;

  arg[0].type = AT_DOUBLE;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Radius";
  arg[0].entType = DE_ATOM;
  arg[0].v.doubleVal = CurrRad;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  refP = PropGetRef(PROP_SELECTED, FALSE);

  AtomTab = HashtabOpen(137, sizeof(AtomData), hashFunc, compFunc);
  DhApplyAtom(refP, addAtom, NULL);

  DhApplyBond(refP, addBond, NULL);

  HashtabApply(AtomTab, addCircle, arg[0].v.exprP);

  ArgCleanup(arg, ARG_NUM);
  HashtabClose(AtomTab);

  GraphMolChanged(PROP_SELECTED);
  GraphRedrawNeeded();

  return EC_OK;
}
