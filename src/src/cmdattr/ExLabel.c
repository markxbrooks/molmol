/*
************************************************************************
*
*   ExLabel.c - labelling commands
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdattr/SCCS/s.ExLabel.c
*   SCCS identification       : 1.3
*
************************************************************************
*/

#include <cmd_attr.h>

#include <stdio.h>
#include <string.h>
#include <memory.h>

#include <arg.h>
#include <data_hand.h>
#include <attr_struc.h>
#include <attr_mng.h>
#include <graph_draw.h>

static char AtomFormat[LABEL_FORMAT_SIZE] = "";
static int ChoiceDistLabel = 0;

static void
setAtomFormat(DSTR str)
{
  int len;

  len = DStrLen(str);
  if (len >= sizeof(AtomFormat))
    len = sizeof(AtomFormat) - 1;

  (void) memcpy(AtomFormat, DStrToStr(str), len);
  (void) memset(AtomFormat + len, 0, sizeof(AtomFormat) - len);
}

static void
setAtomLabel(DhAtomP atomP, void *clientData)
{
  AttrP attrP;
  struct AttrS attr;

  attrP = DhAtomGetAttr(atomP);
  AttrCopy(&attr, attrP);
  AttrReturn(attrP);
  (void) memcpy(attr.labelFormat, AtomFormat, sizeof(AtomFormat));
  DhAtomSetAttr(atomP, AttrGet(&attr));
}

static void
setDistLabel(DhDistP distP, DhAtomP atom1P, DhAtomP atom2P, void *clientData)
{
  AttrP attrP;
  struct AttrS attr;

  attrP = DhDistGetAttr(distP);
  AttrCopy(&attr, attrP);
  AttrReturn(attrP);
  attr.distLabel = ChoiceDistLabel;
  DhDistSetAttr(distP, AttrGet(&attr));
}

#define ARG_NUM 1

ErrCode
ExLabelAtom(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  ErrCode errCode;

  arg[0].type = AT_STR;

  ArgInit(arg, ARG_NUM);

  arg[0].prompt = "Label Format";

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  setAtomFormat(arg[0].v.strVal);

  ArgCleanup(arg, ARG_NUM);

  DhApplyAtom(PropGetRef(PROP_SELECTED, FALSE), setAtomLabel, NULL);
  GraphMolChanged(PROP_SELECTED);

  GraphRedrawNeeded();

  return EC_OK;
}

#define ENUM_SIZE 4

static void
fillEnum(EnumEntryDescr *entry)
{
  entry[0].str = "invisible";
  entry[0].onOff = FALSE;
  entry[1].str = "val";
  entry[1].onOff = FALSE;
  entry[2].str = "limit";
  entry[2].onOff = FALSE;
  entry[3].str = "viol";
  entry[3].onOff = FALSE;
}

ErrCode
ExLabelDist(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  EnumEntryDescr enumEntry[ENUM_SIZE];
  ErrCode errCode;

  arg[0].type = AT_ENUM;

  ArgInit(arg, ARG_NUM);

  fillEnum(enumEntry);
  enumEntry[ChoiceDistLabel].onOff = TRUE;

  arg[0].prompt = "Dist Label";
  arg[0].u.enumD.entryP = enumEntry;
  arg[0].u.enumD.n = ENUM_SIZE;
  arg[0].v.intVal = ChoiceDistLabel;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  ChoiceDistLabel = arg[0].v.intVal;

  ArgCleanup(arg, ARG_NUM);

  DhApplyDist(PropGetRef(PROP_SELECTED, FALSE), setDistLabel, NULL);

  GraphMolChanged(PROP_SELECTED);
  GraphRedrawNeeded();

  return EC_OK;
}

#undef ARG_NUM
#define ARG_NUM 2

ErrCode
ExLabelInit(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  EnumEntryDescr enumEntry[ENUM_SIZE];
  AttrP attrP;
  struct AttrS attr;
  ErrCode errCode;

  arg[0].type = AT_STR;
  arg[1].type = AT_ENUM;

  ArgInit(arg, ARG_NUM);

  attrP = AttrGetInit();
  AttrCopy(&attr, attrP);
  AttrReturn(attrP);

  arg[0].prompt = "Atom Format";
  DStrAssignStr(arg[0].v.strVal, attr.labelFormat);

  fillEnum(enumEntry);
  enumEntry[ChoiceDistLabel].onOff = TRUE;

  arg[1].prompt = "Dist Label";
  arg[1].u.enumD.entryP = enumEntry;
  arg[1].u.enumD.n = ENUM_SIZE;
  arg[1].v.intVal = attr.distLabel;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  setAtomFormat(arg[0].v.strVal);
  ChoiceDistLabel = arg[1].v.intVal;

  ArgCleanup(arg, ARG_NUM);

  (void) memcpy(attr.labelFormat, AtomFormat, sizeof(AtomFormat));
  attr.distLabel = ChoiceDistLabel;
  AttrSetInit(&attr);

  return EC_OK;
}
