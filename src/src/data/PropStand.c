/*
************************************************************************
*
*   PropStand.c - handle standard (predefined) properties
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/data/SCCS/s.PropStand.c
*   SCCS identification       : 1.10
*
************************************************************************
*/

#include <data_hand.h>
#include "prop_stand.h"

#include <stdio.h>
#include <string.h>

#include <dstr.h>
#include <linlist.h>
#include <expr.h>
#include <par_names.h>
#include <setup_file.h>
#include "prop_cb.h"

#define PROP_NAME_LEN 100
#define EXPR_LEN 1000

typedef struct {
  DataEntityType entType;
  DSTR name;
  PropRefP refP;
  DSTR exprStr;
  ExprP exprP;
  LINLIST molList;
  LINLIST dependList;
} ListEntry;

typedef struct {
  GFile gf;
  BOOL ok;
} DumpData;

static LINLIST ExprList = NULL;
static ListEntry *CurrEntryP = NULL;
static BOOL InvalidCBAdded = FALSE;

static void
addDepend(char *name, PropRefP refP, void *clientData)
{
  ListEntry *entryP = clientData;

  if (CurrEntryP == NULL)
    return;

  if (CurrEntryP->dependList == NULL)
    CurrEntryP->dependList = ListOpen(sizeof(ListEntry *));
  
  (void) ListInsertLast(CurrEntryP->dependList, &entryP);
}

static void
readList(void)
/* Read expressions from setup file. Silently ignores errors! */
{
  GFile gf;
  GFileRes res;
  ListEntry entry, *entryP;
  char entName[14], propName[PROP_NAME_LEN], exprStr[EXPR_LEN];
  ExprResType resType;

  if (ExprList != NULL)
    return;

  ExprList = ListOpen(sizeof(ListEntry));

  gf = SetupOpen(PN_PROP_DEF, "PropDef", FALSE);
  if (gf == NULL)
    return;
  
  while (! GFileEOF(gf)) {
    res = GFileReadStr(gf, entName, sizeof(entName));
    if (res != GF_RES_OK)
      break;

    if (strcmp(entName, "mol") == 0)
      entry.entType = DE_MOL;
    else if (strcmp(entName, "res") == 0)
      entry.entType = DE_RES;
    else if (strcmp(entName, "atom") == 0)
      entry.entType = DE_ATOM;
    else if (strcmp(entName, "bond") == 0)
      entry.entType = DE_BOND;
    else if (strcmp(entName, "angle") == 0)
      entry.entType = DE_ANGLE;
    else if (strcmp(entName, "dist") == 0)
      entry.entType = DE_DIST;
    else
      break;

    res = GFileReadStr(gf, propName, sizeof(propName));
    if (res != GF_RES_OK)
      break;

    entry.refP = PropGetRef(propName, TRUE);

    res = GFileReadQuotedStr(gf, exprStr, sizeof(exprStr));
    if (res != GF_RES_OK)
      break;

    entry.dependList = NULL;
    CurrEntryP = &entry;  /* addDepend must add to dependList */
    entry.exprP = ExprCompile(entry.entType, exprStr, &resType);
    CurrEntryP = NULL;

    if (entry.exprP == NULL) {
      ListClose(entry.dependList);
      continue;
    }

    if (resType != ER_BOOL) {
      ListClose(entry.dependList);
      ExprFree(entry.exprP);
      continue;
    }

    entry.name = DStrNew();
    DStrAssignStr(entry.name, propName);

    entry.exprStr = DStrNew();
    DStrAssignStr(entry.exprStr, exprStr);

    entry.molList = NULL;

    entryP = ListInsertLast(ExprList, &entry);
    PropAddAccessCB(propName, addDepend, entryP);
  }

  entryP = ListFirst(ExprList);
  while (entryP != NULL) {
    PropRemoveAccessCB(DStrToStr(entryP->name), addDepend, entryP);
    entryP = ListNext(ExprList, entryP);
  }

  GFileClose(gf);
}

static void
setRes(DhResP resP, void *clientData)
{
  ListEntry *entryP = clientData;
  ExprRes exprRes;

  ExprEval(resP, entryP->exprP, &exprRes);
  if (exprRes.u.boolVal)
    DhResSetProp(entryP->refP, resP, TRUE);
}

static void
setAtom(DhAtomP atomP, void *clientData)
{
  ListEntry *entryP = clientData;
  ExprRes exprRes;

  ExprEval(atomP, entryP->exprP, &exprRes);
  if (exprRes.u.boolVal)
    DhAtomSetProp(entryP->refP, atomP, TRUE);
}

static void
setBond(DhBondP bondP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  ListEntry *entryP = clientData;
  ExprRes exprRes;

  ExprEval(bondP, entryP->exprP, &exprRes);
  if (exprRes.u.boolVal)
    DhBondSetProp(entryP->refP, bondP, TRUE);
}

static void
setAngle(DhAngleP angleP, void *clientData)
{
  ListEntry *entryP = clientData;
  ExprRes exprRes;

  ExprEval(angleP, entryP->exprP, &exprRes);
  if (exprRes.u.boolVal)
    DhAngleSetProp(entryP->refP, angleP, TRUE);
}

static void
setDist(DhDistP distP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  ListEntry *entryP = clientData;
  ExprRes exprRes;

  ExprEval(distP, entryP->exprP, &exprRes);
  if (exprRes.u.boolVal)
    DhDistSetProp(entryP->refP, distP, TRUE);
}

static void setEntryCB(char *name, PropRefP refP, void *clientData);

static void
setEntry(ListEntry *entryP)
{
  ListEntry **dependEntryPP;
  PropRefP allRefP;
  DhMolP *molPP, molP;
  ExprRes exprRes;

  if (ListSize(entryP->molList) == 0)
    return;

  dependEntryPP = ListFirst(entryP->dependList);
  while (dependEntryPP != NULL) {
    setEntry(*dependEntryPP);
    dependEntryPP = ListNext(entryP->dependList, dependEntryPP);
  }

  allRefP = PropGetRef(PROP_ALL, FALSE);

  molPP = ListFirst(entryP->molList);
  while (molPP != NULL) {
    molP = *molPP;

    switch (entryP->entType) {
      case DE_MOL:
	ExprEval(molP, entryP->exprP, &exprRes);
	if (exprRes.u.boolVal)
	  DhMolSetProp(entryP->refP, molP, TRUE);
	break;
      case DE_RES:
	DhMolApplyRes(allRefP, molP, setRes, entryP);
	break;
      case DE_ATOM:
	DhMolApplyAtom(allRefP, molP, setAtom, entryP);
	break;
      case DE_BOND:
	DhMolApplyBond(allRefP, molP, setBond, entryP);
	break;
      case DE_ANGLE:
	DhMolApplyAngle(allRefP, molP, setAngle, entryP);
	break;
      case DE_DIST:
	DhMolApplyDist(allRefP, molP, setDist, entryP);
	break;
    }

    molPP = ListNext(entryP->molList, molPP);
  }

  PropRemoveAccessCB(DStrToStr(entryP->name), setEntryCB, entryP);
  ListClose(entryP->molList);
  entryP->molList = NULL;
}

static void
setEntryCB(char *name, PropRefP refP, void *clientData)
{
  ListEntry *entryP = clientData;

  setEntry(entryP);
}

static void
invalidMolCB(DhMolP molP, void *clientData)
{
  ListEntry *entryP;
  DhMolP *molPP;

  entryP = ListFirst(ExprList);
  while (entryP != NULL) {
    if (ListSize(entryP->molList) > 0) {
      molPP = ListFirst(entryP->molList);
      while (molPP != NULL) {
	if (*molPP == molP) {
	  ListRemove(entryP->molList, molPP);
	  break;
	}
	molPP = ListNext(entryP->molList, molPP);
      }

      if (ListSize(entryP->molList) == 0)
	PropRemoveAccessCB(DStrToStr(entryP->name), setEntryCB, entryP);
    }
    entryP = ListNext(ExprList, entryP);
  }
}

void
PropStandSetMol(DhMolP molP)
{
  ListEntry *entryP;

  readList();

  entryP = ListFirst(ExprList);
  while (entryP != NULL) {
    if (ListSize(entryP->molList) == 0) {
      if (entryP->molList == NULL)
	entryP->molList = ListOpen(sizeof(DhMolP));
      PropAddAccessCB(DStrToStr(entryP->name), setEntryCB, entryP);
    }

    (void) ListInsertLast(entryP->molList, &molP);

    entryP = ListNext(ExprList, entryP);
  }

  if (! InvalidCBAdded) {
    DhAddMolInvalidCB(invalidMolCB, NULL);
    InvalidCBAdded = TRUE;
  }
}

void
PropStandSetRes(DhResP resP)
{
  PropRefP allRefP;
  ListEntry *entryP;
  ExprRes exprRes;

  readList();

  allRefP = PropGetRef(PROP_ALL, FALSE);

  entryP = ListFirst(ExprList);
  while (entryP != NULL) {
    switch (entryP->entType) {
      case DE_RES:
	ExprEval(resP, entryP->exprP, &exprRes);
	if (exprRes.u.boolVal)
	  DhResSetProp(entryP->refP, resP, TRUE);
	break;
      case DE_ATOM:
	DhResApplyAtom(allRefP, resP, setAtom, entryP);
	break;
      case DE_BOND:
	DhResApplyBond(allRefP, resP, setBond, entryP);
	break;
      case DE_ANGLE:
	DhResApplyAngle(allRefP, resP, setAngle, entryP);
	break;
    }
    entryP = ListNext(ExprList, entryP);
  }
}

void
PropStandSetAtom(DhAtomP atomP)
{
  ListEntry *entryP;
  ExprRes exprRes;

  readList();

  entryP = ListFirst(ExprList);
  while (entryP != NULL) {
    if (entryP->entType == DE_ATOM) {
	ExprEval(atomP, entryP->exprP, &exprRes);
	if (exprRes.u.boolVal)
	  DhAtomSetProp(entryP->refP, atomP, TRUE);
    }
    entryP = ListNext(ExprList, entryP);
  }
}

void
PropStandSetBond(DhBondP bondP)
{
  ListEntry *entryP;
  ExprRes exprRes;

  readList();

  entryP = ListFirst(ExprList);
  while (entryP != NULL) {
    if (entryP->entType == DE_BOND) {
	ExprEval(bondP, entryP->exprP, &exprRes);
	if (exprRes.u.boolVal)
	  DhBondSetProp(entryP->refP, bondP, TRUE);
    }
    entryP = ListNext(ExprList, entryP);
  }
}

void
PropStandSetDist(DhDistP distP)
{
  ListEntry *entryP;
  ExprRes exprRes;

  readList();

  entryP = ListFirst(ExprList);
  while (entryP != NULL) {
    if (entryP->entType == DE_DIST) {
	ExprEval(distP, entryP->exprP, &exprRes);
	if (exprRes.u.boolVal)
	  DhDistSetProp(entryP->refP, distP, TRUE);
    }
    entryP = ListNext(ExprList, entryP);
  }
}

#define CHECK_RES(s) if (s != GF_RES_OK) return FALSE

BOOL
PropStandDump(GFile gf)
{
  ListEntry *entryP;
  DhMolP *molPP;

  CHECK_RES(GFileWriteINT32(gf, ListSize(ExprList)));

  entryP = ListFirst(ExprList);
  while (entryP != NULL) {
    CHECK_RES(GFileWriteINT16(gf, (INT16) entryP->entType));
    CHECK_RES(GFileWriteStr(gf, DStrToStr(entryP->name)));
    CHECK_RES(GFileWriteQuotedStr(gf, DStrToStr(entryP->exprStr)));
    CHECK_RES(GFileWriteINT32(gf, ListSize(entryP->molList)));
    molPP = ListFirst(entryP->molList);
    while (molPP != NULL) {
      CHECK_RES(GFileWriteINT32(gf, DhMolGetNumber(*molPP)));
      molPP = ListNext(entryP->molList, molPP);
    }

    entryP = ListNext(ExprList, entryP);
  }

  return TRUE;
}

void
PropStandDestroyAll(void)
{
  ListEntry *entryP;

  entryP = ListFirst(ExprList);
  while (entryP != NULL) {
    DStrFree(entryP->name);
    DStrFree(entryP->exprStr);
    ExprFree(entryP->exprP);
    ListClose(entryP->molList);
    ListClose(entryP->dependList);
    entryP = ListNext(ExprList, entryP);
  }

  ListClose(ExprList);
  ExprList = NULL;
}

BOOL
PropStandUndump(GFile gf, int vers)
{
  ListEntry entry, *entryP;
  int listSize, entryI;
  char propName[PROP_NAME_LEN], exprStr[EXPR_LEN];
  ExprResType resType;
  int molNo, molI;
  DhMolP molP;
  INT32 i32;
  INT16 i16;

  if (vers < 6)
    /* does not contain standard properties, current table is already
       deleted, just let it re-read from file when it's needed again */
    return TRUE;

  CHECK_RES(GFileReadINT32(gf, &i32));
  listSize = i32;
  if (listSize == 0)
    return TRUE;

  ExprList = ListOpen(sizeof(ListEntry));

  for (entryI = 0; entryI < listSize; entryI++) {
    CHECK_RES(GFileReadINT16(gf, &i16));
    entry.entType = i16;
    CHECK_RES(GFileReadStr(gf, propName, sizeof(propName)));
    entry.name = DStrNew();
    entry.refP = PropGetRef(propName, TRUE);
    DStrAssignStr(entry.name, propName);
    CHECK_RES(GFileReadQuotedStr(gf, exprStr, sizeof(exprStr)));
    entry.exprStr = DStrNew();
    DStrAssignStr(entry.exprStr, exprStr);

    entry.dependList = NULL;
    CurrEntryP = &entry;  /* addDepend must add to dependList */
    entry.exprP = ExprCompile(entry.entType, exprStr, &resType);
    CurrEntryP = NULL;

    CHECK_RES(GFileReadINT32(gf, &i32));
    molNo = i32;
    if (molNo > 0) {
      entry.molList = ListOpen(sizeof(DhMolP));
      for (molI = 0; molI < molNo; molI++) {
	CHECK_RES(GFileReadINT32(gf, &i32));
	molP = DhMolFindNumber(i32);
	(void) ListInsertLast(entry.molList, &molP);
      }
    } else {
      entry.molList = NULL;
    }

    entryP = ListInsertLast(ExprList, &entry);
    PropAddAccessCB(propName, addDepend, entryP);
  }

  entryP = ListFirst(ExprList);
  while (entryP != NULL) {
    PropRemoveAccessCB(DStrToStr(entryP->name), addDepend, entryP);

    if (entryP->molList != NULL)
      PropAddAccessCB(DStrToStr(entryP->name), setEntryCB, entryP);

    entryP = ListNext(ExprList, entryP);
  }

  if (! InvalidCBAdded) {
    DhAddMolInvalidCB(invalidMolCB, NULL);
    InvalidCBAdded = TRUE;
  }

  return TRUE;
}
