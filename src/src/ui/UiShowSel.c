/*
************************************************************************
*
*   UiShowSel.c - feedback for interactive selection
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
*   Pathname of SCCS file     : /sgiext/molmol/src/ui/SCCS/s.UiShowSel.c
*   SCCS identification       : 1.6
*
************************************************************************
*/

#include <ui_show_sel.h>

#include <stdio.h>

#include <dstr.h>
#include <pu.h>
#include <data_hand.h>
#include <data_sel.h>
#include <prim_hand.h>
#include <prim_sel.h>

static void
showSel(DataEntityType entType, void *entP, SelKind kind)
{
  DSTR msg = DStrNew();
  char numBuf[20];
  DhMolP molP = NULL;
  DhResP resP = NULL;
  DhAtomP atomP;
  AttrP attrP = NULL;

  if (kind == SK_ADD)
    DStrAssignStr(msg, "additionally selected ");
  else if (kind == SK_REMOVE)
    DStrAssignStr(msg, "deselected ");
  else
    DStrAssignStr(msg, "selected ");

  switch(entType) {
    case DE_MOL:
      DStrAppStr(msg, "molecule ");
      molP = entP;
      break;
    case DE_RES:
      DStrAppStr(msg, "residue ");
      resP = entP;
      break;
    case DE_ATOM:
      DStrAppStr(msg, "atom ");
      DStrAppDStr(msg, DhAtomGetName(entP));
      resP = DhAtomGetRes(entP);
      attrP = DhAtomGetAttr(entP);
      break;
    case DE_BOND:
      DStrAppStr(msg, "bond between ");
      DStrAppDStr(msg, DhAtomGetName(DhBondGetAtom1(entP)));
      DStrAppStr(msg, " and ");
      DStrAppDStr(msg, DhAtomGetName(DhBondGetAtom2(entP)));
      resP = DhBondGetRes(entP);
      attrP = DhBondGetAttr(entP);
      break;
    case DE_ANGLE:
      DStrAppStr(msg, "angle ");
      DStrAppDStr(msg, DhAngleGetName(entP));
      resP = DhAngleGetRes(entP);
      break;
    case DE_DIST:
      DStrAppStr(msg, "distance between ");
      atomP = DhDistGetAtom1(entP);
      DStrAppDStr(msg, DhAtomGetName(atomP));
      DStrAppStr(msg, " of ");
      resP = DhAtomGetRes(atomP);
      DStrAppDStr(msg, DhResGetName(resP));
      (void) sprintf(numBuf, "%d", DhResGetNumber(resP));
      DStrAppStr(msg, numBuf);
      DStrAppStr(msg, " and ");
      atomP = DhDistGetAtom2(entP);
      DStrAppDStr(msg, DhAtomGetName(atomP));
      resP = DhAtomGetRes(atomP);
      attrP = DhDistGetAttr(entP);
      break;
    case DE_PRIM:
      DStrAppStr(msg, "primitive ");
      (void) sprintf(numBuf, "%d", PrimGetNumber(entP));
      DStrAppStr(msg, numBuf);
      attrP = PrimGetAttr(entP);
      break;
  }

  if (resP != NULL) {
    if (entType != DE_RES)
      DStrAppStr(msg, " of ");

    DStrAppDStr(msg, DhResGetName(resP));
    (void) sprintf(numBuf, "%d", DhResGetNumber(resP));
    DStrAppStr(msg, numBuf);
    molP = DhResGetMol(resP);
  }

  if (molP != NULL) {
    if (entType != DE_MOL)
      DStrAppStr(msg, " of ");

    DStrAppDStr(msg, DhMolGetName(molP));
  }

  if (attrP != NULL) {
    DStrAppStr(msg, " (attr: ");
    (void) sprintf(numBuf, "%d", AttrGetIndex(attrP));
    DStrAppStr(msg, numBuf);
    DStrAppStr(msg, ")");
  }

  PuSetTextField(PU_TF_STATUS, DStrToStr(msg));

  DStrFree(msg);
}

static void
showPrimSel(PrimObjP primP, SelKind kind)
{
  showSel(DE_PRIM, primP, kind);
}

void
UiShowSelInit(void)
{
  SelAddCB(showSel);
  PrimSelAddCB(showPrimSel);
}
