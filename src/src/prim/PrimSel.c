/*
************************************************************************
*
*   PrimSel.c - utility functions for selection
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
*   Pathname of SCCS file     : /sgiext/molmol/src/prim/SCCS/s.PrimSel.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include <prim_sel.h>

#include <stdio.h>

#include <linlist.h>

typedef struct {
  PrimSelCB func;
} SelCBInfo;

typedef struct {
  PrimObjP *selArr;
  int size;
  int selNo;
} SelInfo;

static PropRefP AllRefP = NULL;
static PropRefP SelRefP = NULL;
static LINLIST SelCBList = NULL;

void
PrimSelAddCB(PrimSelCB func)
{
  SelCBInfo entry;

  if (SelCBList == NULL)
    SelCBList = ListOpen(sizeof(SelCBInfo));
  
  entry.func = func;
  (void) ListInsertLast(SelCBList, &entry);
}

static void
callCB(PrimObjP primP, SelKind kind)
{
  SelCBInfo *entryP;

  entryP = ListFirst(SelCBList);
  while (entryP != NULL) {
    entryP->func(primP, kind);
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
getSelPrim(PrimObjP primP, void *clientData)
{
  SelInfo *selInfoP = clientData;

  selInfoP->selNo++;
  if (selInfoP->selNo <= selInfoP->size)
    selInfoP->selArr[selInfoP->selNo - 1] = primP;
}

int
PrimSelGet(PrimObjP primPA[], int size)
{
  SelInfo selInfo;

  getRef();

  selInfo.selArr = primPA;
  selInfo.size = size;
  selInfo.selNo = 0;

  PrimApply(PT_ALL, SelRefP, getSelPrim, &selInfo);

  return selInfo.selNo;
}

void
PrimSel(PrimObjP primP, SelKind kind)
{
  getRef();

  if (kind == SK_ADD) {
    PrimSetProp(SelRefP, primP, TRUE);
  } else if (kind == SK_REMOVE) {
    PrimSetProp(SelRefP, primP, FALSE);
  } else {
    PrimDeselAll();
    PrimSetProp(SelRefP, primP, TRUE);
  }

  callCB(primP, kind);
}

static void
deselPrim(PrimObjP primP, void *clientData)
{
  PrimSetProp(SelRefP, primP, FALSE);
}

void
PrimDeselAll(void)
{
  getRef();
  PrimApply(PT_ALL, AllRefP, deselPrim, NULL);
}
