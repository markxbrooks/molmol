/*
************************************************************************
*
*   ExStyle.c - display style setting commands
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
*   Date of last modification : 99/10/16
*   Pathname of SCCS file     : /net/sn/homeb/rkoradi/molmol-master/src/cmdattr/SCCS/s.ExStyle.c
*   SCCS identification       : 1.9
*
************************************************************************
*/

#include <cmd_attr.h>

#include <stdio.h>

#include <arg.h>
#include <prop_tab.h>
#include <data_hand.h>
#include <attr_struc.h>
#include <attr_mng.h>
#include <graph_draw.h>

static int ChoiceAtom = 0, ChoiceBond = 1, ChoiceDist = 1;
static PropRefP VisibleRefP;

static void
setAtomStyle(DhAtomP atomP, void *clientData)
{
  AttrP attrP;
  struct AttrS attr;

  attrP = DhAtomGetAttr(atomP);
  AttrCopy(&attr, attrP);
  AttrReturn(attrP);
  attr.atomStyle = ChoiceAtom;
  if (attr.atomStyle == AS_SPHERE)
    attr.shadeModel = SHADE_DEFAULT;
  else
    attr.shadeModel = -1;
  DhAtomSetAttr(atomP, AttrGet(&attr));

  DhAtomSetProp(VisibleRefP, atomP, attr.atomStyle != AS_INVISIBLE);
}

static void
setBondStyle(DhBondP bondP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  AttrP attrP;
  struct AttrS attr;

  attrP = DhBondGetAttr(bondP);
  AttrCopy(&attr, attrP);
  AttrReturn(attrP);
  attr.bondStyle = ChoiceBond;
  if (attr.bondStyle == BS_CYLINDER || attr.bondStyle == BS_HALF_CYLINDER ||
      attr.bondStyle == BS_NEON || attr.bondStyle == BS_HALF_NEON ||
      attr.bondStyle == BS_CONE || attr.bondStyle == BS_HALF_CONE)
    attr.shadeModel = SHADE_DEFAULT;
  else
    attr.shadeModel = -1;
  DhBondSetAttr(bondP, AttrGet(&attr));

  DhBondSetProp(VisibleRefP, bondP, attr.bondStyle != BS_INVISIBLE);
}

static void
setDistStyle(DhDistP distP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  AttrP attrP;
  struct AttrS attr;

  attrP = DhDistGetAttr(distP);
  AttrCopy(&attr, attrP);
  AttrReturn(attrP);
  attr.distStyle = ChoiceDist;
  if (attr.distStyle == DS_CYLINDER || attr.distStyle == DS_NEON ||
      attr.distStyle == DS_CONE)
    attr.shadeModel = SHADE_DEFAULT;
  else
    attr.shadeModel = -1;
  DhDistSetAttr(distP, AttrGet(&attr));

  DhDistSetProp(VisibleRefP, distP, attr.distStyle != DS_INVISIBLE);
}

#define ARG_NUM 1

ErrCode
ExStyleAtom(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  EnumEntryDescr enumEntry[3];
  ErrCode errCode;

  arg[0].type = AT_ENUM;

  ArgInit(arg, ARG_NUM);

  enumEntry[0].str = "invisible";
  enumEntry[0].onOff = FALSE;
  enumEntry[1].str = "sphere";
  enumEntry[1].onOff = FALSE;
  enumEntry[2].str = "tetrahedron";
  enumEntry[2].onOff = FALSE;

  enumEntry[ChoiceAtom].onOff = TRUE;

  arg[0].prompt = "Atom Style";
  arg[0].u.enumD.entryP = enumEntry;
  arg[0].u.enumD.n = 3;
  arg[0].v.intVal = ChoiceAtom;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  ChoiceAtom = arg[0].v.intVal;
  ArgCleanup(arg, ARG_NUM);

  VisibleRefP = PropGetRef(PROP_VISIBLE, FALSE);
  DhApplyAtom(PropGetRef(PROP_SELECTED, FALSE), setAtomStyle, NULL);

  GraphMolChanged(PROP_SELECTED);
  GraphRedrawNeeded();

  return EC_OK;
}

ErrCode
ExStyleBond(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  EnumEntryDescr enumEntry[9];
  ErrCode errCode;

  arg[0].type = AT_ENUM;

  ArgInit(arg, ARG_NUM);

  enumEntry[0].str = "invisible";
  enumEntry[0].onOff = FALSE;
  enumEntry[1].str = "line";
  enumEntry[1].onOff = FALSE;
  enumEntry[2].str = "half_line";
  enumEntry[2].onOff = FALSE;
  enumEntry[3].str = "cylinder";
  enumEntry[3].onOff = FALSE;
  enumEntry[4].str = "half_cylinder";
  enumEntry[4].onOff = FALSE;
  enumEntry[5].str = "neon";
  enumEntry[5].onOff = FALSE;
  enumEntry[6].str = "half_neon";
  enumEntry[6].onOff = FALSE;
  enumEntry[7].str = "cone";
  enumEntry[7].onOff = FALSE;
  enumEntry[8].str = "half_cone";
  enumEntry[8].onOff = FALSE;

  enumEntry[ChoiceBond].onOff = TRUE;

  arg[0].prompt = "Bond Style";
  arg[0].u.enumD.entryP = enumEntry;
  arg[0].u.enumD.n = 9;
  arg[0].u.enumD.lineNo = 2;
  arg[0].v.intVal = ChoiceBond;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  ChoiceBond = arg[0].v.intVal;
  ArgCleanup(arg, ARG_NUM);

  VisibleRefP = PropGetRef(PROP_VISIBLE, FALSE);
  DhApplyBond(PropGetRef(PROP_SELECTED, FALSE), setBondStyle, NULL);

  GraphMolChanged(PROP_SELECTED);
  GraphRedrawNeeded();

  return EC_OK;
}

ErrCode
ExStyleDist(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  EnumEntryDescr enumEntry[6];
  ErrCode errCode;

  arg[0].type = AT_ENUM;

  ArgInit(arg, ARG_NUM);

  enumEntry[0].str = "invisible";
  enumEntry[0].onOff = FALSE;
  enumEntry[1].str = "line";
  enumEntry[1].onOff = FALSE;
  enumEntry[2].str = "viol";
  enumEntry[2].onOff = FALSE;
  enumEntry[3].str = "cylinder";
  enumEntry[3].onOff = FALSE;
  enumEntry[4].str = "neon";
  enumEntry[4].onOff = FALSE;
  enumEntry[5].str = "cone";
  enumEntry[5].onOff = FALSE;

  enumEntry[ChoiceDist].onOff = TRUE;

  arg[0].prompt = "Dist Style";
  arg[0].u.enumD.entryP = enumEntry;
  arg[0].u.enumD.n = 6;
  arg[0].u.enumD.lineNo = 2;
  arg[0].v.intVal = ChoiceDist;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  ChoiceDist = arg[0].v.intVal;
  ArgCleanup(arg, ARG_NUM);

  VisibleRefP = PropGetRef(PROP_VISIBLE, FALSE);
  DhApplyDist(PropGetRef(PROP_SELECTED, FALSE), setDistStyle, NULL);

  GraphMolChanged(PROP_SELECTED);
  GraphRedrawNeeded();

  return EC_OK;
}

#undef ARG_NUM
#define ARG_NUM 3

ErrCode
ExStyleInit(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  AttrP attrP;
  struct AttrS attr;
  EnumEntryDescr atomEntry[3];
  EnumEntryDescr bondEntry[9];
  EnumEntryDescr distEntry[6];
  ErrCode errCode;

  arg[0].type = AT_ENUM;
  arg[1].type = AT_ENUM;
  arg[2].type = AT_ENUM;

  ArgInit(arg, ARG_NUM);

  attrP = AttrGetInit();
  AttrCopy(&attr, attrP);
  AttrReturn(attrP);

  ChoiceAtom = attr.atomStyle;
  ChoiceBond = attr.bondStyle;
  ChoiceDist = attr.distStyle;

  atomEntry[0].str = "invisible";
  atomEntry[0].onOff = FALSE;
  atomEntry[1].str = "sphere";
  atomEntry[1].onOff = FALSE;
  atomEntry[2].str = "tetrahedron";
  atomEntry[2].onOff = FALSE;

  atomEntry[ChoiceAtom].onOff = TRUE;

  arg[0].prompt = "Atom Style";
  arg[0].u.enumD.entryP = atomEntry;
  arg[0].u.enumD.n = 3;
  arg[0].v.intVal = ChoiceAtom;

  bondEntry[0].str = "invisible";
  bondEntry[0].onOff = FALSE;
  bondEntry[1].str = "line";
  bondEntry[1].onOff = FALSE;
  bondEntry[2].str = "half_line";
  bondEntry[2].onOff = FALSE;
  bondEntry[3].str = "cylinder";
  bondEntry[3].onOff = FALSE;
  bondEntry[4].str = "half_cylinder";
  bondEntry[4].onOff = FALSE;
  bondEntry[5].str = "neon";
  bondEntry[5].onOff = FALSE;
  bondEntry[6].str = "half_neon";
  bondEntry[6].onOff = FALSE;
  bondEntry[7].str = "cone";
  bondEntry[7].onOff = FALSE;
  bondEntry[8].str = "half_cone";
  bondEntry[8].onOff = FALSE;

  bondEntry[ChoiceBond].onOff = TRUE;

  arg[1].prompt = "Bond Style";
  arg[1].u.enumD.entryP = bondEntry;
  arg[1].u.enumD.n = 9;
  arg[1].u.enumD.lineNo = 2;
  arg[1].v.intVal = ChoiceBond;

  distEntry[0].str = "invisible";
  distEntry[0].onOff = FALSE;
  distEntry[1].str = "line";
  distEntry[1].onOff = FALSE;
  distEntry[2].str = "viol";
  distEntry[2].onOff = FALSE;
  distEntry[3].str = "cylinder";
  distEntry[3].onOff = FALSE;
  distEntry[4].str = "neon";
  distEntry[4].onOff = FALSE;
  distEntry[5].str = "cone";
  distEntry[5].onOff = FALSE;

  distEntry[ChoiceDist].onOff = TRUE;

  arg[2].prompt = "Dist Style";
  arg[2].u.enumD.entryP = distEntry;
  arg[2].u.enumD.n = 6;
  arg[2].u.enumD.lineNo = 2;
  arg[2].v.intVal = ChoiceDist;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  ChoiceAtom = arg[0].v.intVal;
  ChoiceBond = arg[1].v.intVal;
  ChoiceDist = arg[2].v.intVal;
  ArgCleanup(arg, ARG_NUM);

  attr.atomStyle = ChoiceAtom;
  attr.bondStyle = ChoiceBond;
  attr.distStyle = ChoiceDist;
  AttrSetInit(&attr);

  return EC_OK;
}
