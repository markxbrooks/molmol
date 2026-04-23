/*
************************************************************************
*
*   %M% - handle callbacks for data initialization
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
*   Date of last modification : %E%
*   Pathname of SCCS file     : %P%
*   SCCS identification       : %I%
*
************************************************************************
*/

#include <data_hand.h>

#include <stdlib.h>

#include <bool.h>
#include <linlist.h>
#include "prop_stand.h"

typedef struct {
  DhMolApplyFunc initCB;
  void *clientData;
} MolInitData;

typedef struct {
  DhResApplyFunc initCB;
  void *clientData;
} ResInitData;

typedef struct {
  DhAtomApplyFunc initCB;
  void *clientData;
} AtomInitData;

typedef struct {
  DhBondApplyFunc initCB;
  void *clientData;
} BondInitData;

typedef struct {
  DhDistApplyFunc initCB;
  void *clientData;
} DistInitData;

static LINLIST MolInitList = NULL;
static LINLIST ResInitList = NULL;
static LINLIST AtomInitList = NULL;
static LINLIST BondInitList = NULL;
static LINLIST DistInitList = NULL;

/* Do not execute "lower-level" callbacks while "higher-level" callback
   is running. E. g. if macro for initialization of molecule adds bonds,
   it should do the desired intialization itself, and not execute the
   macro for initialization of bonds for each bond. */
static BOOL InitRunning = FALSE;

void
DhAddMolInitCB(DhMolApplyFunc initCB, void *clientData)
{
  MolInitData entry;

  entry.initCB = initCB;
  entry.clientData = clientData;

  if (MolInitList == NULL)
    MolInitList = ListOpen(sizeof(MolInitData));

  (void) ListInsertLast(MolInitList, &entry);
}

void
DhAddResInitCB(DhResApplyFunc initCB, void *clientData)
{
  ResInitData entry;

  entry.initCB = initCB;
  entry.clientData = clientData;

  if (ResInitList == NULL)
    ResInitList = ListOpen(sizeof(ResInitData));

  (void) ListInsertLast(ResInitList, &entry);
}

void
DhAddAtomInitCB(DhAtomApplyFunc initCB, void *clientData)
{
  AtomInitData entry;

  entry.initCB = initCB;
  entry.clientData = clientData;

  if (AtomInitList == NULL)
    AtomInitList = ListOpen(sizeof(AtomInitData));

  (void) ListInsertLast(AtomInitList, &entry);
}

void
DhAddBondInitCB(DhBondApplyFunc initCB, void *clientData)
{
  BondInitData entry;

  entry.initCB = initCB;
  entry.clientData = clientData;

  if (BondInitList == NULL)
    BondInitList = ListOpen(sizeof(BondInitData));

  (void) ListInsertLast(BondInitList, &entry);
}

void
DhAddDistInitCB(DhDistApplyFunc initCB, void *clientData)
{
  DistInitData entry;

  entry.initCB = initCB;
  entry.clientData = clientData;

  if (DistInitList == NULL)
    DistInitList = ListOpen(sizeof(DistInitData));

  (void) ListInsertLast(DistInitList, &entry);
}

void
DhMolInit(DhMolP molP)
{
  MolInitData *entryP;

  PropStandSetMol(molP);

  InitRunning = TRUE;

  entryP = ListFirst(MolInitList);
  while (entryP != NULL) {
    entryP->initCB(molP, entryP->clientData);
    entryP = ListNext(MolInitList, entryP);
  }

  InitRunning = FALSE;
}

void
DhResInit(DhResP resP)
{
  ResInitData *entryP;

  PropStandSetRes(resP);

  if (InitRunning)
    return;

  InitRunning = TRUE;

  entryP = ListFirst(ResInitList);
  while (entryP != NULL) {
    entryP->initCB(resP, entryP->clientData);
    entryP = ListNext(ResInitList, entryP);
  }

  InitRunning = FALSE;
}

void
DhAtomInit(DhAtomP atomP)
{
  AtomInitData *entryP;

  PropStandSetAtom(atomP);

  if (InitRunning)
    return;

  entryP = ListFirst(AtomInitList);
  while (entryP != NULL) {
    entryP->initCB(atomP, entryP->clientData);
    entryP = ListNext(AtomInitList, entryP);
  }
}

void
DhBondInit(DhBondP bondP)
{
  BondInitData *entryP;

  PropStandSetBond(bondP);

  if (InitRunning)
    return;

  entryP = ListFirst(BondInitList);
  while (entryP != NULL) {
    entryP->initCB(bondP, DhBondGetAtom1(bondP), DhBondGetAtom2(bondP),
        entryP->clientData);
    entryP = ListNext(BondInitList, entryP);
  }
}

void
DhDistInit(DhDistP distP)
{
  DistInitData *entryP;

  PropStandSetDist(distP);

  if (InitRunning)
    return;

  entryP = ListFirst(DistInitList);
  while (entryP != NULL) {
    entryP->initCB(distP, DhDistGetAtom1(distP), DhDistGetAtom2(distP),
        entryP->clientData);
    entryP = ListNext(DistInitList, entryP);
  }
}
