/*
************************************************************************
*
*   ExShade.c - shade commands
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdattr/SCCS/s.ExShade.c
*   SCCS identification       : 1.8
*
************************************************************************
*/

#include <cmd_attr.h>

#include <stdio.h>
#include <string.h>

#include <arg.h>
#include <data_hand.h>
#include <prim_hand.h>
#include <attr_struc.h>
#include <attr_mng.h>
#include <graph_draw.h>

static short ShadeModel = SHADE_DEFAULT;

static AttrP
changeShade(AttrP attrP)
{
  struct AttrS attr;

  AttrCopy(&attr, attrP);
  AttrReturn(attrP);
  attr.shadeModel = ShadeModel;
  return AttrGet(&attr);
}

static void
setAtomShade(DhAtomP atomP, void *clientData)
{
  DhAtomSetAttr(atomP, changeShade(DhAtomGetAttr(atomP)));
}

static void
setBondShade(DhBondP bondP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  DhBondSetAttr(bondP, changeShade(DhBondGetAttr(bondP)));
}

static void
setDistShade(DhDistP distP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  DhDistSetAttr(distP, changeShade(DhDistGetAttr(distP)));
}

static void
setPrimShade(PrimObjP primP, void *clientData)
{
  PrimSetAttr(primP, changeShade(PrimGetAttr(primP)));
}

#define ARG_NUM 1

ErrCode
ExShade(char *cmd)
{
  DataEntityType entType;
  ArgDescr arg[ARG_NUM];
  EnumEntryDescr enumEntry[6];
  ErrCode errCode;
  PropRefP refP;

  if (strcmp(cmd, "ShadeAtom") == 0) {
    entType = DE_ATOM;
  } else if (strcmp(cmd, "ShadeBond") == 0) {
    entType = DE_BOND;
  } else if (strcmp(cmd, "ShadeDist") == 0) {
    entType = DE_DIST;
  } else {
    entType = DE_PRIM;
  }

  arg[0].type = AT_ENUM;

  ArgInit(arg, ARG_NUM);

  enumEntry[0].str = "none";
  enumEntry[0].onOff = FALSE;
  enumEntry[1].str = "flat";
  enumEntry[1].onOff = FALSE;
  enumEntry[2].str = "gouraud";
  enumEntry[2].onOff = FALSE;
  enumEntry[3].str = "phong";
  enumEntry[3].onOff = FALSE;
  enumEntry[4].str = "dots";
  enumEntry[4].onOff = FALSE;
  enumEntry[5].str = "lines";
  enumEntry[5].onOff = FALSE;

  enumEntry[ShadeModel + 1].onOff = TRUE;

  arg[0].prompt = "Shading";
  arg[0].u.enumD.entryP = enumEntry;
  arg[0].u.enumD.n = 6;
  arg[0].u.enumD.lineNo = 2;
  arg[0].v.intVal = ShadeModel + 1;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  ShadeModel = arg[0].v.intVal - 1;
  ArgCleanup(arg, ARG_NUM);

  refP = PropGetRef(PROP_SELECTED, FALSE);

  switch (entType) {
    case DE_ATOM:
      DhApplyAtom(refP, setAtomShade, NULL);
      break;
    case DE_BOND:
      DhApplyBond(refP, setBondShade, NULL);
      break;
    case DE_DIST:
      DhApplyDist(refP, setDistShade, NULL);
      break;
    case DE_PRIM:
      PrimApply(PT_ALL, refP, setPrimShade, NULL);
      break;
  }

  GraphMolChanged(PROP_SELECTED);
  GraphRedrawNeeded();

  return EC_OK;
}
