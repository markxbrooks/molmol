/*
************************************************************************
*
*   DataDist.c - manage list of distances
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
*   Date of last modification : 99/10/30
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/src/data/SCCS/s.DataDist.c
*   SCCS identification       : 1.8
*
************************************************************************
*/

#include <data_dist.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <hashtab.h>

struct DistListS {
  DhDistKind kind;
  BOOL interMol;
  HASHTABLE tab;
  int entryNo;
};

typedef struct {
  DhDistP distP;
  int res1I, res2I;
  int atom1I, atom2I;
  float limit;
} TabEntryIntra;

typedef struct {
  DhDistP distP;
  DhAtomP atom1P;
  DhAtomP atom2P;
} TabEntryInter;

typedef struct {
  DistListApplyF applyF;
  void *clientData;
} ApplyData;

static unsigned
hashIntra(void *p, unsigned size)
{
  TabEntryIntra *entryP = p;
  unsigned idx;

  idx = entryP->res1I;
  idx = 37 * idx + entryP->res2I;
  idx = 37 * idx + entryP->atom1I;
  idx = 37 * idx + entryP->atom2I;

  return idx % size;
}

static int
compIntra(void *p1, void *p2)
{
  TabEntryIntra *entry1P = p1;
  TabEntryIntra *entry2P = p2;

  if (entry1P->res1I == entry2P->res1I &&
      entry1P->res2I == entry2P->res2I &&
      entry1P->atom1I == entry2P->atom1I &&
      entry1P->atom2I == entry2P->atom2I)
    return 0;
  
  return 1;
}

static unsigned
hashInter(void *p, unsigned size)
{
  TabEntryInter *entryP = p;

  return ((unsigned)(uintptr_t) entryP->atom1P + (unsigned)(uintptr_t) entryP->atom2P) % size;
}

static int
compInter(void *p1, void *p2)
{
  TabEntryInter *entry1P = p1;
  TabEntryInter *entry2P = p2;

  if (entry1P->atom1P == entry2P->atom1P &&
      entry1P->atom2P == entry2P->atom2P)
    return 0;
  
  return 1;
}

static void
fillIntra(TabEntryIntra *entryP, DhAtomP atom1P, DhAtomP atom2P)
{
  int t;

  entryP->res1I = DhResGetNumber(DhAtomGetRes(atom1P));
  entryP->res2I = DhResGetNumber(DhAtomGetRes(atom2P));
  entryP->atom1I = DhAtomGetNumber(atom1P);
  entryP->atom2I = DhAtomGetNumber(atom2P);

  if (entryP->res1I > entryP->res2I ||
      (entryP->res1I == entryP->res2I && entryP->atom1I > entryP->atom2I)) {
    t = entryP->res1I;
    entryP->res1I = entryP->res2I;
    entryP->res2I = t;

    t = entryP->atom1I;
    entryP->atom1I = entryP->atom2I;
    entryP->atom2I = t;
  }
}

static void
fillInter(TabEntryInter *entryP, DhAtomP atom1P, DhAtomP atom2P)
{
  if (atom1P < atom2P) {
    entryP->atom1P = atom1P;
    entryP->atom2P = atom2P;
  } else {
    entryP->atom1P = atom2P;
    entryP->atom2P = atom1P;
  }
}

static void
addIntra(DhDistP distP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  DistList list = clientData;
  TabEntryIntra entry, *entryP;
  float limit, val;

  if (DhDistGetKind(distP) != list->kind)
    return;

  fillIntra(&entry, atom1P, atom2P);

  limit = DhDistGetLimit(distP);
  val = DhDistGetVal(distP);

  entryP = HashtabSearch(list->tab, &entry);
  if (entryP == NULL) {
    entry.distP = distP;
    entry.limit = limit;
    (void) HashtabInsert(list->tab, &entry, FALSE);
    list->entryNo++;
  } else {
    /* store strongest constraint or shortest distance */
    if (list->kind == DK_LOWER) {
      if (limit > entryP->limit) {
	entryP->distP = distP;
	entryP->limit = limit;
      }
    } else if (list->kind == DK_UPPER) {
      if (limit < entryP->limit) {
	entryP->distP = distP;
	entryP->limit = limit;
      }
    } else {
      if (val < entryP->limit) {
	entryP->distP = distP;
	entryP->limit = val;
      }
    }
  }
}

static void
addInter(DhDistP distP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  DistList list = clientData;
  TabEntryInter entry;

  if (DhDistGetKind(distP) != list->kind)
    return;

  entry.distP = distP;
  fillInter(&entry, atom1P, atom2P);

  (void) HashtabInsert(list->tab, &entry, TRUE);
  list->entryNo++;
}

DistList
DistListBuildIntra(PropRefP refP, DhMolP *molPA, int molNo, DhDistKind kind)
{
  DistList list;
  int i;

  list = malloc(sizeof(*list));
  list->kind = kind;
  list->interMol = FALSE;
  list->tab = HashtabOpen(997, sizeof(TabEntryIntra), hashIntra, compIntra);
  list->entryNo = 0;
  for (i = 0; i < molNo; i++)
    DhMolApplyDist(refP, molPA[i], addIntra, list);

  return list;
}

DistList
DistListBuildInter(PropRefP refP, DhDistKind kind)
{
  DistList list;

  list = malloc(sizeof(*list));
  list->kind = kind;
  list->interMol = TRUE;
  list->tab = HashtabOpen(997, sizeof(TabEntryInter), hashInter, compInter);
  list->entryNo = 0;
  DhApplyDist(refP, addInter, list);

  return list;
}

static DhDistP
findIntra(DistList list, DhAtomP atom1P, DhAtomP atom2P)
{
  TabEntryIntra entry, *entryP;

  fillIntra(&entry, atom1P, atom2P);

  entryP = HashtabSearch(list->tab, &entry);
  if (entryP == NULL)
    return NULL;
  else
    return entryP->distP;
}

static DhDistP
findInter(DistList list, DhAtomP atom1P, DhAtomP atom2P)
{
  TabEntryInter entry, *entryP;

  fillInter(&entry, atom1P, atom2P);

  entryP = HashtabSearch(list->tab, &entry);
  if (entryP == NULL)
    return NULL;
  else
    return entryP->distP;
}

DhDistP
DistListFind(DistList list, DhAtomP atom1P, DhAtomP atom2P)
{
  if (list->entryNo == 0)
    return NULL;

  if (list->interMol)
    return findInter(list, atom1P, atom2P);
  else
    return findIntra(list, atom1P, atom2P);
}

static void
callIntra(void *p, void *clientData)
{
  TabEntryIntra *entryP = p;
  ApplyData *applyP = clientData;

  applyP->applyF(entryP->distP, applyP->clientData);
}

static void
callInter(void *p, void *clientData)
{
  TabEntryInter *entryP = p;
  ApplyData *applyP = clientData;

  applyP->applyF(entryP->distP, applyP->clientData);
}

void
DistListApply(DistList list, DistListApplyF applyF, void *clientData)
{
  ApplyData data;

  data.applyF = applyF;
  data.clientData = clientData;

  if (list->interMol)
    HashtabApply(list->tab, callInter, &data);
  else
    HashtabApply(list->tab, callIntra, &data);
}

void
DistListFree(DistList list)
{
  HashtabClose(list->tab);
  free(list);
}
