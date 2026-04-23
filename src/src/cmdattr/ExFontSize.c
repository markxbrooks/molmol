/*
************************************************************************
*
*   ExFontSize.c - SizeText command
*
*   Copyright (c) 1994-95
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
*   Pathname of SCCS file     : /tmp_mnt/net/sn/homeb/rkoradi/molmol-master/src/cmdattr/SCCS/s.ExFontSize.c
*   SCCS identification       : 1.2
*
************************************************************************
*/

#include <cmd_attr.h>

#include <stdio.h>

#include <arg.h>
#include <prim_hand.h>
#include <attr_struc.h>
#include <attr_mng.h>
#include <graph_draw.h>

#define MIN_SIZE 0.005f

static BOOL SizeRel = FALSE;
static float CurrSize = 0.05f;

static void
setPrimSize(PrimObjP primP, void *clientData)
{
  AttrP attrP;
  struct AttrS attr;

  attrP = PrimGetAttr(primP);
  AttrCopy(&attr, attrP);
  AttrReturn(attrP);

  if (SizeRel)
    attr.fontSize += CurrSize;
  else
    attr.fontSize = CurrSize;

  if (attr.fontSize < MIN_SIZE)
    attr.fontSize = MIN_SIZE;

  PrimSetAttr(primP, AttrGet(&attr));
}

#define ARG_NUM 2

ErrCode
ExFontSize(char *cmd)
{
  ArgDescr arg[ARG_NUM];
  EnumEntryDescr enumEntry[2];
  ErrCode errCode;

  arg[0].type = AT_ENUM;
  arg[1].type = AT_DOUBLE;

  ArgInit(arg, ARG_NUM);

  enumEntry[0].str = "abs";
  enumEntry[0].onOff = ! SizeRel;
  enumEntry[1].str = "rel";
  enumEntry[1].onOff = SizeRel;

  arg[0].prompt = "Mode";
  arg[0].u.enumD.entryP = enumEntry;
  arg[0].u.enumD.n = 2;
  if (SizeRel)
    arg[0].v.intVal = 1;
  else
    arg[0].v.intVal = 0;

  arg[1].prompt = "Size";
  arg[1].v.doubleVal = CurrSize;

  errCode = ArgGet(arg, ARG_NUM);
  if (errCode != EC_OK) {
    ArgCleanup(arg, ARG_NUM);
    return errCode;
  }

  SizeRel = (arg[0].v.intVal == 1);
  CurrSize = (float) arg[1].v.doubleVal;

  ArgCleanup(arg, ARG_NUM);

  PrimApply(PT_ALL, PropGetRef(PROP_SELECTED, FALSE), setPrimSize, NULL);

  GraphRedrawNeeded();

  return EC_OK;
}
