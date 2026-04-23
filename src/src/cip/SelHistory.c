/*
************************************************************************
*
*   SelHistory.c - put interactive selection into history
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
*   Date of last modification : 97/03/03
*   Pathname of SCCS file     : /local/home/kor/molmol/src/cip/SCCS/s.SelHistory.c
*   SCCS identification       : 1.7
*
************************************************************************
*/

#include "sel_history.h"

#include <stdio.h>

#include <dstr.h>
#include <data_hand.h>
#include <data_sel.h>
#include <prim_hand.h>
#include <prim_sel.h>
#include <history.h>

static void
selHistory(DataEntityType entType, void *entP, SelKind kind)
{
  char *cmd;
  DSTR arg = DStrNew();
  char numBuf[20];
  DhMolP molP = NULL;
  DhResP resP = NULL;

  if (kind == SK_ADD)
    DStrAssignStr(arg, "selected | ");
  else if (kind == SK_REMOVE)
    DStrAssignStr(arg, "selected & ! (");

  switch(entType) {
    case DE_MOL:
      cmd = "SelectMol";
      molP = entP;
      break;
    case DE_RES:
      cmd = "SelectRes";
      resP = entP;
      break;
    case DE_ATOM:
      cmd = "SelectAtom";
      DStrAppStr(arg, "name = ");
      DStrAppChar(arg, '"');
      DStrAppDStr(arg, DhAtomGetName(entP));
      DStrAppChar(arg, '"');
      DStrAppStr(arg, " &");
      resP = DhAtomGetRes(entP);
      break;
    case DE_BOND:
      cmd = "SelectBond";
      DStrAppStr(arg, "atom1.name = ");
      DStrAppChar(arg, '"');
      DStrAppDStr(arg, DhAtomGetName(DhBondGetAtom1(entP)));
      DStrAppChar(arg, '"');
      DStrAppStr(arg, " & atom2.name = ");
      DStrAppChar(arg, '"');
      DStrAppDStr(arg, DhAtomGetName(DhBondGetAtom2(entP)));
      DStrAppChar(arg, '"');
      DStrAppStr(arg, " &");
      resP = DhBondGetRes(entP);
      break;
    case DE_ANGLE:
      cmd = "SelectAngle";
      DStrAppStr(arg, "name = ");
      DStrAppChar(arg, '"');
      DStrAppDStr(arg, DhAngleGetName(entP));
      DStrAppChar(arg, '"');
      DStrAppStr(arg, " &");
      resP = DhAngleGetRes(entP);
      break;
    case DE_DIST:
      cmd = "SelectDist";
      DStrAppStr(arg, "atom1.name = ");
      DStrAppChar(arg, '"');
      DStrAppDStr(arg, DhAtomGetName(DhDistGetAtom1(entP)));
      DStrAppChar(arg, '"');
      DStrAppStr(arg, " & atom2.name = ");
      DStrAppChar(arg, '"');
      DStrAppDStr(arg, DhAtomGetName(DhDistGetAtom2(entP)));
      DStrAppChar(arg, '"');
      DStrAppStr(arg, " &");
      molP = DhDistGetMol(entP);
      break;
    case DE_PRIM:
      cmd = "SelectPrim";
      DStrAppStr(arg, "num = ");
      (void) sprintf(numBuf, "%d", PrimGetNumber(entP));
      DStrAppStr(arg, numBuf);
      break;
  }

  if (resP != NULL) {
    DStrAppChar(arg, ' ');
    if (entType != DE_RES)
      DStrAppStr(arg, "res.");
    DStrAppStr(arg, "num = ");
    (void) sprintf(numBuf, "%d", DhResGetNumber(resP));
    DStrAppStr(arg, numBuf);
    DStrAppStr(arg, " &");
    molP = DhResGetMol(resP);
  }

  if (molP != NULL) {
    DStrAppChar(arg, ' ');
    if (entType != DE_MOL)
      DStrAppStr(arg, "mol.");
    DStrAppStr(arg, "name = ");
    DStrAppChar(arg, '"');
    DStrAppDStr(arg, DhMolGetName(molP));
    DStrAppChar(arg, '"');
  }

  if (kind == SK_REMOVE)
    DStrAppStr(arg, ")");

  HistoryArgStr(DStrToStr(arg));
  HistoryEndCmd(cmd, HCR_EXECUTED);

  DStrFree(arg);
}

static void
primSelHistory(PrimObjP primP, SelKind kind)
{
  selHistory(DE_PRIM, primP, kind);
}

void
SelHistoryInit(void)
{
  SelAddCB(selHistory);
  PrimSelAddCB(primSelHistory);
}
