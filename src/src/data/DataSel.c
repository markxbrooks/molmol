/*
************************************************************************
*
*   DataSel.c - utility functions for selection
*
*   Copyright (c) 1994
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
*   Date of last modification : 95/04/07
*   Pathname of SCCS file     : /sgiext/molmol/src/data/SCCS/s.DataSel.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <data_sel.h>

#include <stdio.h>

#include <linlist.h>

typedef struct {
  SelCB func;
} SelCBInfo;

typedef struct {
  void **selArr;
  int size;
  int selNo;
} SelInfo;

static PropRefP AllRefP = NULL;
static PropRefP SelRefP = NULL;
static LINLIST SelCBList = NULL;

void
SelAddCB(SelCB func)
{
  SelCBInfo entry;

  if (SelCBList == NULL)
    SelCBList = ListOpen(sizeof(SelCBInfo));
  
  entry.func = func;
  (void) ListInsertLast(SelCBList, &entry);
}

static void
callCB(DataEntityType entType, void *entP, SelKind kind)
{
  SelCBInfo *entryP;

  entryP = ListFirst(SelCBList);
  while (entryP != NULL) {
    entryP->func(entType, entP, kind);
    entryP = ListNext(SelCBList, entryP);
  }
}

static void
getRef(void)
{
  if (SelRefP == NULL) {
    AllRefP = PropGetRef(PROP_ALL, FALSE);
    SelRefP = PropGetRef(PROP_SELECTED, FALSE);
  }
}

static void
getSelMol(DhMolP molP, void *clientData)
{
  SelInfo *selInfoP = clientData;

  selInfoP->selNo++;
  if (selInfoP->selNo <= selInfoP->size)
    selInfoP->selArr[selInfoP->selNo - 1] = molP;
}

int
SelMolGet(DhMolP molPA[], int size)
{
  SelInfo selInfo;

  getRef();

  selInfo.selArr = (void **) molPA;
  selInfo.size = size;
  selInfo.selNo = 0;

  DhApplyMol(SelRefP, getSelMol, &selInfo);

  return selInfo.selNo;
}

static void
getSelRes(DhResP resP, void *clientData)
{
  SelInfo *selInfoP = clientData;

  selInfoP->selNo++;
  if (selInfoP->selNo <= selInfoP->size)
    selInfoP->selArr[selInfoP->selNo - 1] = resP;
}

int
SelResGet(DhResP resPA[], int size)
{
  SelInfo selInfo;

  getRef();

  selInfo.selArr = (void **) resPA;
  selInfo.size = size;
  selInfo.selNo = 0;

  DhApplyRes(SelRefP, getSelRes, &selInfo);

  return selInfo.selNo;
}

static void
getSelAtom(DhAtomP atomP, void *clientData)
{
  SelInfo *selInfoP = clientData;

  selInfoP->selNo++;
  if (selInfoP->selNo <= selInfoP->size)
    selInfoP->selArr[selInfoP->selNo - 1] = atomP;
}

int
SelAtomGet(DhAtomP atomPA[], int size)
{
  SelInfo selInfo;

  getRef();

  selInfo.selArr = (void **) atomPA;
  selInfo.size = size;
  selInfo.selNo = 0;

  DhApplyAtom(SelRefP, getSelAtom, &selInfo);

  return selInfo.selNo;
}

static void
getSelBond(DhBondP bondP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  SelInfo *selInfoP = clientData;

  selInfoP->selNo++;
  if (selInfoP->selNo <= selInfoP->size)
    selInfoP->selArr[selInfoP->selNo - 1] = bondP;
}

int
SelBondGet(DhBondP bondPA[], int size)
{
  SelInfo selInfo;

  getRef();

  selInfo.selArr = (void **) bondPA;
  selInfo.size = size;
  selInfo.selNo = 0;

  DhApplyBond(SelRefP, getSelBond, &selInfo);

  return selInfo.selNo;
}

static void
getSelAngle(DhAngleP angleP, void *clientData)
{
  SelInfo *selInfoP = clientData;

  selInfoP->selNo++;
  if (selInfoP->selNo <= selInfoP->size)
    selInfoP->selArr[selInfoP->selNo - 1] = angleP;
}

int
SelAngleGet(DhAngleP anglePA[], int size)
{
  SelInfo selInfo;

  getRef();

  selInfo.selArr = (void **) anglePA;
  selInfo.size = size;
  selInfo.selNo = 0;

  DhApplyAngle(SelRefP, getSelAngle, &selInfo);

  return selInfo.selNo;
}

static void
getSelDist(DhDistP distP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  SelInfo *selInfoP = clientData;

  selInfoP->selNo++;
  if (selInfoP->selNo <= selInfoP->size)
    selInfoP->selArr[selInfoP->selNo - 1] = distP;
}

int
SelDistGet(DhDistP distPA[], int size)
{
  SelInfo selInfo;

  getRef();

  selInfo.selArr = (void **) distPA;
  selInfo.size = size;
  selInfo.selNo = 0;

  DhApplyDist(SelRefP, getSelDist, &selInfo);

  return selInfo.selNo;
}

void
SelMol(DhMolP molP, SelKind kind)
{
  getRef();

  if (kind == SK_ADD) {
    DhMolSetProp(SelRefP, molP, TRUE);
  } else if (kind == SK_REMOVE) {
    DhMolSetProp(SelRefP, molP, FALSE);
  } else {
    DeselAllMol();
    DhMolSetProp(SelRefP, molP, TRUE);
  }

  callCB(DE_MOL, molP, kind);
}

void
SelRes(DhResP resP, SelKind kind)
{
  getRef();

  if (kind == SK_ADD) {
    DhResSetProp(SelRefP, resP, TRUE);
  } else if (kind == SK_REMOVE) {
    DhResSetProp(SelRefP, resP, FALSE);
  } else {
    DeselAllRes();
    DhResSetProp(SelRefP, resP, TRUE);
  }

  callCB(DE_RES, resP, kind);
}

void
SelAtom(DhAtomP atomP, SelKind kind)
{
  getRef();

  if (kind == SK_ADD) {
    DhAtomSetProp(SelRefP, atomP, TRUE);
  } else if (kind == SK_REMOVE) {
    DhAtomSetProp(SelRefP, atomP, FALSE);
  } else {
    DeselAllAtom();
    DhAtomSetProp(SelRefP, atomP, TRUE);
  }

  callCB(DE_ATOM, atomP, kind);
}

void
SelBond(DhBondP bondP, SelKind kind)
{
  getRef();
  if (kind == SK_ADD) {
    DhBondSetProp(SelRefP, bondP, TRUE);
  } else if (kind == SK_REMOVE) {
    DhBondSetProp(SelRefP, bondP, FALSE);
  } else {
    DeselAllBond();
    DhBondSetProp(SelRefP, bondP, TRUE);
  }

  callCB(DE_BOND, bondP, kind);
}

void
SelAngle(DhAngleP angleP, SelKind kind)
{
  getRef();

  if (kind == SK_ADD) {
    DhAngleSetProp(SelRefP, angleP, TRUE);
  } else if (kind == SK_REMOVE) {
    DhAngleSetProp(SelRefP, angleP, FALSE);
  } else {
    DeselAllAngle();
    DhAngleSetProp(SelRefP, angleP, TRUE);
  }

  callCB(DE_ANGLE, angleP, kind);
}

void
SelDist(DhDistP distP, SelKind kind)
{
  getRef();

  if (kind == SK_ADD) {
    DhDistSetProp(SelRefP, distP, TRUE);
  } else if (kind == SK_REMOVE) {
    DhDistSetProp(SelRefP, distP, FALSE);
  } else {
    DeselAllDist();
    DhDistSetProp(SelRefP, distP, TRUE);
  }

  callCB(DE_DIST, distP, kind);
}

static void
deselMol(DhMolP molP, void *clientData)
{
  DhMolSetProp(SelRefP, molP, FALSE);
}

void
DeselAllMol(void)
{
  getRef();
  DhApplyMol(AllRefP, deselMol, NULL);
}

static void
deselRes(DhResP resP, void *clientData)
{
  DhResSetProp(SelRefP, resP, FALSE);
}

void
DeselAllRes(void)
{
  getRef();
  DhApplyRes(AllRefP, deselRes, NULL);
}

static void
deselAtom(DhAtomP atomP, void *clientData)
{
  DhAtomSetProp(SelRefP, atomP, FALSE);
}

void
DeselAllAtom(void)
{
  getRef();
  DhApplyAtom(AllRefP, deselAtom, NULL);
}

static void
deselBond(DhBondP bondP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  DhBondSetProp(SelRefP, bondP, FALSE);
}

void
DeselAllBond(void)
{
  getRef();
  DhApplyBond(AllRefP, deselBond, NULL);
}

static void
deselAngle(DhAngleP angleP, void *clientData)
{
  DhAngleSetProp(SelRefP, angleP, FALSE);
}

void
DeselAllAngle(void)
{
  getRef();
  DhApplyAngle(AllRefP, deselAngle, NULL);
}

static void
deselDist(DhDistP distP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  DhDistSetProp(SelRefP, distP, FALSE);
}

void
DeselAllDist(void)
{
  getRef();
  DhApplyDist(AllRefP, deselDist, NULL);
}
