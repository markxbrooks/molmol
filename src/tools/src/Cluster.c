/*
************************************************************************
*
*   Cluster.c - clustering algorithm
*
*   Copyright (c) 1995-96
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
*   Date of last modification : 01/05/26
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/tools/src/SCCS/s.Cluster.c
*   SCCS identification       : 1.6
*
************************************************************************
*/

#include <cluster.h>

#include <stdio.h>
#include <stdlib.h>
#include <values.h>

#include <linlist.h>

typedef struct {
  LINLIST entryL;
  float maxDiff;
  LINLIST pairL;
} Group, *GroupP;

typedef struct PairS *PairP;

typedef struct PairS {
  GroupP group1P, group2P;
  PairP *ref1P, *ref2P;
  float diff;
} Pair;

static LINLIST GroupL = NULL;
static LINLIST PairL = NULL;

ClusterEntryP
ClusterAddEntry(void *entryP)
{
  Group group;

  if (GroupL == NULL)
    GroupL = ListOpen(sizeof(Group));

  group.entryL = ListOpen(sizeof(void *));
  (void) ListInsertLast(group.entryL, &entryP);
  group.maxDiff = 0.0f;
  group.pairL = ListOpen(sizeof(PairP));

  return (ClusterEntryP) ListInsertLast(GroupL, &group);
}

static int
compGroupPair(void *p1, void *p2)
{
  PairP *pair1PP = p1;
  PairP *pair2PP = p2;
  PairP pair1P, pair2P;
  GroupP group1P, group2P;

  pair1P = *pair1PP;
  pair2P = *pair2PP;

  if (pair1P->group1P == pair2P->group1P) {
    group1P = pair1P->group2P;
    group2P = pair2P->group2P;
  } else if (pair1P->group1P == pair2P->group2P) {
    group1P = pair1P->group2P;
    group2P = pair2P->group1P;
  } else if (pair1P->group2P == pair2P->group1P) {
    group1P = pair1P->group1P;
    group2P = pair2P->group2P;
  } else {
    group1P = pair1P->group1P;
    group2P = pair2P->group1P;
  }

  if (group1P < group2P)
    return -1;

  if (group1P > group2P)
    return 1;
  
  return 0;
}

void
ClusterAddPair(ClusterEntryP entry1P, ClusterEntryP entry2P, float diff)
{
  GroupP group1P = (GroupP) entry1P;
  GroupP group2P = (GroupP) entry2P;
  Pair pair, *pairP;

  if (PairL == NULL)
    PairL = ListOpen(sizeof(Pair));

  pair.group1P = group1P;
  pair.group2P = group2P;
  pair.diff = diff;
  pairP = ListInsertLast(PairL, &pair);

  pairP->ref1P = ListInsertLast(group1P->pairL, &pairP);
  pairP->ref2P = ListInsertLast(group2P->pairL, &pairP);
}

static int
compPair(void *p1, void *p2)
{
  PairP pair1P = p1;
  PairP pair2P = p2;

  if (pair1P->diff < pair2P->diff)
    return -1;

  if (pair1P->diff > pair2P->diff)
    return 1;
  
  return 0;
}

static GroupP
getOtherGroup(PairP pairP, GroupP groupP)
{
  if (groupP == pairP->group1P)
    return pairP->group2P;
  else
    return pairP->group1P;
}

static void
removePair(PairP pairP)
{
  ListRemove(pairP->group1P->pairL, pairP->ref1P);
  ListRemove(pairP->group2P->pairL, pairP->ref2P);
  ListRemove(PairL, pairP);
}

static void
mergePair(void)
{
  PairP oldPairP, delPairP, pair2P;
  PairP *pair1PP, *pair2PP, *nextPairPP;
  GroupP group1P, group2P;
  GroupP otherGroup1P, otherGroup2P;
  void **entryPP;

  oldPairP = ListFirst(PairL);
  group1P = oldPairP->group1P;
  group2P = oldPairP->group2P;

  group1P->maxDiff = oldPairP->diff;

  removePair(oldPairP);

  /* merge pairs */
  pair1PP = ListFirst(group1P->pairL);
  pair2PP = ListFirst(group2P->pairL);
  while (pair1PP != NULL || pair2PP != NULL) {
    if (pair1PP == NULL) {
      nextPairPP = ListNext(group2P->pairL, pair2PP);
      removePair(*pair2PP);
      pair2PP = nextPairPP;

      continue;
    }

    if (pair2PP == NULL) {
      nextPairPP = ListNext(group1P->pairL, pair1PP);
      removePair(*pair1PP);
      pair1PP = nextPairPP;

      continue;
    }

    otherGroup1P = getOtherGroup(*pair1PP, group1P);
    otherGroup2P = getOtherGroup(*pair2PP, group2P);

    if (otherGroup1P == otherGroup2P) {
      if ((*pair1PP)->diff < (*pair2PP)->diff) {
	nextPairPP = ListNext(group1P->pairL, pair1PP);
	delPairP = *pair1PP;
	pair2P = *pair2PP;
	pair2PP = ListNext(group2P->pairL, pair2PP);
	ListRemove(pair2P->group1P->pairL, pair2P->ref1P);
	ListRemove(pair2P->group2P->pairL, pair2P->ref2P);
	pair2P->group1P = (*pair1PP)->group1P;
	pair2P->group2P = (*pair1PP)->group2P;
	pair2P->ref1P = (*pair1PP)->ref1P;
	pair2P->ref2P = (*pair1PP)->ref2P;
	*(*pair1PP)->ref1P = pair2P;
	*(*pair1PP)->ref2P = pair2P;
	ListRemove(PairL, delPairP);
	*pair1PP = pair2P;
	pair1PP = nextPairPP;
      } else {
	nextPairPP = ListNext(group2P->pairL, pair2PP);
	removePair(*pair2PP);
	pair1PP = ListNext(group1P->pairL, pair1PP);
	pair2PP = nextPairPP;
      }
    } else if (otherGroup1P < otherGroup2P) {
      nextPairPP = ListNext(group1P->pairL, pair1PP);
      removePair(*pair1PP);
      pair1PP = nextPairPP;
    } else {
      nextPairPP = ListNext(group2P->pairL, pair2PP);
      removePair(*pair2PP);
      pair2PP = nextPairPP;
    }
  }

  /* merge groups */
  entryPP = ListFirst(group2P->entryL);
  while (entryPP != NULL) {
    (void) ListInsertLast(group1P->entryL, entryPP);
    entryPP = ListNext(group2P->entryL, entryPP);
  }

  ListClose(group2P->entryL);
  ListClose(group2P->pairL);
  ListRemove(GroupL, group2P);
}

static void
freeAll(void)
{
  GroupP groupP;

  groupP = ListFirst(GroupL);
  while (groupP != NULL) {
    ListClose(groupP->pairL);
    ListClose(groupP->entryL);
    groupP = ListNext(GroupL, groupP);
  }

  ListClose(GroupL);
  GroupL = NULL;

  ListClose(PairL);
  PairL = NULL;
}

void
ClusterCalc(int groupNo, float minDiff,
    ClusterReportF reportF, void *clientData)
{
  PairP pairP, *pairPP;
  GroupP groupP;
  int groupSize, idx;
  float diff;
  void **entryPP;

  groupP = ListFirst(GroupL);
  while (groupP != NULL) {
    ListSort(groupP->pairL, compGroupPair);
    groupP = ListNext(GroupL, groupP);
  }
  ListSort(PairL, compPair);

  for (;;) {
    pairP = ListFirst(PairL);
    if (pairP == NULL)
      break;
    if (pairP->diff > minDiff && ListSize(GroupL) <= (unsigned) groupNo)
      break;
    mergePair();
  }

  groupP = ListFirst(GroupL);
  while (groupP != NULL) {
    groupSize = ListSize(groupP->entryL);

    pairPP = ListFirst(groupP->pairL);
    diff = MAXFLOAT;
    while (pairPP != NULL) {
      if ((*pairPP)->diff < diff)
	diff = (*pairPP)->diff;
      pairPP = ListNext(groupP->pairL, pairPP);
    }

    entryPP = ListFirst(groupP->entryL);
    idx = 0;
    while (entryPP != NULL) {
      reportF(idx, groupSize, *entryPP, groupP->maxDiff, diff, clientData);
      idx++;
      entryPP = ListNext(groupP->entryL, entryPP);
    }

    groupP = ListNext(GroupL, groupP);
  }

  freeAll();
}
