/*
************************************************************************
*
*   UnknownErr.c - utility functions for printing error messages (unknown ...)
*
*   Copyright (c) 1994-96
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
*   Date of last modification : 96/12/04
*   Pathname of SCCS file     : /local/home/kor/molmol/src/cmdio/SCCS/s.UnknownErr.c
*   SCCS identification       : 1.1
*
************************************************************************
*/

#include "unknown_err.h"

#include <stdio.h>

#include <tree.h>

typedef struct {
  DSTR name;
} ResDescr;

typedef struct {
  DSTR resName;
  DSTR atomName;
} AtomDescr;

typedef struct {
  DSTR resErrStr;
  DSTR atomErrStr;
  DSTR lastResName;
} ErrMsg;

static TREE ResTree = NULL, AtomTree = NULL;

static int
compRes(void *p1, void *p2)
{
  ResDescr *descr1P = p1;
  ResDescr *descr2P = p2;

  return DStrCmp(descr1P->name, descr2P->name);
}

static int
compAtom(void *p1, void *p2)
{
  AtomDescr *descr1P = p1;
  AtomDescr *descr2P = p2;
  int res;

  res = DStrCmp(descr1P->resName, descr2P->resName);
  if (res != 0)
    return res;

  return DStrCmp(descr1P->atomName, descr2P->atomName);
}

static void
freeRes(void *p, void *clientData)
{
  ResDescr *descrP = p;

  DStrFree(descrP->name);
}

static void
freeAtom(void *p, void *clientData)
{
  AtomDescr *descrP = p;

  DStrFree(descrP->resName);
  DStrFree(descrP->atomName);
}

void
UnknownErrInit(void)
{
  if (ResTree != NULL)
    TreeClose(ResTree);

  if (AtomTree != NULL)
    TreeClose(AtomTree);
  
  ResTree = TreeOpen(sizeof(ResDescr), compRes);
  TreeAddDestroyCB(ResTree, freeRes, NULL, NULL);

  AtomTree = TreeOpen(sizeof(AtomDescr), compAtom);
  TreeAddDestroyCB(AtomTree, freeAtom, NULL, NULL);
}

void
UnknownErrAddRes(DSTR name)
{
  ResDescr descr;

  descr.name = DStrNew();
  DStrAssignDStr(descr.name, name);

  if (TreeInsert(ResTree, &descr, TIB_RETURN_NULL) == NULL)
    DStrFree(descr.name);
}

void
UnknownErrAddAtom(DSTR resName, DSTR atomName)
{
  AtomDescr descr;

  descr.resName = DStrNew();
  DStrAssignDStr(descr.resName, resName);

  descr.atomName = DStrNew();
  DStrAssignDStr(descr.atomName, atomName);

  if (TreeInsert(AtomTree, &descr, TIB_RETURN_NULL) == NULL) {
    DStrFree(descr.resName);
    DStrFree(descr.atomName);
  }
}

static void
getRes(void *p, void *clientData)
{
  ResDescr *descrP = p;
  ErrMsg *msgP = clientData;

  if (DStrLen(msgP->resErrStr) > 0)
    DStrAppStr(msgP->resErrStr, ", ");
  
  DStrAppDStr(msgP->resErrStr, descrP->name);
}

static void
getAtom(void *p, void *clientData)
{
  AtomDescr *descrP = p;
  ErrMsg *msgP = clientData;

  if (msgP->lastResName == NULL) {
    msgP->lastResName = descrP->resName;
  } else if (DStrCmp(descrP->resName, msgP->lastResName) != 0) {
    DStrAppStr(msgP->atomErrStr, " (");
    DStrAppDStr(msgP->atomErrStr, msgP->lastResName);
    DStrAppStr(msgP->atomErrStr, ")");
    msgP->lastResName = descrP->resName;
  }
  
  if (DStrLen(msgP->atomErrStr) > 0)
    DStrAppStr(msgP->atomErrStr, ", ");

  DStrAppDStr(msgP->atomErrStr, descrP->atomName);
}

DSTR
UnknownErrGet(void)
/* return value must be freed with DStrFree() */
{
  ErrMsg msg;
  DSTR errStr;

  msg.resErrStr = DStrNew();
  TreeApply(ResTree, getRes, &msg);

  msg.atomErrStr = DStrNew();
  msg.lastResName = NULL;
  TreeApply(AtomTree, getAtom, &msg);
  if (msg.lastResName != NULL) {
    DStrAppStr(msg.atomErrStr, " (");
    DStrAppDStr(msg.atomErrStr, msg.lastResName);
    DStrAppStr(msg.atomErrStr, ")");
  }

  errStr = DStrNew();
  if (DStrLen(msg.resErrStr) > 0) {
    DStrAppStr(errStr, "unknown residues: ");
    DStrAppDStr(errStr, msg.resErrStr);
    if (DStrLen(msg.atomErrStr) > 0) {
      DStrAppChar(errStr, '\n');
      DStrAppStr(errStr, "unknown atoms: ");
      DStrAppDStr(errStr, msg.atomErrStr);
    }
  } else {
    if (DStrLen(msg.atomErrStr) > 0) {
      DStrAppStr(errStr, "unknown atoms: ");
      DStrAppDStr(errStr, msg.atomErrStr);
    } else {
      DStrFree(errStr);
      errStr = NULL;
    }
  }

  DStrFree(msg.resErrStr);
  DStrFree(msg.atomErrStr);

  return errStr;
}
