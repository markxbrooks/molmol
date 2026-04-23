/*
************************************************************************
*
*   DataHand.c - data structure handling
*
*   Copyright (c) 1994-2000
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/data/SCCS/s.DataHand.c
*   SCCS identification       : 1.60
*
************************************************************************
*/

#include <data_hand.h>

#include <stdio.h>
#include <stdlib.h>
#include <values.h>
#include <math.h>

#include <prop_tab.h>
#include <attr_mng.h>
#include "data_struc.h"
#include "data_res_lib.h"
#include "data_calc.h"
#include "prop_stand.h"

typedef struct {
  DhMolListCB listCB;
  void *clientData;
} ListMolData;

typedef struct {
  DhMolInvalidCB invalidCB;
  void *clientData;
} InvalidMolData;

typedef struct {
  DhAtomInvalidCB invalidCB;
  void *clientData;
} InvalidAtomData;

typedef struct {
  DhBondInvalidCB invalidCB;
  void *clientData;
} InvalidBondData;

typedef struct {
  DhMolP molP;
  DhResP resP;
  DhAngleP angleP;
  int prevI;
  BOOL forward;
  float val;
  Mat4 mat, invMat;
} RotEntry;

static LINLIST MolList = NULL;
static LINLIST BondList = NULL;
static LINLIST AngleList = NULL;
static LINLIST DistList = NULL;
static LINLIST AltCoordList = NULL;

static LINLIST ListMolList = NULL;
static LINLIST InvalidMolList = NULL;
static LINLIST InvalidAtomList = NULL;
static LINLIST InvalidBondList = NULL;

static DhRotFunc RotF;
static BOOL MolListChanged = FALSE;
static BOOL GroupsActive = FALSE;
static BOOL PseudoMode = FALSE;
static BOOL RotMode = FALSE;
static LINLIST RotList = NULL;

void
DhSetAltCoord(BOOL onOff)
{
  DhAltCoordP altP;
  Vec3 x;

  altP = ListFirst(AltCoordList);
  while (altP != NULL) {
    Vec3Copy(x, altP->atomP->coord);
    Vec3Copy(altP->atomP->coord, altP->coord);
    Vec3Copy(altP->coord, x);
    altP = ListNext(AltCoordList, altP);
  }
}

static DhAltCoordP
findAltCoord(DhAtomP atomP)
{
  DhAltCoordP altP;

  altP = ListFirst(AltCoordList);
  while (altP != NULL) {
    if (altP->atomP == atomP)
      return altP;
    altP = ListNext(AltCoordList, altP);
  }

  return NULL;
}

void
DhAtomSetAltCoord(DhAtomP atomP, Vec3 coord)
{
  DhAltCoordP altP;
  struct DhAltCoordS altS;

  altP = findAltCoord(atomP);
  if (coord == NULL) {
    if (altP != NULL)
      ListRemove(AltCoordList, altP);
  } else {
    if (altP == NULL) {
      if (AltCoordList == NULL)
	AltCoordList = ListOpen(sizeof(altS));
      altS.atomP = atomP;
      altP = ListInsertLast(AltCoordList, &altS);
    }
    Vec3Copy(altP->coord, coord);
  }
}

void
DhAddMolListCB(DhMolListCB listCB, void *clientData)
{
  ListMolData entry;

  entry.listCB = listCB;
  entry.clientData = clientData;

  if (ListMolList == NULL)
    ListMolList = ListOpen(sizeof(ListMolData));

  (void) ListInsertLast(ListMolList, &entry);
}

void
DhAddMolInvalidCB(DhMolInvalidCB invalidCB, void *clientData)
{
  InvalidMolData entry;

  entry.invalidCB = invalidCB;
  entry.clientData = clientData;

  if (InvalidMolList == NULL)
    InvalidMolList = ListOpen(sizeof(InvalidMolData));

  (void) ListInsertLast(InvalidMolList, &entry);
}

void
DhRemoveMolListCB(DhMolListCB listCB, void *clientData)
{
  ListMolData *entryP;

  entryP = ListFirst(ListMolList);
  while (entryP != NULL) {
    if (entryP->listCB == listCB && entryP->clientData == clientData) {
      ListRemove(ListMolList, entryP);
      return;
    }
    entryP = ListNext(ListMolList, entryP);
  }
}

void
DhRemoveMolInvalidCB(DhMolInvalidCB invalidCB, void *clientData)
{
  InvalidMolData *entryP;

  entryP = ListFirst(InvalidMolList);
  while (entryP != NULL) {
    if (entryP->invalidCB == invalidCB && entryP->clientData == clientData) {
      ListRemove(InvalidMolList, entryP);
      return;
    }
    entryP = ListNext(InvalidMolList, entryP);
  }
}

void
DhCallMolListCB(void *clientData)
{
  ListMolData *entryP, *nextEntryP;

  if (! MolListChanged)
    return;

  entryP = ListFirst(ListMolList);
  while (entryP != NULL) {
    /* entryP could be removed from callback */
    nextEntryP = ListNext(ListMolList, entryP);
    entryP->listCB(entryP->clientData);
    entryP = nextEntryP;
  }

  MolListChanged = FALSE;
}

void
callMolInvalid(DhMolP molP)
{
  InvalidMolData *entryP, *nextEntryP;

  entryP = ListFirst(InvalidMolList);
  while (entryP != NULL) {
    /* entryP could be removed from callback */
    nextEntryP = ListNext(InvalidMolList, entryP);
    entryP->invalidCB(molP, entryP->clientData);
    entryP = nextEntryP;
  }
}

void
DhAddAtomInvalidCB(DhAtomInvalidCB invalidCB, void *clientData)
{
  InvalidAtomData entry;

  entry.invalidCB = invalidCB;
  entry.clientData = clientData;

  if (InvalidAtomList == NULL)
    InvalidAtomList = ListOpen(sizeof(InvalidAtomData));

  (void) ListInsertLast(InvalidAtomList, &entry);
}

void
DhRemoveAtomInvalidCB(DhAtomInvalidCB invalidCB, void *clientData)
{
  InvalidAtomData *entryP;

  entryP = ListFirst(InvalidAtomList);
  while (entryP != NULL) {
    if (entryP->invalidCB == invalidCB && entryP->clientData == clientData) {
      ListRemove(InvalidAtomList, entryP);
      return;
    }
    entryP = ListNext(InvalidAtomList, entryP);
  }
}

void
atomInvalid(DhAtomP atomA, int num)
{
  InvalidAtomData *entryP, *nextEntryP;
  DhAltCoordP altP;
  int i;

  entryP = ListFirst(InvalidAtomList);
  while (entryP != NULL) {
    /* entryP could be removed from callback */
    nextEntryP = ListNext(InvalidAtomList, entryP);
    entryP->invalidCB(atomA, atomA + num - 1, entryP->clientData);
    entryP = nextEntryP;
  }

  for (i = 0; i < num; i++) {
    altP = findAltCoord(atomA + i);
    if (altP != NULL)
      ListRemove(AltCoordList, altP);
  }
}

void
DhAddBondInvalidCB(DhBondInvalidCB invalidCB, void *clientData)
{
  InvalidBondData entry;

  entry.invalidCB = invalidCB;
  entry.clientData = clientData;

  if (InvalidBondList == NULL)
    InvalidBondList = ListOpen(sizeof(InvalidBondData));

  (void) ListInsertLast(InvalidBondList, &entry);
}

void
DhRemoveBondInvalidCB(DhBondInvalidCB invalidCB, void *clientData)
{
  InvalidBondData *entryP;

  entryP = ListFirst(InvalidBondList);
  while (entryP != NULL) {
    if (entryP->invalidCB == invalidCB && entryP->clientData == clientData) {
      ListRemove(InvalidBondList, entryP);
      return;
    }
    entryP = ListNext(InvalidBondList, entryP);
  }
}

void
bondInvalid(DhBondP bondP)
{
  InvalidBondData *entryP, *nextEntryP;

  entryP = ListFirst(InvalidBondList);
  while (entryP != NULL) {
    /* entryP could be removed from callback */
    nextEntryP = ListNext(InvalidBondList, entryP);
    entryP->invalidCB(bondP, entryP->clientData);
    entryP = nextEntryP;
  }
}

void
DhDestroyAll(void)
{
  ListClose(MolList);
  MolList = NULL;
  MolListChanged = TRUE;

  ListClose(BondList);
  BondList = NULL;

  ListClose(AngleList);
  AngleList = NULL;

  ListClose(DistList);
  DistList = NULL;

  ListClose(RotList);
  RotList = NULL;
  RotMode = FALSE;

  ListClose(AltCoordList);
  AltCoordList = NULL;

  DhResLibDestroyAll();

  PropStandDestroyAll();
}

static void
freeResBond(DhResP resP, LINLIST bondL)
/* search and destroy all bonds to the given residue in the list */
{
  DhBondAddP bondAddP, nextBondAddP;

  bondAddP = ListFirst(bondL);
  while (bondAddP != NULL) {
    nextBondAddP = ListNext(bondL, bondAddP);
    if (bondAddP->res1P == resP || bondAddP->res2P == resP)
      DhBondDestroy((DhBondP) bondAddP);
    bondAddP = nextBondAddP;
  }
}

static void
freeResDist(DhResP resP, LINLIST distL)
/* search and destroy all distances to the given residue in the list */
{
  DhDistP distP, nextDistP;

  distP = ListFirst(distL);
  while (distP != NULL) {
    nextDistP = ListNext(distL, distP);
    if (distP->res1P == resP || distP->res2P == resP)
      DhDistDestroy(distP);
    distP = nextDistP;
  }
}

static void
freeBond(void *p, void *clientData)
{
  DhBondP bondP = p;

  bondInvalid(bondP);

  PropFreeTab(bondP->propTab);
  AttrReturn(bondP->attrP);
}

static void
freeRes(void *p, void *clientData)
{
  DhResP resP = p;
  RotEntry *rotP, *nextRotP;
  DhAngleAddP angleP, nextAngleP;
  DhResDefP defP = resP->defP;
  int i;

  atomInvalid(resP->atomA, defP->atomNo);

  freeResBond(resP, resP->molP->bondL);
  freeResBond(resP, BondList);

  angleP = ListFirst(AngleList);
  while (angleP != NULL) {
    nextAngleP = ListNext(AngleList, angleP);
    if (angleP->res1P == resP || angleP->res2P == resP ||
	angleP->res3P == resP || angleP->res4P == resP)
      ListRemove(AngleList, angleP);
    angleP = nextAngleP;
  }

  freeResDist(resP, resP->molP->distL);
  freeResDist(resP, DistList);

  rotP = ListFirst(RotList);
  while (rotP != NULL) {
    nextRotP = ListNext(RotList, rotP);
    if (rotP->resP == resP)
      ListRemove(RotList, rotP);
    rotP = nextRotP;
  }

  if (resP->neighLeftP != NULL)
    resP->neighLeftP->neighRightP = NULL;
  if (resP->neighRightP != NULL)
    resP->neighRightP->neighLeftP = NULL;

  for (i = 0; i < defP->atomNo; i++) {
    PropFreeTab(resP->atomA[i].propTab);
    AttrReturn(resP->atomA[i].attrP);
  }

  for (i = defP->firstBondI; i <= defP->lastBondI; i++)
    freeBond(resP->bondA + i, NULL);

  for (i = 0; i < defP->angleNo; i++)
    PropFreeTab(resP->angleA[i].propTab);

  free(resP->atomA);
  free(resP->bondA);
  free(resP->angleA);

  DhResDefReturn(defP);

  PropFreeTab(resP->propTab);
}

static void
freeAngleAdd(void *p, void *clientData)
{
  DhAngleAddP angleAddP = p;

  DStrFree(angleAddP->name);
}

static void
freeDist(void *p, void *clientData)
{
  DhDistP distP = p;

  PropFreeTab(distP->propTab);
  AttrReturn(distP->attrP);
}

static void
removeMolFromGroup(DhMolP molP)
{
  if (molP->groupP == NULL)
    return;
  
  if (molP->groupP->prevP != NULL)
    molP->groupP->prevP->groupP->nextP = molP->groupP->nextP;
  
  if (molP->groupP->nextP != NULL)
    molP->groupP->nextP->groupP->prevP = molP->groupP->prevP;
}

static void
freeMol(void *p, void *clientData)
{
  DhMolP molP = p;

  callMolInvalid(molP);

  if (molP->groupP != NULL) {
    removeMolFromGroup(molP);
    free(molP->groupP);
  }

  DStrFree(molP->name);
  ListClose(molP->resL);
  if (molP->bondL != NULL)
    ListClose(molP->bondL);
  ListClose(molP->distL);
  PropFreeTab(molP->propTab);
  MolAttrReturn(molP->attrP);
}

DhMolP
DhMolNew(void)
{
  struct DhMolS molS, *prevMolP;

  if (MolList == NULL) {
    MolList = ListOpen(sizeof(struct DhMolS));
    ListAddDestroyCB(MolList, freeMol, NULL, NULL);
    /* Numbering of molecules starts at 0 so that it can be
       used as index into the list by dump etc. We'll add 1
       to the number for the user. */
    molS.num = 0;
  } else if (ListSize(MolList) == 0) {
    molS.num = 0;
  } else {
    prevMolP = ListLast(MolList);
    molS.num = prevMolP->num + 1;
  }

  molS.name = DStrNew();
  molS.groupP = NULL;
  molS.resL = ListOpen(sizeof(struct DhResS));
  ListAddDestroyCB(molS.resL, freeRes, NULL, NULL);
  molS.bondL = NULL;
  molS.distL = ListOpen(sizeof(struct DhDistS));
  ListAddDestroyCB(molS.distL, freeDist, NULL, NULL);
  Mat4Ident(molS.rotMat);
  Vec3Zero(molS.rotPoint);
  Vec3Zero(molS.transVect);
  molS.propTab = PropNewTab(FALSE);
  molS.attrP = MolAttrGet();

  MolListChanged = TRUE;

  return ListInsertLast(MolList, &molS);
}

DhMolP
DhMolCopy(DhMolP molP)
{
  DhMolP newMolP;
  DhResP resP, newResP;
  DhResDefP defP;
  struct DhResS newResS;
  DhBondAddP bondP, newBondP;
  struct DhBondAddS newBondS;
  DhDistP distP, newDistP;
  struct DhDistS newDistS;
  int i;

  newMolP = DhMolNew();

  DStrAssignDStr(newMolP->name, molP->name);
  DStrAppStr(newMolP->name, ".copy");

  resP = ListFirst(molP->resL);
  while (resP != NULL) {
    defP = resP->defP;

    newResP = ListInsertLast(newMolP->resL, &newResS);
    *newResP = *resP;

    newResP->molP = newMolP;
    newResP->defP = DhResDefGetRef(defP);

    newResP->atomA = malloc(defP->atomNo * sizeof(*newResP->atomA));
    newResP->bondA = malloc(defP->bondNo * sizeof(*newResP->bondA));
    newResP->angleA = malloc(defP->angleNo * sizeof(*newResP->angleA));

    for (i = 0; i < defP->atomNo; i++) {
      newResP->atomA[i] = resP->atomA[i];
      newResP->atomA[i].resP = newResP;
      newResP->atomA[i].propTab = PropGet(resP->atomA[i].propTab);
      newResP->atomA[i].attrP = AttrGet(resP->atomA[i].attrP);
    }

    for (i = 0; i < defP->bondNo; i++)
      newResP->bondA[i].resP = newResP;

    for (i = defP->firstBondI; i <= defP->lastBondI; i++) {
      newResP->bondA[i].propTab = PropGet(resP->bondA[i].propTab);
      newResP->bondA[i].attrP = AttrGet(resP->bondA[i].attrP);
    }

    for (i = 0; i < defP->angleNo; i++) {
      newResP->angleA[i] = resP->angleA[i];
      newResP->angleA[i].resP = newResP;
      newResP->angleA[i].propTab = PropGet(resP->angleA[i].propTab);
    }

    newResP->propTab = PropGet(resP->propTab);

    resP = ListNext(molP->resL, resP);
  }

  resP = ListFirst(molP->resL);
  newResP = ListFirst(newMolP->resL);
  while (resP != NULL) {
    if (resP->neighLeftP == NULL)
      newResP->neighLeftP = NULL;
    else
      newResP->neighLeftP = DhResFind(newMolP, resP->neighLeftP->num);

    if (resP->neighRightP == NULL)
      newResP->neighRightP = NULL;
    else
      newResP->neighRightP = DhResFind(newMolP, resP->neighRightP->num);

    resP = ListNext(molP->resL, resP);
    newResP = ListNext(newMolP->resL, newResP);
  }

  if (molP->bondL != NULL)
    newMolP->bondL = ListOpen(sizeof(struct DhBondAddS));

  bondP = ListFirst(molP->bondL);
  while (bondP != NULL) {
    newBondP = ListInsertLast(newMolP->bondL, &newBondS);
    *newBondP = *bondP;

    newBondP->bondS.propTab = PropGet(bondP->bondS.propTab);
    newBondP->bondS.attrP = AttrGet(bondP->bondS.attrP);

    newBondP->res1P = DhResFind(newMolP, bondP->res1P->num);
    newBondP->res2P = DhResFind(newMolP, bondP->res2P->num);

    bondP = ListNext(molP->bondL, bondP);
  }

  distP = ListFirst(molP->distL);
  while (distP != NULL) {
    newDistP = ListInsertLast(newMolP->distL, &newDistS);
    *newDistP = *distP;

    newDistP->res1P = DhResFind(newMolP, distP->res1P->num);
    newDistP->res2P = DhResFind(newMolP, distP->res2P->num);

    newDistP->propTab = PropGet(distP->propTab);
    newDistP->attrP = AttrGet(distP->attrP);

    distP = ListNext(molP->distL, distP);
  }

  Mat4Copy(newMolP->rotMat, molP->rotMat);
  Vec3Copy(newMolP->rotPoint, molP->rotPoint);
  Vec3Copy(newMolP->transVect, molP->transVect);

  PropFreeTab(newMolP->propTab);
  newMolP->propTab = PropGet(molP->propTab);

  return newMolP;
}

static void
renumMol(void)
{
  DhMolP molP;
  int ind;

  molP = ListFirst(MolList);
  ind = 0;
  while (molP != NULL) {
    molP->num = ind;
    molP = ListNext(MolList, molP);
    ind++;
  }
}

void
DhMolMoveFirst(DhMolP molP)
{
  ListMoveFirst(MolList, molP);
  MolListChanged = TRUE;

  renumMol();
}

void
DhMolDestroy(DhMolP molP)
{
  ListRemove(MolList, molP);
  MolListChanged = TRUE;

  renumMol();
}

static DhAtomP
getEquivAtom(DhAtomP atomP)
{
  DhResP resP, prevResP, nextResP;
  int ind, i;

  resP = atomP->resP;
  ind = atomP - resP->atomA;

  if (ind < EQUIV_NO) {
    /* possible overlap with previous residue */
    if (resP->equivI[ind] >= 0) {
      prevResP = ListPrev(resP->molP->resL, resP);
      return prevResP->atomA + resP->equivI[ind];
    }
  } else {
    /* possible overlap with next residue */
    nextResP = ListNext(resP->molP->resL, resP);
    if (nextResP != NULL) {
      for (i = 0; i < EQUIV_NO; i++)
	if (nextResP->equivI[i] == ind)
	  return nextResP->atomA + i;
    }
  }

  return NULL;
}

static void
initAtom(DhAtomP atomP, DhResP resP)
{
  atomP->resP = resP;
  Vec3Zero(atomP->coord);
  atomP->state = AS_UNKNOWN_COORD;
#ifdef NMR
  atomP->shiftI = 0;
  atomP->shift = DH_SHIFT_UNKNOWN;
#endif
  atomP->bFactor = 0.0f;
  /* Atoms are invisible by default. It's not nice that we
     assume that here, but it makes things easier. */
  atomP->propTab = PropNewTab(FALSE);
  atomP->attrP = AttrGetInit();
}

static void
initBond(DhBondP bondP, DhResP resP)
{
  bondP->resP = resP;
  bondP->propTab = PropNewTab(TRUE);
  bondP->attrP = AttrGetInit();
}

static void
setEquivI(DhResP resP, DhResP prevResP)
{
  DSTR name;
  int overlapNo, i, k;

  if (resP == NULL)
    return;

  for (i = 0; i < EQUIV_NO; i++)
    resP->equivI[i] = -1;

  if (prevResP == NULL)
    return;

  overlapNo = resP->defP->firstAtomI +
      prevResP->defP->atomNo - (prevResP->defP->lastAtomI + 1);
  if (overlapNo > EQUIV_NO)
    overlapNo = EQUIV_NO;
  if (overlapNo > resP->defP->atomNo)
    overlapNo = resP->defP->atomNo;

  for (i = 0; i < overlapNo; i++) {
    name = resP->defP->atomA[i].name;
    for (k = prevResP->defP->atomNo - 1; k >= EQUIV_NO; k--)
      if (DStrCmp(name, prevResP->defP->atomA[k].name) == 0) {
	resP->equivI[i] = k;
	break;
      }
  }
}

static void
copyAtomData(DhAtomP atom1P, DhAtomP atom2P)
{
  if (atom2P->state == AS_VALID)
    Vec3Copy(atom1P->coord, atom2P->coord);

#ifdef NMR
  atom1P->shiftI = atom2P->shiftI;
  atom1P->shift = atom2P->shift;
#endif
  atom1P->bFactor = atom2P->bFactor;
    atom1P->state = atom2P->state;
}

static void
initRes(DhResP resP)
{
  DhResDefP defP;
  DhResP prevResP, nextResP;
  DhAtomP atomP, equivAtomP;
  int i;

  defP = resP->defP;

  resP->neighLeftP = NULL;
  resP->neighRightP = NULL;

  resP->atomA = malloc(defP->atomNo * sizeof(*resP->atomA));
  resP->bondA = malloc(defP->bondNo * sizeof(*resP->bondA));
  resP->angleA = malloc(defP->angleNo * sizeof(*resP->angleA));

  for (i = 0; i < defP->atomNo; i++)
    initAtom(resP->atomA + i, resP);

  for (i = defP->firstBondI; i <= defP->lastBondI; i++)
    initBond(resP->bondA + i, resP);

  for (i = 0; i < defP->angleNo; i++) {
    resP->angleA[i].resP = resP;
    resP->angleA[i].val = 0.0f;
    resP->angleA[i].minVal = DH_ANGLE_MIN;
    resP->angleA[i].maxVal = DH_ANGLE_MAX;
    resP->angleA[i].changed = FALSE;
    resP->angleA[i].propTab = PropNewTab(FALSE);
  }

  resP->propTab = PropNewTab(FALSE);

  prevResP = ListPrev(resP->molP->resL, resP);
  nextResP = ListNext(resP->molP->resL, resP);

  setEquivI(resP, prevResP);
  setEquivI(nextResP, resP);

  if (prevResP != NULL) {
    /* copy coordinates from equivalent atoms of previous residue */
    for (i = 0; i < EQUIV_NO; i++) {
      if (resP->equivI[i] < 0)
	continue;

      atomP = resP->atomA + i;
      equivAtomP = prevResP->atomA + resP->equivI[i];

      copyAtomData(atomP, equivAtomP);
    }
  }

  if (nextResP != NULL) {
    /* copy coordinates from equivalent atoms of next residue */
    for (i = 0; i < EQUIV_NO; i++) {
      if (nextResP->equivI[i] < 0)
	continue;

      equivAtomP = nextResP->atomA + i;
      atomP = resP->atomA + nextResP->equivI[i];

      copyAtomData(atomP, equivAtomP);
    }
  }
}

DhResP
DhResNew(DhMolP molP, DhResDefP defP, DhSeqPos pos)
{
  struct DhResS resS;
  DhResP resP, prevResP, nextResP;

  resS.molP = molP;
  resS.defP = defP;

  if (pos == SP_FIRST) {
    nextResP = ListFirst(molP->resL);
    if (nextResP == NULL)
      resS.num = 1;
    else
      resS.num = nextResP->num - 1;

    resP = ListInsertFirst(molP->resL, &resS);
  } else {
    prevResP = ListLast(molP->resL);
    if (prevResP == NULL)
      resS.num = 1;
    else
      resS.num = prevResP->num + 1;

    resP = ListInsertLast(molP->resL, &resS);
  }

  initRes(resP);

  return resP;
}

void
DhResMutate(DhResP resP, DhResDefP defP, DhMutationKind kind)
{
  struct DhResS oldResS;
  DhAngleP oldAngleP;
  DhAtomP oldAtomP;
  DhResP nextResP;
  int startI, endI, i, k;

  oldResS = *resP;
  resP->defP = defP;
  initRes(resP);

  if (kind == MK_KEEP_ANGLES)
    DhResStandGeom(resP);

  for (i = 0; i < defP->atomNo; i++) {
    oldAtomP = NULL;

    if (i < defP->firstAtomI) {
      startI = 0;
      endI = oldResS.defP->firstAtomI - 1;
    } else if (i <= defP->lastAtomI) {
      startI = oldResS.defP->firstAtomI;
      endI = oldResS.defP->lastAtomI;
    } else {
      startI = oldResS.defP->lastAtomI + 1;
      endI = oldResS.defP->atomNo - 1;
    }

    for (k = startI; k <= endI; k++)
      if (oldResS.atomA[k].state != AS_DELETED &&
	  DStrCmp(oldResS.defP->atomA[k].name, defP->atomA[i].name) == 0) {
	oldAtomP = oldResS.atomA + k;
	break;
      }

    if (oldAtomP != NULL) {
      if (kind == MK_KEEP_ATOMS && oldAtomP->state == AS_VALID)
	Vec3Copy(resP->atomA[i].coord, oldAtomP->coord);
      resP->atomA[i].state = oldAtomP->state;
#ifdef NMR
      resP->atomA[i].shiftI = oldAtomP->shiftI;
      resP->atomA[i].shift = oldAtomP->shift;
#endif
      resP->atomA[i].bFactor = oldAtomP->bFactor;
      PropFreeTab(resP->atomA[i].propTab);
      resP->atomA[i].propTab = PropGet(oldAtomP->propTab);
      AttrReturn(resP->atomA[i].attrP);
      resP->atomA[i].attrP = AttrGet(oldAtomP->attrP);
    }
  }

  for (i = 0; i < defP->angleNo; i++) {
    oldAngleP = DhAngleFind(&oldResS, defP->angleA[i].name);
    if (oldAngleP != NULL) {
      resP->angleA[i].val = oldAngleP->val;
      resP->angleA[i].minVal = oldAngleP->minVal;
      resP->angleA[i].maxVal = oldAngleP->maxVal;
      resP->angleA[i].changed = TRUE;
      PropFreeTab(resP->angleA[i].propTab);
      resP->angleA[i].propTab = PropGet(oldAngleP->propTab);
    }
  }

  if (kind == MK_KEEP_ANGLES) {
    DhResAnglesChanged(resP);
    DhResDockPrev(resP);
    nextResP = ListNext(resP->molP->resL, resP);
    if (nextResP != NULL)
      DhResDockPrev(nextResP);
  } else {
    DhMolCoordsChanged(resP->molP);
  }

  freeRes(&oldResS, NULL);
}

void
DhResSetNeigh(DhResP resP, DhNeighChoice nc, DhResP neighResP)
{
  if (neighResP != NULL && resP->molP != neighResP->molP)
    return;

  if (nc == NC_LEFT) {
    /* break previous connections of both residues */
    if (resP->neighLeftP != NULL)
      resP->neighLeftP->neighRightP = NULL;
    if (neighResP != NULL && neighResP->neighRightP != NULL)
      neighResP->neighRightP->neighLeftP = NULL;

    resP->neighLeftP = neighResP;
    if (neighResP != NULL)
      neighResP->neighRightP = resP;
  } else {
    if (resP->neighRightP != NULL)
      resP->neighRightP->neighLeftP = NULL;
    if (neighResP != NULL && neighResP->neighLeftP != NULL)
      neighResP->neighLeftP->neighRightP = NULL;

    resP->neighRightP = neighResP;
    if (neighResP != NULL)
      neighResP->neighLeftP = resP;
  }
}

BOOL
DhResDestroy(DhResP resP)
{
  /* can only destroy first or last residue of molecule! */
  if (resP == ListFirst(resP->molP->resL)) {
    ListRemove(resP->molP->resL, resP);
    setEquivI(ListFirst(resP->molP->resL), NULL);
    return TRUE;
  } else if (resP == ListLast(resP->molP->resL)) {
    ListRemove(resP->molP->resL, resP);
    return TRUE;
  } else {
    return FALSE;
  }
}

static void
updateAtomBond(DhResP resP, int ind, int inc, LINLIST bondL)
/* update atom indices in given bond list */
{
  DhBondAddP bondAddP;

  bondAddP = ListFirst(bondL);
  while (bondAddP != NULL) {
    if (bondAddP->res1P == resP && bondAddP->atom1I >= ind)
      bondAddP->atom1I += inc;
    if (bondAddP->res2P == resP && bondAddP->atom2I >= ind)
      bondAddP->atom2I += inc;
    bondAddP = ListNext(bondL, bondAddP);
  }
}

static void
updateAtomDist(DhResP resP, int ind, int inc, LINLIST distL)
/* update atom indices in given dist list */
{
  DhDistP distP;

  distP = ListFirst(distL);
  while (distP != NULL) {
    if (distP->res1P == resP && distP->atom1I >= ind)
      distP->atom1I += inc;
    if (distP->res2P == resP && distP->atom2I >= ind)
      distP->atom2I += inc;
    distP = ListNext(distL, distP);
  }
}

static void
updateAtomAngle(DhResP resP, int ind, int inc, LINLIST angleL)
/* update atom indices in given angle list */
{
  DhAngleAddP angleAddP;

  angleAddP = ListFirst(angleL);
  while (angleAddP != NULL) {
    if (angleAddP->res1P == resP && angleAddP->atom1I >= ind)
      angleAddP->atom1I += inc;
    if (angleAddP->res2P == resP && angleAddP->atom2I >= ind)
      angleAddP->atom2I += inc;
    if (angleAddP->res3P == resP && angleAddP->atom3I >= ind)
      angleAddP->atom3I += inc;
    if (angleAddP->res4P == resP && angleAddP->atom4I >= ind)
      angleAddP->atom4I += inc;
    angleAddP = ListNext(angleL, angleAddP);
  }
}

DhAtomP
DhAtomNew(DhResP resP, DSTR name)
{
  int ind, i;
  DhResP nextResP;
  DhAtomP atomP;

  ind = resP->defP->lastAtomI + 1;

  nextResP = ListNext(resP->molP->resL, resP);
  if (nextResP != NULL)
    for (i = 0; i < EQUIV_NO; i++)
      if (nextResP->equivI[i] >= 0 && nextResP->equivI[i] < ind)
	ind = nextResP->equivI[i];

  ind = DhResDefAddAtom(&resP->defP, ind, name);
  if (ind == -1)
    return NULL;

  if (resP->defP->atomNo == 1) {
    resP->atomA = malloc(sizeof(*resP->atomA));
  } else {
    atomInvalid(resP->atomA, resP->defP->atomNo - 1);
    resP->atomA = realloc(resP->atomA,
	resP->defP->atomNo * sizeof(*resP->atomA));
    for (i = resP->defP->atomNo - 1; i > ind; i--)
      resP->atomA[i] = resP->atomA[i - 1];
  }

  atomP = resP->atomA + ind;
  initAtom(atomP, resP);

  if (ind < EQUIV_NO)
    setEquivI(resP, ListPrev(resP->molP->resL, resP));

  if (nextResP != NULL)
    setEquivI(nextResP, resP);
    
  if (ind < resP->defP->atomNo - 1) {
    updateAtomBond(resP, ind, 1, resP->molP->bondL);
    updateAtomBond(resP, ind, 1, BondList);
    updateAtomDist(resP, ind, 1, resP->molP->distL);
    updateAtomDist(resP, ind, 1, DistList);
    updateAtomAngle(resP, ind, 1, AngleList);
  }

  return atomP;
}

DhAtomP
DhAtomNewPseudo(DhAtomP atomPA[], int atomNo, DSTR name)
{
  DhResP resP;
  int *indA, pseudoI;
  DhAtomP pseudoP;
  int i;

  resP = atomPA[0]->resP;
  for (i = 1; i < atomNo; i++)
    if (atomPA[i]->resP != resP)
      return NULL;

  indA = malloc(atomNo * sizeof(*indA));
  for (i = 0; i < atomNo; i++)
    indA[i] = atomPA[i] - resP->atomA;

  pseudoP = DhAtomNew(resP, name);
  if (pseudoP == NULL) {
    free(indA);
    return NULL;
  }

  pseudoI = pseudoP - resP->atomA;
  for (i = 0; i < atomNo; i++)
    if (indA[i] >= pseudoI)
      indA[i]++;

  DhResDefSetPseudo(&resP->defP, pseudoI, indA, atomNo);

  free(indA);

  DhCalcPseudo(resP);

  return pseudoP;
}

void
DhAtomDestroy(DhAtomP atomP)
{
  atomInvalid(atomP, 1);
  atomP->state = AS_DELETED;

  atomP = getEquivAtom(atomP);
  if (atomP != NULL) {
    atomInvalid(atomP, 1);
    atomP->state = AS_DELETED;
  }
}

static BOOL
bondInList(LINLIST bondL, DhBondAddP newP)
{
  DhResP resP;
  int atomI;
  DhBondAddP oldP;

  if (newP->res1P->num > newP->res2P->num) {
    resP = newP->res1P;
    newP->res1P = newP->res2P;
    newP->res2P = resP;

    atomI = newP->atom1I;
    newP->atom1I = newP->atom2I;
    newP->atom2I = atomI;
  }

  oldP = ListFirst(bondL);
  while (oldP != NULL) {
    if (newP->res1P == oldP->res1P &&
	newP->res2P == oldP->res2P &&
	newP->atom1I == oldP->atom1I &&
	newP->atom2I == oldP->atom2I)
      return TRUE;
    oldP = ListNext(bondL, oldP);
  }

  return FALSE;
}

DhBondP
DhBondNew(DhAtomP atom1P, DhAtomP atom2P)
{
  int atom1I, atom2I;
  DhAtomP eAtomP;
  struct DhBondAddS bondAddS;
  LINLIST bondL;

  atom1I = atom1P - atom1P->resP->atomA;
  atom2I = atom2P - atom2P->resP->atomA;

  if (atom1P->resP == atom2P->resP)
    return DhBondNewIndex(atom1P->resP, atom1I, atom2I);

  eAtomP = getEquivAtom(atom2P);
  if (eAtomP != NULL && eAtomP->resP == atom1P->resP) {
    atom2P = eAtomP;
    atom2I = atom2P - atom2P->resP->atomA;
    return DhBondNewIndex(atom1P->resP, atom1I, atom2I);
  }

  bondAddS.res1P = atom1P->resP;
  bondAddS.res2P = atom2P->resP;
  bondAddS.atom1I = atom1I;
  bondAddS.atom2I = atom2I;

  if (bondAddS.res1P->molP == bondAddS.res2P->molP) {
    bondL = bondAddS.res1P->molP->bondL;
    if (bondL == NULL) {
      bondL = ListOpen(sizeof(struct DhBondAddS));
      ListAddDestroyCB(bondL, freeBond, NULL, NULL);
      bondAddS.res1P->molP->bondL = bondL;
    }
  } else {
    if (BondList == NULL) {
      BondList = ListOpen(sizeof(struct DhBondAddS));
      ListAddDestroyCB(BondList, freeBond, NULL, NULL);
    }
    bondL = BondList;
  }

  if (bondInList(bondL, &bondAddS))
    return NULL;

  initBond((DhBondP) &bondAddS, NULL);

  return ListInsertLast(bondL, &bondAddS);
}

DhBondP
DhBondNewIndex(DhResP resP, int atom1I, int atom2I)
{
  int ind, i;
  DhBondP bondP;

  if (atom1I > atom2I) {
    ind = atom1I;
    atom1I = atom2I;
    atom2I = ind;
  }

  ind = DhResDefAddBond(&resP->defP, atom1I, atom2I);
  if (ind == -1)
    return NULL;

  if (resP->defP->bondNo == 1) {
    resP->bondA = malloc(sizeof(*resP->bondA));
  } else {
    resP->bondA = realloc(resP->bondA,
	resP->defP->bondNo * sizeof(*resP->bondA));
    for (i = resP->defP->bondNo - 1; i > ind; i--)
      resP->bondA[i] = resP->bondA[i - 1];
  }

  bondP = resP->bondA + ind;
  initBond(bondP, resP);

  return bondP;
}

void
DhBondDestroy(DhBondP bondP)
{
  DhResP resP;
  DhBondAddP bondAddP = (DhBondAddP) bondP;
  int ind, i;

  if (bondP->resP != NULL) {
    resP = bondP->resP;
    ind = bondP - resP->bondA;
    freeBond(bondP, NULL);
    for (i = ind + 1; i < resP->defP->bondNo; i++)
      resP->bondA[i - 1] = resP->bondA[i];
    DhResDefRemoveBond(&resP->defP, ind);
    return;
  }

  if (bondAddP->res1P->molP == bondAddP->res2P->molP)
    ListRemove(bondAddP->res1P->molP->bondL, bondAddP);
  else
    ListRemove(BondList, bondAddP);
}

BOOL
DhAngleNew(DhAtomP atom1P, DhAtomP atom2P, DhAtomP atom3P, DhAtomP atom4P,
    DSTR name)
{
  DhResP resP, prevResP, nextResP;
  struct DhAngleAddS angleAddS;
  DhAngleAddP angleAddP;

  resP = atom2P->resP;
  prevResP = ListPrev(resP->molP->resL, resP);
  nextResP = ListNext(resP->molP->resL, resP);

  if (atom1P->resP != resP && atom1P->resP != prevResP)
    return FALSE;

  if (atom3P->resP != resP)
    return FALSE;

  if (atom4P->resP != resP && atom4P->resP != nextResP)
    return FALSE;

  angleAddS.res1P = atom1P->resP;
  angleAddS.res2P = atom2P->resP;
  angleAddS.res3P = atom3P->resP;
  angleAddS.res4P = atom4P->resP;
  angleAddS.atom1I = atom1P - atom1P->resP->atomA;
  angleAddS.atom2I = atom2P - atom2P->resP->atomA;
  angleAddS.atom3I = atom3P - atom3P->resP->atomA;
  angleAddS.atom4I = atom4P - atom4P->resP->atomA;

  angleAddP = ListFirst(AngleList);
  while (angleAddP != NULL) {
    if (angleAddP->res1P == angleAddS.res1P &&
	angleAddP->res2P == angleAddS.res2P &&
	angleAddP->res3P == angleAddS.res3P &&
	angleAddP->res4P == angleAddS.res4P &&
	angleAddP->atom1I == angleAddS.atom1I &&
	angleAddP->atom2I == angleAddS.atom2I &&
	angleAddP->atom3I == angleAddS.atom3I &&
	angleAddP->atom4I == angleAddS.atom4I)
      return TRUE;
    angleAddP = ListNext(AngleList, angleAddP);
  }

  angleAddS.name = DStrNew();
  DStrAssignDStr(angleAddS.name, name);

  if (AngleList == NULL) {
    AngleList = ListOpen(sizeof(struct DhAngleAddS));
    ListAddDestroyCB(AngleList, freeAngleAdd, NULL, NULL);
  }

  (void) ListInsertLast(AngleList, &angleAddS);

  return TRUE;
}

void
DhAngleDestroy(DhAngleP angleP)
{
  DhResP resP;
  RotEntry *rotP, *nextRotP;
  int ind, i;

  resP = angleP->resP;
  ind = angleP - resP->angleA;
  for (i = ind + 1; i < resP->defP->angleNo; i++)
    resP->angleA[i - 1] = resP->angleA[i];
  DhResDefRemoveAngle(&resP->defP, ind);

  rotP = ListFirst(RotList);
  while (rotP != NULL) {
    nextRotP = ListNext(RotList, rotP);
    if (rotP->angleP == angleP)
      ListRemove(RotList, rotP);
    rotP = nextRotP;
  }
}

DhDistP
DhDistNew(DhAtomP atom1P, DhAtomP atom2P)
{
  struct DhDistS distS;
  LINLIST distL;

  distS.kind = DK_ACTUAL;
  distS.res1P = atom1P->resP;
  distS.res2P = atom2P->resP;
  distS.atom1I = atom1P - distS.res1P->atomA;
  distS.atom2I = atom2P - distS.res2P->atomA;
  distS.limit = 0.0f;
  distS.propTab = PropNewTab(TRUE);
  distS.attrP = AttrGetInit();

  DhDistCalc(&distS);

  if (distS.res1P->molP == distS.res2P->molP) {
    distL = distS.res1P->molP->distL;
  } else {
    if (DistList == NULL) {
      DistList = ListOpen(sizeof(struct DhDistS));
      ListAddDestroyCB(DistList, freeDist, NULL, NULL);
    }
    distL = DistList;
  }

  return ListInsertLast(distL, &distS);
}

void
DhDistDestroy(DhDistP distP)
{
  if (distP->res1P->molP == distP->res2P->molP)
    ListRemove(distP->res1P->molP->distL, distP);
  else
    ListRemove(DistList, distP);
}

static void
recalcDist(DhMolP molP)
{
  DhDistP distP;

  distP = ListFirst(DistList);
  while (distP != NULL) {
    if (distP->res1P->molP == molP || distP->res2P->molP == molP)
      DhDistCalc(distP);
    distP = ListNext(DistList, distP);
  }
}

void
DhMakeGroup(DhMolP molPA[], int molNo)
{
  int molI;

  for (molI = 0; molI < molNo; molI++)
    if (molPA[molI]->groupP == NULL)
      molPA[molI]->groupP = malloc(sizeof(*molPA[molI]->groupP));
    else
      removeMolFromGroup(molPA[molI]);

  for (molI = 0; molI < molNo; molI++) {
    if (molI == 0)
      molPA[molI]->groupP->prevP = NULL;
    else
      molPA[molI]->groupP->prevP = molPA[molI - 1];

    if (molI == molNo - 1)
      molPA[molI]->groupP->nextP = NULL;
    else
      molPA[molI]->groupP->nextP = molPA[molI + 1];
  }
}

static void
groupsOn(void)
{
  DhMolP headP, molP;
  int groupNum;
  Mat4 rotInv;
  Vec3 t1, t2, v3;
  Vec4 v4;
  DhResP resP;
  int offs, minNum, maxNum;

  headP = ListFirst(MolList);
  groupNum = 0;
  while (headP != NULL) {
    if (headP->groupP == NULL || headP->groupP->prevP != NULL) {
      headP = ListNext(MolList, headP);
      continue;
    }

    headP->groupP->mapMod = FALSE;

    if (headP->groupP->nextP == NULL) {
      headP->groupP->num = -1;
      headP = ListNext(MolList, headP);
      continue;
    }

    headP->groupP->num = groupNum;

    Mat4Copy(rotInv, headP->rotMat);
    Mat4Transp(rotInv);

    Vec3To4(v4, headP->rotPoint);
    Mat4VecMult(v4, headP->rotMat);
    Vec4To3(v3, v4);
    Vec3Copy(t1, headP->transVect);
    Vec3Sub(t1, v3);

    offs = 0;
    resP = ListLast(headP->resL);
    maxNum = resP->num;

    molP = headP->groupP->nextP;
    for (;;) {
      molP->groupP->headP = headP;
      molP->groupP->num = groupNum;

      Mat4Copy(molP->groupP->rotMat, rotInv);
      Mat4Mult(molP->groupP->rotMat, molP->rotMat);

      Vec3To4(v4, molP->rotPoint);
      Mat4VecMult(v4, molP->rotMat);
      Vec4To3(v3, v4);
      Vec3Copy(t2, molP->transVect);
      Vec3Sub(t2, v3);

      Vec3Sub(t2, t1);
      Vec3To4(v4, t2);
      Mat4VecMult(v4, rotInv);
      Vec4To3(molP->groupP->transVect, v4);

      resP = ListFirst(molP->resL);
      if (resP == NULL)
	minNum = 1;
      else
	minNum = resP->num;

      if (offs + maxNum >= minNum) {
	if (maxNum < 900)
	  offs += (maxNum / 100 + 1) * 100;
	else if (maxNum < 9000)
	  offs += (maxNum / 1000 + 1) * 1000;
	else
	  offs += (maxNum / 10000 + 1) * 10000;
      }

      molP->groupP->resNumOffs = offs;

      molP = molP->groupP->nextP;
      if (molP == NULL)
	break;

      resP = ListLast(molP->resL);
      if (resP == NULL)
	maxNum = 1;
      else
	maxNum = resP->num;
    }

    groupNum++;

    headP = ListNext(MolList, headP);
  }
}

static void
groupsOff(void)
{
  DhMolP headP, molP;
  Mat4 rotInv;
  Vec3 v3;
  Vec4 v4;

  headP = ListFirst(MolList);
  while (headP != NULL) {
    if (headP->groupP == NULL || headP->groupP->prevP != NULL) {
      headP = ListNext(MolList, headP);
      continue;
    }

    if (headP->groupP->mapMod) {
      molP = headP->groupP->nextP;
      while (molP != NULL) {
	Mat4Copy(molP->rotMat, headP->rotMat);
	Mat4Mult(molP->rotMat, molP->groupP->rotMat);

	Vec3Copy(molP->transVect, headP->transVect);

	Mat4Copy(rotInv, molP->groupP->rotMat);
	Mat4Transp(rotInv);
	Vec3Copy(v3, headP->rotPoint);
	Vec3Sub(v3, molP->groupP->transVect);
	Vec3To4(v4, v3);
	Mat4VecMult(v4, rotInv);
	Vec4To3(molP->rotPoint, v4);

	recalcDist(molP);

	molP = molP->groupP->nextP;
      }
    }

    headP = ListNext(MolList, headP);
  }
}

void
DhActivateGroups(BOOL onOff)
{
  GroupsActive = onOff;

  if (onOff)
    groupsOn();
  else
    groupsOff();
}

extern BOOL
DhMolEqualStruc(DhMolP mol1P, DhMolP mol2P)
{
  DhResP res1P, res2P;
  BOOL more1, more2;

  if (ListSize(mol1P->resL) != ListSize(mol2P->resL))
    return FALSE;

  res1P = ListFirst(mol1P->resL);
  res2P = ListFirst(mol2P->resL);
  while (res1P != NULL) {
    if (res1P->num != res2P->num)
      return FALSE;
    if (res1P->defP != res2P->defP)
      return FALSE;
    res1P = ListNext(mol1P->resL, res1P);
    res2P = ListNext(mol2P->resL, res2P);
  }

  if (GroupsActive) {
    more1 = (mol1P->groupP != NULL && mol1P->groupP->nextP != NULL);
    more2 = (mol2P->groupP != NULL && mol2P->groupP->nextP != NULL);
    if (more1 != more2)
      return FALSE;
    if (more1)
      return DhMolEqualStruc(mol1P->groupP->nextP, mol2P->groupP->nextP);
  }

  return TRUE;
}

void
DhSetPseudoMode(BOOL onOff)
{
  PseudoMode = onOff;
}

int
DhGetMolNo(void)
{
  return ListSize(MolList);
}

static BOOL
isGroupHead(DhMolP molP)
{
  if (! GroupsActive)
    return TRUE;
  
  return molP->groupP == NULL || molP->groupP->prevP == NULL;
}

void
DhApplyMol(PropRefP refP, DhMolApplyFunc applyF, void *clientData)
{
  DhMolP molP, nextMolP;

  molP = ListFirst(MolList);
  while (molP != NULL) {
    nextMolP = ListNext(MolList, molP);
    if (molP->propTab[refP->index] & refP->mask && isGroupHead(molP))
      applyF(molP, clientData);
    molP = nextMolP;
  }
}

DhMolP
DhMolFindName(DSTR name)
{
  DhMolP molP;

  molP = ListFirst(MolList);
  while (molP != NULL) {
    if (DStrCmp(molP->name, name) == 0)
      return molP;
    molP = ListNext(MolList, molP);
  }

  return NULL;
}

DhMolP
DhMolFindNumber(int num)
{
  DhMolP molP;

  molP = ListFirst(MolList);
  while (molP != NULL) {
    if (molP->num == num)
      return molP;
    molP = ListNext(MolList, molP);
  }

  return NULL;
}

DhMolP
DhResGetMol(DhResP resP)
{
  DhMolP molP;

  molP = resP->molP;
  if (GroupsActive && molP->groupP != NULL)
    while (molP->groupP->prevP != NULL)
      molP = molP->groupP->prevP;

  return molP;
}

DhMolP
DhDistGetMol(DhDistP distP)
{
  return DhResGetMol(distP->res1P);
}

void
DhApplyRes(PropRefP refP, DhResApplyFunc applyF, void *clientData)
{
  DhMolP molP;

  molP = ListFirst(MolList);
  while (molP != NULL) {
    if (molP->propTab[refP->index] & refP->mask && isGroupHead(molP))
      DhMolApplyRes(refP, molP, applyF, clientData);
    molP = ListNext(MolList, molP);
  }
}

void
DhMolApplyRes(PropRefP refP, DhMolP molP,
    DhResApplyFunc applyF, void *clientData)
{
  DhResP resP, nextResP;

  resP = ListFirst(molP->resL);
  while (resP != NULL) {
    nextResP = ListNext(molP->resL, resP);
    if (resP->propTab[refP->index] & refP->mask)
      applyF(resP, clientData);
    resP = nextResP;
  }

  if (GroupsActive && molP->groupP != NULL && molP->groupP->nextP != NULL)
    DhMolApplyRes(refP, molP->groupP->nextP, applyF, clientData);
}

DhResP
DhResFind(DhMolP molP, int num)
{
  int searchNum;
  DhResP resP;

  if (GroupsActive && molP->groupP != NULL && molP->groupP->prevP != NULL)
    searchNum = num - molP->groupP->resNumOffs;
  else
    searchNum = num;

  resP = ListFirst(molP->resL);
  while (resP != NULL) {
    if (resP->num == searchNum)
      return resP;
    resP = ListNext(molP->resL, resP);
  }

  if (GroupsActive && molP->groupP != NULL && molP->groupP->nextP != NULL)
    return DhResFind(molP->groupP->nextP, num);

  return NULL;
}

DhResP
DhResGetNeigh(DhResP resP, DhNeighChoice nc)
{
  if (nc == NC_LEFT)
    return resP->neighLeftP;
  else
    return resP->neighRightP;
}

DhResP
DhAtomGetRes(DhAtomP atomP)
{
  return atomP->resP;
}

DhResP
DhBondGetRes(DhBondP bondP)
{
  if (bondP->resP == NULL)
    return ((DhBondAddP) bondP)->res1P;
  else
    return bondP->resP;
}

DhResP
DhAngleGetRes(DhAngleP angleP)
{
  return angleP->resP;
}

void
DhApplyAtom(PropRefP refP, DhAtomApplyFunc applyF, void *clientData)
{
  DhMolP molP;

  molP = ListFirst(MolList);
  while (molP != NULL) {
    if (molP->propTab[refP->index] & refP->mask && isGroupHead(molP))
      DhMolApplyAtom(refP, molP, applyF, clientData);
    molP = ListNext(MolList, molP);
  }
}

static void
doInvRot(DhMolP molP)
{
  RotEntry *entryP;

  if (! RotMode)
    return;

  entryP = ListLast(RotList);
  while (entryP != NULL && molP != entryP->molP)
    entryP = ListPrev(RotList, entryP);

  while (entryP != NULL && molP == entryP->molP) {
    if (! entryP->forward)
      RotF(entryP->invMat);
    entryP = ListPrev(RotList, entryP);
  }
}

void
DhMolApplyAtom(PropRefP refP, DhMolP molP,
    DhAtomApplyFunc applyF, void *clientData)
{
  DhResP resP;

  doInvRot(molP);

  resP = ListFirst(molP->resL);
  while (resP != NULL) {
    if (resP->propTab[refP->index] & refP->mask)
      DhResApplyAtom(refP, resP, applyF, clientData);
    resP = ListNext(molP->resL, resP);
  }

  if (GroupsActive && molP->groupP != NULL && molP->groupP->nextP != NULL)
    DhMolApplyAtom(refP, molP->groupP->nextP, applyF, clientData);
}

static RotEntry *
findRotEntry(DhResP resP)
{
  RotEntry *entryP;

  if (! RotMode)
    return NULL;

  entryP = ListFirst(RotList);
  while (entryP != NULL && resP->molP != entryP->molP)
    entryP = ListNext(RotList, entryP);

  while (entryP != NULL &&
      resP->molP == entryP->molP && resP != entryP->resP)
    entryP = ListNext(RotList, entryP);

  if (entryP == NULL || resP->molP != entryP->molP)
    return NULL;
  else
    return entryP;
}

void
DhResApplyAtom(PropRefP refP, DhResP resP,
    DhAtomApplyFunc applyF, void *clientData)
{
  RotEntry *rotP;
  int i;

  rotP = findRotEntry(resP);

  for (i = resP->defP->firstAtomI; i <= resP->defP->lastAtomI; i++) {
    while (rotP != NULL && i > rotP->prevI) {
      RotF(rotP->mat);
      rotP = ListNext(RotList, rotP);
      if (rotP != NULL && rotP->resP != resP)
	rotP = NULL;
    }
    if ((resP->atomA[i].propTab[refP->index] & refP->mask) &&
        resP->atomA[i].state == AS_VALID)
      applyF(resP->atomA + i, clientData);
  }

  while (rotP != NULL && rotP->resP == resP) {
    RotF(rotP->mat);
    rotP = ListNext(RotList, rotP);
  }
}

DhAtomP
DhAtomFindName(DhResP resP, DSTR name, BOOL alsoUndefined)
{
  int i;

  for (i = resP->defP->firstAtomI; i <= resP->defP->lastAtomI; i++) {
    if (resP->atomA[i].state == AS_DELETED)
      continue;

    if (resP->atomA[i].state == AS_UNKNOWN_COORD && ! alsoUndefined)
      continue;

    if (DStrCmp(resP->defP->atomA[i].name, name) == 0)
      return resP->atomA + i;
  }

  return NULL;
}

DhAtomP
DhAtomFindNumber(DhResP resP, int num, BOOL alsoUndefined)
{
  if (resP->atomA[num].state == AS_DELETED)
    return NULL;

  if (resP->atomA[num].state == AS_UNKNOWN_COORD && ! alsoUndefined)
    return NULL;

  return resP->atomA + num;
}

DhAtomP
DhBondGetAtom1(DhBondP bondP)
{
  DhResP resP;
  int ind;

  if (bondP->resP == NULL) {
    resP = ((DhBondAddP) bondP)->res1P;
    return resP->atomA + ((DhBondAddP) bondP)->atom1I;
  } else {
    resP = bondP->resP;
    ind = bondP - resP->bondA;
    return resP->atomA + resP->defP->bondA[ind].atom1I;
  }
}

DhAtomP
DhBondGetAtom2(DhBondP bondP)
{
  DhResP resP;
  int ind;
  DhBondDefP defP;

  if (bondP->resP == NULL) {
    resP = ((DhBondAddP) bondP)->res2P;
    return resP->atomA + ((DhBondAddP) bondP)->atom2I;
  } else {
    resP = bondP->resP;
    ind = bondP - resP->bondA;
    defP = resP->defP->bondA + ind;
    if (PseudoMode && defP->pseudoI >= 0)
      return resP->atomA + defP->pseudoI;
    else
      return resP->atomA + defP->atom2I;
  }
}

DhAtomP
DhAngleGetAtom1(DhAngleP angleP)
{
  DhResP resP = angleP->resP;
  int ind = angleP - resP->angleA;

  return resP->atomA + resP->defP->angleA[ind].atom1I;
}

DhAtomP
DhAngleGetAtom2(DhAngleP angleP)
{
  DhResP resP = angleP->resP;
  int ind = angleP - resP->angleA;

  return resP->atomA + resP->defP->angleA[ind].atom2I;
}

DhAtomP
DhAngleGetAtom3(DhAngleP angleP)
{
  DhResP resP = angleP->resP;
  int ind = angleP - resP->angleA;

  return resP->atomA + resP->defP->angleA[ind].atom3I;
}

DhAtomP
DhAngleGetAtom4(DhAngleP angleP)
{
  DhResP resP = angleP->resP;
  int ind = angleP - resP->angleA;

  return resP->atomA + resP->defP->angleA[ind].atom4I;
}

DhAtomP
DhDistGetAtom1(DhDistP distP)
{
  return distP->res1P->atomA + distP->atom1I;
}

DhAtomP
DhDistGetAtom2(DhDistP distP)
{
  return distP->res2P->atomA + distP->atom2I;
}

static int
getGroupNum(DhMolP molP)
{
  if (! GroupsActive)
    return -1;

  if (molP->groupP == NULL)
    return -1;
  
  return molP->groupP->num;
}

static void
applyBondList(PropRefP refP, LINLIST bondL, int groupNum,
    DhBondApplyFunc applyF, void *clientData)
{
  DhBondAddP bondAddP, nextBondAddP;
  DhBondP bondP;
  DhAtomP atom1P, atom2P;
  int groupNum1, groupNum2;

  if (RotMode)  /* skip additional bonds in rotation mode */
    return;
  
  bondAddP = ListFirst(bondL);
  while (bondAddP != NULL) {
    nextBondAddP = ListNext(bondL, bondAddP);
    bondP = (DhBondP) bondAddP;
    if (bondP->propTab[refP->index] & refP->mask) {
      atom1P = bondAddP->res1P->atomA + bondAddP->atom1I;
      atom2P = bondAddP->res2P->atomA + bondAddP->atom2I;
      groupNum1 = getGroupNum(bondAddP->res1P->molP);
      groupNum2 = getGroupNum(bondAddP->res2P->molP);
      if (atom1P->state == AS_VALID && atom2P->state == AS_VALID &&
	  ((groupNum1 == groupNum && groupNum2 == groupNum) ||
	  (groupNum < 0 && groupNum1 != groupNum2)))
	applyF(bondP, atom1P, atom2P, clientData);
    }
    bondAddP = nextBondAddP;
  }
}

void
DhApplyBond(PropRefP refP, DhBondApplyFunc applyF, void *clientData)
{
  DhMolP molP;

  molP = ListFirst(MolList);
  while (molP != NULL) {
    if (molP->propTab[refP->index] & refP->mask && isGroupHead(molP))
      DhMolApplyBond(refP, molP, applyF, clientData);
    molP = ListNext(MolList, molP);
  }

  applyBondList(refP, BondList, -1, applyF, clientData);
}

void
DhMolApplyBond(PropRefP refP, DhMolP molP,
    DhBondApplyFunc applyF, void *clientData)
{
  DhResP resP;

  doInvRot(molP);

  resP = ListFirst(molP->resL);
  while (resP != NULL) {
    if (resP->propTab[refP->index] & refP->mask)
      DhResApplyBond(refP, resP, applyF, clientData);
    resP = ListNext(molP->resL, resP);
  }

  applyBondList(refP, molP->bondL, getGroupNum(molP), applyF, clientData);

  if (GroupsActive && molP->groupP != NULL && molP->groupP->nextP != NULL)
    DhMolApplyBond(refP, molP->groupP->nextP, applyF, clientData);

  if (isGroupHead(molP))
    applyBondList(refP, BondList, getGroupNum(molP), applyF, clientData);
}

void
DhResApplyBond(PropRefP refP, DhResP resP,
    DhBondApplyFunc applyF, void *clientData)
{
  RotEntry *rotP;
  int bondNo, lastBondI, i, k;
  DhBondDefP defP;
  int atom1I, atom2I;
  DhAtomP atom1P, atom2P;

  rotP = findRotEntry(resP);

  bondNo = resP->defP->bondNo;
  lastBondI = resP->defP->lastBondI;
  for (k = resP->defP->firstBondI; k <= lastBondI; k++) {
    /* make sure that all bonds get visited even if part of them
       get deleted during the process */
    i = resP->defP->bondNo - bondNo + k;
    defP = resP->defP->bondA + i;

    if (PseudoMode) {
      if (defP->pseudoI < 0)
	continue;
      atom2I = defP->pseudoI;
    } else {
      atom2I = defP->atom2I;
    }
    atom1I = defP->atom1I;

    while (rotP != NULL && atom1I >= rotP->prevI) {
      RotF(rotP->mat);
      rotP = ListNext(RotList, rotP);
      if (rotP != NULL && rotP->resP != resP)
	rotP = NULL;
    }
    if (resP->bondA[i].propTab[refP->index] & refP->mask) {
      atom1P = resP->atomA + atom1I;
      atom2P = resP->atomA + atom2I;
      if (atom1P->state == AS_VALID && atom2P->state == AS_VALID)
	applyF(resP->bondA + i, atom1P, atom2P, clientData);
    }
  }

  while (rotP != NULL && rotP->resP == resP) {
    RotF(rotP->mat);
    rotP = ListNext(RotList, rotP);
  }
}

static void
applyAtomBondList(PropRefP refP, DhAtomP atomP, LINLIST bondL,
    DhBondApplyFunc applyF, void *clientData)
{
  DhBondAddP bondAddP, nextBondAddP;
  DhBondP bondP;
  DhAtomP atom1P, atom2P;

  bondAddP = ListFirst(bondL);
  while (bondAddP != NULL) {
    nextBondAddP = ListNext(bondL, bondAddP);
    bondP = (DhBondP) bondAddP;
    atom1P = bondAddP->res1P->atomA + bondAddP->atom1I;
    atom2P = bondAddP->res2P->atomA + bondAddP->atom2I;

    if ((atom1P == atomP || atom2P == atomP) &&
        bondP->propTab[refP->index] & refP->mask &&
        atom1P->state == AS_VALID && atom2P->state == AS_VALID)
      applyF(bondP, atom1P, atom2P, clientData);

    bondAddP = nextBondAddP;
  }
}

static void
atomApplyBond(PropRefP refP, DhAtomP atomP,
    DhBondApplyFunc applyF, void *clientData)
{
  DhResP resP;
  DhResDefP defP;
  int atomI, atom1I, atom2I, i;
  DhAtomP atom1P, atom2P;

  resP = atomP->resP;
  defP = resP->defP;

  atomI = atomP - resP->atomA;

  for (i = defP->firstBondI; i <= defP->lastBondI; i++) {
    if (! (resP->bondA[i].propTab[refP->index] & refP->mask))
      continue;

    atom1I = defP->bondA[i].atom1I;
    atom2I = defP->bondA[i].atom2I;
    if (atom1I == atomI) {
      if (atom2I <= defP->lastAtomI) {
	atom2P = resP->atomA + atom2I;
      } else {
	atom2P = getEquivAtom(resP->atomA + atom2I);
	if (atom2P == NULL)
	  atom2P = resP->atomA + atom2I;
      }
      if (atom2P->state == AS_VALID)
        applyF(resP->bondA + i,
	    resP->atomA + atom1I, atom2P, clientData);
    } else if (atom2I == atomI) {
      atom1P = resP->atomA + atom1I;
      if (atom1P->state == AS_VALID)
        applyF(resP->bondA + i,
	    atom1P, resP->atomA + atom2I, clientData);
    }
  }

  applyAtomBondList(refP, atomP, resP->molP->bondL, applyF, clientData);
  applyAtomBondList(refP, atomP, BondList, applyF, clientData);
}

void
DhAtomApplyBond(PropRefP refP, DhAtomP atomP,
    DhBondApplyFunc applyF, void *clientData)
{
  atomApplyBond(refP, atomP, applyF, clientData);

  atomP = getEquivAtom(atomP);
  if (atomP != NULL)
    atomApplyBond(refP, atomP, applyF, clientData);
}

DhBondP
DhBondFind(DhResP resP, DSTR atomName1, DSTR atomName2)
{
  int i, i1, i2;

  i1 = -1;
  i2 = -1;
  for (i = 0; i < resP->defP->atomNo; i++) {
    if (resP->atomA[i].state == AS_DELETED)
      continue;

    if (DStrCmp(resP->defP->atomA[i].name, atomName1) == 0)
      i1 = i;
    if (DStrCmp(resP->defP->atomA[i].name, atomName2) == 0)
      i2 = i;
  }
  if (i1 == -1 || i2 == -1)
    return NULL;

  for (i = resP->defP->firstBondI; i <= resP->defP->lastBondI; i++) {
    if (resP->defP->bondA[i].atom1I == i1 &&
        resP->defP->bondA[i].atom2I == i2)
      return resP->bondA + i;
    if (resP->defP->bondA[i].atom1I == i2 &&
        resP->defP->bondA[i].atom2I == i1)
      return resP->bondA + i;
  }

  return NULL;
}

DhBondP
DhAngleGetBond(DhAngleP angleP)
{
  DhResP resP;
  DhResDefP defP;
  int ind, i;

  resP = angleP->resP;
  defP = resP->defP;
  ind = angleP - resP->angleA;

  for (i = defP->firstBondI; i <= defP->lastBondI; i++) {
    if (defP->bondA[i].atom1I == defP->angleA[ind].atom2I &&
        defP->bondA[i].atom2I == defP->angleA[ind].atom3I)
      return resP->bondA + i;
  }

  return NULL;
}

void
DhApplyAngle(PropRefP refP, DhAngleApplyFunc applyF, void *clientData)
{
  DhMolP molP;

  molP = ListFirst(MolList);
  while (molP != NULL) {
    if (molP->propTab[refP->index] & refP->mask && isGroupHead(molP))
      DhMolApplyAngle(refP, molP, applyF, clientData);
    molP = ListNext(MolList, molP);
  }
}

void
DhMolApplyAngle(PropRefP refP, DhMolP molP,
    DhAngleApplyFunc applyF, void *clientData)
{
  DhResP resP;

  resP = ListFirst(molP->resL);
  while (resP != NULL) {
    if (resP->propTab[refP->index] & refP->mask)
      DhResApplyAngle(refP, resP, applyF, clientData);
    resP = ListNext(molP->resL, resP);
  }

  if (GroupsActive && molP->groupP != NULL && molP->groupP->nextP != NULL)
    DhMolApplyAngle(refP, molP->groupP->nextP, applyF, clientData);
}

void
DhResApplyAngle(PropRefP refP, DhResP resP,
    DhAngleApplyFunc applyF, void *clientData)
{
  int i;
  DhAngleDefP defP;
  DhAtomP atom1P, atom2P, atom3P, atom4P;

  for (i = 0; i < resP->defP->angleNo; i++)
    if (resP->angleA[i].propTab[refP->index] & refP->mask) {
      defP = resP->defP->angleA + i;
      atom1P = resP->atomA + defP->atom1I;
      atom2P = resP->atomA + defP->atom2I;
      atom3P = resP->atomA + defP->atom3I;
      atom4P = resP->atomA + defP->atom4I;
      if (atom1P->state == AS_VALID && atom2P->state == AS_VALID &&
          atom3P->state == AS_VALID && atom4P->state == AS_VALID)
	applyF(resP->angleA + i, clientData);
    }
}

DhAngleP
DhAngleFind(DhResP resP, DSTR name)
{
  int i;

  for (i = 0; i < resP->defP->angleNo; i++)
    if (DStrCmp(resP->defP->angleA[i].name, name) == 0)
      return resP->angleA + i;

  return NULL;
}

DhAngleP
DhBondGetAngle(DhBondP bondP)
{
  DhResP resP;
  DhResDefP defP;
  int ind, i;

  resP = bondP->resP;
  if (resP == NULL)
    return NULL;

  defP = resP->defP;
  ind = bondP - resP->bondA;

  for (i = 0; i < defP->angleNo; i++) {
    if (defP->angleA[i].atom2I == defP->bondA[ind].atom1I &&
        defP->angleA[i].atom3I == defP->bondA[ind].atom2I)
      return resP->angleA + i;
  }

  return NULL;
}

static void
callDistApply(PropRefP refP, DhDistP distP, int groupNum,
    DhDistApplyFunc applyF, void *clientData)
{
  DhAtomP atom1P, atom2P;
  int groupNum1, groupNum2;

  if (distP->propTab[refP->index] & refP->mask) {
    atom1P = distP->res1P->atomA + distP->atom1I;
    atom2P = distP->res2P->atomA + distP->atom2I;
    groupNum1 = getGroupNum(distP->res1P->molP);
    groupNum2 = getGroupNum(distP->res2P->molP);
    if (atom1P->state == AS_VALID && atom2P->state == AS_VALID &&
	((groupNum1 == groupNum && groupNum2 == groupNum) ||
	(groupNum < 0 && groupNum1 != groupNum2)))
      applyF(distP, atom1P, atom2P, clientData);
  }
}

void
DhApplyDist(PropRefP refP, DhDistApplyFunc applyF, void *clientData)
{
  DhMolP molP;

  molP = ListFirst(MolList);
  while (molP != NULL) {
    if (molP->propTab[refP->index] & refP->mask && isGroupHead(molP))
      DhMolApplyDist(refP, molP, applyF, clientData);
    molP = ListNext(MolList, molP);
  }

  DhApplyInterDist(refP, applyF, clientData);
}

void
DhApplyInterDist(PropRefP refP, DhDistApplyFunc applyF, void *clientData)
{
  DhDistP distP, nextDistP;

  distP = ListFirst(DistList);
  while (distP != NULL) {
    nextDistP = ListNext(DistList, distP);
    callDistApply(refP, distP, -1, applyF, clientData);
    distP = nextDistP;
  }
}

void
DhMolApplyDist(PropRefP refP, DhMolP molP,
    DhDistApplyFunc applyF, void *clientData)
{
  int groupNum;
  DhDistP distP, nextDistP;

  groupNum = getGroupNum(molP);

  distP = ListFirst(molP->distL);
  while (distP != NULL) {
    nextDistP = ListNext(molP->distL, distP);
    callDistApply(refP, distP, groupNum, applyF, clientData);
    distP = nextDistP;
  }

  if (GroupsActive && molP->groupP != NULL && molP->groupP->nextP != NULL)
    DhMolApplyDist(refP, molP->groupP->nextP, applyF, clientData);

  if (isGroupHead(molP) && groupNum >= 0) {
    distP = ListFirst(DistList);
    while (distP != NULL) {
      nextDistP = ListNext(DistList, distP);
      callDistApply(refP, distP, groupNum, applyF, clientData);
      distP = nextDistP;
    }
  }
}

void
DhMolSetProp(PropRefP refP, DhMolP molP, BOOL onOff)
{
  molP->propTab = PropChange(molP->propTab, refP, onOff);

  if (GroupsActive && molP->groupP != NULL && molP->groupP->nextP != NULL)
    DhMolSetProp(refP, molP->groupP->nextP, onOff);

  MolListChanged = TRUE;
}

void
DhResSetProp(PropRefP refP, DhResP resP, BOOL onOff)
{
  DhMolP molP = resP->molP;

  resP->propTab = PropChange(resP->propTab, refP, onOff);
  if (onOff) {
    molP->propTab = PropChange(molP->propTab, refP, onOff);
    MolListChanged = TRUE;
  }
}

void
DhAtomSetProp(PropRefP refP, DhAtomP atomP, BOOL onOff)
{
  DhResP resP;
  DhMolP molP;

  atomP->propTab = PropChange(atomP->propTab, refP, onOff);
  if (onOff) {
    resP = atomP->resP;
    resP->propTab = PropChange(resP->propTab, refP, onOff);
    molP = resP->molP;
    molP->propTab = PropChange(molP->propTab, refP, onOff);
    MolListChanged = TRUE;
  }

  atomP = getEquivAtom(atomP);
  if (atomP != NULL) {
    atomP->propTab = PropChange(atomP->propTab, refP, onOff);
    if (onOff) {
      resP = atomP->resP;
      resP->propTab = PropChange(resP->propTab, refP, onOff);
      molP = resP->molP;
      molP->propTab = PropChange(molP->propTab, refP, onOff);
    }
  }
}

void
DhBondSetProp(PropRefP refP, DhBondP bondP, BOOL onOff)
{
  DhResP resP, res1P, res2P;
  DhMolP molP, mol1P, mol2P;
  DhBondAddP bondAddP;

  bondP->propTab = PropChange(bondP->propTab, refP, onOff);
  if (! onOff)
    return;

  resP = bondP->resP;
  if (resP == NULL) {
    bondAddP = (DhBondAddP) bondP;
    res1P = bondAddP->res1P;
    res2P = bondAddP->res2P;
    if (res1P == res2P)
      res1P->propTab = PropChange(res1P->propTab, refP, onOff);
    mol1P = bondAddP->res1P->molP;
    mol2P = bondAddP->res2P->molP;
    if (mol1P == mol2P) {
      mol1P->propTab = PropChange(mol1P->propTab, refP, onOff);
      MolListChanged = TRUE;
    }
  } else {
    resP->propTab = PropChange(resP->propTab, refP, onOff);
    molP = resP->molP;
    molP->propTab = PropChange(molP->propTab, refP, onOff);
    MolListChanged = TRUE;
  }
}

void
DhAngleSetProp(PropRefP refP, DhAngleP angleP, BOOL onOff)
{
  DhResP resP = angleP->resP;
  DhMolP molP = resP->molP;

  angleP->propTab = PropChange(angleP->propTab, refP, onOff);
  if (onOff) {
    resP->propTab = PropChange(resP->propTab, refP, onOff);
    molP->propTab = PropChange(molP->propTab, refP, onOff);
    MolListChanged = TRUE;
  }
}

void
DhDistSetProp(PropRefP refP, DhDistP distP, BOOL onOff)
{
  DhMolP mol1P = distP->res1P->molP;
  DhMolP mol2P = distP->res2P->molP;

  distP->propTab = PropChange(distP->propTab, refP, onOff);
  if (onOff && mol1P == mol2P) {
    mol1P->propTab = PropChange(mol1P->propTab, refP, onOff);
    MolListChanged = TRUE;
  }
}

unsigned *
DhMolGetPropTab(DhMolP molP)
{
  return molP->propTab;
}

unsigned *
DhResGetPropTab(DhResP resP)
{
  return resP->propTab;
}

unsigned *
DhAtomGetPropTab(DhAtomP atomP)
{
  return atomP->propTab;
}

unsigned *
DhBondGetPropTab(DhBondP bondP)
{
  return bondP->propTab;
}

unsigned *
DhAngleGetPropTab(DhAngleP angleP)
{
  return angleP->propTab;
}

unsigned *
DhDistGetPropTab(DhDistP distP)
{
  return distP->propTab;
}

BOOL
DhMolGetProp(PropRefP refP, DhMolP molP)
{
  return (molP->propTab[refP->index] & refP->mask) != 0;
}

BOOL
DhResGetProp(PropRefP refP, DhResP resP)
{
  return (resP->propTab[refP->index] & refP->mask) != 0;
}

BOOL
DhAtomGetProp(PropRefP refP, DhAtomP atomP)
{
  return (atomP->propTab[refP->index] & refP->mask) != 0;
}

BOOL
DhBondGetProp(PropRefP refP, DhBondP bondP)
{
  return (bondP->propTab[refP->index] & refP->mask) != 0;
}

BOOL
DhAngleGetProp(PropRefP refP, DhAngleP angleP)
{
  return (angleP->propTab[refP->index] & refP->mask) != 0;
}

BOOL
DhDistGetProp(PropRefP refP, DhDistP distP)
{
  return (distP->propTab[refP->index] & refP->mask) != 0;
}

void
DhMolSetName(DhMolP molP, DSTR name)
{
  DStrAssignDStr(molP->name, name);
  MolListChanged = TRUE;
}

void
DhMolSetRotMat(DhMolP molP, Mat4 mat)
{
  Mat4Copy(molP->rotMat, mat);
  recalcDist(molP);

  if (GroupsActive && molP->groupP != NULL)
    molP->groupP->mapMod = TRUE;
}

void
DhMolSetRotPoint(DhMolP molP, Vec3 p)
{
  Vec3Copy(molP->rotPoint, p);
  recalcDist(molP);

  if (GroupsActive && molP->groupP != NULL)
    molP->groupP->mapMod = TRUE;
}

void
DhMolSetTransVect(DhMolP molP, Vec3 v)
{
  Vec3Copy(molP->transVect, v);
  recalcDist(molP);

  if (GroupsActive && molP->groupP != NULL)
    molP->groupP->mapMod = TRUE;
}

void
DhMolSetAttr(DhMolP molP, MolAttrP attrP)
{
  molP->attrP = attrP;
}

void
DhResSetNumber(DhResP resP, int num)
{
  LINLIST resL;
  DhResP p;

  resL = resP->molP->resL;

  p = ListPrev(resL, resP);
  if (p != NULL && p->num >= num)
    return;

  p = ListNext(resL, resP);
  if (p != NULL && p->num <= num)
    return;

  resP->num = num;
}

void
DhAtomSetCoord(DhAtomP atomP, Vec3 coord)
{
  if (atomP->state == AS_DELETED)
    return;

  if (coord == NULL) {
    atomP->state = AS_UNKNOWN_COORD;
  } else {
    Vec3Copy(atomP->coord, coord);
    atomP->state = AS_VALID;
  }

  atomP = getEquivAtom(atomP);
  if (atomP == NULL)
    return;

  if (coord == NULL) {
    atomP->state = AS_UNKNOWN_COORD;
  } else {
    Vec3Copy(atomP->coord, coord);
    atomP->state = AS_VALID;
  }
}

void
DhAtomSetShiftIndex(DhAtomP atomP, int shiftI)
{
#ifdef NMR
  atomP->shiftI = shiftI;

  atomP = getEquivAtom(atomP);
  if (atomP != NULL)
    atomP->shiftI = shiftI;
#endif
}

void
DhAtomSetShift(DhAtomP atomP, float shift)
{
#ifdef NMR
  atomP->shift = shift;

  atomP = getEquivAtom(atomP);
  if (atomP != NULL)
    atomP->shift = shift;
#endif
}

void
DhAtomSetBFactor(DhAtomP atomP, float bFactor)
{
  atomP->bFactor = bFactor;

  atomP = getEquivAtom(atomP);
  if (atomP != NULL)
    atomP->bFactor = bFactor;
}

void
DhAtomSetCharge(DhAtomP atomP, float charge)
{
  DhResP resP;
  int ind;

  resP = atomP->resP;
  ind = atomP - resP->atomA;
  DhResDefSetCharge(&resP->defP, ind, charge);
}

void
DhAtomSetAttr(DhAtomP atomP, AttrP attrP)
{
  atomP->attrP = attrP;

  atomP = getEquivAtom(atomP);
  if (atomP != NULL) {
    AttrReturn(atomP->attrP);
    atomP->attrP = AttrGet(attrP);
  }
}

void
DhBondSetAttr(DhBondP bondP, AttrP attrP)
{
  bondP->attrP = attrP;
}

void
DhAngleSetVal(DhAngleP angleP, float val)
{
  angleP->val = - val / 180.0f * (float) M_PI;
  angleP->changed = TRUE;
}

void
DhAngleSetMinVal(DhAngleP angleP, float val)
{
  /* angle constraints are stored in degrees, angles in radian! */
  angleP->minVal = val;
}

void
DhAngleSetMaxVal(DhAngleP angleP, float val)
{
  /* angle constraints are stored in degrees, angles in radian! */
  angleP->maxVal = val;
}

void
DhDistSetAttr(DhDistP distP, AttrP attrP)
{
  distP->attrP = attrP;
}

void
DhDistSetKind(DhDistP distP, DhDistKind kind)
{
  distP->kind = kind;
}

void
DhDistSetLimit(DhDistP distP, float limit)
{
  distP->limit = limit;
}

DSTR
DhMolGetName(DhMolP molP)
{
  return molP->name;
}

int
DhMolGetNumber(DhMolP molP)
{
  return molP->num;
}

void
DhMolGetRotMat(DhMolP molP, Mat4 mat)
{
  Mat4Copy(mat, molP->rotMat);
}

void
DhMolGetRotPoint(DhMolP molP, Vec3 p)
{
  Vec3Copy(p, molP->rotPoint);
}

void
DhMolGetTransVect(DhMolP molP, Vec3 v)
{
  Vec3Copy(v, molP->transVect);
}

MolAttrP
DhMolGetAttr(DhMolP molP)
{
  return molP->attrP;
}

DSTR
DhResGetName(DhResP resP)
{
  return resP->defP->name;
}

int
DhResGetNumber(DhResP resP)
{
  DhMolP molP;

  if (GroupsActive) {
    molP = resP->molP;
    if (molP->groupP != NULL && molP->groupP->prevP != NULL)
      return molP->groupP->resNumOffs + resP->num;
  }

  return resP->num;
}

DSTR
DhAtomGetName(DhAtomP atomP)
{
  DhResP resP;
  int ind;

  resP = atomP->resP;
  ind = atomP - resP->atomA;
  return resP->defP->atomA[ind].name;
}

int
DhAtomGetNumber(DhAtomP atomP)
{
  return atomP - atomP->resP->atomA;
}

void
DhAtomGetCoord(DhAtomP atomP, Vec3 coord)
{
  DhMolP molP;
  Vec4 v4;

  if (GroupsActive) {
    molP = atomP->resP->molP;
    if (molP->groupP != NULL && molP->groupP->prevP != NULL) {
      Vec3To4(v4, atomP->coord);
      Mat4VecMult(v4, molP->groupP->rotMat);
      Vec4To3(coord, v4);
      Vec3Add(coord, molP->groupP->transVect);
      return;
    }
  }

  Vec3Copy(coord, atomP->coord);
}

void
DhTransfVec(Vec3 x, DhMolP molP)
{
  Vec4 w;

  Vec3Sub(x, molP->rotPoint);
  Vec3To4(w, x);
  Mat4VecMult(w, molP->rotMat);
  Vec4To3(x, w);
  Vec3Add(x, molP->transVect);
}

void
DhAtomGetCoordTransf(DhAtomP atomP, Vec3 coord)
{
  DhResP resP;
  DhMolP molP;
  int ind;
  Vec4 w;
  RotEntry *entryP;

  resP = atomP->resP;
  molP = resP->molP;

  if (GroupsActive && molP->groupP != NULL && molP->groupP->prevP != NULL) {
    Vec3To4(w, atomP->coord);
    Mat4VecMult(w, molP->groupP->rotMat);
    Vec4To3(coord, w);
    Vec3Add(coord, molP->groupP->transVect);
    DhTransfVec(coord, molP->groupP->headP);
    return;
  }

  if (RotMode) {
    ind = atomP - resP->atomA;

    Vec3To4(w, atomP->coord);

    entryP = ListFirst(RotList);
    while (entryP != NULL && molP != entryP->molP)
      entryP = ListNext(RotList, entryP);

    while (entryP != NULL && molP == entryP->molP &&
	(resP->num > entryP->resP->num ||
        (resP->num == entryP->resP->num && ind > entryP->prevI)))
      entryP = ListNext(RotList, entryP);

    if (entryP == NULL)
      entryP = ListLast(RotList);
    else
      entryP = ListPrev(RotList, entryP);

    while (entryP != NULL && molP == entryP->molP) {
      Mat4VecMult(w, entryP->mat);
      entryP = ListPrev(RotList, entryP);
    }

    Vec4To3(coord, w);
  } else {
    Vec3Copy(coord, atomP->coord);
  }

  DhTransfVec(coord, molP);
}

int
DhAtomGetShiftIndex(DhAtomP atomP)
{
#ifdef NMR
  return atomP->shiftI;
#else
  return 0;
#endif
}

float
DhAtomGetShift(DhAtomP atomP)
{
#ifdef NMR
  return atomP->shift;
#else
  return DH_SHIFT_UNKNOWN;
#endif
}

float
DhAtomGetBFactor(DhAtomP atomP)
{
  return atomP->bFactor;
}

float
DhAtomGetVdw(DhAtomP atomP)
{
  DhResP resP;
  int ind;

  resP = atomP->resP;
  ind = atomP - resP->atomA;
  return resP->defP->atomA[ind].typeP->vdw;
}

float
DhAtomGetCharge(DhAtomP atomP, DhChargeKind kind)
{
  DhResP resP;
  int ind;

  resP = atomP->resP;
  ind = atomP - resP->atomA;

  switch (kind) {
    case CK_CHARGE:
      return resP->defP->atomA[ind].charge;
    case CK_HEAVY:
      return resP->defP->atomA[ind].heavyCharge;
    case CK_AVG:
      return resP->defP->atomA[ind].avgCharge;
    case CK_SIMPLE:
      return resP->defP->atomA[ind].simpleCharge;
  }

  /* should not get here */
  return 0.0f;
}

int
DhAtomGetHBondCap(DhAtomP atomP)
{
  DhResP resP;
  int ind;

  resP = atomP->resP;
  ind = atomP - resP->atomA;
  return resP->defP->atomA[ind].typeP->hBondCap;
}

AttrP
DhAtomGetAttr(DhAtomP atomP)
{
  return atomP->attrP;
}

AttrP
DhBondGetAttr(DhBondP bondP)
{
  return bondP->attrP;
}

BOOL
DhAngleIsBackbone(DhAngleP angleP)
{
  DhAngleDefP defP;
  int ind;

  ind = angleP - angleP->resP->angleA;
  defP = angleP->resP->defP->angleA + ind;

  return defP->lastAtomI == -1;
}

DSTR
DhAngleGetName(DhAngleP angleP)
{
  DhResP resP;
  int ind;

  resP = angleP->resP;
  ind = angleP - resP->angleA;
  return resP->defP->angleA[ind].name;
}

float
DhAngleGetVal(DhAngleP angleP)
{
  float a;

  a = - angleP->val / (float) M_PI * 180.0f;
  if (a + 180.0f < 0.001f)  /* convert -180 to 180 */
    a = 180.0f;
  return a;
}

float
DhAngleGetMinVal(DhAngleP angleP)
{
  /* angle constraints are stored in degrees, angles in radian! */
  return angleP->minVal;
}

float
DhAngleGetMaxVal(DhAngleP angleP)
{
  /* angle constraints are stored in degrees, angles in radian! */
  return angleP->maxVal;
}

AttrP
DhDistGetAttr(DhDistP distP)
{
  return distP->attrP;
}

DhDistKind
DhDistGetKind(DhDistP distP)
{
  return distP->kind;
}

float
DhDistGetLimit(DhDistP distP)
{
  return distP->limit;
}

float
DhDistGetVal(DhDistP distP)
{
  return distP->val;
}

float
DhDistGetViol(DhDistP distP)
{
  if (distP->kind == DK_UPPER)
    return distP->val - distP->limit;
  else if (distP->kind == DK_LOWER)
    return distP->limit - distP->val;
  else
    return 0.0f;
}

void
DhRotSetFunc(DhRotFunc rotF)
{
  RotF = rotF;
}

static void
insertRotEntry(RotEntry *newEntryP)
{
  RotEntry *entryP;

  entryP = ListFirst(RotList);
  while (entryP != NULL && newEntryP->molP != entryP->molP)
    entryP = ListNext(RotList, entryP);

  while (entryP != NULL && newEntryP->resP->num > entryP->resP->num)
    entryP = ListNext(RotList, entryP);
  
  while (entryP != NULL && newEntryP->resP->num == entryP->resP->num &&
      newEntryP->prevI > entryP->prevI)
    entryP = ListNext(RotList, entryP);

  if (entryP == NULL)
    ListInsertLast(RotList, newEntryP);
  else if (newEntryP->angleP != entryP->angleP)
    ListInsertBefore(RotList, entryP, newEntryP);
  else
    entryP->forward = newEntryP->forward;
}

void
DhRotAddAngle(DhAngleP angleP, BOOL forward)
{
  int ind;
  DhAngleDefP defP;
  RotEntry entry;

  if (RotList == NULL)
    RotList = ListOpen(sizeof(RotEntry));

  ind = angleP - angleP->resP->angleA;
  defP = angleP->resP->defP->angleA + ind;

  entry.resP = angleP->resP;
  entry.molP = entry.resP->molP;
  entry.angleP = angleP;
  entry.prevI = defP->atom3I;
  entry.forward = forward || defP->lastAtomI != -1;
  entry.val = angleP->val;
  Mat4Ident(entry.mat);
  Mat4Ident(entry.invMat);
  insertRotEntry(&entry);

  if (defP->lastAtomI != -1) {
    entry.prevI = defP->lastAtomI;
    insertRotEntry(&entry);
  }

  RotMode = TRUE;
}

void
DhRotSetAngle(DhAngleP angleP, float val)
{
  RotEntry *entry1P, *entry2P;
  int ind;
  DhAngleDefP defP;

  entry1P = ListFirst(RotList);
  while (entry1P != NULL && entry1P->angleP != angleP)
    entry1P = ListNext(RotList, entry1P);

  if (entry1P == NULL)  /* this can only happen if molecule is deleted */
    return;

  entry1P->val = - val / 180.0f * (float) M_PI;

  entry2P = ListNext(RotList, entry1P);
  while (entry2P != NULL && entry2P->angleP != angleP)
    entry2P = ListNext(RotList, entry2P);

  ind = angleP - angleP->resP->angleA;
  defP = angleP->resP->defP->angleA + ind;

  DhRotMatCalc(entry1P->mat, entry1P->invMat,
      entry1P->resP->atomA[defP->atom1I].coord,
      entry1P->resP->atomA[defP->atom2I].coord,
      entry1P->resP->atomA[defP->atom3I].coord,
      entry1P->resP->atomA[defP->atom4I].coord,
      entry1P->val);

  if (entry2P != NULL) {
    entry2P->val = entry1P->val;
    Mat4Copy(entry2P->mat, entry1P->invMat);
  }
}

static void
molMap(DhMolP molP, Mat4 m)
{
  Mat4 rotM;
  Vec3 transV;
  Vec4 v4;

  Mat4Copy(rotM, m);
  rotM[3][0] = 0.0f;
  rotM[3][1] = 0.0f;
  rotM[3][2] = 0.0f;
  Mat4Mult(molP->rotMat, rotM);

  transV[0] = m[3][0];
  transV[1] = m[3][1];
  transV[2] = m[3][2];
  Vec3Sub(molP->rotPoint, transV);
  Vec3To4(v4, molP->rotPoint);
  Mat4Transp(rotM);
  Mat4VecMult(v4, rotM);
  Vec4To3(molP->rotPoint, v4);
}

void
DhRotEnd(BOOL update)
{
  RotEntry *entryP, *prevEntryP;
  Mat4 m;

  if (update) {
    entryP = ListFirst(RotList);
    while (entryP != NULL) {
      entryP->angleP->val = entryP->val;
      entryP->angleP->changed = TRUE;
      entryP = ListNext(RotList, entryP);
    }

    Mat4Ident(m);
    entryP = ListLast(RotList);
    while (entryP != NULL) {
      prevEntryP = ListPrev(RotList, entryP);
      if (! entryP->forward)
	Mat4Mult(m, entryP->invMat);
      if (prevEntryP == NULL || prevEntryP->molP != entryP->molP) {
	molMap(entryP->molP, m);
	Mat4Ident(m);
      }
      entryP = prevEntryP;
    }
  }

  ListClose(RotList);
  RotList = NULL;
  RotMode = FALSE;
}

BOOL
DhRotMode(void)
{
  return RotMode;
}

LINLIST
DhMolListGet(void)
{
  return MolList;
}

LINLIST
DhBondListGet(void)
{
  return BondList;
}

LINLIST
DhAngleListGet(void)
{
  return AngleList;
}

LINLIST
DhDistListGet(void)
{
  return DistList;
}

LINLIST
DhAltCoordListGet(void)
{
  return AltCoordList;
}
